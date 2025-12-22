/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2006 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Routines for retrieving CTF data from a .SUNW_ctf ELF section
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <gelf.h>
#include <strings.h>
#include <sys/types.h>
#ifdef __linux__
#include <string.h>
#endif

#include "ctftools.h"
#include "memory.h"
#include "symbol.h"

typedef int read_cb_f(tdata_t *, char *, void *);

/*
 * Return the source types that the object was generated from.
 */
source_types_t
built_source_types(Elf *elf, char const *file)
{
	source_types_t types = SOURCE_NONE;
	symit_data_t *si;

	if ((si = symit_new(elf, file)) == NULL)
		return (SOURCE_NONE);

	while (symit_next(si, STT_FILE) != NULL) {
		char *name = symit_name(si);
		size_t len = strlen(name);
		if (len < 2 || name[len - 2] != '.') {
			types |= SOURCE_UNKNOWN;
			continue;
		}

		switch (name[len - 1]) {
		case 'c':
			types |= SOURCE_C;
			break;
		case 'h':
			/* ignore */
			break;
		case 's':
			types |= SOURCE_S;
			break;
		default:
			types |= SOURCE_UNKNOWN;
		}
	}

	symit_free(si);
	return (types);
}

static int
read_file(Elf *elf, char *file, char *label, read_cb_f *func, void *arg,
    int require_ctf)
{
	Elf_Scn *ctfscn;
	Elf_Data *ctfdata;
	symit_data_t *si = NULL;
	int ctfscnidx;
	tdata_t *td;

	if ((ctfscnidx = findelfsecidx(elf, file, ".SUNW_ctf")) < 0) {
		if (require_ctf &&
		    (built_source_types(elf, file) & SOURCE_C)) {
			terminate("Input file %s was partially built from "
			    "C sources, but no CTF data was present\n", file);
		}
		return (0);
	}

	if ((ctfscn = elf_getscn(elf, ctfscnidx)) == NULL ||
	    (ctfdata = elf_getdata(ctfscn, NULL)) == NULL)
		elfterminate(file, "Cannot read CTF section");

	/* Reconstruction of type tree */
	if ((si = symit_new(elf, file)) == NULL) {
#if !defined(__APPLE__)
		warning("%s has no symbol table - skipping", file);
#endif
		return (0);
	}

	td = ctf_load(file, ctfdata->d_buf, ctfdata->d_size, si, label);
	tdata_build_hashes(td);

	symit_free(si);

	if (td != NULL) {
		if (func(td, file, arg) < 0)
			return (-1);
		else
			return (1);
	}
	return (0);
}

static int
read_ctf_common(char *file, char *label, read_cb_f *func, void *arg,
    int require_ctf)
{
	Elf *elf;
	int found = 0;
	int fd;

	debug(3, "Reading %s (label %s)\n", file, (label ? label : "NONE"));

	(void) elf_version(EV_CURRENT);

	if ((fd = open(file, O_RDONLY)) < 0)
		terminate("%s: Cannot open for reading", file);
	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		elfterminate(file, "Cannot read");

	switch (elf_kind(elf)) {
	case ELF_K_ELF:
#if defined(__APPLE__)
	case ELF_K_MACHO: /* Underlying file is Mach-o */
#endif /* __APPLE__ */
		found = read_file(elf, file, label,
		    func, arg, require_ctf);
		break;

	default:
		terminate("%s: Unknown elf kind %d\n", file, elf_kind(elf));
	}

	(void) elf_end(elf);
	(void) close(fd);

	return (found);
}

/*ARGSUSED*/
int
read_ctf_save_cb(tdata_t *td, char *name, void *retp)
{
	tdata_t **tdp = retp;

	*tdp = td;

	return (1);
}

int
read_ctf(char **files, int n, char *label, read_cb_f *func, void *private,
    int require_ctf)
{
	int found;
	int i, rc;

	for (i = 0, found = 0; i < n; i++) {
		if ((rc = read_ctf_common(files[i], label, func,
		    private, require_ctf)) < 0)
			return (rc);
		found += rc;
	}

	return (found);
}

int
count_files(char **files, int n)
{
	int nfiles = 0, err = 0;
	Elf *elf;
	int fd, rc, i;

	(void) elf_version(EV_CURRENT);

	for (i = 0; i < n; i++) {
		char *file = files[i];

		if ((fd = open(file, O_RDONLY)) < 0) {
			warning("Can't read input file %s", file);
			err++;
			continue;
		}

		if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
			warning("Can't open input file %s: %s\n", file,
			    elf_errmsg(-1));
			err++;
			(void) close(fd);
			continue;
		}

		switch (elf_kind(elf)) {
		case ELF_K_ELF:
#if defined(__APPLE__)
		case ELF_K_MACHO: /* Underlying file is Mach-o */
#endif /* __APPLE__ */
			nfiles++;
			break;
		default:
			warning("Input file %s is corrupt\n", file);
			err++;
		}

		(void) elf_end(elf);
		(void) close(fd);
	}

	if (err > 0)
		return (-1);

	debug(2, "Found %d files in %d input files\n", nfiles, n);

	return (nfiles);
}

struct symit_data {
	GElf_Shdr si_shdr;
	Elf_Data *si_symd;
	Elf_Data *si_strd;
	GElf_Sym si_cursym;
	char *si_curname;
	char *si_curfile;
	int si_nument;
	int si_next;
};

symit_data_t *
symit_new(Elf *elf, const char *file)
{
	symit_data_t *si;
	Elf_Scn *scn;
	int symtabidx;

	if ((symtabidx = findelfsecidx(elf, file, ".symtab")) < 0)
		return (NULL);

	si = xcalloc(sizeof (symit_data_t));

	if ((scn = elf_getscn(elf, symtabidx)) == NULL ||
	    gelf_getshdr(scn, &si->si_shdr) == NULL ||
	    (si->si_symd = elf_getdata(scn, NULL)) == NULL)
		elfterminate(file, "Cannot read .symtab");

#if !defined(__APPLE__)
	if ((scn = elf_getscn(elf, si->si_shdr.sh_link)) == NULL ||
	    (si->si_strd = elf_getdata(scn, NULL)) == NULL)
		elfterminate(file, "Cannot read strings for .symtab");
#else
	if (SHN_MACHO != si->si_shdr.sh_link && SHN_MACHO_64 != si->si_shdr.sh_link) {
		if ((scn = elf_getscn(elf, si->si_shdr.sh_link)) == NULL || 
		    (si->si_strd = elf_getdata(scn, NULL)) == NULL)
			elfterminate(file, "Cannot read strings for .symtab");
	} else {
		/* Underlying file is Mach-o */
		int dir_idx;
		
		if ((dir_idx = findelfsecidx(elf, file, ".dir_str_table")) < 0 ||
		    (scn = elf_getscn(elf, dir_idx)) == NULL ||
		    (si->si_strd = elf_getdata(scn, NULL)) == NULL)
			elfterminate(file, "Cannot read strings for .dir_str_table");
	}
#endif /* __APPLE__ */
	si->si_nument = si->si_shdr.sh_size / si->si_shdr.sh_entsize;

	return (si);
}

void
symit_free(symit_data_t *si)
{
	free(si);
}

void
symit_reset(symit_data_t *si)
{
	si->si_next = 0;
}

char *
symit_curfile(symit_data_t *si)
{
	return (si->si_curfile);
}

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

static GElf_Sym *
gelf_getsym_macho(Elf_Data * data, int ndx, GElf_Sym * sym, const char *base)
{
	const struct nlist *nsym = (const struct nlist *)(data->d_buf);
	const char *name;
	
	nsym += ndx;
	name = base + nsym->n_un.n_strx;
	
	if (0 == nsym->n_un.n_strx) // iff a null, "", name.
		name = "null name"; // return NULL;

	if ('_' == name[0])
		name++; // Lop off omnipresent underscore to match DWARF convention

	sym->st_name = (GElf_Sxword)(name - base);
	sym->st_value = nsym->n_value;
	sym->st_size = 0;
	sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_NOTYPE));
	sym->st_other = 0;
	sym->st_shndx = SHN_MACHO; /* Mark underlying file as Mach-o */
	
	if (nsym->n_type & N_STAB) { /* Detect C++ methods */
	
		switch(nsym->n_type) {
		case N_FUN:
			sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_FUNC));
			break;
		case N_GSYM:
			sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT));
			break;
		default:
			break;
		}
		
	} else if ((N_ABS | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) ||
		(N_SECT | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT))) {

		sym->st_info = GELF_ST_INFO((STB_GLOBAL), (nsym->n_desc)); 
	} else if ((N_UNDF | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) &&
				nsym->n_sect == NO_SECT) {
		sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT)); /* Common */
	}
	
	return sym;
}

static GElf_Sym *
gelf_getsym_macho_64(Elf_Data * data, int ndx, GElf_Sym * sym, const char *base)
{
	const struct nlist_64 *nsym = (const struct nlist_64 *)(data->d_buf);
	const char *name;
	
	nsym += ndx;
	name = base + nsym->n_un.n_strx;
	
	if (0 == nsym->n_un.n_strx) // iff a null, "", name.
		name = "null name"; // return NULL;

	if ('_' == name[0])
		name++; // Lop off omnipresent underscore to match DWARF convention

	sym->st_name = (GElf_Sxword)(name - base);
	sym->st_value = nsym->n_value;
	sym->st_size = 0;
	sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_NOTYPE));
	sym->st_other = 0;
	sym->st_shndx = SHN_MACHO_64; /* Mark underlying file as Mach-o 64 */
	
	if (nsym->n_type & N_STAB) { /* Detect C++ methods */
	
		switch(nsym->n_type) {
		case N_FUN:
			sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_FUNC));
			break;
		case N_GSYM:
			sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT));
			break;
		default:
			break;
		}
		
	} else if ((N_ABS | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) ||
		(N_SECT | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT))) {

		sym->st_info = GELF_ST_INFO((STB_GLOBAL), (nsym->n_desc)); 
	} else if ((N_UNDF | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) &&
				nsym->n_sect == NO_SECT) {
		sym->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT)); /* Common */
	}
	
	return sym;
}

#endif /* __APPLE__ */

GElf_Sym *
symit_next(symit_data_t *si, int type)
{
	GElf_Sym sym;
	int check_sym = (type == STT_OBJECT || type == STT_FUNC);

	for (; si->si_next < si->si_nument; si->si_next++) {
#if !defined(__APPLE__)
		gelf_getsym(si->si_symd, si->si_next, &si->si_cursym);
		gelf_getsym(si->si_symd, si->si_next, &sym);
#else
		if (si->si_shdr.sh_link == SHN_MACHO) { /* Underlying file is Mach-o */
			gelf_getsym_macho(si->si_symd, si->si_next, &si->si_cursym, (const char *)(si->si_strd->d_buf));
			gelf_getsym_macho(si->si_symd, si->si_next, &sym, (const char *)(si->si_strd->d_buf));
		} else if (si->si_shdr.sh_link == SHN_MACHO_64) { /* Underlying file is Mach-o 64 */
			gelf_getsym_macho_64(si->si_symd, si->si_next, &si->si_cursym, (const char *)(si->si_strd->d_buf));
			gelf_getsym_macho_64(si->si_symd, si->si_next, &sym, (const char *)(si->si_strd->d_buf));
		} else {
			gelf_getsym(si->si_symd, si->si_next, &si->si_cursym);
			gelf_getsym(si->si_symd, si->si_next, &sym);
		}
#endif /* __APPLE__ */
		si->si_curname = (caddr_t)si->si_strd->d_buf + sym.st_name;

		if (GELF_ST_TYPE(sym.st_info) == STT_FILE)
			si->si_curfile = si->si_curname;

		if (GELF_ST_TYPE(sym.st_info) != type ||
		    sym.st_shndx == SHN_UNDEF)
			continue;

		if (check_sym && ignore_symbol(&sym, si->si_curname))
			continue;

		si->si_next++;

		return (&si->si_cursym);
	}

	return (NULL);
}

char *
symit_name(symit_data_t *si)
{
	return (si->si_curname);
}
