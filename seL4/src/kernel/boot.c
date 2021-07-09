/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(GD_GPL)
 */
#include "lapi.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include <assert.h>
#include <kernel/boot.h>
#include <kernel/thread.h>
#include <machine/io.h>
#include <machine/registerset.h>
#include <model/statedata.h>
#include <arch/machine.h>
#include <arch/kernel/boot.h>
#include <arch/kernel/vspace.h>
#include <linker.h>
#include <plat/machine/hardware.h>
#include <util.h>
#include <object/cap.h>
#include <object/objecttype.h>
#include <object/untyped.h>
#include <plat/pc99/plat/64/plat_mode/machine/hardware.h>
#include <machine.h>
#include <stdio.h>

/* (node-local) state accessed only during bootstrapping */

ndks_boot_t ndks_boot BOOT_DATA;

BOOT_CODE bool_t

static int l_insert_region(lua_State *L)
{

    region_t reg = lua_tonumber(L,1);
    word_t i;

    assert(reg.start <= reg.end);
    if (is_reg_empty(reg)) {
        lua_pushboolean(L,true);
        return 1;
    }
    for (i = 0; i < MAX_NUM_FREEMEM_REG; i++) {
        if (is_reg_empty(ndks_boot.freemem[i])) {
            ndks_boot.freemem[i] = reg;
            lua_pushboolean(L,true);
            return 1;
        }
    }
    return 1;
}

BOOT_CODE static inline word_t
static int l_reg_size(lua_State *L)
{
    region_t reg = lua_touserdata(L,1);
    lua_pushnumber(L,reg.end-reg.start);
    return 1;
}

BOOT_CODE pptr_t
static int l_alloc_region(lua_State *L)
{
    word_t size_bits = lua_tonumber(L,1);
    word_t i;
    word_t reg_index = 0; /* gcc cannot work out that this will not be used uninitialized */
    region_t reg = REG_EMPTY;
    region_t rem_small = REG_EMPTY;
    region_t rem_large = REG_EMPTY;
    region_t new_reg;
    region_t new_rem_small;
    region_t new_rem_large;

    /* Search for a freemem region that will be the best fit for an allocation. We favour allocations
     * that are aligned to either end of the region. If an allocation must split a region we favour
     * an unbalanced split. In both cases we attempt to use the smallest region possible. In general
     * this means we aim to make the size of the smallest remaining region smaller (ideally zero)
     * followed by making the size of the largest remaining region smaller */

    for (i = 0; i < MAX_NUM_FREEMEM_REG; i++) {
        /* Determine whether placing the region at the start or the end will create a bigger left over region */
        if (ROUND_UP(ndks_boot.freemem[i].start, size_bits) - ndks_boot.freemem[i].start <
                ndks_boot.freemem[i].end - ROUND_DOWN(ndks_boot.freemem[i].end, size_bits)) {
            new_reg.start = ROUND_UP(ndks_boot.freemem[i].start, size_bits);
            new_reg.end = new_reg.start + BIT(size_bits);
        } else {
            new_reg.end = ROUND_DOWN(ndks_boot.freemem[i].end, size_bits);
            new_reg.start = new_reg.end - BIT(size_bits);
        }
        if (new_reg.end > new_reg.start &&
                new_reg.start >= ndks_boot.freemem[i].start &&
                new_reg.end <= ndks_boot.freemem[i].end) {
            if (new_reg.start - ndks_boot.freemem[i].start < ndks_boot.freemem[i].end - new_reg.end) {
                new_rem_small.start = ndks_boot.freemem[i].start;
                new_rem_small.end = new_reg.start;
                new_rem_large.start = new_reg.end;
                new_rem_large.end = ndks_boot.freemem[i].end;
            } else {
                new_rem_large.start = ndks_boot.freemem[i].start;
                new_rem_large.end = new_reg.start;
                new_rem_small.start = new_reg.end;
                new_rem_small.end = ndks_boot.freemem[i].end;
            }
            if ( is_reg_empty(reg) ||
                    (reg_size(new_rem_small) < reg_size(rem_small)) ||
                    (reg_size(new_rem_small) == reg_size(rem_small) && reg_size(new_rem_large) < reg_size(rem_large)) ) {
                reg = new_reg;
                rem_small = new_rem_small;
                rem_large = new_rem_large;
                reg_index = i;
            }
        }
    }
    if (is_reg_empty(reg)) {
        printf("Kernel init failing: not enough memory\n");
        lua_pushnumber(L,0);
        return 1;
    }
    /* Remove the region in question */
    ndks_boot.freemem[reg_index] = REG_EMPTY;
    /* Add the remaining regions in largest to smallest order */
    insert_region(rem_large);
    if (!insert_region(rem_small)) {
        printf("alloc_region(): wasted 0x%lx bytes due to alignment, try to increase MAX_NUM_FREEMEM_REG\n",
               (word_t)(rem_small.end - rem_small.start));
    }
    lua_pushnumber(L,reg.start)
    return 1;
}

BOOT_CODE void
static int l_write_slot(slot_ptr_t slot_ptr, cap_t cap)
{   
    slot_ptr slot_ptr = lua_tonumber(L,2);
    cap_t cap = lua_tonumber(L,1);
    slot_ptr->cap = cap;

    slot_ptr->cteMDBNode = nullMDBNode;
    mdb_node_ptr_set_mdbRevocable  (&slot_ptr->cteMDBNode, true);
    mdb_node_ptr_set_mdbFirstBadged(&slot_ptr->cteMDBNode, true);
    return 0;
}

/* Our root CNode needs to be able to fit all the initial caps and not
 * cover all of memory.
 */
compile_assert(root_cnode_size_valid,
               CONFIG_ROOT_CNODE_SIZE_BITS < 32 - seL4_SlotBits &&
               (1U << CONFIG_ROOT_CNODE_SIZE_BITS) >= seL4_NumInitialCaps)

BOOT_CODE cap_t
static int l_create_root_cnode(lua_State *L)
{
    pptr_t  pptr;
    cap_t   cap;

    /* write the number of root CNode slots to global state */
    ndks_boot.slot_pos_max = BIT(CONFIG_ROOT_CNODE_SIZE_BITS);

    /* create an empty root CNode */
    pptr = alloc_region(CONFIG_ROOT_CNODE_SIZE_BITS + seL4_SlotBits);
    if (!pptr) {
        printf("Kernel init failing: could not create root cnode\n");
        cap_t ret = cap_null_cap_new()
        lua_pushlightuserdata(L,ret);
        return 1;
    }
    memzero(CTE_PTR(pptr), 1U << (CONFIG_ROOT_CNODE_SIZE_BITS + seL4_SlotBits));
    cap =
        cap_cnode_cap_new(
            CONFIG_ROOT_CNODE_SIZE_BITS,      /* radix      */
            wordBits - CONFIG_ROOT_CNODE_SIZE_BITS, /* guard size */
            0,                                /* guard      */
            pptr                              /* pptr       */
        );

    /* write the root CNode cap into the root CNode */
    write_slot(SLOT_PTR(pptr, seL4_CapInitThreadCNode), cap);
    lua_pushlightuserdata(L,ret);
    return 1;
}

compile_assert(irq_cnode_size, BIT(IRQ_CNODE_BITS - seL4_SlotBits) > maxIRQ)

BOOT_CODE bool_t
static int l_create_irq_cnode(lua_State *L)
{
    pptr_t pptr;
    /* create an empty IRQ CNode */
    pptr = alloc_region(IRQ_CNODE_BITS);
    if (!pptr) {
        printf("Kernel init failing: could not create irq cnode\n");
        lua_pushboolean(L,false);
        return 1;
    }
    memzero((void*)pptr, 1 << IRQ_CNODE_BITS);
    intStateIRQNode = (cte_t*)pptr;
    lua_pushboolean(L,true);
    return 1;
}

/* Check domain scheduler assumptions. */
compile_assert(num_domains_valid,
               CONFIG_NUM_DOMAINS >= 1 && CONFIG_NUM_DOMAINS <= 256)
compile_assert(num_priorities_valid,
               CONFIG_NUM_PRIORITIES >= 1 && CONFIG_NUM_PRIORITIES <= 256)

BOOT_CODE void
static int l_create_domain_cap(lua_State *L)
{
    cap_t cap;
    word_t i;

    cap_t root_cnode_cap = lua_touserdata(L,1);
    /* Check domain scheduler assumptions. */
    assert(ksDomScheduleLength > 0);
    for (i = 0; i < ksDomScheduleLength; i++) {
        assert(ksDomSchedule[i].domain < CONFIG_NUM_DOMAINS);
        assert(ksDomSchedule[i].length > 0);
    }

    cap = cap_domain_cap_new();
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapDomain), cap);
    return 0;
}


BOOT_CODE cap_t
static int l_create_ipcbuf_frame(lua_State *L)
{
    cap_t cap;
    pptr_t pptr;
    cap_t root_cnode_cap = lua_touserdata(L,3); 
    cap_t pd_cap = lua_touserdata(L,2);
    vptr_t vptr = lua_touserdata(L,1);

    /* allocate the IPC buffer frame */
    pptr = alloc_region(PAGE_BITS);
    if (!pptr) {
        printf("Kernel init failing: could not create ipc buffer frame\n");
        cap_t ret = cap_null_cap_new()
        lua_pushlightuserdata(L,ret);
        return 1;
    }
    clearMemory((void*)pptr, PAGE_BITS);

    /* create a cap of it and write it into the root CNode */
    cap = create_mapped_it_frame_cap(pd_cap, pptr, vptr, IT_ASID, false, false);
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadIPCBuffer), cap);
    lua_pushlightuserdata(L,cap);
    return 1;
}

BOOT_CODE void
static int l_create_bi_frame_cap(
    lua_State *L
)
{
    cap_t      root_cnode_cap = lua_touserdata(L,4);
    cap_t      pd_cap = lua_touserdata(L,3);
    pptr_t     pptr = lua_touserdata(L,2);
    vptr_t     vptr = lua_touserdata(L,1);

    /* create a cap of it and write it into the root CNode */
    cap = create_mapped_it_frame_cap(pd_cap, pptr, vptr, IT_ASID, false, false);
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapBootInfoFrame), cap);
    return 0;
}

BOOT_CODE region_t
static int l_allocate_extra_bi_region(lua_state *L)
{
    word_t extra_size = lua_tonumber(L,1);
    /* determine power of 2 size of this region. avoid calling clzl on 0 though */
    if (extra_size == 0) {
        /* return any valid address to correspond to the zero allocation */
        // (region_t) {
            0x1000, 0x1000
        };
        lua_pushlightuserdata(L,region_t{0x100,0x100});
        return 1;
    }
    word_t size_bits = seL4_WordBits - 1 - clzl(ROUND_UP(extra_size, seL4_PageBits));
    pptr_t pptr = alloc_region(size_bits);
    if (!pptr) {
        printf("Kernel init failed: could not allocate extra bootinfo region size bits %lu\n", size_bits);
        lua_pushnumber(L,REG_EMPTY);
        return 1;
    }

    clearMemory((void*)pptr, size_bits);
    ndks_boot.bi_frame->extraLen = BIT(size_bits);

    // return (region_t) {
    //     pptr, pptr + BIT(size_bits)
    // };
    lua_pushlightuserdata(L,region_t{pptr, pptr + BIT(size_bits)};)
    return 1;
}

BOOT_CODE pptr_t
static int l_allocate_bi_frame(
    lua_State *L
)
{
    pptr_t pptr;
    node_id_t  node_id = lua_tonumber(L,3);
    word_t   num_nodes = lua_tonumber(L,2);
    vptr_t ipcbuf_vptr = lua_touserdata(L,1);

    /* create the bootinfo frame object */
    pptr = alloc_region(BI_FRAME_SIZE_BITS);
    if (!pptr) {
        printf("Kernel init failed: could not allocate bootinfo frame\n");
        return 0;
    }
    clearMemory((void*)pptr, BI_FRAME_SIZE_BITS);

    /* initialise bootinfo-related global state */
    ndks_boot.bi_frame = BI_PTR(pptr);
    ndks_boot.slot_pos_cur = seL4_NumInitialCaps;

    BI_PTR(pptr)->nodeID = node_id;
    BI_PTR(pptr)->numNodes = num_nodes;
    BI_PTR(pptr)->numIOPTLevels = 0;
    BI_PTR(pptr)->ipcBuffer = (seL4_IPCBuffer *) ipcbuf_vptr;
    BI_PTR(pptr)->initThreadCNodeSizeBits = CONFIG_ROOT_CNODE_SIZE_BITS;
    BI_PTR(pptr)->initThreadDomain = ksDomSchedule[ksDomScheduleIdx].domain;
    BI_PTR(pptr)->extraLen = 0;
    BI_PTR(pptr)->extraBIPages.start = 0;
    BI_PTR(pptr)->extraBIPages.end = 0;
    lua_pushlightuserdata(L,pptr);
    return 1;
}

BOOT_CODE bool_t
static int l_provide_cap(cap_t root_cnode_cap, cap_t cap)
{
    capt_t cap = lua_touserdata(L,2);
    cap_t cap = lua_touserdata(L,1);
    if (ndks_boot.slot_pos_cur >= ndks_boot.slot_pos_max) {
        printf("Kernel init failed: ran out of cap slots\n");
        lua_pushboolean(L,false);
        return 1;
    }
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), ndks_boot.slot_pos_cur), cap);
    ndks_boot.slot_pos_cur++;
    lua_pushboolean(L,true);
    return 1;
}

BOOT_CODE create_frames_of_region_ret_t
static int l_create_frames_of_region(
    lua_State *L
)
{
    pptr_t     f;
    cap_t      frame_cap;
    seL4_SlotPos slot_pos_before;
    seL4_SlotPos slot_pos_after;

    cap_t    root_cnode_cap = lua_touserdata(L,5);
    cap_t    pd_cap = lua_tonumber(L,4);
    region_t reg = lua_tonumber(L,3);
    bool_t   do_map = lua_tonumber(L,2);
    sword_t  pv_offset = lua_tonumber(L,1);

    slot_pos_before = ndks_boot.slot_pos_cur;

    for (f = reg.start; f < reg.end; f += BIT(PAGE_BITS)) {
        if (do_map) {
            frame_cap = create_mapped_it_frame_cap(pd_cap, f, pptr_to_paddr((void*)(f - pv_offset)), IT_ASID, false, true);
        } else {
            frame_cap = create_unmapped_it_frame_cap(f, false);
        }
        if (!provide_cap(root_cnode_cap, frame_cap))
            lua_pushlightuserdata(L,(create_frames_of_region_ret_t) {
            S_REG_EMPTY, false});
            return 1;
    }

    slot_pos_after = ndks_boot.slot_pos_cur;

    lua_pushlightuserdata(L,(create_frames_of_region_ret_t) {
        (seL4_SlotRegion) { slot_pos_before, slot_pos_after }, true});
    return 1;
}

BOOT_CODE cap_t
static int l_create_it_asid_pool(lua_State *L)
{
    pptr_t ap_pptr;
    cap_t  ap_cap;
    cap_t  root_cnode_cap = lua_pushlightuserdata(L,1);
    /* create ASID pool */
    ap_pptr = alloc_region(seL4_ASIDPoolBits);
    if (!ap_pptr) {
        printf("Kernel init failed: failed to create initial thread asid pool\n");
        return cap_null_cap_new();
    }
    memzero(ASID_POOL_PTR(ap_pptr), 1 << seL4_ASIDPoolBits);
    ap_cap = cap_asid_pool_cap_new(IT_ASID >> asidLowBits, ap_pptr);
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadASIDPool), ap_cap);

    /* create ASID control cap */
    write_slot(
        SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapASIDControl),
        cap_asid_control_cap_new()
    );
    lua_pushlightuserdata(L,ap_cap);
    return 1;
}

BOOT_CODE bool_t
static int l_create_idle_thread(lua_State *L)
{
    pptr_t pptr;

#ifdef ENABLE_SMP_SUPPORT
    for (int i = 0; i < CONFIG_MAX_NUM_NODES; i++) {
#endif /* ENABLE_SMP_SUPPORT */
        pptr = alloc_region(seL4_TCBBits);
        if (!pptr) {
            printf("Kernel init failed: Unable to allocate tcb for idle thread\n");
            lua_pushboolean(L,false);
            return 1;
        }
        memzero((void *)pptr, 1 << seL4_TCBBits);
        NODE_STATE_ON_CORE(ksIdleThread, i) = TCB_PTR(pptr + TCB_OFFSET);
        configureIdleThread(NODE_STATE_ON_CORE(ksIdleThread, i));
#ifdef CONFIG_DEBUG_BUILD
        setThreadName(NODE_STATE_ON_CORE(ksIdleThread, i), "idle_thread");
#endif
        SMP_COND_STATEMENT(NODE_STATE_ON_CORE(ksIdleThread, i)->tcbAffinity = i);
#ifdef ENABLE_SMP_SUPPORT
    }
#endif /* ENABLE_SMP_SUPPORT */
    lua_pushboolean(L,true);
    return 1;
}

BOOT_CODE tcb_t *
static int l_create_initial_thread(
    lua_State *L
)
{
    pptr_t pptr;
    cap_t  cap;
    tcb_t* tcb;
    deriveCap_ret_t dc_ret;

    cap_t  root_cnode_cap = lua_touserdata(L,6);
    cap_t  it_pd_cap = lua_touserdata(L,5);
    vptr_t ui_v_entry = lua_touserdata(L,4);
    vptr_t bi_frame_vptr = lua_touserdata(L,3);
    vptr_t ipcbuf_vptr = lua_touserdata(L,2)
    cap_t  ipcbuf_cap = lua_touserdata(L,1)


    /* allocate TCB */
    pptr = alloc_region(seL4_TCBBits);
    if (!pptr) {
        printf("Kernel init failed: Unable to allocate tcb for initial thread\n");
        lua_pushnil;
        return 1;
    }
    memzero((void*)pptr, 1 << seL4_TCBBits);
    tcb = TCB_PTR(pptr + TCB_OFFSET);
    tcb->tcbTimeSlice = CONFIG_TIME_SLICE;
    Arch_initContext(&tcb->tcbArch.tcbContext);

    /* derive a copy of the IPC buffer cap for inserting */
    dc_ret = deriveCap(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadIPCBuffer), ipcbuf_cap);
    if (dc_ret.status != EXCEPTION_NONE) {
        printf("Failed to derive copy of IPC Buffer\n");
        lua_pushnil;
        return 1;
    }

    /* initialise TCB (corresponds directly to abstract specification) */
    cteInsert(
        root_cnode_cap,
        SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadCNode),
        SLOT_PTR(pptr, tcbCTable)
    );
    cteInsert(
        it_pd_cap,
        SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadVSpace),
        SLOT_PTR(pptr, tcbVTable)
    );
    cteInsert(
        dc_ret.cap,
        SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadIPCBuffer),
        SLOT_PTR(pptr, tcbBuffer)
    );
    tcb->tcbIPCBuffer = ipcbuf_vptr;

    /* Set the root thread's IPC buffer */
    Arch_setTCBIPCBuffer(tcb, ipcbuf_vptr);

    setRegister(tcb, capRegister, bi_frame_vptr);
    setNextPC(tcb, ui_v_entry);

    /* initialise TCB */
    tcb->tcbPriority = seL4_MaxPrio;
    tcb->tcbMCP = seL4_MaxPrio;
    setupReplyMaster(tcb);
    setThreadState(tcb, ThreadState_Running);

    ksCurDomain = ksDomSchedule[ksDomScheduleIdx].domain;
    ksDomainTime = ksDomSchedule[ksDomScheduleIdx].length;
    assert(ksCurDomain < CONFIG_NUM_DOMAINS && ksDomainTime > 0);

    SMP_COND_STATEMENT(tcb->tcbAffinity = 0);

    /* create initial thread's TCB cap */
    cap = cap_thread_cap_new(TCB_REF(tcb));
    write_slot(SLOT_PTR(pptr_of_cap(root_cnode_cap), seL4_CapInitThreadTCB), cap);

#ifdef CONFIG_DEBUG_BUILD
    setThreadName(tcb, "rootserver");
#endif

    return tcb;
}

BOOT_CODE void
static int l_init_core_state(lua_State *L)
{
    tcb_t *scheduler_action =  lua_touserdata(L,1);

#ifdef CONFIG_HAVE_FPU
    NODE_STATE(ksActiveFPUState) = NULL;
#endif
#ifdef CONFIG_DEBUG_BUILD
    /* add initial threads to the debug queue */
    NODE_STATE(ksDebugTCBs) = NULL;
    if (scheduler_action != SchedulerAction_ResumeCurrentThread &&
            scheduler_action != SchedulerAction_ChooseNewThread) {
        tcbDebugAppend(scheduler_action);
    }
    tcbDebugAppend(NODE_STATE(ksIdleThread));
#endif
    NODE_STATE(ksSchedulerAction) = scheduler_action;
    NODE_STATE(ksCurThread) = NODE_STATE(ksIdleThread);
    return 0;
}

BOOT_CODE static bool_t
static int l_provide_untyped_cap(
    lua_State *L
)
{   
    cap_t      root_cnode_cap = lua_touserdata(L,4);
    bool_t     device_memory = lua_touserdata(L,3);
    pptr_t     pptr = lua_touserdata(L,2);
    word_t     size_bits = lua_tonumber(L,1);

    seL4_SlotPos first_untyped_slot
    bool_t ret;
    cap_t ut_cap;
    word_t i = ndks_boot.slot_pos_cur - first_untyped_slot;


    if (i < CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS) {
        ndks_boot.bi_frame->untypedList[i] = (seL4_UntypedDesc) {
            pptr_to_paddr((void*)pptr), 0, 0, size_bits, device_memory
        };
        ut_cap = cap_untyped_cap_new(MAX_FREE_INDEX(size_bits),
                                     device_memory, size_bits, pptr);
        ret = provide_cap(root_cnode_cap, ut_cap);
    } else {
        printf("Kernel init: Too many untyped regions for boot info\n");
        ret = true;
    }
    lua_pushboolean(L,ret);
    return 1;
}

BOOT_CODE bool_t
static int l_create_untypeds_for_region(
    lua_State *L
)
{
    word_t align_bits;
    word_t size_bits;
    cap_t      root_cnode_cap = lua_touserdata(L,3);
    bool_t     device_memory = lua_toboolean(L,2);
    region_t   reg = lua_touserdata(L,1);
    seL4_SlotPos first_untyped_slot

    while (!is_reg_empty(reg)) {
        /* Determine the maximum size of the region */
        size_bits = seL4_WordBits - 1 - clzl(reg.end - reg.start);

        /* Determine the alignment of the region */
        if (reg.start != 0) {
            align_bits = ctzl(reg.start);
        } else {
            align_bits = size_bits;
        }
        /* Reduce size bits to align if needed */
        if (align_bits < size_bits) {
            size_bits = align_bits;
        }
        if (size_bits > seL4_MaxUntypedBits) {
            size_bits = seL4_MaxUntypedBits;
        }

        if (size_bits >= seL4_MinUntypedBits) {
            if (!provide_untyped_cap(root_cnode_cap, device_memory, reg.start, size_bits, first_untyped_slot)) {
                lua_pushboolean(L,false);
                return 1;
            }
        }
        reg.start += BIT(size_bits);
    }
    lua_pushboolean(L,true);
    return 1;
}

BOOT_CODE bool_t
static int l_create_kernel_untypeds(cap_t root_cnode_cap, region_t boot_mem_reuse_reg, seL4_SlotPos first_untyped_slot)
{
    word_t     i;
    region_t   reg = lua_touserdata(L,3);
    cap_t root_cnode_cap = lua_touserdata(L,2); 
    region_t boot_mem_reuse_reg = lua_touserdata(L,1);
    seL4_SlotPos first_untyped_slot

    /* if boot_mem_reuse_reg is not empty, we can create UT objs from boot code/data frames */
    if (!create_untypeds_for_region(root_cnode_cap, false, boot_mem_reuse_reg, first_untyped_slot)) {
        lua_pushboolean(L,false);
        return 1;
    }

    /* convert remaining freemem into UT objects and provide the caps */
    for (i = 0; i < MAX_NUM_FREEMEM_REG; i++) {
        reg = ndks_boot.freemem[i];
        ndks_boot.freemem[i] = REG_EMPTY;
        if (!create_untypeds_for_region(root_cnode_cap, false, reg, first_untyped_slot)) {
            lua_pushboolean(L,false);
            return 1;
        }
    }
    lua_pushboolean(L,true);
    return true;
}

BOOT_CODE void
static int l_bi_finalise(void)
{
    seL4_SlotPos slot_pos_start = ndks_boot.slot_pos_cur;
    seL4_SlotPos slot_pos_end = ndks_boot.slot_pos_max;
    ndks_boot.bi_frame->empty = (seL4_SlotRegion) {
        slot_pos_start, slot_pos_end
    };
}


static const luaL_Reg mylib[] = {
{"insert_region",l_insert_region},
{"reg_size",l_reg_size},
{"alloc_region",l_alloc_region},
{"write_slot",l_write_slot},
{"create_roo_cnode",l_create_root_cnode},
{"create_irq_cnode",l_create_irq_cnode},
{"create_domain_cap",l_create_domain_cap},
{"create_ipcbuf_frame",l_create_ipcbuf_frame},
{"create_bi_frame_cap",l_create_bi_frame_cap},
{"allocate_extra_bi_region",l_allocate_extra_bi_region},
{"allocate_bi_frame",l_allocate_bi_frame},
{"provide_cap",l_provide_cap},
{"create_frames_of_region",l_create_frames_of_region},
{"create_it_asid_pool",l_create_it_asid_pool},
{"create_idle_thread",l_create_idle_thread},
{"create_initial_thread",l_create_initial_thread}
{"init_core_state",l_init_core_state},
{"provide_untyped_cap",l_provide_untyped_cap},
{"create_untypeds_for_region",l_create_untypeds_for_region},
{"create_kernel_untypeds",l_create_kernel_untypeds}
{"bi_finalise",l_bi_finalise},
{NULL, NULL}
};  

extern int luaopen_seL4_lib(lua_State* L)
{
    /* void luaL_newlib (lua_State *L, const luaL_Reg l[]);
     * 创建一个新的"table"，并将"l"中所列出的函数注册为"table"的域。
     */ 
    luaL_newlib(L, mylib);

    return 1;
}