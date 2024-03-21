/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2020 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This is an open source non-commercial project. Dear PVS-Studio, please check it.
 * PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
 */

#include <config.h>
#include <string.h>

#include <ctype.h>
#include <stdlib.h>

#include <sudo_gettext.h>
#include <sudo_compat.h>
#include "sudo_python_debug.h"
#include <sudo_queue.h>
#include <sudo_conf.h>
#include <sudo_fatal.h>


static int python_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
static unsigned int python_debug_refcnt;

static const char *const python_subsystem_names[] = {
    "py_calls",  // logs c  -> py calls
    "c_calls",   // logs py -> c  calls
    "load",      // logs python plugin loading / unloading
    "sudo_cb",   // logs sudo callback calls
    "internal",  // logs internal functions of the language wrapper plugin
    "plugin",    // logs whatever log the python module would like to log through sudo.debug API
    NULL
};

#define NUM_SUBSYSTEMS sizeof(python_subsystem_names) / sizeof(*python_subsystem_names) - 1

/* Subsystem IDs assigned at registration time. */
unsigned int python_subsystem_ids[NUM_SUBSYSTEMS];

/*
 * Parse the "filename flags,..." debug_flags entry and insert a new
 * sudo_debug_file struct into debug_files.
 */
bool
python_debug_parse_flags(struct sudo_conf_debug_file_list *debug_files,
    const char *entry)
{
    /* Already initialized? */
    if (python_debug_instance != SUDO_DEBUG_INSTANCE_INITIALIZER)
        return true;

    return sudo_debug_parse_flags(debug_files, entry) != -1;
}

/*
 * Register the specified debug files and program with the
 * debug subsystem, freeing the debug list when done.
 * Sets the active debug instance as a side effect.
 */
bool
python_debug_register(const char *program,
    struct sudo_conf_debug_file_list *debug_files)
{
    int instance = python_debug_instance;
    struct sudo_debug_file *debug_file, *debug_next;

    /* Setup debugging if indicated. */
    if (debug_files != NULL && !TAILQ_EMPTY(debug_files)) {
        if (program != NULL) {
            instance = sudo_debug_register(program, python_subsystem_names,
                python_subsystem_ids, debug_files, -1);
        }
        TAILQ_FOREACH_SAFE(debug_file, debug_files, entries, debug_next) {
            TAILQ_REMOVE(debug_files, debug_file, entries);
            free(debug_file->debug_file);
            free(debug_file->debug_flags);
            free(debug_file);
        }
    }

    switch (instance) {
    case SUDO_DEBUG_INSTANCE_ERROR:
        return false;
    case SUDO_DEBUG_INSTANCE_INITIALIZER:
        /* Nothing to do */
        break;
    default:
        /* New debug instance or additional reference on existing one. */
        python_debug_instance = instance;
        sudo_debug_set_active_instance(python_debug_instance);
        python_debug_refcnt++;
        break;
    }

    return true;
}

/*
 * Deregister python_debug_instance if it is registered.
 */
void
python_debug_deregister(void)
{
    debug_decl(python_debug_deregister, PYTHON_DEBUG_INTERNAL);

    if (python_debug_refcnt != 0) {
        sudo_debug_exit(__func__, __FILE__, __LINE__, sudo_debug_subsys);
        if (--python_debug_refcnt == 0) {
            if (sudo_debug_deregister(python_debug_instance) < 1)
                python_debug_instance = SUDO_DEBUG_INSTANCE_INITIALIZER;
        }
    }
}
