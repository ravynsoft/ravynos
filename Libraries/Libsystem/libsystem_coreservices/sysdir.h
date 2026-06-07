/*
 * Re-implementation of the sysdir API based on
 *    https://www.manpagez.com/man/3/sysdir/ and
 *    https://github.com/artichoke/sysdir-rs/blob/trunk/src/sys.rs
 * Copyright (C) 2026 Zoe Knox <zoe@pixin.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _SYSDIR_H_
#define _SYSDIR_H_

#include <sys/types.h>

typedef enum {
    SYSDIR_DIRECTORY_APPLICATION = 1,
    SYSDIR_DIRECTORY_DEMO_APPLICATION,
    SYSDIR_DIRECTORY_DEVELOPER_APPLICATION,
    SYSDIR_DIRECTORY_ADMIN_APPLICATION,
    SYSDIR_DIRECTORY_LIBRARY,
    SYSDIR_DIRECTORY_DEVELOPER,
    SYSDIR_DIRECTORY_USER,
    SYSDIR_DIRECTORY_DOCUMENTATION,
    SYSDIR_DIRECTORY_DOCUMENT,
    SYSDIR_DIRECTORY_CORESERVICE,
    SYSDIR_DIRECTORY_AUTOSAVED_INFORMATION,
    SYSDIR_DIRECTORY_DESKTOP,
    SYSDIR_DIRECTORY_CACHES,
    SYSDIR_DIRECTORY_APPLICATION_SUPPORT,
    SYSDIR_DIRECTORY_DOWNLOADS,
    SYSDIR_DIRECTORY_INPUT_METHODS,
    SYSDIR_DIRECTORY_MOVIES,
    SYSDIR_DIRECTORY_MUSIC,
    SYSDIR_DIRECTORY_PICTURES,
    SYSDIR_DIRECTORY_PRINTER_DESCRIPTION,
    SYSDIR_DIRECTORY_SHARED_PUBLIC,
    SYSDIR_DIRECTORY_PREFERENCE_PANES,
    SYSDIR_DIRECTORY_ALL_APPLICATIONS = 100,
    SYSDIR_DIRECTORY_ALL_LIBRARIES = 101,
    __SYSDIR_DIRECTORY_MAX
} sysdir_search_path_directory_t;

typedef enum {
    SYSDIR_DOMAIN_MASK_USER = 1,
    SYSDIR_DOMAIN_MASK_LOCAL = 2,
    SYSDIR_DOMAIN_MASK_NETWORK = 4,
    SYSDIR_DOMAIN_MASK_SYSTEM = 8,
    SYSDIR_DOMAIN_MASK_ALL = 65535
} sysdir_search_path_domain_mask_t;

typedef u_int32_t sysdir_search_path_enumeration_state;

sysdir_search_path_enumeration_state
sysdir_start_search_path_enumeration(
        sysdir_search_path_directory_t dir,
        sysdir_search_path_domain_mask_t domainMask);

sysdir_search_path_enumeration_state
sysdir_get_next_search_path_enumeration(
        sysdir_search_path_enumeration_state state,
        char* path);

#endif /* _SYSDIR_H_ */
