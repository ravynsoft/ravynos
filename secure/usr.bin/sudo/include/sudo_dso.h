/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2010, 2013, 2014 Todd C. Miller <Todd.Miller@sudo.ws>
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

#ifndef SUDO_DSO_H
#define SUDO_DSO_H

/* Values for sudo_dso_load() mode. */
#define SUDO_DSO_LAZY	 0x1
#define SUDO_DSO_NOW	 0x2
#define SUDO_DSO_GLOBAL	 0x4
#define SUDO_DSO_LOCAL	 0x8

/* Special handle arguments for sudo_dso_findsym(). */
#define SUDO_DSO_NEXT	 ((void *)-1)	/* Search subsequent objects. */
#define SUDO_DSO_DEFAULT ((void *)-2)	/* Use default search algorithm. */
#define SUDO_DSO_SELF	 ((void *)-3)	/* Search the caller itself. */

/* Internal structs for static linking of plugins. */
struct sudo_preload_symbol {
    const char *name;
    void *addr;
};
struct sudo_preload_table {
    const char *path;
    void *handle;
    struct sudo_preload_symbol *symbols;
};

/* Public functions. */
sudo_dso_public char *sudo_dso_strerror_v1(void);
sudo_dso_public int sudo_dso_unload_v1(void *handle);
sudo_dso_public void *sudo_dso_findsym_v1(void *handle, const char *symbol);
sudo_dso_public void *sudo_dso_load_v1(const char *path, int mode);
sudo_dso_public void sudo_dso_preload_table_v1(struct sudo_preload_table *table);

#define sudo_dso_strerror() sudo_dso_strerror_v1()
#define sudo_dso_unload(_a) sudo_dso_unload_v1((_a))
#define sudo_dso_findsym(_a, _b) sudo_dso_findsym_v1((_a), (_b))
#define sudo_dso_load(_a, _b) sudo_dso_load_v1((_a), (_b))
#define sudo_dso_preload_table(_a) sudo_dso_preload_table_v1((_a))

#endif /* SUDO_DSO_H */
