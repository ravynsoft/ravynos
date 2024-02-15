/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2014 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_PYTHON_DEBUG_H
#define SUDO_PYTHON_DEBUG_H

#include <sudo_debug.h>

/*
 * Sudo python plugin debug subsystems.
 * Note that python_subsystem_ids[] is filled in at debug registration time.
 */
extern unsigned int python_subsystem_ids[];
#define PYTHON_DEBUG_PY_CALLS    (python_subsystem_ids[0])
#define PYTHON_DEBUG_C_CALLS     (python_subsystem_ids[1])
#define PYTHON_DEBUG_PLUGIN_LOAD (python_subsystem_ids[2])
#define PYTHON_DEBUG_CALLBACKS   (python_subsystem_ids[3])
#define PYTHON_DEBUG_INTERNAL    (python_subsystem_ids[4])
#define PYTHON_DEBUG_PLUGIN      (python_subsystem_ids[5])

bool python_debug_parse_flags(struct sudo_conf_debug_file_list *debug_files, const char *entry);
bool python_debug_register(const char *program, struct sudo_conf_debug_file_list *debug_files);
void python_debug_deregister(void);

#define debug_return_ptr_pynone \
    do { \
        Py_INCREF(Py_None); \
        debug_return_ptr(Py_None); \
    } while(0)

#endif /* SUDO_PYTHON_DEBUG_H */
