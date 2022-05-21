/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_DIR_H
#define __LABWC_DIR_H

char *config_dir(void);

/**
 * theme_dir - find theme directory containing theme @theme_name
 * @theme_name: theme to search for
 */
char *theme_dir(const char *theme_name);

#endif /* __LABWC_DIR_H */
