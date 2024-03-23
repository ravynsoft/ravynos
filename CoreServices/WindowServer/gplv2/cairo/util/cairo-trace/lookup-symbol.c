/* cairo-trace - a utility to record and replay calls to the Cairo library.
 *
 * Copyright © 2008 Chris Wilson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * A less hacky utility to lookup the debug strings for a particular
 * .text address.
 * Derived from backtrace-symbols.c in cairo by Chris Wilson.
 */

/*
   A hacky replacement for backtrace_symbols in glibc

   backtrace_symbols in glibc looks up symbols using dladdr which is limited
   in the symbols that it sees. libbacktracesymbols opens the executable and
   shared libraries using libbfd and will look up backtrace information using
   the symbol table and the dwarf line information.

   It may make more sense for this program to use libelf instead of libbfd.
   However, I have not investigated that yet.

   Derived from addr2line.c from GNU Binutils by Jeff Muizelaar

   Copyright 2007 Jeff Muizelaar
   */

/* addr2line.c -- convert addresses to line number and function name
   Copyright 1997, 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
   Contributed by Ulrich Lauther <Ulrich.Lauther@mchp.siemens.de>

   This file was part of GNU Binutils.
   */

#include "config.h"

#define true 1
#define false 0

#include "lookup-symbol.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <link.h>
#include <string.h>
#include <pthread.h>

#if HAVE_BFD
#include <bfd.h>

struct symtab {
    bfd *bfd;
    asymbol **syms;
};

struct symbol {
    int found;
    bfd_vma pc;
    struct symtab *symtab;
    const char *filename;
    const char *functionname;
    unsigned int line;
};


static void
_symtab_fini (struct symtab *symtab)
{
    free (symtab->syms);
    if (symtab->bfd != NULL)
	bfd_close (symtab->bfd);
}

/* Read in the symbol table.  */
static int
_symtab_init (struct symtab *symtab, const char *filename)
{
    char **matching;
    long symcount;
    unsigned int size;

    symtab->bfd = NULL;
    symtab->syms = NULL;

    symtab->bfd = bfd_openr (filename, NULL);
    if (symtab->bfd == NULL)
	goto BAIL;

    if (bfd_check_format (symtab->bfd, bfd_archive))
	goto BAIL;

    if (! bfd_check_format_matches (symtab->bfd, bfd_object, &matching))
	goto BAIL;

    symcount = bfd_read_minisymbols (symtab->bfd, false, (void **) &symtab->syms, &size);
    if (symcount == 0) {
	symcount = bfd_read_minisymbols (symtab->bfd, true /* dynamic */ ,
		(void **) &symtab->syms, &size);
    }
    if (symcount < 0)
	goto BAIL;

    if ((bfd_get_file_flags (symtab->bfd) & HAS_SYMS) == 0)
	goto BAIL;

    return 1;

BAIL:
    _symtab_fini (symtab);
    return 0;
}

/* Look for an address in a section.
 * This is called via bfd_map_over_sections.
 */
static void
find_address_in_section (bfd *abfd,
			 asection *section,
			 void *data)
{
    bfd_vma vma;
    bfd_size_type size;
    struct symbol *symbol = data;
    struct symtab *symtab = symbol->symtab;

    if (symbol->found)
	return;

    if ((bfd_section_flags (section) & SEC_ALLOC) == 0)
	return;

    vma = bfd_section_vma (section);
    if (symbol->pc < vma)
	return;

    size = bfd_section_size (section);
    if (symbol->pc >= vma + size)
	return;

    symbol->found = bfd_find_nearest_line (symtab->bfd, section,
	                                   symtab->syms,
					   symbol->pc - vma,
					   &symbol->filename,
					   &symbol->functionname,
					   &symbol->line);
}

static void
_symbol_fini (struct symbol *symbol)
{
}

static void
_symbol_init (struct symbol *symbol, struct symtab *symtab, bfd_vma addr)
{
    symbol->found = false;
    symbol->symtab = symtab;
    symbol->pc = addr;
}

static void
_symbol_print (struct symbol *symbol, char *buf, int buflen, const char *filename)
{
    const char *name, *h;
    char path[1024];

    if (! symbol->found)
	return;

    name = symbol->functionname;
    if (name == NULL || *name == '\0')
	name = "??";

    if (symbol->filename != NULL)
	filename = symbol->filename;
    if (strcmp (filename, "/proc/self/exe") == 0) {
	int len = readlink ("/proc/self/exe", path, sizeof (path) - 1);
	if (len != -1) {
	    path[len] = '\0';
	    filename = path;
	}
    }
    h = strrchr (filename, '/');
    if (h != NULL)
	filename = h + 1;

    if (symbol->line) {
	snprintf (buf, buflen, "%s() [%s:%u]",
		  name, filename, symbol->line);
    } else {
	snprintf (buf, buflen, "%s() [%s]", name, filename);
    }
}
#endif

struct file_match {
    const char *file;
    ElfW(Addr) address;
    ElfW(Addr) base;
    void *hdr;
};

static int
find_matching_file (struct dl_phdr_info *info, size_t size, void *data)
{
    struct file_match *match = data;
    /* This code is modeled from Gfind_proc_info-lsb.c:callback() from libunwind */
    long n;
    const ElfW(Phdr) *phdr;
    ElfW(Addr) load_base = info->dlpi_addr;

    phdr = info->dlpi_phdr;
    for (n = info->dlpi_phnum; --n >= 0; phdr++) {
	if (phdr->p_type == PT_LOAD) {
	    ElfW(Addr) vaddr = phdr->p_vaddr + load_base;
	    if (match->address >= vaddr &&
		match->address < vaddr + phdr->p_memsz)
	    {
		/* we found a match */
		match->file = info->dlpi_name;
		match->base = info->dlpi_addr;
		return 1;
	    }
	}
    }

    return 0;
}

struct symbol_cache_entry {
    const void *ptr;
    struct symbol_cache_entry *hash_prev, *hash_next;
    char name[0];
};

static struct symbol_cache_entry *symbol_cache_hash[13477];
static pthread_mutex_t symbol_cache_mutex = PTHREAD_MUTEX_INITIALIZER;

char *
lookup_symbol (char *buf, int buflen, const void *ptr)
{
    struct file_match match;
#if HAVE_BFD
    struct symtab symtab;
    struct symbol symbol;
#endif
    struct symbol_cache_entry *cache;
    int bucket;
    int len;

    bucket = (uintptr_t) ptr % (sizeof (symbol_cache_hash) / sizeof (symbol_cache_hash[0]));
    pthread_mutex_lock (&symbol_cache_mutex);
    for (cache = symbol_cache_hash[bucket];
	 cache != NULL;
	 cache = cache->hash_next)
    {
	if (cache->ptr == ptr) {
	    if (cache->hash_prev != NULL) {
		cache->hash_prev->hash_next = cache->hash_next;
		if (cache->hash_next != NULL)
		    cache->hash_next->hash_prev = cache->hash_prev;
		cache->hash_prev = NULL;
		cache->hash_next = symbol_cache_hash[bucket];
		symbol_cache_hash[bucket]->hash_prev = cache;
		symbol_cache_hash[bucket] = cache;
	    }

	    pthread_mutex_unlock (&symbol_cache_mutex);
	    return cache->name;
	}
    }
    pthread_mutex_unlock (&symbol_cache_mutex);

    match.file = NULL;
    match.address = (ElfW(Addr)) ptr;
    dl_iterate_phdr (find_matching_file, &match);

    snprintf (buf, buflen, "0x%llx",
	      (long long unsigned int) match.address);

    if (match.file == NULL || *match.file == '\0')
	match.file = "/proc/self/exe";

#if HAVE_BFD
    if (_symtab_init (&symtab, match.file)) {
	_symbol_init (&symbol, &symtab, match.address - match.base);
	bfd_map_over_sections (symtab.bfd, find_address_in_section, &symbol);
	if (symbol.found)
	    _symbol_print (&symbol, buf, buflen, match.file);
	_symbol_fini (&symbol);

	_symtab_fini (&symtab);
    }
#endif

    len = strlen (buf);
    cache = malloc (sizeof (struct symbol_cache_entry) + len + 1);
    if (cache != NULL) {
	cache->ptr = ptr;
	memcpy (cache->name, buf, len + 1);

	pthread_mutex_lock (&symbol_cache_mutex);
	cache->hash_prev = NULL;
	cache->hash_next = symbol_cache_hash[bucket];
	if (symbol_cache_hash[bucket] != NULL)
	    symbol_cache_hash[bucket]->hash_prev = cache;
	symbol_cache_hash[bucket] = cache;
	pthread_mutex_unlock (&symbol_cache_mutex);
    }

    return buf;
}
