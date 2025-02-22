/*
 * Cogl
 *
 * A Low Level GPU Graphics and Utilities API
 *
 * Copyright (C) 2007,2008,2009,2010 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */

#if !defined(__COGL_H_INSIDE__) && !defined(COGL_COMPILATION)
#error "Only <cogl/cogl.h> can be included directly."
#endif

#ifndef __COGL_DEFINES_H__
#define __COGL_DEFINES_H__


#define COGL_SYSDEF_POLLIN 1
#define COGL_SYSDEF_POLLPRI 2
#define COGL_SYSDEF_POLLOUT 4
#define COGL_SYSDEF_POLLERR 8
#define COGL_SYSDEF_POLLHUP 16
#define COGL_SYSDEF_POLLNVAL 32

#define COGL_HAS_GLIB_SUPPORT 1
#define COGL_HAS_GTYPE_SUPPORT 1
#define COGL_HAS_COGL_PATH_SUPPORT 1
#define COGL_HAS_GLES 1
#define CLUTTER_COGL_HAS_GLES 1
#define COGL_HAS_GLES2 1
#define COGL_HAS_GL 1
#define CLUTTER_COGL_HAS_GL 1
#define COGL_HAS_GLX_SUPPORT 1
#define COGL_HAS_EGL_PLATFORM_WAYLAND_SUPPORT 1
#define COGL_HAS_EGL_PLATFORM_KMS_SUPPORT 1
#define COGL_HAS_WAYLAND_EGL_SERVER_SUPPORT 1
#define COGL_HAS_EGL_PLATFORM_XLIB_SUPPORT 1
#define COGL_HAS_EGL_SUPPORT 1
#define COGL_HAS_X11 1
#define COGL_HAS_X11_SUPPORT 1
#define COGL_HAS_XLIB 1
#define COGL_HAS_XLIB_SUPPORT 1
#define COGL_HAS_POLL_SUPPORT 1

#define COGL_VERSION_MAJOR_INTERNAL 1
#define COGL_VERSION_MINOR_INTERNAL 22
#define COGL_VERSION_MICRO_INTERNAL 6
#define COGL_VERSION_STRING_INTERNAL "1.22.6"

#endif
