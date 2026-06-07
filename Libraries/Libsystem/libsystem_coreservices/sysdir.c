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


#include "sysdir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define ITER_ALL_DOMAINS    0x80000000
#define ITER_ALL_APPS       0x40000000

static const char* _base_paths[__SYSDIR_DIRECTORY_MAX] = {
    "NULL",
    "Applications",
    "Applications/Demos",
    "Library/Developer/Applications",
    "Applications/Utilities",
    "Library",
    "Library/Developer",
    "Users",
    "Library/Documentation",
    "Documents",
    "Library/CoreServices",
    "Library/Autosave Information",
    "Desktop",
    "Library/Caches",
    "Library/Application Support",
    "Downloads",
    "Library/Input Methods",
    "Movies",
    "Music",
    "Pictures",
    "Library/Printers/PPDs",
    "Public",
    "Library/Preference Panes",
    "__special__", /* handled specially */
    "__special__" /* handled specially */
};

static char*
_domain_path(sysdir_search_path_domain_mask_t mask);

static unsigned int
_prepend_next_root(char* path);

sysdir_search_path_enumeration_state
sysdir_start_search_path_enumeration(
        sysdir_search_path_directory_t dir,
        sysdir_search_path_domain_mask_t domain_mask);

sysdir_search_path_enumeration_state
sysdir_get_next_search_path_enumeration(
        sysdir_search_path_enumeration_state state,
        char* path);



static sysdir_search_path_domain_mask_t
_next_domain(sysdir_search_path_domain_mask_t domain)
{
    int all_domains = domain & 0x8000;
    sysdir_search_path_domain_mask_t next = 0;

    if (!all_domains)
        return 0; /* there is no next domain */

    domain &= 0x000F;
    switch (domain) {
        case SYSDIR_DOMAIN_MASK_USER:
            next = SYSDIR_DOMAIN_MASK_LOCAL | all_domains; break;
        case SYSDIR_DOMAIN_MASK_LOCAL:
            next = SYSDIR_DOMAIN_MASK_NETWORK | all_domains; break;
        case SYSDIR_DOMAIN_MASK_NETWORK:
            next = SYSDIR_DOMAIN_MASK_SYSTEM | all_domains; break;
        default: break;
    }
    return next;
}

static char*
_domain_path(sysdir_search_path_domain_mask_t mask)
{
    switch (mask & 0x000F) {
        case SYSDIR_DOMAIN_MASK_USER: return (char*)"~";
        case SYSDIR_DOMAIN_MASK_LOCAL: return (char*)""; /* /, but avoid doubles */
        case SYSDIR_DOMAIN_MASK_NETWORK: return (char*)"/Network";
        case SYSDIR_DOMAIN_MASK_SYSTEM: return (char*)"/System";
        case SYSDIR_DOMAIN_MASK_ALL: break;
    }

    return NULL;
}

static unsigned int
_prepend_next_root(char* path)
{
    const char* next_root = getenv("NEXT_ROOT");
    unsigned int path_len = 0;

    if (!next_root)
        return 0;

    if (getuid() != geteuid() || getgid() != getegid())
        /* FIXME: or if codesigned with entitlements */
        return 0;

    path_len = strlen(next_root);
    memcpy(path, next_root, path_len);
    char* ch = &path[path_len];
    if (*ch != '/')
        path[path_len++] = '/';

    return path_len;
}
    

sysdir_search_path_enumeration_state
sysdir_start_search_path_enumeration(
        sysdir_search_path_directory_t dir,
        sysdir_search_path_domain_mask_t domain_mask)
{
    if (domain_mask == SYSDIR_DOMAIN_MASK_ALL)
        return ITER_ALL_DOMAINS | (SYSDIR_DOMAIN_MASK_USER << 16) | dir;
    return (domain_mask << 16) | dir;
}

sysdir_search_path_enumeration_state
sysdir_get_next_search_path_enumeration(
        sysdir_search_path_enumeration_state state,
        char* path)
{
    sysdir_search_path_enumeration_state next_state = 0; /* end of search */
    sysdir_search_path_directory_t dir = (state & 0xFFFF);
    sysdir_search_path_domain_mask_t domain_mask = (state & 0xFFFF0000) >> 16;
    int has_entry = 1;
    sysdir_search_path_enumeration_state state_flags = 0;
    
    if (path)
        memset(path, 0, PATH_MAX);

    if ((domain_mask == SYSDIR_DOMAIN_MASK_USER && dir == SYSDIR_DIRECTORY_USER)
        || ((domain_mask & 0x000f) != SYSDIR_DOMAIN_MASK_USER
         && (dir == SYSDIR_DIRECTORY_DOCUMENT ||
             dir == SYSDIR_DIRECTORY_DESKTOP ||
             dir == SYSDIR_DIRECTORY_AUTOSAVED_INFORMATION ||
             dir == SYSDIR_DIRECTORY_DOWNLOADS ||
             dir == SYSDIR_DIRECTORY_MOVIES ||
             dir == SYSDIR_DIRECTORY_MUSIC ||
             dir == SYSDIR_DIRECTORY_PICTURES ||
             dir == SYSDIR_DIRECTORY_SHARED_PUBLIC))) {
        has_entry = 0;
    }
    else if (dir == SYSDIR_DIRECTORY_ALL_APPLICATIONS) {
        dir = SYSDIR_DIRECTORY_APPLICATION;
        domain_mask = SYSDIR_DOMAIN_MASK_USER | ITER_ALL_DOMAINS >> 16
            | ITER_ALL_APPS >> 16;
        state_flags = ITER_ALL_APPS|ITER_ALL_DOMAINS;
    }
    else if (dir == SYSDIR_DIRECTORY_ALL_LIBRARIES) {
        dir = SYSDIR_DIRECTORY_LIBRARY;
        domain_mask = SYSDIR_DOMAIN_MASK_ALL;
    } else {
        state_flags = (state & 0xF0000000);
    }

    if (path && has_entry) {
        int path_len = _prepend_next_root(path);
        char* p = _domain_path(domain_mask);
        if (p) {
            strncat(path, p, PATH_MAX - path_len - 1);
            path_len += strlen(p);
        }
        if (path_len + 1 < PATH_MAX - 1)
            path[path_len++] = '/';
        strncat(path, _base_paths[dir], PATH_MAX - path_len - 1);
    }

    sysdir_search_path_domain_mask_t next_domain = _next_domain(domain_mask);
    if (next_domain) {
        next_state = next_domain << 16 | state_flags | dir;
    }
    else if (state_flags & ITER_ALL_APPS) {
        switch (dir) {
            case SYSDIR_DIRECTORY_APPLICATION:
                next_state = ITER_ALL_DOMAINS | SYSDIR_DOMAIN_MASK_USER << 16
                    | SYSDIR_DIRECTORY_DEMO_APPLICATION | ITER_ALL_APPS;
                break;
            case SYSDIR_DIRECTORY_DEMO_APPLICATION:
                next_state = ITER_ALL_DOMAINS | SYSDIR_DOMAIN_MASK_USER << 16
                    | SYSDIR_DIRECTORY_DEVELOPER_APPLICATION | ITER_ALL_APPS;
                break;
            case SYSDIR_DIRECTORY_DEVELOPER_APPLICATION:
                next_state = ITER_ALL_DOMAINS | SYSDIR_DOMAIN_MASK_USER << 16
                    | SYSDIR_DIRECTORY_ADMIN_APPLICATION | ITER_ALL_APPS;
                break;
            default: break;
        }
    }

    return next_state;
}


#if TEST
#include <assert.h>

int main(int argc, char** argv)
{
    char path[PATH_MAX];
    sysdir_search_path_enumeration_state state = 0;
    sysdir_search_path_directory_t dir = 0;
    sysdir_search_path_domain_mask_t domain = 0;

    printf("T1: get applications directories - ");
    state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_APPLICATION,
                                                 SYSDIR_DOMAIN_MASK_ALL);
    assert(state == 0x80010001);
    
    do {
        state = sysdir_get_next_search_path_enumeration(state, path);
        printf("%s ", path);
        assert(state & ITER_ALL_DOMAINS || state == 0);
    } while(state != 0);
    printf("\n");


    printf("T2: get desktop directories - ");
    state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_DESKTOP,
                                                 SYSDIR_DOMAIN_MASK_ALL);
    assert(state == 0x8001000C);
    
    do {
        state = sysdir_get_next_search_path_enumeration(state, path);
        printf("%s ", path);
        assert(state & ITER_ALL_DOMAINS || state == 0);
    } while(state != 0);
    printf("\n");


    printf("T3: get system library directories - ");
    state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_LIBRARY,
                                                 SYSDIR_DOMAIN_MASK_SYSTEM);    
    state = sysdir_get_next_search_path_enumeration(state, path);
    printf("%s ", path);
    assert(state == 0);
    printf("\n");

    printf("T4: get user cache directories - ");
    state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_CACHES,
                                                 SYSDIR_DOMAIN_MASK_USER);    
    state = sysdir_get_next_search_path_enumeration(state, path);
    printf("%s ", path);
    assert(state == 0);
    printf("\n");

    printf("T5: get all application directories - ");
    state = sysdir_start_search_path_enumeration(SYSDIR_DIRECTORY_ALL_APPLICATIONS,
                                                 SYSDIR_DOMAIN_MASK_ALL);
    do {
        state = sysdir_get_next_search_path_enumeration(state, path);
        printf("%s ", path);
        assert(state & ITER_ALL_DOMAINS || state == 0);
    } while(state != 0);
    printf("\n");
}

#endif
