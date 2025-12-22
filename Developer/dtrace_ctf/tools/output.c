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
 * Routines for preparing tdata trees for conversion into CTF data, and
 * for placing the resulting data into an output file.
 */

#ifdef __linux__
#include <string.h>
#define __APPLE__
#endif
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <unistd.h>

#include "ctftools.h"
#include "list.h"
#include "memory.h"
#include "traverse.h"
#include "symbol.h"

#if defined(__APPLE__)
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <sys/mman.h>

static GElf_Sym *
gelf_getsym_macho(Elf_Data * data, int ndx, int nent, GElf_Sym * dst, const char *base) 
{
	const struct nlist *nsym = ((const struct nlist *)(data->d_buf)) + ndx;
	const char *name = base + nsym->n_un.n_strx;

	if (0 == nsym->n_un.n_strx) // iff a null, "", name.
		name = "null name"; // return NULL;

	if ('_' == name[0])
		name++; // Lop off omnipresent underscore to match DWARF convention

	dst->st_name = (GElf_Sxword)(name - base);
	dst->st_value = nsym->n_value;
	dst->st_size = 0;
	dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_NOTYPE));
	dst->st_other = 0;
	dst->st_shndx = SHN_MACHO; /* Mark underlying file as Mach-o */
	
	if (nsym->n_type & N_STAB) {
	
		switch(nsym->n_type) {
		case N_FUN:
			dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_FUNC));
			break;
		case N_GSYM:
			dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT));
			break;
		default:
			break;
		}
		
	} else if ((N_ABS | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) ||
		(N_SECT | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT))) {

		dst->st_info = GELF_ST_INFO((STB_GLOBAL), (nsym->n_desc)); 
	} else if ((N_UNDF | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) &&
				nsym->n_sect == NO_SECT) {
		dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT)); /* Common */
	} 
		
	return dst;
}

static GElf_Sym *
gelf_getsym_macho_64(Elf_Data * data, int ndx, int nent, GElf_Sym * dst, const char *base)
{
	const struct nlist_64 *nsym = ((const struct nlist_64 *)(data->d_buf)) + ndx;
	const char *name = base + nsym->n_un.n_strx;

	if (0 == nsym->n_un.n_strx) // iff a null, "", name.
		name = "null name"; // return NULL;

	if ('_' == name[0])
		name++; // Lop off omnipresent underscore to match DWARF convention

	dst->st_name = (GElf_Sxword)(name - base);
	dst->st_value = nsym->n_value;
	dst->st_size = 0;
	dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_NOTYPE));
	dst->st_other = 0;
	dst->st_shndx = SHN_MACHO_64; /* Mark underlying file as Mach-o 64 */
	
	if (nsym->n_type & N_STAB) {
	
		switch(nsym->n_type) {
		case N_FUN:
			dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_FUNC));
			break;
		case N_GSYM:
			dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT));
			break;
		default:
			break;
		}
		
	} else if ((N_ABS | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) ||
		(N_SECT | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT))) {

		dst->st_info = GELF_ST_INFO((STB_GLOBAL), (nsym->n_desc)); 
	} else if ((N_UNDF | N_EXT) == (nsym->n_type & (N_TYPE | N_EXT)) &&
				nsym->n_sect == NO_SECT) {
		dst->st_info = GELF_ST_INFO((STB_GLOBAL), (STT_OBJECT)); /* Common */
	} 
		
	return dst;
}
#endif /* __APPLE__ */

typedef struct iidesc_match {
	int iim_fuzzy;
	iidesc_t *iim_ret;
	atom_t *iim_name;
	atom_t *iim_file;
	uchar_t iim_bind;
} iidesc_match_t;

static int
burst_iitypes(void *data, void *arg)
{
	iidesc_t *ii = data;
	iiburst_t *iiburst = arg;

	switch (ii->ii_type) {
	case II_GFUN:
	case II_SFUN:
	case II_GVAR:
	case II_SVAR:
		if (!(ii->ii_flags & IIDESC_F_USED))
			return (0);
		break;
	default:
		break;
	}

	ii->ii_dtype->t_flags |= TDESC_F_ISROOT;
	(void) iitraverse_td(ii, iiburst->iib_tdtd);
	return (1);
}

/*ARGSUSED1*/
static int
save_type_by_id(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	iiburst_t *iiburst = private;

	/*
	 * Doing this on every node is horribly inefficient, but given that
	 * we may be suppressing some types, we can't trust nextid in the
	 * tdata_t.
	 */
	if (tdp->t_id > iiburst->iib_maxtypeid)
		iiburst->iib_maxtypeid = tdp->t_id;

	array_add(&iiburst->iib_types, tdp);

	return (1);
}

static tdtrav_cb_f burst_types_cbs[] = {
	NULL,
	save_type_by_id,	/* intrinsic */
	save_type_by_id,	/* pointer */
	save_type_by_id,	/* array */
	save_type_by_id,	/* function */
	save_type_by_id,	/* struct */
	save_type_by_id,	/* union */
	save_type_by_id,	/* enum */
	save_type_by_id,	/* forward */
	save_type_by_id,	/* typedef */
	tdtrav_assert,		/* typedef_unres */
	save_type_by_id,	/* volatile */
	save_type_by_id,	/* const */
	save_type_by_id,	/* restrict */
	save_type_by_id		/* ptrauth */
};


static iiburst_t *
iiburst_new(tdata_t *td, int max)
{
	iiburst_t *iiburst = xcalloc(sizeof (iiburst_t));
	iiburst->iib_td = td;
	iiburst->iib_funcs = xcalloc(sizeof (iidesc_t *) * max);
	iiburst->iib_nfuncs = 0;
	iiburst->iib_objts = xcalloc(sizeof (iidesc_t *) * max);
	iiburst->iib_nobjts = 0;
	return (iiburst);
}

static void
iiburst_types(iiburst_t *iiburst)
{
	tdtrav_data_t tdtd;

	tdtrav_init(&tdtd, &iiburst->iib_td->td_curvgen, NULL, burst_types_cbs,
	    NULL, (void *)iiburst);

	iiburst->iib_tdtd = &tdtd;

	(void) hash_iter(iiburst->iib_td->td_iihash, burst_iitypes, iiburst);
}

static void
iiburst_free(iiburst_t *iiburst)
{
	free(iiburst->iib_funcs);
	free(iiburst->iib_objts);
	array_free(&iiburst->iib_types, NULL, NULL);
	free(iiburst);
}

/*
 * See if this iidesc matches the ELF symbol data we pass in.
 *
 * A fuzzy match is where we have a local symbol matching the name of a
 * global type description. This is common when a mapfile is used for a
 * DSO, but we don't accept it by default.
 *
 * A weak fuzzy match is when a weak symbol was resolved and matched to
 * a global type description.
 */
static int
matching_iidesc(iidesc_t *iidesc, iidesc_match_t *match)
{
	if (iidesc->ii_name != match->iim_name)
		return (0);

	switch (iidesc->ii_type) {
	case II_GFUN:
	case II_GVAR:
		if (match->iim_bind == STB_GLOBAL) {
			match->iim_ret = iidesc;
			return (-1);
		} else if (match->iim_fuzzy && match->iim_ret == NULL) {
			match->iim_ret = iidesc;
			/* continue to look for strong match */
			return (0);
		}
		break;
	case II_SFUN:
	case II_SVAR:
		if (match->iim_bind == STB_LOCAL &&
		    match->iim_file != NULL &&
		    iidesc->ii_owner == match->iim_file) {
			match->iim_ret = iidesc;
			return (-1);
		}
		break;
	default:
		break;
	}
	return (0);
}

static iidesc_t *
find_iidesc(tdata_t *td, iidesc_match_t *match)
{
	match->iim_ret = NULL;
	iter_iidescs_by_name(td, match->iim_name->value,
	    (int (*)())matching_iidesc, match);
	return (match->iim_ret);
}

/*
 * If we have a weak symbol, attempt to find the strong symbol it will
 * resolve to.  Note: the code where this actually happens is in
 * sym_process() in cmd/sgs/libld/common/syms.c
 *
 * Finding the matching symbol is unfortunately not trivial.  For a
 * symbol to be a candidate, it must:
 *
 * - have the same type (function, object)
 * - have the same value (address)
 * - have the same size
 * - not be another weak symbol
 * - belong to the same section (checked via section index)
 *
 * If such a candidate is global, then we assume we've found it.  The
 * linker generates the symbol table such that the curfile might be
 * incorrect; this is OK for global symbols, since find_iidesc() doesn't
 * need to check for the source file for the symbol.
 *
 * We might have found a strong local symbol, where the curfile is
 * accurate and matches that of the weak symbol.  We assume this is a
 * reasonable match.
 *
 * If we've got a local symbol with a non-matching curfile, there are
 * two possibilities.  Either this is a completely different symbol, or
 * it's a once-global symbol that was scoped to local via a mapfile.  In
 * the latter case, curfile is likely inaccurate since the linker does
 * not preserve the needed curfile in the order of the symbol table (see
 * the comments about locally scoped symbols in libld's update_osym()).
 * As we can't tell this case from the former one, we use this symbol
 * iff no other matching symbol is found.
 *
 * What we really need here is a SUNW section containing weak<->strong
 * mappings that we can consume.
 */
static int
check_for_weak(GElf_Sym *weak, char const *weakfile,
    Elf_Data *data, int nent, Elf_Data *strdata,
    GElf_Sym *retsym, atom_t **atomp)
{
	char *curfile = NULL;
	char *tmpfile;
	GElf_Sym tmpsym;
	int candidate = 0;
	int i;

	if (GELF_ST_BIND(weak->st_info) != STB_WEAK)
		return (0);

	for (i = 0; i < nent; i++) {
		GElf_Sym sym;
		uchar_t type;

		if (gelf_getsym(data, i, &sym) == NULL)
			continue;

		type = GELF_ST_TYPE(sym.st_info);

		if (type == STT_FILE)
			curfile = (char *)strdata->d_buf + sym.st_name;

		if (GELF_ST_TYPE(weak->st_info) != type ||
		    weak->st_value != sym.st_value)
			continue;

		if (weak->st_size != sym.st_size)
			continue;

		if (GELF_ST_BIND(sym.st_info) == STB_WEAK)
			continue;

		if (sym.st_shndx != weak->st_shndx)
			continue;

		if (GELF_ST_BIND(sym.st_info) == STB_LOCAL &&
		    (curfile == NULL || weakfile == NULL ||
		    strcmp(curfile, weakfile) != 0)) {
			candidate = 1;
			tmpfile = curfile;
			tmpsym = sym;
			continue;
		}

		*atomp = atom_get(curfile);
		*retsym = sym;
		return (1);
	}

	if (candidate) {
		*atomp = atom_get(tmpfile);
		*retsym = tmpsym;
		return (1);
	}

	return (0);
}

/*
 * When we've found the underlying symbol's type description
 * for a weak symbol, we need to copy it and rename it to match
 * the weak symbol. We also need to add it to the td so it's
 * handled along with the others later.
 */
static iidesc_t *
copy_from_strong(tdata_t *td, GElf_Sym *sym, iidesc_t *strongdesc,
    const char *weakname, const char *weakfile)
{
	iidesc_t *new = iidesc_dup_rename(strongdesc, weakname, weakfile);
	uchar_t type = GELF_ST_TYPE(sym->st_info);

	switch (type) {
	case STT_OBJECT:
		new->ii_type = II_GVAR;
		break;
	case STT_FUNC:
		new->ii_type = II_GFUN;
		break;
	}

	hash_add(td->td_iihash, new);

	return (new);
}

/*
 * Process the symbol table of the output file, associating each symbol
 * with a type description if possible, and sorting them into functions
 * and data, maintaining symbol table order.
 */
static iiburst_t *
sort_iidescs(Elf *elf, const char *file, tdata_t *td, int fuzzymatch,
    int dynsym)
{
	iiburst_t *iiburst;
	Elf_Scn *scn;
	GElf_Shdr shdr;
	Elf_Data *data, *strdata;
	int i, stidx;
	int nent;
	iidesc_match_t match;

	match.iim_fuzzy = fuzzymatch;
	match.iim_file = ATOM_NULL;

	if ((stidx = findelfsecidx(elf, file,
	    dynsym ? ".dynsym" : ".symtab")) < 0)
#if !defined(__APPLE__)
		terminate("%s: Can't open symbol table\n", file);
#else
	return (iiburst_new(td, 0)); /* missing symbol table is most likely an empty binary,
                                      * produce an empty output */
#endif
	scn = elf_getscn(elf, stidx);
	data = elf_getdata(scn, NULL);
	gelf_getshdr(scn, &shdr);
	nent = shdr.sh_size / shdr.sh_entsize;

#if !defined(__APPLE__)
	scn = elf_getscn(elf, shdr.sh_link);
	strdata = elf_getdata(scn, NULL);
#else
	if (SHN_MACHO !=  shdr.sh_link && SHN_MACHO_64 !=  shdr.sh_link) { 
		scn = elf_getscn(elf, shdr.sh_link);
		strdata = elf_getdata(scn, NULL);
	} else {
		/* Underlying file is Mach-o */
		int dir_idx;

		if ((dir_idx = findelfsecidx(elf, file, ".dir_str_table")) < 0 || 
		    (scn = elf_getscn(elf, dir_idx)) == NULL ||
		    (strdata = elf_getdata(scn, NULL)) == NULL)
			terminate("%s: Can't open direct string table\n", file);
	}
#endif /* __APPLE__ */

	iiburst = iiburst_new(td, nent);

#if !defined(__APPLE__)
	for (i = 0; i < nent; i++) {
		GElf_Sym sym;
		iidesc_t **tolist;
		GElf_Sym ssym;
		iidesc_match_t smatch;
		int *curr;
		iidesc_t *iidesc;

		if (gelf_getsym(data, i, &sym) == NULL)
			elfterminate(file, "Couldn't read symbol %d", i);

		match.iim_name = atom_get(strdata->d_buf + sym.st_name);
		match.iim_bind = GELF_ST_BIND(sym.st_info);

		switch (GELF_ST_TYPE(sym.st_info)) {
		case STT_FILE:
			match.iim_file = match.iim_name;
			continue;
		case STT_OBJECT:
			tolist = iiburst->iib_objts;
			curr = &iiburst->iib_nobjts;
			break;
		case STT_FUNC:
			tolist = iiburst->iib_funcs;
			curr = &iiburst->iib_nfuncs;
			break;
		default:
			continue;
		}

		if (ignore_symbol(&sym, match.iim_name->value))
			continue;

		iidesc = find_iidesc(td, &match);

		if (iidesc != NULL) {
			tolist[*curr] = iidesc;
			iidesc->ii_flags |= IIDESC_F_USED;
			(*curr)++;
			continue;
		}

		if (!check_for_weak(&sym, match.iim_file, data, nent, strdata,
		    &ssym, &smatch.iim_file)) {
			(*curr)++;
			continue;
		}

		smatch.iim_fuzzy = fuzzymatch;
		smatch.iim_name = atom_get(strdata->d_buf + ssym.st_name);
		smatch.iim_bind = GELF_ST_BIND(ssym.st_info);

		debug(3, "Weak symbol %s resolved to %s\n", match.iim_name->value,
		    smatch.iim_name->value);

		iidesc = find_iidesc(td, &smatch);

		if (iidesc != NULL) {
			tolist[*curr] = copy_from_strong(td, &sym,
			    iidesc, match.iim_name->value, match.iim_file->value);
			tolist[*curr]->ii_flags |= IIDESC_F_USED;
		}

		(*curr)++;
	}
#else
	for (i = 0; i < nent; i++) {
		GElf_Sym sym;
		iidesc_t **tolist;
		int *curr;
		iidesc_t *iidesc;

		if (SHN_MACHO == shdr.sh_link) {
			if (gelf_getsym_macho(data, i, nent, &sym, (const char *)strdata->d_buf) == NULL)
				elfterminate(file, "Couldn't read symbol %d", i);
		} else if (SHN_MACHO_64 == shdr.sh_link) {
			if (gelf_getsym_macho_64(data, i, nent, &sym, (const char *)strdata->d_buf) == NULL)
				elfterminate(file, "Couldn't read symbol %d", i);
		}

		match.iim_name = atom_get(strdata->d_buf + sym.st_name);
		match.iim_bind = GELF_ST_BIND(sym.st_info);
		
		switch (GELF_ST_TYPE(sym.st_info)) {
		case STT_FILE:
			match.iim_file = match.iim_name;
			continue;
		case STT_OBJECT:
			tolist = iiburst->iib_objts;
			curr = &iiburst->iib_nobjts;
			break;
		case STT_FUNC:
			tolist = iiburst->iib_funcs;
			curr = &iiburst->iib_nfuncs;
			break;
		default:
			continue;
		}

		if (ignore_symbol(&sym, match.iim_name->value))
			continue;

		iidesc = find_iidesc(td, &match);

		if (iidesc != NULL) {
			tolist[*curr] = iidesc;
			iidesc->ii_flags |= IIDESC_F_USED;
			(*curr)++;
			continue;
		}

		if (ignore_symbol(&sym, match.iim_name->value))
			continue;

#warning FIXME: deal with weak bindings.

		(*curr)++;
	}	
#endif /* __APPLE__ */

	/*
	 * Stabs are generated for every function declared in a given C source
	 * file.  When converting an object file, we may encounter a stab that
	 * has no symbol table entry because the optimizer has decided to omit
	 * that item (for example, an unreferenced static function).  We may
	 * see iidescs that do not have an associated symtab entry, and so
	 * we do not write records for those functions into the CTF data.
	 * All others get marked as a root by this function.
	 */
	iiburst_types(iiburst);

	/*
	 * By not adding some of the functions and/or objects, we may have
	 * caused some types that were referenced solely by those
	 * functions/objects to be suppressed.  This could cause a label,
	 * generated prior to the evisceration, to be incorrect.  Find the
	 * highest type index, and change the label indicies to be no higher
	 * than this value.
	 */
	tdata_label_newmax(td, iiburst->iib_maxtypeid);

	return (iiburst);
}
#include "decl.h"
static void
write_file_64(Elf *src, const char *srcname, Elf *dst, const char *dstname,
    caddr_t ctfdata, size_t ctfsize, int flags); /* Forward reference. */

static void
fill_ctf_segments(struct segment_command *seg, struct section *sect, uint32_t vmaddr, size_t size, uint32_t offset)
{
	struct segment_command tmpseg = {
		LC_SEGMENT, 
		sizeof(struct segment_command) + sizeof(struct section),
		SEG_CTF,
		vmaddr,
		0, /* Do not map. Do not reserve virtual address range. */
		offset,
		size,
		VM_PROT_READ,
		VM_PROT_READ,
		1,
		0
	};

	struct section tmpsect = {
		SECT_CTF,
		SEG_CTF,
		vmaddr,
		size,
		offset,
		0, /* byte aligned */
		0,
		0,
		0,
		0, 
		0
	};
	
	*seg = tmpseg;
	*sect = tmpsect;
}

/*
 * In CTF_MINIMIZE mode, the output file is stripped of the file data for
 * all sections other than the symbol table and CTF data. The section headers
 * remain with the original vmaddr, vmsize, but with filesize and offset zero.
 * The type is changed to MH_DSYM to reflect this; this is required by other
 * mach-o parsing code.
 */

void
write_file(Elf *src, const char *srcname, Elf *dst, const char *dstname,
    caddr_t ctfdata, size_t ctfsize, int flags)
{
	size_t imagesz = 0;
	struct mach_header *mh = (struct mach_header *)elf_getimage(src, &imagesz);
	struct mach_header newhdr;
	struct segment_command ctfseg_command;
	struct section ctf_sect; 
	struct segment_command *curcmd, *ctfcmd;
	struct symtab_command *symcmd = NULL;
	int    minimize = (0 != (CTF_MINIMIZE & flags));
	uint32_t origcmdcount;
	uint64_t origcmdsize;
	uint64_t ctfoffset;
	uint32_t idx;
	int size;
	uint32_t thecmd;
	int fd;
	size_t sz;
	char *p;
	uint64_t ctf_vmaddr = 0, t;

	/* Get a pristine instance of the source mach-o */
	if ((fd = open(srcname, O_RDONLY)) < 0)
		terminate("%s: Cannot open for re-reading", srcname);
		
	sz = (size_t)lseek(fd, (off_t)0, SEEK_END);
	
	p = mmap((char *)0, sz, PROT_READ, MAP_PRIVATE, fd, (off_t)0);
	if ((char *)-1 == p)
		terminate("%s: Cannot mmap for re-reading", srcname);
		
	if (MH_MAGIC != ((struct mach_header *)p)->magic)
		terminate("%s: is not a thin (single architecture) mach-o binary.\n", srcname);

	/* Iterate through load commands looking for CTF data */
	ctfcmd = NULL;
	origcmdcount = 0;
	origcmdsize  = 0;

	for (idx = 0, curcmd = (struct segment_command *) (p + sizeof(struct mach_header));
		 idx < mh->ncmds;
		 idx++, curcmd = (struct segment_command *) (((char *)curcmd) + size)) {
		size = curcmd->cmdsize;
		thecmd = curcmd->cmd;
		int copy = 0;

		if (LC_SEGMENT == thecmd) {
			uint64_t vmaddr = curcmd->vmaddr;
			uint64_t vmsize = curcmd->vmsize;
			t = vmaddr + vmsize;
			if (t > ctf_vmaddr)
				ctf_vmaddr = t;
			if (!strcmp(curcmd->segname, SEG_CTF)) {
				ctfcmd = curcmd;
			}
			copy = 1;
		} else if (LC_SYMTAB == thecmd) {
			copy = 1;
			symcmd = (struct symtab_command *) curcmd;
		} else if (LC_UUID == thecmd) {
			copy = 1;
		}

		if (copy || !minimize) {
			origcmdcount++;
			origcmdsize += size;
		}
	}

	ctf_vmaddr = (ctf_vmaddr + getpagesize() - 1) & (~(getpagesize() - 1)); // page aligned

	if (ctfcmd) {
		/* CTF segment command exists: overwrite it */
		fill_ctf_segments(&ctfseg_command, &ctf_sect, 
			((struct segment_command *)curcmd)->vmaddr, ctfsize, sz /* file offset */);

		write(dst->ed_fd, p, sz); // byte-for-byte copy of input mach-o file
		write(dst->ed_fd, ctfdata, ctfsize); // append CTF 
		
		lseek(dst->ed_fd, (off_t)((char *)ctfcmd - p), SEEK_SET);
		write(dst->ed_fd, &ctfseg_command, sizeof(ctfseg_command)); // lay down CTF_SEG
		write(dst->ed_fd, &ctf_sect, sizeof(ctf_sect)); // lay down CTF_SECT
	} else { 
		struct symtab_command tmpsymcmd;
		int cmdlength, dataoffset, datalength;
		int ctfhdrsz = sizeof(ctfseg_command) + sizeof(ctf_sect);

		dataoffset = sizeof(*mh) + mh->sizeofcmds; // where all real data starts
		datalength = imagesz - dataoffset;

		cmdlength = origcmdsize;
		cmdlength += ctfhdrsz;

		newhdr = *mh;
		newhdr.sizeofcmds = cmdlength;
		/* Add one segment command to header */
		newhdr.ncmds = origcmdcount + 1;

		if (symcmd) {
			tmpsymcmd = *symcmd;
		} else {
			bzero(&tmpsymcmd, sizeof(tmpsymcmd));
		}
		if (minimize) {
			newhdr.filetype  = MH_DSYM;
			tmpsymcmd.symoff = sizeof(*mh) + cmdlength;
			tmpsymcmd.stroff = tmpsymcmd.symoff + tmpsymcmd.nsyms * sizeof(struct nlist);
			ctfoffset        = tmpsymcmd.stroff + tmpsymcmd.strsize;
		} else {
			tmpsymcmd.symoff += ctfhdrsz;
			tmpsymcmd.stroff += ctfhdrsz;
			ctfoffset         = ctfhdrsz + dataoffset + datalength;
		}

		fill_ctf_segments(&ctfseg_command, &ctf_sect, ctf_vmaddr, ctfsize, ctfoffset);

		write(dst->ed_fd, &newhdr, sizeof(newhdr));

		for (idx = 0, curcmd = (struct segment_command *) (p + sizeof(struct mach_header));
			 idx < mh->ncmds;
			 idx++, curcmd = (struct segment_command *) (((char *)curcmd) + size)) {
			size = curcmd->cmdsize;
			thecmd = curcmd->cmd;

			if (LC_SEGMENT == thecmd) {
				struct segment_command tmpcmd;
				tmpcmd = *curcmd;
				if (minimize) {
					tmpcmd.filesize = 0;
					tmpcmd.fileoff  = 0;
				} else {
					tmpcmd.filesize += ctfhdrsz;
					tmpcmd.fileoff  += ctfhdrsz;
				}
				write(dst->ed_fd, &tmpcmd, sizeof(tmpcmd));

				struct section * sects = (struct section *)(curcmd + 1);
				for (int i = 0; i < curcmd->nsects; i++) {
					struct section tmpsect;
					tmpsect = sects[i];
					if (minimize) {
						tmpsect.offset = 0;
						tmpsect.reloff = 0;
						tmpsect.nreloc = 0;
					} else {
						tmpsect.offset += ctfhdrsz;
						if (tmpsect.reloff) {
							tmpsect.reloff += ctfhdrsz;
						}
					}
					write(dst->ed_fd, &tmpsect, sizeof(tmpsect));
				}
			} else if (LC_SYMTAB == thecmd) {
				write(dst->ed_fd, &tmpsymcmd, sizeof(tmpsymcmd));
			} else if (!minimize || (LC_UUID == thecmd)) {
				write(dst->ed_fd, curcmd, size);
			}
		}
		write(dst->ed_fd, &ctfseg_command, sizeof(ctfseg_command));
		write(dst->ed_fd, &ctf_sect, sizeof(ctf_sect));
		if (minimize) {
			write(dst->ed_fd, p + symcmd->symoff, symcmd->nsyms * sizeof(struct nlist));
			write(dst->ed_fd, p + symcmd->stroff, symcmd->strsize);
		} else {
			write(dst->ed_fd, p + dataoffset, datalength);
		}
		write(dst->ed_fd, ctfdata, ctfsize);
	}

	(void)munmap(p, sz);
	(void)close(fd);

	return;
}

static void
fill_ctf_segments_64(struct segment_command_64 *seg, struct section_64 *sect, uint64_t vmaddr, size_t size, uint32_t offset)
{
	struct segment_command_64 tmpseg = {
		LC_SEGMENT_64,
		sizeof(struct segment_command_64) + sizeof(struct section_64),
		SEG_CTF,
		vmaddr,
		0, /* Do not map. Do not reserve virtual address range. */
		offset,
		size,
		VM_PROT_READ,
		VM_PROT_READ,
		1,
		0
	};

	struct section_64 tmpsect = {
		SECT_CTF,
		SEG_CTF,
		vmaddr,
		size,
		offset,
		0, /* byte aligned */
		0,
		0,
		0,
		0, 
		0
	};
	
	*seg = tmpseg;
	*sect = tmpsect;
}

static void
write_file_64(Elf *src, const char *srcname, Elf *dst, const char *dstname,
    caddr_t ctfdata, size_t ctfsize, int flags)
{
	size_t imagesz;
	struct mach_header_64 *mh = (struct mach_header_64 *)elf_getimage(src, &imagesz);
	struct mach_header_64 newhdr;
	struct segment_command_64 ctfseg_command;
	struct section_64 ctf_sect; 
	struct segment_command_64 *curcmd, *ctfcmd;
	struct symtab_command *symcmd = NULL;
	int    minimize = (0 != (CTF_MINIMIZE & flags));
	uint32_t origcmdcount;
	uint64_t origcmdsize;
	uint64_t ctfoffset;
	uint32_t idx;
	int size;
	uint32_t thecmd;
	int fd;
	size_t sz;
	char *p;
	uint64_t ctf_vmaddr = 0, t;

	/* Get a pristine instance of the source mach-o */
	if ((fd = open(srcname, O_RDONLY)) < 0)
		terminate("%s: Cannot open for re-reading", srcname);
		
	sz = (size_t)lseek(fd, (off_t)0, SEEK_END);
	
	p = mmap((char *)0, sz, PROT_READ, MAP_PRIVATE, fd, (off_t)0);
	if ((char *)-1 == p)
		terminate("%s: Cannot mmap for re-reading", srcname);
		
	if (MH_MAGIC_64 != ((struct mach_header *)p)->magic)
		terminate("%s: is not a thin (single architecture) mach-o binary.\n", srcname);

	/* Iterate through load commands looking for CTF data */
	ctfcmd = NULL;
	origcmdcount = 0;
	origcmdsize  = 0;

	for (idx = 0, curcmd = (struct segment_command_64 *) (p + sizeof(struct mach_header_64));
		 idx < mh->ncmds;
		 idx++, curcmd = (struct segment_command_64 *) (((char *)curcmd) + size)) {
		size = curcmd->cmdsize;
		thecmd = curcmd->cmd;
		int copy = 0;

		if (LC_SEGMENT_64 == thecmd) {
			uint64_t vmaddr = curcmd->vmaddr;
			uint64_t vmsize = curcmd->vmsize;
			t = vmaddr + vmsize;
			if (t > ctf_vmaddr)
				ctf_vmaddr = t;
			if (!strcmp(curcmd->segname, SEG_CTF)) {
				ctfcmd = curcmd;
			}
			copy = 1;
		} else if (LC_SYMTAB == thecmd) {
			copy = 1;
			symcmd = (struct symtab_command *) curcmd;
		} else if (LC_UUID == thecmd) {
			copy = 1;
		}

		if (copy || !minimize) {
			origcmdcount++;
			origcmdsize += size;
		}
	}

	ctf_vmaddr = (ctf_vmaddr + getpagesize() - 1) & (~(getpagesize() - 1)); // page aligned

	if (ctfcmd) {
		/* CTF segment command exists: overwrite it */
		fill_ctf_segments_64(&ctfseg_command, &ctf_sect, 
			((struct segment_command_64 *)curcmd)->vmaddr, ctfsize, sz /* file offset */);

		write(dst->ed_fd, p, sz); // byte-for-byte copy of input mach-o file
		write(dst->ed_fd, ctfdata, ctfsize); // append CTF 
		
		lseek(dst->ed_fd, (off_t)((char *)ctfcmd - p), SEEK_SET);
		write(dst->ed_fd, &ctfseg_command, sizeof(ctfseg_command)); // lay down CTF_SEG
		write(dst->ed_fd, &ctf_sect, sizeof(ctf_sect)); // lay down CTF_SECT
	} else { 
		struct symtab_command tmpsymcmd;
		int cmdlength, dataoffset, datalength;
		int ctfhdrsz = sizeof(ctfseg_command) + sizeof(ctf_sect);

		dataoffset = sizeof(*mh) + mh->sizeofcmds; // where all real data starts
		datalength = imagesz - dataoffset;

		cmdlength = origcmdsize;
		cmdlength += ctfhdrsz;

		newhdr = *mh;
		newhdr.sizeofcmds = cmdlength;
		/* Add one segment command to header */
		newhdr.ncmds = origcmdcount + 1;

		if (symcmd) {
			tmpsymcmd = *symcmd;
		} else {
			bzero(&tmpsymcmd, sizeof(tmpsymcmd));
		}
		if (minimize) {
			newhdr.filetype  = MH_DSYM;
			tmpsymcmd.symoff = sizeof(*mh) + cmdlength;
			tmpsymcmd.stroff = tmpsymcmd.symoff + tmpsymcmd.nsyms * sizeof(struct nlist_64);
			ctfoffset        = tmpsymcmd.stroff + tmpsymcmd.strsize;
		} else {
			tmpsymcmd.symoff += ctfhdrsz;
			tmpsymcmd.stroff += ctfhdrsz;
			ctfoffset         = ctfhdrsz + dataoffset + datalength;
		}

		fill_ctf_segments_64(&ctfseg_command, &ctf_sect, ctf_vmaddr, ctfsize, ctfoffset);

		write(dst->ed_fd, &newhdr, sizeof(newhdr));

		for (idx = 0, curcmd = (struct segment_command_64 *) (p + sizeof(struct mach_header_64));
			 idx < mh->ncmds;
			 idx++, curcmd = (struct segment_command_64 *) (((char *)curcmd) + size)) {
			size = curcmd->cmdsize;
			thecmd = curcmd->cmd;

			if (LC_SEGMENT_64 == thecmd) {
				struct segment_command_64 tmpcmd;
				tmpcmd = *curcmd;
				if (minimize) {
					tmpcmd.filesize = 0;
					tmpcmd.fileoff  = 0;
				} else {
					tmpcmd.filesize += ctfhdrsz;
					tmpcmd.fileoff  += ctfhdrsz;
				}
				write(dst->ed_fd, &tmpcmd, sizeof(tmpcmd));

				struct section_64 * sects = (struct section_64 *)(curcmd + 1);
				for (int i = 0; i < curcmd->nsects; i++) {
					struct section_64 tmpsect;
					tmpsect = sects[i];
					if (minimize) {
						tmpsect.offset = 0;
						tmpsect.reloff = 0;
						tmpsect.nreloc = 0;
					} else {
						tmpsect.offset += ctfhdrsz;
						if (tmpsect.reloff) {
							tmpsect.reloff += ctfhdrsz;
						}
					}
					write(dst->ed_fd, &tmpsect, sizeof(tmpsect));
				}
			} else if (LC_SYMTAB == thecmd) {
				write(dst->ed_fd, &tmpsymcmd, sizeof(tmpsymcmd));
			} else if (!minimize || (LC_UUID == thecmd)) {
				write(dst->ed_fd, curcmd, size);
			}
		}
		write(dst->ed_fd, &ctfseg_command, sizeof(ctfseg_command));
		write(dst->ed_fd, &ctf_sect, sizeof(ctf_sect));
		if (minimize) {
			write(dst->ed_fd, p + symcmd->symoff, symcmd->nsyms * sizeof(struct nlist_64));
			write(dst->ed_fd, p + symcmd->stroff, symcmd->strsize);
		} else {
			write(dst->ed_fd, p + dataoffset, datalength);
		}
		write(dst->ed_fd, ctfdata, ctfsize);
	}

	(void)munmap(p, sz);
	(void)close(fd);

	return;
}

static caddr_t
make_ctf_data(tdata_t *td, Elf *elf, const char *file, size_t *lenp, int flags)
{
	iiburst_t *iiburst;
	caddr_t data;

	iiburst = sort_iidescs(elf, file, td, flags & CTF_FUZZY_MATCH,
	    flags & CTF_USE_DYNSYM);
	data = ctf_gen(iiburst, lenp, flags & CTF_COMPRESS);

	iiburst_free(iiburst);

	return (data);
}

void
write_ctf(tdata_t *td, const char *curname, const char *newname, int flags)
{
	struct stat st;
	Elf *elf = NULL;
	Elf *telf = NULL;
	caddr_t data;
	size_t len;
	int fd = -1;
	int tfd = -1;

	(void) elf_version(EV_CURRENT);
	if ((fd = open(curname, O_RDONLY)) < 0 || fstat(fd, &st) < 0)
		terminate("%s: Cannot open for re-reading", curname);
	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		elfterminate(curname, "Cannot re-read");

	if ((tfd = open(newname, O_RDWR | O_CREAT | O_TRUNC, st.st_mode)) < 0)
		terminate("Cannot open temp file %s for writing", newname);
	if ((telf = elf_begin(tfd, ELF_C_WRITE, NULL)) == NULL)
		elfterminate(curname, "Cannot write");

#if defined(__APPLE__)
	if (ELFCLASS32 == elf->ed_class) {
		data = make_ctf_data(td, elf, curname, &len, flags);
		if (flags & CTF_RAW_OUTPUT) {
			if (write(tfd, data, len) != len) {
				perror("Attempt to write raw CTF data failed");
				terminate("Attempt to write raw CTF data failed");
			}
		} else {
			write_file(elf, curname, telf, newname, data, len, flags);
		}
	} else if (ELFCLASS64 == elf->ed_class) {
		data = make_ctf_data(td, elf, curname, &len, flags);
		if (flags & CTF_RAW_OUTPUT) {
			if (write(tfd, data, len) != len) {
				perror("Attempt to write raw CTF data failed");
				terminate("Attempt to write raw CTF data failed");
			}
		} else {		
			write_file_64(elf, curname, telf, newname, data, len, flags);
		}
	} else
		terminate("%s: Unknown ed_class", curname);
#else
	data = make_ctf_data(td, elf, curname, &len, flags);
	write_file(elf, curname, telf, newname, data, len, flags);
#endif /* __APPLE__ */
	free(data);

	elf_end(telf);
	elf_end(elf);
	(void) close(fd);
	(void) close(tfd);
}
