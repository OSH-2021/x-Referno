/*
 * Copyright (C) 2012 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#if !defined(__WEBKIT2_H_INSIDE__) && !defined(WEBKIT2_COMPILATION) && !defined(__WEBKIT_WEB_EXTENSION_H_INSIDE__)
#error "Only <webkit2/webkit2.h> can be included directly."
#endif

#ifndef WebKitVersion_h
#define WebKitVersion_h

#include <webkit2/WebKitDefines.h>

G_BEGIN_DECLS

/**
 * WEBKIT_MAJOR_VERSION:
 *
 * Like webkit_get_major_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 */
#define WEBKIT_MAJOR_VERSION (2)

/**
 * WEBKIT_MINOR_VERSION:
 *
 * Like webkit_get_minor_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 */
#define WEBKIT_MINOR_VERSION (32)

/**
 * WEBKIT_MICRO_VERSION:
 *
 * Like webkit_get_micro_version(), but from the headers used at
 * application compile time, rather than from the library linked
 * against at application run time.
 */
#define WEBKIT_MICRO_VERSION (0)

/**
 * WEBKIT_CHECK_VERSION:
 * @major: major version (e.g. 1 for version 1.2.5)
 * @minor: minor version (e.g. 2 for version 1.2.5)
 * @micro: micro version (e.g. 5 for version 1.2.5)
 *
 * Returns: %TRUE if the version of the WebKit header files
 * is the same as or newer than the passed-in version.
 */
#define WEBKIT_CHECK_VERSION(major, minor, micro) \
    (WEBKIT_MAJOR_VERSION > (major) || \
    (WEBKIT_MAJOR_VERSION == (major) && WEBKIT_MINOR_VERSION > (minor)) || \
    (WEBKIT_MAJOR_VERSION == (major) && WEBKIT_MINOR_VERSION == (minor) && \
     WEBKIT_MICRO_VERSION >= (micro)))

WEBKIT_API guint
webkit_get_major_version (void);

WEBKIT_API guint
webkit_get_minor_version (void);

WEBKIT_API guint
webkit_get_micro_version (void);

G_END_DECLS

#endif
