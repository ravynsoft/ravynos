/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_SESSION_H
#define __LABWC_SESSION_H

/**
 * session_environment_init - set enrivonment variables based on <key>=<value>
 * pairs in `${XDG_CONFIG_DIRS:-/etc/xdg}/lawbc/environment` with user override
 * in `${XDG_CONFIG_HOME:-$HOME/.config}`
 */
void session_environment_init(void);

/**
 * session_autostart_init - run autostart file as shell script
 * Note: Same as `sh ~/.config/labwc/autostart` (or equivalent XDG config dir)
 */
void session_autostart_init(void);

#endif /* __LABWC_SESSION_H */
