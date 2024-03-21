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

#ifndef SUDOERS_DEBUG_H
#define SUDOERS_DEBUG_H

#include <sudo_debug.h>

/*
 * Sudoers debug subsystems.
 * Note that sudoers_subsystem_ids[] is filled in at debug registration time.
 */
extern unsigned int sudoers_subsystem_ids[];
#define SUDOERS_DEBUG_ALIAS	(sudoers_subsystem_ids[ 0]) /* sudoers alias functions */
#define SUDOERS_DEBUG_AUDIT	(sudoers_subsystem_ids[ 1]) /* audit */
#define SUDOERS_DEBUG_AUTH	(sudoers_subsystem_ids[ 2]) /* authentication functions */
#define SUDOERS_DEBUG_DEFAULTS	(sudoers_subsystem_ids[ 3]) /* sudoers defaults settings */
#define SUDOERS_DEBUG_ENV	(sudoers_subsystem_ids[ 4]) /* environment handling */
#define SUDOERS_DEBUG_EVENT	(sudoers_subsystem_ids[ 5]) /* event handling */
#define SUDOERS_DEBUG_LDAP	(sudoers_subsystem_ids[ 6]) /* sudoers LDAP */
#define SUDOERS_DEBUG_LOGGING	(sudoers_subsystem_ids[ 7]) /* logging functions */
#define SUDOERS_DEBUG_MAIN	(sudoers_subsystem_ids[ 8]) /* main() */
#define SUDOERS_DEBUG_MATCH	(sudoers_subsystem_ids[ 9]) /* sudoers matching */
#define SUDOERS_DEBUG_NETIF	(sudoers_subsystem_ids[10]) /* network interface functions */
#define SUDOERS_DEBUG_NSS	(sudoers_subsystem_ids[11]) /* network service switch */
#define SUDOERS_DEBUG_PARSER	(sudoers_subsystem_ids[12]) /* sudoers parser */
#define SUDOERS_DEBUG_PERMS	(sudoers_subsystem_ids[13]) /* uid/gid swapping functions */
#define SUDOERS_DEBUG_PLUGIN	(sudoers_subsystem_ids[14]) /* main plugin functions */
#define SUDOERS_DEBUG_RBTREE	(sudoers_subsystem_ids[15]) /* red-black tree functions */
#define SUDOERS_DEBUG_SSSD	(sudoers_subsystem_ids[16]) /* sudoers SSSD */
#define SUDOERS_DEBUG_UTIL	(sudoers_subsystem_ids[17]) /* utility functions */

bool sudoers_debug_parse_flags(struct sudo_conf_debug_file_list *debug_files, const char *entry);
bool sudoers_debug_register(const char *plugin_path, struct sudo_conf_debug_file_list *debug_files);
void sudoers_debug_deregister(void);

#endif /* SUDOERS_DEBUG_H */
