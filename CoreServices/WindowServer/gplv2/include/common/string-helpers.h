/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __LABWC_STRING_HELPERS_H
#define __LABWC_STRING_HELPERS_H

/**
 * string_strip - strip white space left and right
 * Note: this function does a left skip, so the returning pointer cannot be
 * used to free any allocated memory
 */
char *string_strip(char *s);

/**
 * string_truncate_at_pattern - remove pattern and everything after it
 * @buf: pointer to buffer
 * @pattern: string to remove
 */
void string_truncate_at_pattern(char *buf, const char *pattern);

#endif /* __LABWC_STRING_HELPERS_H */
