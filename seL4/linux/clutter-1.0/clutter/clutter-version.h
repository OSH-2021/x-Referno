/*
 * Clutter.
 *
 * An OpenGL based 'interactive canvas' library.
 *
 * Authored By Matthew Allum  <mallum@openedhand.com>
 *
 * Copyright (C) 2006 OpenedHand
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */

#ifndef __CLUTTER_VERSION_H__
#define __CLUTTER_VERSION_H__

#if !defined(__CLUTTER_H_INSIDE__) && !defined(CLUTTER_COMPILATION)
#error "Only <clutter/clutter.h> can be included directly."
#endif

/**
 * SECTION:clutter-version
 * @short_description: Versioning utility macros
 *
 * Clutter offers a set of macros for checking the version of the library
 * at compile time; it also provides a function to perform the same check
 * at run time.
 *
 * Clutter adds version information to both API deprecations and additions;
 * by definining the macros %CLUTTER_VERSION_MIN_REQUIRED and
 * %CLUTTER_VERSION_MAX_ALLOWED, you can specify the range of Clutter versions
 * whose API you want to use. Functions that were deprecated before, or
 * introduced after, this range will trigger compiler warnings. For instance,
 * if we define the following symbols:
 *
 * |[
 *   CLUTTER_VERSION_MIN_REQUIRED = CLUTTER_VERSION_1_6
 *   CLUTTER_VERSION_MAX_ALLOWED  = CLUTTER_VERSION_1_8
 * ]|
 *
 * and we have the following functions annotated in the Clutter headers:
 *
 * |[
 *   void clutter_function_A (void) CLUTTER_DEPRECATED_IN_1_4;
 *   void clutter_function_B (void) CLUTTER_DEPRECATED_IN_1_6;
 *   void clutter_function_C (void) CLUTTER_AVAILABLE_IN_1_8;
 *   void clutter_function_D (void) CLUTTER_AVAILABLE_IN_1_10;
 * ]|
 *
 * then any application code using the functions above will get the output:
 *
 * |[
 *   clutter_function_A: deprecation warning
 *   clutter_function_B: no warning
 *   clutter_function_C: no warning
 *   clutter_function_D: symbol not available warning
 * ]|
 *
 * It is possible to disable the compiler warnings by defining the macro
 * %CLUTTER_DISABLE_DEPRECATION_WARNINGS before including the clutter.h
 * header.
 */

#include <glib.h>

G_BEGIN_DECLS

/**
 * CLUTTER_MAJOR_VERSION:
 *
 * The major version of the Clutter library (1, if %CLUTTER_VERSION is 1.2.3)
 */
#define CLUTTER_MAJOR_VERSION   (1)

/**
 * CLUTTER_MINOR_VERSION:
 *
 * The minor version of the Clutter library (2, if %CLUTTER_VERSION is 1.2.3)
 */
#define CLUTTER_MINOR_VERSION   (26)

/**
 * CLUTTER_MICRO_VERSION:
 *
 * The micro version of the Clutter library (3, if %CLUTTER_VERSION is 1.2.3)
 */
#define CLUTTER_MICRO_VERSION   (4)

/**
 * CLUTTER_VERSION:
 *
 * The full version of the Clutter library, like 1.2.3
 */
#define CLUTTER_VERSION         1.26.4

/**
 * CLUTTER_VERSION_S:
 *
 * The full version of the Clutter library, in string form (suited for
 * string concatenation)
 */
#define CLUTTER_VERSION_S       "1.26.4"

/**
 * CLUTTER_VERSION_HEX:
 *
 * Numerically encoded version of the Clutter library, like 0x010203
 */
#define CLUTTER_VERSION_HEX     ((CLUTTER_MAJOR_VERSION << 24) | \
                                 (CLUTTER_MINOR_VERSION << 16) | \
                                 (CLUTTER_MICRO_VERSION << 8))

/* XXX - Every new stable minor release bump should add a macro here */

/**
 * CLUTTER_VERSION_1_0:
 *
 * A macro that evaluates to the 1.0 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_0     (G_ENCODE_VERSION (1, 0))

/**
 * CLUTTER_VERSION_1_2:
 *
 * A macro that evaluates to the 1.2 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_2     (G_ENCODE_VERSION (1, 2))

/**
 * CLUTTER_VERSION_1_4:
 *
 * A macro that evaluates to the 1.4 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_4     (G_ENCODE_VERSION (1, 4))

/**
 * CLUTTER_VERSION_1_6:
 *
 * A macro that evaluates to the 1.6 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_6     (G_ENCODE_VERSION (1, 6))

/**
 * CLUTTER_VERSION_1_8:
 *
 * A macro that evaluates to the 1.8 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_8     (G_ENCODE_VERSION (1, 8))

/**
 * CLUTTER_VERSION_1_10:
 *
 * A macro that evaluates to the 1.10 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.10
 */
#define CLUTTER_VERSION_1_10    (G_ENCODE_VERSION (1, 10))

/**
 * CLUTTER_VERSION_1_12:
 *
 * A macro that evaluates to the 1.12 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.12
 */
#define CLUTTER_VERSION_1_12    (G_ENCODE_VERSION (1, 12))

/**
 * CLUTTER_VERSION_1_14:
 *
 * A macro that evaluates to the 1.14 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.14
 */
#define CLUTTER_VERSION_1_14    (G_ENCODE_VERSION (1, 14))

/**
 * CLUTTER_VERSION_1_16:
 *
 * A macro that evaluates to the 1.16 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.16
 */
#define CLUTTER_VERSION_1_16    (G_ENCODE_VERSION (1, 16))

/**
 * CLUTTER_VERSION_1_18:
 *
 * A macro that evaluates to the 1.18 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.18
 */
#define CLUTTER_VERSION_1_18    (G_ENCODE_VERSION (1, 18))

/**
 * CLUTTER_VERSION_1_20:
 *
 * A macro that evaluates to the 1.20 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.20
 */
#define CLUTTER_VERSION_1_20    (G_ENCODE_VERSION (1, 20))

/**
 * CLUTTER_VERSION_1_22:
 *
 * A macro that evaluates to the 1.22 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.22
 */
#define CLUTTER_VERSION_1_22    (G_ENCODE_VERSION (1, 22))

/**
 * CLUTTER_VERSION_1_24:
 *
 * A macro that evaluates to the 1.24 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.24
 */
#define CLUTTER_VERSION_1_24    (G_ENCODE_VERSION (1, 24))

/**
 * CLUTTER_VERSION_1_26:
 *
 * A macro that evaluates to the 1.26 version of Clutter, in a format
 * that can be used by the C pre-processor.
 *
 * Since: 1.26
 */
#define CLUTTER_VERSION_1_26    (G_ENCODE_VERSION (1, 26))

/* evaluates to the current stable version; for development cycles,
 * this means the next stable target
 */
#if (CLUTTER_MINOR_VERSION % 2)
# define CLUTTER_VERSION_CUR_STABLE      (G_ENCODE_VERSION (CLUTTER_MAJOR_VERSION, CLUTTER_MINOR_VERSION + 1))
#else
# define CLUTTER_VERSION_CUR_STABLE      (G_ENCODE_VERSION (CLUTTER_MAJOR_VERSION, CLUTTER_MINOR_VERSION))
#endif

/* evaluates to the previous stable version */
#if (CLUTTER_MINOR_VERSION % 2)
# define CLUTTER_VERSION_PREV_STABLE     (G_ENCODE_VERSION (CLUTTER_MAJOR_VERSION, CLUTTER_MINOR_VERSION - 1))
#else
# define CLUTTER_VERSION_PREV_STABLE     (G_ENCODE_VERSION (CLUTTER_MAJOR_VERSION, CLUTTER_MINOR_VERSION - 2))
#endif

/**
 * CLUTTER_CHECK_VERSION:
 * @major: major version, like 1 in 1.2.3
 * @minor: minor version, like 2 in 1.2.3
 * @micro: micro version, like 3 in 1.2.3
 *
 * Evaluates to %TRUE if the version of the Clutter library is greater
 * than @major, @minor and @micro
 */
#define CLUTTER_CHECK_VERSION(major,minor,micro) \
        (CLUTTER_MAJOR_VERSION > (major) || \
         (CLUTTER_MAJOR_VERSION == (major) && CLUTTER_MINOR_VERSION > (minor)) || \
         (CLUTTER_MAJOR_VERSION == (major) && CLUTTER_MINOR_VERSION == (minor) && CLUTTER_MICRO_VERSION >= (micro)))

#ifndef _CLUTTER_EXTERN
#define _CLUTTER_EXTERN extern
#endif

#ifdef CLUTTER_WINDOWING_WIN32
# ifdef CLUTTER_COMPILATION
#  ifdef DLL_EXPORT
#   define CLUTTER_VAR __declspec(dllexport)
#  else
#   define CLUTTER_VAR extern
#  endif
# else
#  define CLUTTER_VAR __declspec(dllimport)
# endif
#else
# define CLUTTER_VAR _CLUTTER_EXTERN
#endif

/**
 * clutter_major_version:
 *
 * The major component of the Clutter library version, e.g. 1 if the version
 * is 1.2.3
 *
 * This value can be used for run-time version checks
 *
 * For a compile-time check, use %CLUTTER_MAJOR_VERSION
 *
 * Since: 1.2
 */
CLUTTER_VAR const guint clutter_major_version;

/**
 * clutter_minor_version:
 *
 * The minor component of the Clutter library version, e.g. 2 if the version
 * is 1.2.3
 *
 * This value can be used for run-time version checks
 *
 * For a compile-time check, use %CLUTTER_MINOR_VERSION
 *
 * Since: 1.2
 */
CLUTTER_VAR const guint clutter_minor_version;

/**
 * clutter_micro_version:
 *
 * The micro component of the Clutter library version, e.g. 3 if the version
 * is 1.2.3
 *
 * This value can be used for run-time version checks
 *
 * For a compile-time check, use %CLUTTER_MICRO_VERSION
 *
 * Since: 1.2
 */
CLUTTER_VAR const guint clutter_micro_version;

G_END_DECLS

#endif /* __CLUTTER_VERSION_H__ */
