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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * DWARF to tdata conversion
 *
 * For the most part, conversion is straightforward, proceeding in two passes.
 * On the first pass, we iterate through every die, creating new type nodes as
 * necessary.  Referenced tdesc_t's are created in an uninitialized state, thus
 * allowing type reference pointers to be filled in.  If the tdesc_t
 * corresponding to a given die can be completely filled out (sizes and offsets
 * calculated, and so forth) without using any referenced types, the tdesc_t is
 * marked as resolved.  Consider an array type.  If the type corresponding to
 * the array contents has not yet been processed, we will create a blank tdesc
 * for the contents type (only the type ID will be filled in, relying upon the
 * later portion of the first pass to encounter and complete the referenced
 * type).  We will then attempt to determine the size of the array.  If the
 * array has a byte size attribute, we will have completely characterized the
 * array type, and will be able to mark it as resolved.  The lack of a byte
 * size attribute, on the other hand, will prevent us from fully resolving the
 * type, as the size will only be calculable with reference to the contents
 * type, which has not, as yet, been encountered.  The array type will thus be
 * left without the resolved flag, and the first pass will continue.
 *
 * When we begin the second pass, we will have created tdesc_t nodes for every
 * type in the section.  We will traverse the tree, from the iidescs down,
 * processing each unresolved node.  As the referenced nodes will have been
 * populated, the array type used in our example above will be able to use the
 * size of the referenced types (if available) to determine its own type.  The
 * traversal will be repeated until all types have been resolved or we have
 * failed to make progress.  When all tdescs have been resolved, the conversion
 * is complete.
 *
 * There are, as always, a few special cases that are handled during the first
 * and second passes:
 *
 *  1. Empty enums - GCC will occasionally emit an enum without any members.
 *     Later on in the file, it will emit the same enum type, though this time
 *     with the full complement of members.  All references to the memberless
 *     enum need to be redirected to the full definition.  During the first
 *     pass, each enum is entered in dm_enumhash, along with a pointer to its
 *     corresponding tdesc_t.  If, during the second pass, we encounter a
 *     memberless enum, we use the hash to locate the full definition.  All
 *     tdescs referencing the empty enum are then redirected.
 *
 *  2. Forward declarations - If the compiler sees a forward declaration for
 *     a structure, followed by the definition of that structure, it will emit
 *     DWARF data for both the forward declaration and the definition.  We need
 *     to resolve the forward declarations when possible, by redirecting
 *     forward-referencing tdescs to the actual struct/union definitions.  This
 *     redirection is done completely within the first pass.  We begin by
 *     recording all forward declarations in dw_fwdhash.  When we define a
 *     structure, we check to see if there have been any corresponding forward
 *     declarations.  If so, we redirect the tdescs which referenced the forward
 *     declarations to the structure or union definition.
 *
 * XXX see if a post traverser will allow the elimination of repeated pass 2
 * traversals.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <libelf.h>
#include <libdwarf.h>
#include <libgen.h>
#include <dwarf.h>

#include "ctf_headers.h"
#include "ctftools.h"
#include "memory.h"
#include "list.h"
#include "alist.h"
#include "traverse.h"

#ifdef __linux__
#include <string.h>
#endif

#if defined(__APPLE__) || defined(__linux__)
/* Sun extensions not present in Darwin's dwarf.h */
#define DW_ATE_SUN_interval_float       0x91
#define DW_ATE_SUN_imaginary_float      0x92 /* Obsolete: See DW_ATE_imaginary_float */

/* it is EXTREMELY slow, and doesn't affect memory that much */
#define dwarf_dealloc(...)
#endif /* __APPLE__ */

/* The version of DWARF which we support. */
#define	DWARF_VERSION	2
#define	DWARF_VERSION3	3
#define	DWARF_VERSION4	4

/*
 * We need to define a couple of our own intrinsics, to smooth out some of the
 * differences between the GCC and DevPro DWARF emitters.  See the referenced
 * routines and the special cases in the file comment for more details.
 *
 * Type IDs are 32 bits wide.  We're going to use the top of that field to
 * indicate types that we've created ourselves.
 */
#define	TID_FILEMAX		0x3fffffff	/* highest tid from file */
#define	TID_VOID		0x40000001	/* see die_void() */
#define	TID_LONG		0x40000002	/* see die_array() */

#define	TID_MFGTID_BASE		0x40000003	/* first mfg'd tid */

/*
 * To reduce the staggering amount of error-handling code that would otherwise
 * be required, the attribute-retrieval routines handle most of their own
 * errors.  If the following flag is supplied as the value of the `req'
 * argument, they will also handle the absence of a requested attribute by
 * terminating the program.
 */
#define	DW_ATTR_REQ	1

#define	TDESC_HASH_BUCKETS	511

typedef struct dwarf {
	Dwarf_Debug dw_dw;		/* for libdwarf */
	Dwarf_Error dw_err;		/* for libdwarf */
	Dwarf_Unsigned dw_maxoff;	/* highest legal offset in this cu */
	Dwarf_Off dw_cuoff;		/* lowest offset in this cu */
	tdata_t *dw_td;			/* root of the tdesc/iidesc tree */
	alist_t *dw_tidhash;		/* hash of tdescs by t_id */
	hash_t *dw_fwdhash;		/* hash of fwd decls by name */
	hash_t *dw_enumhash;		/* hash of memberless enums by name */
	tdesc_t *dw_void;		/* manufactured void type */
	tdesc_t *dw_long;		/* manufactured long type for arrays */
	size_t dw_ptrsz;		/* size of a pointer in this file */
	tid_t dw_mfgtid_last;		/* last mfg'd type ID used */
	uint_t dw_nunres;		/* count of unresolved types */
	Dwarf_Die dw_cu;		/* die of cu */
	atom_t *dw_cuname;		/* name of compilation unit */
} dwarf_t;

typedef struct cu_data
{
	struct cu_data *cud_next;
	atom_t *cud_name;
	Dwarf_Unsigned cud_nxthdr;
} cu_data_t;


static tdesc_t * die_create_one(dwarf_t *, Dwarf_Die);
static void die_create(dwarf_t *, Dwarf_Die);

static tid_t
mfgtid_next(dwarf_t *dw)
{
	return (++dw->dw_mfgtid_last);
}

static void
tdesc_add(dwarf_t *dw, tdesc_t *tdp)
{
	alist_add(dw->dw_tidhash, (void *)(uintptr_t)tdp->t_id, tdp);
}

static tdesc_t *
tdesc_lookup(dwarf_t *dw, int tid)
{
	void *tpd = NULL;
	(void)alist_find(dw->dw_tidhash, (void *)(uintptr_t)tid, &tpd);
	return tpd;
}

/*
 * Resolve a tdesc down to a node which should have a size.  Returns the size,
 * SIZE_MAX if the size hasn't yet been determined.
 */
static size_t
tdesc_size(tdesc_t *tdp)
{
	for (;;) {
		switch (tdp->t_type) {
		case INTRINSIC:
		case POINTER:
		case ARRAY:
		case FUNCTION:
		case STRUCT:
		case UNION:
		case ENUM:
		case PTRAUTH:
			return (tdp->t_size);

		case FORWARD:
			return (SIZE_MAX);

		case TYPEDEF:
		case VOLATILE:
		case CONST:
		case RESTRICT:
			tdp = tdp->t_tdesc;
			continue;

		case 0: /* not yet defined */
			return (SIZE_MAX);

		default:
			terminate("tdp %u: tdesc_size on unknown type %d\n",
			    tdp->t_id, tdp->t_type);
		}
	}
}

static size_t
tdesc_bitsize(tdesc_t *tdp)
{
	for (;;) {
		switch (tdp->t_type) {
		case INTRINSIC:
			return (tdp->t_intr->intr_nbits);

		case ARRAY:
		case FUNCTION:
		case STRUCT:
		case UNION:
		case ENUM:
		case POINTER:
		case PTRAUTH:
			return (tdp->t_size * NBBY);

		case FORWARD:
			return (0);

		case TYPEDEF:
		case VOLATILE:
		case RESTRICT:
		case CONST:
			tdp = tdp->t_tdesc;
			continue;

		case 0: /* not yet defined */
			return (0);

		default:
			terminate("tdp %u: tdesc_bitsize on unknown type %d\n",
			    tdp->t_id, tdp->t_type);
		}
	}
}

static tdesc_t *
tdesc_basetype(tdesc_t *tdp)
{
	for (;;) {
		switch (tdp->t_type) {
		case TYPEDEF:
		case VOLATILE:
		case RESTRICT:
		case CONST:
			tdp = tdp->t_tdesc;
			break;
		case 0: /* not yet defined */
			return (NULL);
		default:
			return (tdp);
		}
	}
}

static Dwarf_Off
die_off(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Off off;

	if (dwarf_dieoffset(die, &off, &dw->dw_err) == DW_DLV_OK)
		return (off);

	terminate("failed to get offset for die: %s\n",
	    dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (0);
}

static Dwarf_Die
die_sibling(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Die sib;
	int rc;

	if ((rc = dwarf_siblingof(dw->dw_dw, die, &sib, &dw->dw_err)) ==
	    DW_DLV_OK)
		return (sib);
	else if (rc == DW_DLV_NO_ENTRY)
		return (NULL);

	terminate("die %llu: failed to find type sibling: %s\n",
	    die_off(dw, die), dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (NULL);
}

static Dwarf_Die
die_child(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Die child;
	int rc;

	if ((rc = dwarf_child(die, &child, &dw->dw_err)) == DW_DLV_OK)
		return (child);
	else if (rc == DW_DLV_NO_ENTRY)
		return (NULL);

	terminate("die %llu: failed to find type child: %s\n",
	    die_off(dw, die), dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (NULL);
}

static Dwarf_Half
die_tag(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Half tag;

	if (dwarf_tag(die, &tag, &dw->dw_err) == DW_DLV_OK)
		return (tag);

	terminate("die %llu: failed to get tag for type: %s\n",
	    die_off(dw, die), dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (0);
}

static Dwarf_Attribute
die_attr(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, int req)
{
	Dwarf_Attribute attr;
	int rc;

	if ((rc = dwarf_attr(die, name, &attr, &dw->dw_err)) == DW_DLV_OK) {
		return (attr);
	} else if (rc == DW_DLV_NO_ENTRY) {
		if (req) {
			terminate("die %llu: no attr 0x%x\n", die_off(dw, die),
			    name);
		} else {
			return (NULL);
		}
	}

	terminate("die %llu: failed to get attribute for type: %s\n",
	    die_off(dw, die), dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (NULL);
}

static Dwarf_Half
die_attr_form(dwarf_t *dw, Dwarf_Attribute attr)
{
	Dwarf_Half form;

	if (dwarf_whatform(attr, &form, &dw->dw_err) == DW_DLV_OK)
		return (form);

	terminate("failed to get attribute form for type: %s\n",
	    dwarf_errmsg(dw->dw_err));
	/*NOTREACHED*/
	return (0);
}

static int
die_signed(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, Dwarf_Signed *valp,
    int req)
{
	Dwarf_Attribute attr;
	Dwarf_Signed val;

	if ((attr = die_attr(dw, die, name, req)) == NULL)
		return (0); /* die_attr will terminate for us if necessary */

	if (!dwarf_formisdata(attr)) {
		if (req) {
			terminate("die %llu: is not data (form 0x%x)\n",
			    die_off(dw, die), die_attr_form(dw, attr));
		}
		else {
			dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);
			return (0);
		}
	}

	if (dwarf_formsdata(attr, &val, &dw->dw_err) != DW_DLV_OK) {
		terminate("die %llu: failed to get signed (form 0x%x)\n",
		    die_off(dw, die), die_attr_form(dw, attr));
	}

	dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

	*valp = val;
	return (1);
}

static int
die_unsigned(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, Dwarf_Unsigned *valp,
    int req)
{
	Dwarf_Attribute attr;
	Dwarf_Unsigned val;

	if ((attr = die_attr(dw, die, name, req)) == NULL)
		return (0); /* die_attr will terminate for us if necessary */

	if (!dwarf_formisdata(attr)) {
		if (req) {
			terminate("die %llu: is not data (form 0x%x)\n",
			    die_off(dw, die), die_attr_form(dw, attr));
		}
		else {
			dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);
			return (0);
		}
	}

	if (dwarf_formudata(attr, &val, &dw->dw_err) != DW_DLV_OK) {
		terminate("die %llu: failed to get unsigned (form 0x%x)\n",
		    die_off(dw, die), die_attr_form(dw, attr));
	}

	dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

	*valp = val;
	return (1);
}

static int
die_bool(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, Dwarf_Bool *valp, int req)
{
	Dwarf_Attribute attr;
	Dwarf_Bool val;

	if ((attr = die_attr(dw, die, name, req)) == NULL)
		return (0); /* die_attr will terminate for us if necessary */

	if (dwarf_formflag(attr, &val, &dw->dw_err) != DW_DLV_OK) {
		terminate("die %llu: failed to get bool (form 0x%x)\n",
			die_off(dw, die), die_attr_form(dw, attr));
	}

	dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

	*valp = val;
	return (1);
}

static int
die_atom(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, atom_t **atomp, int req)
{
	Dwarf_Attribute attr;
	char *str;

	if ((attr = die_attr(dw, die, name, req)) == NULL) {
		*atomp = ATOM_NULL;
		return (0); /* die_attr will terminate for us if necessary */
	}

	if (dwarf_formstring(attr, &str, &dw->dw_err) != DW_DLV_OK) {
		terminate("die %llu: failed to get string (form 0x%x)\n",
				  die_off(dw, die), die_attr_form(dw, attr));
	}

	*atomp = atom_get(str);
	dwarf_dealloc(dw->dw_dw, str, DW_DLA_STRING);

	return (1);
}

static int
die_string(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name, char **strp, int req)
{
	atom_t *atom;
	int rc;

	rc = die_atom(dw, die, name, &atom, req);
	*strp = atom ? xstrdup(atom->value) : NULL;
	return rc;
}

static Dwarf_Off
die_attr_ref(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name)
{
	Dwarf_Attribute attr;
	Dwarf_Off off;

	attr = die_attr(dw, die, name, DW_ATTR_REQ);

	if (dwarf_global_formref(attr, &off, &dw->dw_err) != DW_DLV_OK) {
		terminate("die %llu: failed to get ref (form 0x%x)\n",
			die_off(dw, die), die_attr_form(dw, attr));
	}

	dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

	return (off);
}

static atom_t *
die_name(dwarf_t *dw, Dwarf_Die die)
{
	atom_t *atom = ATOM_NULL;

	(void) die_atom(dw, die, DW_AT_name, &atom, 0);

	return (atom);
}

static int
die_isdecl(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Bool val;

	return (die_bool(dw, die, DW_AT_declaration, &val, 0) && val);
}

#if defined(__APPLE__)
static atom_t *
die_linkage_name(dwarf_t *dw, Dwarf_Die die)
{
	atom_t *atom = ATOM_NULL;

	(void) die_atom(dw, die, DW_AT_MIPS_linkage_name, &atom, 0);

	return (atom);
}

static Dwarf_Die
die_specification_die(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Attribute attr;
	Dwarf_Off ref;
	Dwarf_Die new_die;
	int rc;
	
	if ((attr = die_attr(dw, die, DW_AT_specification, 0)) == NULL)
		return (NULL);
		
	ref = die_attr_ref(dw, die, DW_AT_specification);
	
	if ((rc = dwarf_offdie(dw->dw_dw, ref, &new_die, &dw->dw_err)) != DW_DLV_OK) {
		terminate("die %llu: failed to get DW_AT_specificaiton ref\n", die);
	}
	
	return (new_die);
}

static int
die_isvirtual(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Signed val;

	return (die_signed(dw, die, DW_AT_virtuality, &val, 0) && val);
}
#endif /* __APPLE__ */

static int
die_isglobal(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Signed vis;
	Dwarf_Bool ext;

	/*
	 * Some compilers (gcc) use DW_AT_external to indicate function
	 * visibility.  Others (Sun) use DW_AT_visibility.
	 */
	if (die_signed(dw, die, DW_AT_visibility, &vis, 0))
		return (vis == DW_VIS_exported);
	else
		return (die_bool(dw, die, DW_AT_external, &ext, 0) && ext);
}

static tdesc_t *
die_add(dwarf_t *dw, Dwarf_Off off)
{
	tdesc_t *tdp = xcalloc(sizeof (tdesc_t));

	tdp->t_id = off;

	tdesc_add(dw, tdp);

	return (tdp);
}

static tdesc_t *
die_lookup_pass1(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name)
{
	Dwarf_Off ref = die_attr_ref(dw, die, name);
	tdesc_t *tdp;

	if ((tdp = tdesc_lookup(dw, ref)) != NULL)
		return (tdp);

	if ((ref < dw->dw_cuoff) || (ref > dw->dw_maxoff))
	{
	    Dwarf_Die ref_die;
		int rc;
		if ((rc = dwarf_offdie(dw->dw_dw, ref, &ref_die, &dw->dw_err)) != DW_DLV_OK) {
			terminate("die %llu: failed to find oob ref\n", die);
		}
		tdp = die_create_one(dw, ref_die);
	    return (tdp);
	}

	return (die_add(dw, ref));
}

static int
die_mem_offset(dwarf_t *dw, Dwarf_Die die, Dwarf_Half name,
    Dwarf_Unsigned *valp, int req)
{
	Dwarf_Attribute attr;
	Dwarf_Locdesc *loc;
	Dwarf_Signed locnum;
	Dwarf_Half form;

	if ((attr = die_attr(dw, die, name, req)) == NULL)
		return (0); /* die_attr will terminate for us if necessary */

	if (dwarf_whatform(attr, &form, &dw->dw_err) != DW_DLV_OK)
		return (0);

	switch (form) {
	case DW_FORM_sec_offset:
	case DW_FORM_block:
	case DW_FORM_block1:
	case DW_FORM_block2:
	case DW_FORM_block4:
		if (dwarf_loclist(attr, &loc, &locnum, &dw->dw_err) != DW_DLV_OK) {
			terminate("die %llu: failed to get mem offset location list\n",
			    die_off(dw, die));
		}

		dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

		if (locnum != 1 || loc->ld_s->lr_atom != DW_OP_plus_uconst) {
			terminate("die %llu: cannot parse member offset\n",
			    die_off(dw, die));
		}

		*valp = loc->ld_s->lr_number;

		dwarf_dealloc(dw->dw_dw, loc->ld_s, DW_DLA_LOC_BLOCK);
		dwarf_dealloc(dw->dw_dw, loc, DW_DLA_LOCDESC);
		break;
	case DW_FORM_data1:
	case DW_FORM_data2:
	case DW_FORM_data4:
	case DW_FORM_data8:
	case DW_FORM_udata:
		if (!die_unsigned(dw, die, name, valp, req))
			return 0;
		break;
	default:
		terminate("die %llu: cannot parse member offset with form "
			"%u\n", die_off(dw, die), form);
	}


	return (1);
}

static tdesc_t *
tdesc_intr_common(dwarf_t *dw, int tid, const char *name, size_t sz)
{
	tdesc_t *tdp;
	intr_t *intr;

	intr = xcalloc(sizeof (intr_t));
	intr->intr_type = INTR_INT;
	intr->intr_signed = 1;
	intr->intr_nbits = sz * NBBY;

	tdp = xcalloc(sizeof (tdesc_t));
	tdp->t_name = atom_get(name);
	tdp->t_size = sz;
	tdp->t_id = tid;
	tdp->t_type = INTRINSIC;
	tdp->t_intr = intr;
	tdp->t_flags = TDESC_F_RESOLVED;

	tdesc_add(dw, tdp);

	return (tdp);
}

/*
 * Manufacture a void type.  Used for gcc-emitted stabs, where the lack of a
 * type reference implies a reference to a void type.  A void *, for example
 * will be represented by a pointer die without a DW_AT_type.  CTF requires
 * that pointer nodes point to something, so we'll create a void for use as
 * the target.  Note that the DWARF data may already create a void type.  Ours
 * would then be a duplicate, but it'll be removed in the self-uniquification
 * merge performed at the completion of DWARF->tdesc conversion.
 */
static tdesc_t *
tdesc_intr_void(dwarf_t *dw)
{
	if (dw->dw_void == NULL)
		dw->dw_void = tdesc_intr_common(dw, TID_VOID, "void", 0);

	return (dw->dw_void);
}

static tdesc_t *
tdesc_intr_long(dwarf_t *dw)
{
	if (dw->dw_long == NULL) {
		dw->dw_long = tdesc_intr_common(dw, TID_LONG, "long",
		    dw->dw_ptrsz);
	}

	return (dw->dw_long);
}

/*
 * Used for creating bitfield types.  We create a copy of an existing intrinsic,
 * adjusting the size of the copy to match what the caller requested.  The
 * caller can then use the copy as the type for a bitfield structure member.
 */
static tdesc_t *
tdesc_intr_clone(dwarf_t *dw, tdesc_t *old, size_t bitsz)
{
	tdesc_t *new = xcalloc(sizeof (tdesc_t));

	if (!(old->t_flags & TDESC_F_RESOLVED)) {
		terminate("tdp %u: attempt to make a bit field from an "
		    "unresolved type\n", old->t_id);
	}

	new->t_name = old->t_name;
	new->t_size = old->t_size;
	new->t_id = mfgtid_next(dw);
	new->t_type = INTRINSIC;
	new->t_flags = TDESC_F_RESOLVED;

	new->t_intr = xcalloc(sizeof (intr_t));
	bcopy(old->t_intr, new->t_intr, sizeof (intr_t));
	new->t_intr->intr_nbits = bitsz;

	tdesc_add(dw, new);

	return (new);
}

static void
tdesc_array_create(dwarf_t *dw, Dwarf_Die dim, tdesc_t *arrtdp,
    tdesc_t *dimtdp)
{
	Dwarf_Unsigned uval;
	Dwarf_Signed sval;
	tdesc_t *ctdp;
	Dwarf_Die dim2;
	ardef_t *ar;

	if ((dim2 = die_sibling(dw, dim)) == NULL) {
		ctdp = arrtdp;
	} else if (die_tag(dw, dim2) == DW_TAG_subrange_type) {
		ctdp = xcalloc(sizeof (tdesc_t));
		ctdp->t_id = mfgtid_next(dw);
		debug(3, "die %llu: creating new type %u for sub-dimension\n",
		    die_off(dw, dim2), ctdp->t_id);
		tdesc_array_create(dw, dim2, arrtdp, ctdp);
	} else {
		terminate("die %llu: unexpected non-subrange node in array\n",
		    die_off(dw, dim2));
	}

	dimtdp->t_type = ARRAY;
	dimtdp->t_ardef = ar = xcalloc(sizeof (ardef_t));

	/*
	 * Array bounds can be signed or unsigned, but there are several kinds
	 * of signless forms (data1, data2, etc) that take their sign from the
	 * routine that is trying to interpret them.  That is, data1 can be
	 * either signed or unsigned, depending on whether you use the signed or
	 * unsigned accessor function.  GCC will use the signless forms to store
	 * unsigned values which have their high bit set, so we need to try to
	 * read them first as unsigned to get positive values.  We could also
	 * try signed first, falling back to unsigned if we got a negative
	 * value.
	 */
	if (die_unsigned(dw, dim, DW_AT_upper_bound, &uval, 0))
		ar->ad_nelems = uval + 1;
	else if (die_signed(dw, dim, DW_AT_upper_bound, &sval, 0))
		ar->ad_nelems = sval + 1;
	else if (die_unsigned(dw, dim, DW_AT_count, &uval, 0))
		ar->ad_nelems = uval;
	else if (die_signed(dw, dim, DW_AT_count, &sval, 0))
		ar->ad_nelems = sval;
	else
		ar->ad_nelems = 0;

	/*
	 * Different compilers use different index types.  Force the type to be
	 * a common, known value (long).
	 */
	ar->ad_idxtype = tdesc_intr_long(dw);
	ar->ad_contents = ctdp;

	if (ar->ad_contents->t_size != 0) {
		dimtdp->t_size = ar->ad_contents->t_size * ar->ad_nelems;
		dimtdp->t_flags |= TDESC_F_RESOLVED;
	}
}

/*
 * Create a tdesc from an array node.  Some arrays will come with byte size
 * attributes, and thus can be resolved immediately.  Others don't, and will
 * need to wait until the second pass for resolution.
 */
static void
die_array_create(dwarf_t *dw, Dwarf_Die arr, Dwarf_Off off, tdesc_t *tdp)
{
	tdesc_t *arrtdp = die_lookup_pass1(dw, arr, DW_AT_type);
	Dwarf_Unsigned uval;
	Dwarf_Die dim;

	debug(3, "die %llu: creating array\n", off);

	if ((dim = die_child(dw, arr)) == NULL ||
	    die_tag(dw, dim) != DW_TAG_subrange_type)
		terminate("die %llu: failed to retrieve array bounds\n", off);

	tdesc_array_create(dw, dim, arrtdp, tdp);

	if (die_unsigned(dw, arr, DW_AT_byte_size, &uval, 0)) {
		tdesc_t *dimtdp;
		int flags;

		tdp->t_size = uval;

		/*
		 * Ensure that sub-dimensions have sizes too before marking
		 * as resolved.
		 */
		flags = TDESC_F_RESOLVED;
		for (dimtdp = tdp->t_ardef->ad_contents;
		    dimtdp->t_type == ARRAY;
		    dimtdp = dimtdp->t_ardef->ad_contents) {
			if (!(dimtdp->t_flags & TDESC_F_RESOLVED)) {
				flags = 0;
				break;
			}
		}

		tdp->t_flags |= flags;
	}

	debug(3, "die %llu: array nelems %u size %u\n", off,
	    tdp->t_ardef->ad_nelems, tdp->t_size);
}

/*ARGSUSED1*/
static int
die_array_resolve(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	dwarf_t *dw = private;
	size_t sz;

	if (tdp->t_flags & TDESC_F_RESOLVED)
		return (1);

	debug(3, "trying to resolve array %d (cont %d)\n", tdp->t_id,
	    tdp->t_ardef->ad_contents->t_id);

	if ((sz = tdesc_size(tdp->t_ardef->ad_contents)) == SIZE_MAX) {
		debug(3, "unable to resolve array %s (%d) contents %d\n",
		    tdesc_name(tdp), tdp->t_id,
		    tdp->t_ardef->ad_contents->t_id);

		dw->dw_nunres++;
		return (1);
	}

	tdp->t_size = sz * tdp->t_ardef->ad_nelems;
	tdp->t_flags |= TDESC_F_RESOLVED;

	debug(3, "resolved array %d: %u bytes\n", tdp->t_id, tdp->t_size);

	return (1);
}

/*ARGSUSED1*/
static int
die_array_failed(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	tdesc_t *cont = tdp->t_ardef->ad_contents;

	if (tdp->t_flags & TDESC_F_RESOLVED)
		return (1);

	fprintf(stderr, "Array %d: failed to size contents type %s (%d)\n",
	    tdp->t_id, tdesc_name(cont), cont->t_id);

	return (1);
}

/*
 * Most enums (those with members) will be resolved during this first pass.
 * Others - those without members (see the file comment) - won't be, and will
 * need to wait until the second pass when they can be matched with their full
 * definitions.
 */
static void
die_enum_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Die mem;
	Dwarf_Unsigned uval;
	Dwarf_Signed sval;

	debug(3, "die %llu: creating enum\n", off);

	tdp->t_type = ENUM;

	(void) die_unsigned(dw, die, DW_AT_byte_size, &uval, DW_ATTR_REQ);
	tdp->t_size = uval;

	if ((mem = die_child(dw, die)) != NULL) {
		elist_t **elastp = &tdp->t_emem;

		do {
			elist_t *el;
			Dwarf_Attribute attr;

			if (die_tag(dw, mem) != DW_TAG_enumerator) {
				/* Nested type declaration */
				die_create_one(dw, mem);
				continue;
			}

			el = xcalloc(sizeof (elist_t));
			el->el_name = die_name(dw, mem);

			int is_unsigned = 0;
			if ((attr = die_attr(dw, mem, DW_AT_const_value, 0)) != NULL)
				is_unsigned = dwarf_formisudata(attr);

			if (!is_unsigned && die_signed(dw, mem, DW_AT_const_value, &sval, 0)) {
				el->el_number = sval;
			} else if (die_unsigned(dw, mem, DW_AT_const_value,
			    &uval, 0)) {
				el->el_number = uval;
			} else {
				terminate("die %llu: enum %llu: member without "
				    "value\n", off, die_off(dw, mem));
			}

			debug(3, "die %llu: enum %llu: created %s = %d\n", off,
			    die_off(dw, mem), el->el_name->value, el->el_number);

			*elastp = el;
			elastp = &el->el_next;

		} while ((mem = die_sibling(dw, mem)) != NULL);

		hash_add(dw->dw_enumhash, tdp);

		tdp->t_flags |= TDESC_F_RESOLVED;

		if (tdp->t_name != ATOM_NULL) {
			iidesc_t *ii = xcalloc(sizeof (iidesc_t));
			ii->ii_type = II_SOU;
			ii->ii_name = tdp->t_name;
			ii->ii_dtype = tdp;

			iidesc_add(dw->dw_td->td_iihash, ii);
		}
	}
}

static int
die_enum_match(void *arg1, void *arg2)
{
	tdesc_t *tdp = arg1, **fullp = arg2;

	if (tdp->t_emem != NULL) {
		*fullp = tdp;
		return (-1); /* stop the iteration */
	}

	return (0);
}

/*ARGSUSED1*/
static int
die_enum_resolve(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	dwarf_t *dw = private;
	tdesc_t *full = NULL;

	if (tdp->t_flags & TDESC_F_RESOLVED)
		return (1);

	(void) hash_find_iter(dw->dw_enumhash, tdp, die_enum_match, &full);

	/*
	 * The answer to this one won't change from iteration to iteration,
	 * so don't even try.
	 */
	if (full == NULL) {
		terminate("tdp %u: enum %s has no members\n", tdp->t_id,
		    tdesc_name(tdp));
	}

	debug(3, "tdp %u: enum %s redirected to %u\n", tdp->t_id,
	    tdesc_name(tdp), full->t_id);

	tdp->t_flags |= TDESC_F_RESOLVED;

	return (1);
}

static int
die_fwd_map(void *arg1, void *arg2)
{
	tdesc_t *fwd = arg1, *sou = arg2;

	debug(3, "tdp %u: mapped forward %s to sou %u\n", fwd->t_id,
	    tdesc_name(fwd), sou->t_id);
	fwd->t_tdesc = sou;

	return (0);
}

/*
 * Structures and unions will never be resolved during the first pass, as we
 * won't be able to fully determine the member sizes.  The second pass, which
 * have access to sizing information, will be able to complete the resolution.
 */
static void
die_sou_create(dwarf_t *dw, Dwarf_Die str, Dwarf_Off off, tdesc_t *tdp,
    int type, const char *typename)
{
	Dwarf_Unsigned sz, bitsz, bitoff;
	Dwarf_Die mem;
	mlist_t *ml, **mlastp;
	iidesc_t *ii;

	tdp->t_type = (die_isdecl(dw, str) ? FORWARD : type);

	debug(3, "die %llu: creating %s %s\n", off,
	    (tdp->t_type == FORWARD ? "forward decl" : typename),
	    tdesc_name(tdp));

	if (tdp->t_type == FORWARD) {
		hash_add(dw->dw_fwdhash, tdp);
		return;
	}

	(void) hash_find_iter(dw->dw_fwdhash, tdp, die_fwd_map, tdp);

	(void) die_unsigned(dw, str, DW_AT_byte_size, &sz, DW_ATTR_REQ);
	tdp->t_size = sz;

	/*
	 * GCC allows empty SOUs as an extension.
	 */
	if ((mem = die_child(dw, str)) == NULL)
		goto out;

	mlastp = &tdp->t_members;

	do {
		Dwarf_Off memoff = die_off(dw, mem);
		Dwarf_Half tag = die_tag(dw, mem);
		Dwarf_Unsigned mloff;

		if (tag != DW_TAG_member) {
			/* Nested type declaration */
			die_create_one(dw, mem);
			continue;
		}

		debug(3, "die %llu: mem %llu: creating member\n", off, memoff);

		ml = xcalloc(sizeof (mlist_t));

		/*
		 * This could be a GCC anon struct/union member, so we'll allow
		 * an empty name, even though nothing can really handle them
		 * properly.  Note that some versions of GCC miss out debug
		 * info for anon structs, though recent versions are fixed (gcc
		 * bug 11816).
		 */
		if ((ml->ml_name = die_name(dw, mem)) == NULL)
			ml->ml_name = atom_get("");

        Dwarf_Off ref = die_attr_ref(dw, mem, DW_AT_type);
		ml->ml_type = die_lookup_pass1(dw, mem, DW_AT_type);

		if (die_mem_offset(dw, mem, DW_AT_data_member_location,
		    &mloff, 0)) {
			debug(3, "die %llu: got mloff %llx ref %lld\n", off,
			    (u_longlong_t)mloff, ref);
			ml->ml_offset = mloff * 8;
		}

		if (die_unsigned(dw, mem, DW_AT_bit_size, &bitsz, 0))
			ml->ml_size = bitsz;
		else
			ml->ml_size = tdesc_bitsize(ml->ml_type);

		if (die_unsigned(dw, mem, DW_AT_bit_offset, &bitoff, 0)) {
#ifdef	_BIG_ENDIAN
			ml->ml_offset += bitoff;
#else
			/*
			 * A DW_AT_byte_size attribute whose value (see Section 2.19) is the
			 * number of bytes that contain an instance of the bit field and any
			 * padding bits.
			 *
			 * The byte size attribute may be omitted if the size of the object
			 * containing the bit field can be inferred from the type attribute of
			 * the data member containing the bit field.
			 *
			 * There is some cases where we don't know, yet, the
			 * size of some intrinsic types.  We must be sure that
			 * in this case we have a AT_byte_size attribute.
			 */
			Dwarf_Unsigned bytesz = 0;
			if (die_unsigned(dw, mem, DW_AT_byte_size, &bytesz, 0)) {
				ml->ml_offset += (bytesz * 8) - bitoff - ml->ml_size;
			} else {
				size_t tdbitsz = tdesc_bitsize(ml->ml_type);
				assert((tdbitsz != 0) && "AT_byte_size missing and cannot figure the bitfield size");

				ml->ml_offset += tdbitsz - bitoff - ml->ml_size;
			}
#endif
		}
		/*
		 * Clang generates a DW_AT_data_bit_offset attribute for bitfields
		 * that stores the number of bits from the beginning of the containing
		 * struct to the beginning of the data member
		 */
		else if (die_unsigned(dw, mem, DW_AT_data_bit_offset, &bitoff, 0)) {

			ml->ml_offset += bitoff;
		}
		debug(3, "die %llu: mem %llu: created \"%s\" (off %u sz %u)\n",
		    off, memoff, ml->ml_name->value, ml->ml_offset, ml->ml_size);

		*mlastp = ml;
		mlastp = &ml->ml_next;
	} while ((mem = die_sibling(dw, mem)) != NULL);

	/*
	 * GCC will attempt to eliminate unused types, thus decreasing the
	 * size of the emitted dwarf.  That is, if you declare a foo_t in your
	 * header, include said header in your source file, and neglect to
	 * actually use (directly or indirectly) the foo_t in the source file,
	 * the foo_t won't make it into the emitted DWARF.  So, at least, goes
	 * the theory.
	 *
	 * Occasionally, it'll emit the DW_TAG_structure_type for the foo_t,
	 * and then neglect to emit the members.  Strangely, the loner struct
	 * tag will always be followed by a proper nested declaration of
	 * something else.  This is clearly a bug, but we're not going to have
	 * time to get it fixed before this goo goes back, so we'll have to work
	 * around it.  If we see a no-membered struct with a nested declaration
	 * (i.e. die_child of the struct tag won't be null), we'll ignore it.
	 * Being paranoid, we won't simply remove it from the hash.  Instead,
	 * we'll decline to create an iidesc for it, thus ensuring that this
	 * type won't make it into the output file.  To be safe, we'll also
	 * change the name.
	 */
	if (tdp->t_members == NULL) {
		const char *old = tdesc_name(tdp);
		size_t newsz = 7 + strlen(old) + 1;
		char *new = xmalloc(newsz);
		(void) snprintf(new, newsz, "orphan %s", old);

		debug(3, "die %llu: worked around %s %s\n", off, typename, old);

		tdp->t_name = atom_get_consume(new);
		return;
	}

out:
	if (tdp->t_name != ATOM_NULL) {
		ii = xcalloc(sizeof (iidesc_t));
		ii->ii_type = II_SOU;
		ii->ii_name = tdp->t_name;
		ii->ii_dtype = tdp;

		iidesc_add(dw->dw_td->td_iihash, ii);
	}
}

static void
die_struct_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_sou_create(dw, die, off, tdp, STRUCT, "struct");
}

static void
die_union_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_sou_create(dw, die, off, tdp, UNION, "union");
}

/*ARGSUSED1*/
static int
die_sou_resolve(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	dwarf_t *dw = private;
	mlist_t *ml;
	tdesc_t *mt;

	if (tdp->t_flags & TDESC_F_RESOLVED)
		return (1);

	debug(3, "resolving sou %s\n", tdesc_name(tdp));

	for (ml = tdp->t_members; ml != NULL; ml = ml->ml_next) {

#if defined(__APPLE__)
		// Check for NULL mt before the size check.
		// Seeing lots of crashes due to mt being NULL in
		// the code below.
		if ((mt = tdesc_basetype(ml->ml_type)) == NULL) {
			dw->dw_nunres++;
			return (1);
		}
#endif
		if (ml->ml_size == 0) {
			mt = tdesc_basetype(ml->ml_type);

			if ((ml->ml_size = tdesc_bitsize(mt)) != 0)
				continue;

			/*
			 * For empty members, or GCC/C99 flexible array
			 * members, a size of 0 is correct.
			 */
			if (mt->t_members == NULL)
				continue;
			if (mt->t_type == ARRAY && mt->t_ardef->ad_nelems == 0)
				continue;

			dw->dw_nunres++;
			return (1);
		}

#if defined(__APPLE__)
#else
		if ((mt = tdesc_basetype(ml->ml_type)) == NULL) {
			dw->dw_nunres++;
			return (1);
		}
#endif

		if (ml->ml_size != 0 && mt->t_type == INTRINSIC &&
		    mt->t_intr->intr_nbits != ml->ml_size) {
			/*
			 * This member is a bitfield, and needs to reference
			 * an intrinsic type with the same width.  If the
			 * currently-referenced type isn't of the same width,
			 * we'll copy it, adjusting the width of the copy to
			 * the size we'd like.
			 */
			debug(3, "tdp %u: creating bitfield for %d bits\n",
			    tdp->t_id, ml->ml_size);

			ml->ml_type = tdesc_intr_clone(dw, mt, ml->ml_size);
		}
	}

	tdp->t_flags |= TDESC_F_RESOLVED;

	return (1);
}

/*ARGSUSED1*/
static int
die_sou_failed(tdesc_t *tdp, tdesc_t **tdpp, void *private)
{
	const char *typename = (tdp->t_type == STRUCT ? "struct" : "union");
	mlist_t *ml;

	if (tdp->t_flags & TDESC_F_RESOLVED)
		return (1);

	for (ml = tdp->t_members; ml != NULL; ml = ml->ml_next) {
		if (ml->ml_size == 0) {
			fprintf(stderr, "%s %d: failed to size member \"%s\" "
			    "of type %s (%d)\n", typename, tdp->t_id,
			    ml->ml_name->value, tdesc_name(ml->ml_type),
			    ml->ml_type->t_id);
		}
	}

	return (1);
}

static void
die_funcptr_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Attribute attr;
	Dwarf_Half tag;
	Dwarf_Die arg;
	fndef_t fnbuf = {0};
	fndef_t *fn = &fnbuf;
	int i;

	debug(3, "die %llu: creating function pointer\n", off);

	/*
	 * We'll begin by processing any type definition nodes that may be
	 * lurking underneath this one.
	 */
	for (arg = die_child(dw, die); arg != NULL;
	    arg = die_sibling(dw, arg)) {
		if ((tag = die_tag(dw, arg)) != DW_TAG_formal_parameter &&
		    tag != DW_TAG_unspecified_parameters) {
			/* Nested type declaration */
			die_create_one(dw, arg);
		}
	}

	if (die_isdecl(dw, die)) {
		/*
		 * This is a prototype.  We don't add prototypes to the
		 * tree, so we're going to drop the tdesc.  Unfortunately,
		 * it has already been added to the tree.  Nobody will reference
		 * it, though, and it will be leaked.
		 */
		return;
	}

	tdp->t_type = FUNCTION;

	if ((attr = die_attr(dw, die, DW_AT_type, 0)) != NULL) {
		dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);
		fn->fn_ret = die_lookup_pass1(dw, die, DW_AT_type);
	} else {
		fn->fn_ret = tdesc_intr_void(dw);
	}

	/*
	 * Count the arguments to the function, then read them in.
	 */
	for (fn->fn_nargs = 0, arg = die_child(dw, die); arg != NULL;
	    arg = die_sibling(dw, arg)) {
		if ((tag = die_tag(dw, arg)) == DW_TAG_formal_parameter)
			fn->fn_nargs++;
		else if (tag == DW_TAG_unspecified_parameters &&
		    fn->fn_nargs > 0)
			fn->fn_vargs = 1;
	}

	fn = xcalloc(sizeof(fndef_t) + fn->fn_nargs * sizeof(tdesc_t *));
	*fn = fnbuf;

	if (fn->fn_nargs != 0) {
		debug(3, "die %llu: adding %d argument%s\n", off, fn->fn_nargs,
		    (fn->fn_nargs > 1 ? "s" : ""));

		for (i = 0, arg = die_child(dw, die);
		    arg != NULL && i < fn->fn_nargs;
		    arg = die_sibling(dw, arg)) {
			if (die_tag(dw, arg) != DW_TAG_formal_parameter)
				continue;

			fn->fn_args[i++] = die_lookup_pass1(dw, arg,
			    DW_AT_type);
		}
	}

	tdp->t_fndef = fn;
	tdp->t_flags |= TDESC_F_RESOLVED;
}

/*
 * GCC and DevPro use different names for the base types.  While the terms are
 * the same, they are arranged in a different order.  Some terms, such as int,
 * are implied in one, and explicitly named in the other.  Given a base type
 * as input, this routine will return a common name, along with an intr_t
 * that reflects said name.
 */
static intr_t *
die_base_name_parse(atom_t *atom, atom_t **newp)
{
	const char *name = atom->value;
	char buf[100];
	char *base, *c;
	int nlong = 0, nshort = 0, nchar = 0, nint = 0;
	int sign = 1;
	char fmt = '\0';
	intr_t *intr;

	if (strlen(name) > sizeof (buf) - 1)
		terminate("base type name \"%s\" is too long\n", name);

	strncpy(buf, name, sizeof (buf));

	for (c = strtok(buf, " "); c != NULL; c = strtok(NULL, " ")) {
		if (strcmp(c, "signed") == 0)
			sign = 1;
		else if (strcmp(c, "unsigned") == 0)
			sign = 0;
		else if (strcmp(c, "long") == 0)
			nlong++;
		else if (strcmp(c, "char") == 0) {
			nchar++;
			fmt = 'c';
		} else if (strcmp(c, "short") == 0)
			nshort++;
		else if (strcmp(c, "int") == 0)
			nint++;
		else {
			/*
			 * If we don't recognize any of the tokens, we'll tell
			 * the caller to fall back to the dwarf-provided
			 * encoding information.
			 */
			return (NULL);
		}
	}

	if (nchar > 1 || nshort > 1 || nint > 1 || nlong > 2)
		return (NULL);

	if (nchar > 0) {
		if (nlong > 0 || nshort > 0 || nint > 0)
			return (NULL);

		base = "char";

	} else if (nshort > 0) {
		if (nlong > 0)
			return (NULL);

		base = "short";

	} else if (nlong > 0) {
		base = "long";

	} else {
		base = "int";
	}

	intr = xcalloc(sizeof (intr_t));
	intr->intr_type = INTR_INT;
	intr->intr_signed = sign;
	intr->intr_iformat = fmt;

	snprintf(buf, sizeof (buf), "%s%s%s",
	    (sign ? "" : "unsigned "),
	    (nlong > 1 ? "long " : ""),
	    base);

	*newp = atom_get(buf);
	return (intr);
}

typedef struct fp_size_map {
	size_t fsm_typesz[2];	/* size of {32,64} type */
	uint_t fsm_enc[3];	/* CTF_FP_* for {bare,cplx,imagry} type */
} fp_size_map_t;

static const fp_size_map_t fp_encodings[] = {
	{ { 4, 4 }, { CTF_FP_SINGLE, CTF_FP_CPLX, CTF_FP_IMAGRY } },
	{ { 8, 8 }, { CTF_FP_DOUBLE, CTF_FP_DCPLX, CTF_FP_DIMAGRY } },
#ifdef __sparc
	{ { 16, 16 }, { CTF_FP_LDOUBLE, CTF_FP_LDCPLX, CTF_FP_LDIMAGRY } },
#else
	{ { 12, 16 }, { CTF_FP_LDOUBLE, CTF_FP_LDCPLX, CTF_FP_LDIMAGRY } },
#endif
	{ { 0, 0 } }
};

static uint_t
die_base_type2enc(dwarf_t *dw, Dwarf_Off off, Dwarf_Signed enc, size_t sz)
{
	const fp_size_map_t *map = fp_encodings;
	uint_t szidx = dw->dw_ptrsz == sizeof (uint64_t);
	uint_t mult = 1, col = 0;

	if (enc == DW_ATE_complex_float) {
		mult = 2;
		col = 1;
	} else if (enc == DW_ATE_imaginary_float ||
	    enc == DW_ATE_SUN_imaginary_float)
		col = 2;

	while (map->fsm_typesz[szidx] != 0) {
		if (map->fsm_typesz[szidx] * mult == sz)
			return (map->fsm_enc[col]);
		map++;
	}

	terminate("die %llu: unrecognized real type size %u\n", off, sz);
	/*NOTREACHED*/
	return (0);
}

static intr_t *
die_base_from_dwarf(dwarf_t *dw, Dwarf_Die base, Dwarf_Off off, size_t sz)
{
	intr_t *intr = xcalloc(sizeof (intr_t));
	Dwarf_Signed enc;

	(void) die_signed(dw, base, DW_AT_encoding, &enc, DW_ATTR_REQ);

	switch (enc) {
	case DW_ATE_unsigned:
	case DW_ATE_address:
		intr->intr_type = INTR_INT;
		break;
	case DW_ATE_unsigned_char:
		intr->intr_type = INTR_INT;
		intr->intr_iformat = 'c';
		break;
	case DW_ATE_signed:
		intr->intr_type = INTR_INT;
		intr->intr_signed = 1;
		break;
	case DW_ATE_signed_char:
		intr->intr_type = INTR_INT;
		intr->intr_signed = 1;
		intr->intr_iformat = 'c';
		break;
	case DW_ATE_boolean:
		intr->intr_type = INTR_INT;
		intr->intr_signed = 1;
		intr->intr_iformat = 'b';
		break;
	case DW_ATE_float:
	case DW_ATE_complex_float:
	case DW_ATE_imaginary_float:
	case DW_ATE_SUN_imaginary_float:
	case DW_ATE_SUN_interval_float:
		intr->intr_type = INTR_REAL;
		intr->intr_signed = 1;
		intr->intr_fformat = die_base_type2enc(dw, off, enc, sz);
		break;
	default:
		terminate("die %llu: unknown base type encoding 0x%llx\n",
		    off, enc);
	}

	return (intr);
}

static void
die_base_create(dwarf_t *dw, Dwarf_Die base, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Unsigned sz;
	intr_t *intr;
	atom_t *new;

	debug(3, "die %llu: creating base type %d\n", off, tdp->t_id);

	/*
	 * The compilers have their own clever (internally inconsistent) ideas
	 * as to what base types should look like.  Some times gcc will, for
	 * example, use DW_ATE_signed_char for char.  Other times, however, it
	 * will use DW_ATE_signed.  Needless to say, this causes some problems
	 * down the road, particularly with merging.  We do, however, use the
	 * DWARF idea of type sizes, as this allows us to avoid caring about
	 * the data model.
	 */
	(void) die_unsigned(dw, base, DW_AT_byte_size, &sz, DW_ATTR_REQ);

	if (tdp->t_name == ATOM_NULL)
		terminate("die %llu: base type without name\n", off);

	/* XXX make a name parser for float too */
	if ((intr = die_base_name_parse(tdp->t_name, &new)) != NULL) {
		/* Found it.  We'll use the parsed version */
		debug(3, "die %llu: name \"%s\" remapped to \"%s\" size %d\n", off,
		    tdesc_name(tdp), new->value, sz);
		tdp->t_name = new;
	} else {
		/*
		 * We didn't recognize the type, so we'll create an intr_t
		 * based on the DWARF data.
		 */
		debug(3, "die %llu: using dwarf data for base \"%s\"\n", off,
		    tdesc_name(tdp));

		intr = die_base_from_dwarf(dw, base, off, sz);
	}

	intr->intr_nbits = sz * 8;

	tdp->t_type = INTRINSIC;
	tdp->t_intr = intr;
	tdp->t_size = sz;

	tdp->t_flags |= TDESC_F_RESOLVED;
}

static void
die_through_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp,
    int type, const char *typename)
{
	Dwarf_Attribute attr;

	debug(3, "die %llu: creating %s\n", off, typename);

	if ((attr = die_attr(dw, die, DW_AT_type, 0)) != NULL) {
		dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);
		tdp->t_tdesc = die_lookup_pass1(dw, die, DW_AT_type);
		debug(3, "die %llu: to %llu\n", off, tdp->t_tdesc->t_id);
	} else {
		tdp->t_tdesc = tdesc_intr_void(dw);
	}

    /* set t_type after t_desc since a non-zero t_type requires t_desc */
	tdp->t_type = type;

	if (type == POINTER)
		tdp->t_size = dw->dw_ptrsz;

	tdp->t_flags |= TDESC_F_RESOLVED;

	if (type == TYPEDEF) {
		iidesc_t *ii = xcalloc(sizeof (iidesc_t));
		ii->ii_type = II_TYPE;
		ii->ii_name = tdp->t_name;
		ii->ii_dtype = tdp;

		iidesc_add(dw->dw_td->td_iihash, ii);
	}
}

static void
die_typedef_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_through_create(dw, die, off, tdp, TYPEDEF, "typedef");
}

static void
die_const_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_through_create(dw, die, off, tdp, CONST, "const");
}

static void
die_pointer_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_through_create(dw, die, off, tdp, POINTER, "pointer");
}

static void
die_restrict_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_through_create(dw, die, off, tdp, RESTRICT, "restrict");
}

static void
die_volatile_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	die_through_create(dw, die, off, tdp, VOLATILE, "volatile");
}

static int
die_isdiscriminated(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Bool val;
	return (die_bool(dw, die,
	    DW_AT_APPLE_ptrauth_address_discriminated, &val, 0) && val);
}

/*ARGSUSED*/
static void
die_ptrauth_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Signed discriminator, key;
	Dwarf_Attribute attr;
	ptrauth_t *pta = xcalloc(sizeof (ptrauth_t));

	debug(3, "die %llu: creating ptrauth\n", off);

	if ((attr = die_attr(dw, die, DW_AT_type, 0)) == NULL) {
		terminate("die %llu: no referenced type", die_off(dw, die));
	}

	dwarf_dealloc(dw->dw_dw, attr, DW_DLA_ATTR);

	pta->pta_type = die_lookup_pass1(dw, die, DW_AT_type);

	pta->pta_discriminated = die_isdiscriminated(dw, die);

	die_signed(dw, die, DW_AT_APPLE_ptrauth_key, &key, 1);
	pta->pta_key = key;
	if (die_signed(dw, die, DW_AT_APPLE_ptrauth_extra_discriminator,
	    &discriminator, 0)) {
		pta->pta_discriminator = discriminator;
	}

	tdp->t_type = PTRAUTH;
	tdp->t_size = dw->dw_ptrsz;
	tdp->t_flags |= TDESC_F_RESOLVED;

	tdp->t_ptrauth = pta;

	debug(3, "die %llu: to %llu\n", off, pta->pta_type->t_id);
}

/*ARGSUSED3*/
static void
die_function_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Die arg;
	Dwarf_Half tag;
	iidesc_t *ii;
	atom_t *name;

	debug(3, "die %llu: creating function definition\n", off);

	/*
	 * We'll begin by processing any type definition nodes that may be
	 * lurking underneath this one.
	 */
	for (arg = die_child(dw, die); arg != NULL;
	    arg = die_sibling(dw, arg)) {
		if ((tag = die_tag(dw, arg)) != DW_TAG_formal_parameter &&
		    tag != DW_TAG_variable) {
			/* Nested type declaration */
			die_create_one(dw, arg);
		}
	}

#if defined(__APPLE__)
	if (die_isdecl(dw, die)) {
		return; /* Prototype. */
	}
	
	Dwarf_Die spec_die = die_specification_die(dw, die); // AT_specification indirection?
	if (spec_die)
		die = spec_die; // and we'll deallocate through spec_die below.
	
	if ((name = die_linkage_name(dw, die)) != NULL) {
		/* and press on with this die describing a C++ method ... */
	} else {
		name = die_name(dw, die);
	}
	
	if (name == ATOM_NULL) {
		return; /* Subprogram without name. */
	}
#else
	if (die_isdecl(dw, die) || (name = die_name(dw, die)) == NULL) {
		/*
		 * We process neither prototypes nor subprograms without
		 * names.
		 */
		return;
	}
#endif

	ii = xcalloc(sizeof (iidesc_t));
	ii->ii_type = die_isglobal(dw, die) ? II_GFUN : II_SFUN;
	ii->ii_name = name;
	if (ii->ii_type == II_SFUN)
		ii->ii_owner = dw->dw_cuname;

	debug(3, "die %llu: function %s is %s\n", off, name,
	    (ii->ii_type == II_GFUN ? "global" : "static"));

	if (die_attr(dw, die, DW_AT_type, 0) != NULL)
		ii->ii_dtype = die_lookup_pass1(dw, die, DW_AT_type);
	else
		ii->ii_dtype = tdesc_intr_void(dw);

	for (arg = die_child(dw, die); arg != NULL;
	    arg = die_sibling(dw, arg)) {
		atom_t *tmp;

		debug(3, "die %llu: looking at sub member at %llu\n",
		    off, die_off(dw, die));

		if (die_tag(dw, arg) != DW_TAG_formal_parameter)
			continue;
#if defined(__APPLE__)
		if (die_attr(dw, die, DW_AT_type, 0) == NULL)
			continue; /* C++ "this" and "meta" can land here. */
#endif /* __APPLE__ */

#if !defined(__APPLE__)
		if ((tmp = die_name(dw, arg)) == ATOM_NULL) {
			terminate("die %llu: func arg %d has no name\n",
			    off, ii->ii_nargs + 1);
		}
#else
		if ((tmp = die_name(dw, arg)) == ATOM_NULL) {
			/* C++ admits this, IOKit does it, we'll allow it. */
			debug(1, "die %llu: func arg %d has no name\n",
			    off, ii->ii_nargs + 1);
			ii->ii_nargs++;
			continue;
		}
#endif /* __APPLE__ */

		if (strcmp(tmp->value, "...") == 0) {
			ii->ii_vargs = 1;
			continue;
		}

		ii->ii_nargs++;
	}

	if (ii->ii_nargs > 0) {
		int i;

		debug(3, "die %llu: function has %d argument%s\n", off,
		    ii->ii_nargs, (ii->ii_nargs == 1 ? "" : "s"));

		ii->ii_args = xcalloc(sizeof (tdesc_t) * ii->ii_nargs);

		for (arg = die_child(dw, die), i = 0;
		    arg != NULL && i < ii->ii_nargs;
		    arg = die_sibling(dw, arg)) {
			if (die_tag(dw, arg) != DW_TAG_formal_parameter)
				continue;
#if defined(__APPLE__)
			if (die_attr(dw, die, DW_AT_type, 0) == NULL)
				continue; /* C++ "this" and "meta" can land here. */
#endif /* __APPLE__ */
			ii->ii_args[i++] = die_lookup_pass1(dw, arg,
			    DW_AT_type);
		}
	}

	iidesc_add(dw->dw_td->td_iihash, ii);

#if defined(__APPLE__)
	if (spec_die)
		dwarf_dealloc(dw->dw_dw, spec_die, DW_DLA_DIE);
#endif /* __APPLE__ */
}

/*ARGSUSED3*/
static void
die_variable_create(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	iidesc_t *ii;
	atom_t *name;

	debug(3, "die %llu: creating object definition\n", off);

	if (die_isdecl(dw, die) || (name = die_name(dw, die)) == ATOM_NULL)
		return; /* skip prototypes and nameless objects */

	ii = xcalloc(sizeof (iidesc_t));
	ii->ii_type = die_isglobal(dw, die) ? II_GVAR : II_SVAR;
	ii->ii_name = name;
	ii->ii_dtype = die_lookup_pass1(dw, die, DW_AT_type);
	if (ii->ii_type == II_SVAR)
		ii->ii_owner = dw->dw_cuname;

	iidesc_add(dw->dw_td->td_iihash, ii);
}

/*ARGSUSED2*/
static int
die_fwd_resolve(tdesc_t *fwd, tdesc_t **fwdp, void *private)
{
	if (fwd->t_flags & TDESC_F_RESOLVED)
		return (1);

	if (fwd->t_tdesc != NULL) {
		debug(3, "tdp %u: unforwarded %s\n", fwd->t_id,
		    tdesc_name(fwd));
		*fwdp = fwd->t_tdesc;
	}

	fwd->t_flags |= TDESC_F_RESOLVED;

	return (1);
}

/*ARGSUSED*/
static void
die_lexblk_descend(dwarf_t *dw, Dwarf_Die die, Dwarf_Off off, tdesc_t *tdp)
{
	Dwarf_Die child = die_child(dw, die);

	if (child != NULL)
		die_create(dw, child);
}

/*
 * Used to map the die to a routine which can parse it, using the tag to do the
 * mapping.  While the processing of most tags entails the creation of a tdesc,
 * there are a few which don't - primarily those which result in the creation of
 * iidescs which refer to existing tdescs.
 */

#define	DW_F_NOTDP	0x1	/* Don't create a tdesc for the creator */

typedef struct die_creator {
	Dwarf_Half dc_tag;
	uint16_t dc_flags;
	void (*dc_create)(dwarf_t *, Dwarf_Die, Dwarf_Off, tdesc_t *);
} die_creator_t;

static const die_creator_t die_creators[] = {
	{ DW_TAG_array_type,		0,		die_array_create },
	{ DW_TAG_enumeration_type,	0,		die_enum_create },
	{ DW_TAG_lexical_block,		DW_F_NOTDP,	die_lexblk_descend },
	{ DW_TAG_pointer_type,		0,		die_pointer_create },
	{ DW_TAG_structure_type,	0,		die_struct_create },
	{ DW_TAG_subroutine_type,	0,		die_funcptr_create },
	{ DW_TAG_typedef,		0,		die_typedef_create },
	{ DW_TAG_union_type,		0,		die_union_create },
	{ DW_TAG_base_type,		0,		die_base_create },
	{ DW_TAG_const_type,		0,		die_const_create },
	{ DW_TAG_subprogram,		DW_F_NOTDP,	die_function_create },
	{ DW_TAG_variable,		DW_F_NOTDP,	die_variable_create },
	{ DW_TAG_volatile_type,		0,		die_volatile_create },
	{ DW_TAG_restrict_type,		0,		die_restrict_create },
	{ DW_TAG_APPLE_ptrauth_type,	0,		die_ptrauth_create },
	{ 0, NULL }
};

static const die_creator_t *
die_tag2ctor(Dwarf_Half tag)
{
	const die_creator_t *dc;

	for (dc = die_creators; dc->dc_create != NULL; dc++) {
		if (dc->dc_tag == tag)
			return (dc);
	}

	return (NULL);
}

static tdesc_t *
die_create_one(dwarf_t *dw, Dwarf_Die die)
{
	Dwarf_Off off = die_off(dw, die);
	const die_creator_t *dc;
	Dwarf_Half tag;
	tdesc_t *tdp;

	debug(3, "die %llu: create_one\n", off);
	if ((off < dw->dw_cuoff) || (off > dw->dw_maxoff)) {
		debug(2, "die offset outside CU %llu (cuoff %llu max %llu)\n",
				off, dw->dw_cuoff, dw->dw_maxoff);
	}

	tag = die_tag(dw, die);

	if ((dc = die_tag2ctor(tag)) == NULL) {
		debug(2, "die %llu: ignoring tag type %x\n", off, tag);
		return (NULL);
	}

	if ((tdp = tdesc_lookup(dw, off)) == NULL &&
	    !(dc->dc_flags & DW_F_NOTDP)) {
		tdp = xcalloc(sizeof (tdesc_t));
		tdp->t_id = off;
		tdesc_add(dw, tdp);
	}

	if (tdp != NULL)
		tdp->t_name = die_name(dw, die);

#if defined(__APPLE__)
#warning BOGUS workaround (nameless base die emitted by gcc)!
	/* gcc 5402 emits nameless die that are base types. Workaround imminent failure. */
	if (tdp != NULL && ATOM_NULL == tdp->t_name && dc->dc_create == die_base_create)
		tdp->t_name = atom_get("unsigned int");
#endif /* __APPLE__ */

	dc->dc_create(dw, die, off, tdp);

	return (tdp);
}

static void
die_create(dwarf_t *dw, Dwarf_Die die)
{
	do {
		die_create_one(dw, die);
	} while ((die = die_sibling(dw, die)) != NULL);
}

static tdtrav_cb_f die_resolvers[] = {
	NULL,
	NULL,			/* intrinsic */
	NULL,			/* pointer */
	die_array_resolve,	/* array */
	NULL,			/* function */
	die_sou_resolve,	/* struct */
	die_sou_resolve,	/* union */
	die_enum_resolve,	/* enum */
	die_fwd_resolve,	/* forward */
	NULL,			/* typedef */
	NULL,			/* typedef unres */
	NULL,			/* volatile */
	NULL,			/* const */
	NULL,			/* restrict */
	NULL,			/* ptrauth */
};

static tdtrav_cb_f die_fail_reporters[] = {
	NULL,
	NULL,			/* intrinsic */
	NULL,			/* pointer */
	die_array_failed,	/* array */
	NULL,			/* function */
	die_sou_failed,		/* struct */
	die_sou_failed,		/* union */
	NULL,			/* enum */
	NULL,			/* forward */
	NULL,			/* typedef */
	NULL,			/* typedef unres */
	NULL,			/* volatile */
	NULL,			/* const */
	NULL,			/* restrict */
	NULL,			/* ptrauth */
};

static void
die_resolve(dwarf_t *dw)
{
	int last = -1;
	int pass = 0;

	do {
		pass++;
		dw->dw_nunres = 0;

		(void) iitraverse_hash(dw->dw_td->td_iihash,
		    &dw->dw_td->td_curvgen, NULL, NULL, die_resolvers, dw);

		debug(3, "resolve: pass %d, %u left\n", pass, dw->dw_nunres);

		if (dw->dw_nunres == last) {
			fprintf(stderr, "%s: failed to resolve the following "
			    "types:\n", progname);

			(void) iitraverse_hash(dw->dw_td->td_iihash,
			    &dw->dw_td->td_curvgen, NULL, NULL,
			    die_fail_reporters, dw);

			terminate("failed to resolve types\n");
		}

		last = dw->dw_nunres;

	} while (dw->dw_nunres != 0);
}

/*ARGSUSED*/
int
dw_read(Elf *elf, const char *filename, const char *unitmatch, int verbose, tdata_t **mstrtd)
{
	dwarf_t _dw;
	dwarf_t *dw = &_dw;
	Dwarf_Unsigned abboff, hdrlen, nxthdr;
	Dwarf_Half vers, addrsz;
	Dwarf_Die child;
	char *prod;
	int rc;
	int cucount = 0;
	int cumerged = 0;
	cu_data_t *cufirst = NULL;
	cu_data_t *culast = NULL;
	cu_data_t *cunext = NULL;
	cu_data_t *cudata;
	tdata_t *cutd = NULL;
	merge_cb_data_t mcd = {0};

	bzero(dw, sizeof (dwarf_t));
	dw->dw_ptrsz = elf_ptrsz(elf);

	if ((rc = dwarf_elf_init(elf, DW_DLC_READ, NULL, NULL, &dw->dw_dw,
		&dw->dw_err)) == DW_DLV_NO_ENTRY) {
		return (-1);
	} else if (rc != DW_DLV_OK) {
		if (dwarf_errno(dw->dw_err) == DW_DLE_DEBUG_INFO_NULL) {
			/*
			 * There's no type data in the DWARF section, but
			 * libdwarf is too clever to handle that properly.
			 */
			return (-1);
		}

		terminate("failed to initialize DWARF: %s\n",
			dwarf_errmsg(dw->dw_err));
	}

	/* First pass to find the compilation units we will process */

	while (1) {
		dw->dw_cuname = ATOM_NULL;
		if ((rc = dwarf_next_cu_header(dw->dw_dw, &hdrlen, &vers, &abboff,
			&addrsz, &nxthdr, &dw->dw_err)) != DW_DLV_OK) {
			break;
		}

		dw->dw_maxoff = nxthdr - 1;
		if (dw->dw_maxoff > TID_FILEMAX)
			terminate("file contains too many types\n");

		debug(4, "DWARF version: %d\n", vers);
		if (vers != DWARF_VERSION && vers != DWARF_VERSION3 && vers != DWARF_VERSION4) {
			terminate("file contains incompatible version %d DWARF code "
				"(version 2/3/4 required)\n", vers);
		}

		Dwarf_Error error = NULL;
		rc = dwarf_siblingof(dw->dw_dw, NULL, &dw->dw_cu, &error);
		if(rc != DW_DLV_OK) {
			debug(4, "Error in dwarf_siblingof on CU die (%s)\n", dwarf_errmsg(error));
			continue;
		}

		if (die_string(dw, dw->dw_cu, DW_AT_producer, &prod, 0)) {
			debug(4, "DWARF emitter: %s\n", prod);
			free(prod);
		}

		dw->dw_cuname = die_name(dw, dw->dw_cu);
		if (!dw->dw_cuname) {
			debug(1, "no name for CU\n");
			continue;
		}

		if (unitmatch) {
			/* Only extract from CUs built from a directory path
			 * containing the string unitmatch. */
			char *comp_dir;
			if (!die_string(dw, dw->dw_cu, DW_AT_comp_dir, &comp_dir, 0)) {
				debug(1, "no AT_comp_dir for CU\n");
				continue;
			}
			int skip = (!strstr(comp_dir, unitmatch));
			free(comp_dir);
			if (skip) {
				continue;
			}
		}

		Dwarf_Unsigned lang;
		rc = dwarf_srclang(dw->dw_cu, &lang, &error);
		if (rc == DW_DLV_OK) {
			/* This code doesn't handle the DW_TAG_class_type,
			 *  or scope c++ definitions properly.
			 */
			if (lang == DW_LANG_C_plus_plus
			    || lang == DW_LANG_ObjC_plus_plus
			    || lang == DW_LANG_C_plus_plus_03
			    || lang == DW_LANG_C_plus_plus_11
			    || lang == DW_LANG_C_plus_plus_14) {
				continue;
			}
		}

		cudata = xcalloc(sizeof(*cudata));
		if (!cufirst) {
			cufirst = cudata;
		}
		if (culast) {
			culast->cud_next = cudata;
		}
		culast = cudata;
		cucount++;
		cudata->cud_name = dw->dw_cuname;
		cudata->cud_nxthdr = nxthdr;
	}

	ctfmerge_prepare(cucount);

	/* Second pass to extract & merge */

	dw->dw_tidhash = alist_new(TDESC_HASH_BUCKETS);

	for (cudata = cunext = cufirst; cudata; cudata = cunext) {
		dw->dw_cuname = ATOM_NULL;
		if ((rc = dwarf_next_cu_header(dw->dw_dw, &hdrlen, &vers, &abboff,
			&addrsz, &nxthdr, &dw->dw_err)) != DW_DLV_OK) {
			break;
		}
		if (nxthdr != cudata->cud_nxthdr) {
			continue;
		}
		cunext = cudata->cud_next;

		Dwarf_Die no_die = 0;
		Dwarf_Error error = NULL;
		rc = dwarf_siblingof(dw->dw_dw, no_die, &dw->dw_cu, &error);
		if(rc != DW_DLV_OK) {
			terminate("Error in dwarf_siblingof on CU die (%s)\n", dwarf_errmsg(error));
		}

		dw->dw_cuoff = die_off(dw, dw->dw_cu);
		dw->dw_cuname = cudata->cud_name;
		if (verbose && (cucount > 1)) {
			printf("[%.02fM] %s\n", nxthdr / 1024. / 1024., dw->dw_cuname->value);
		}
		char *tmp = xstrdup(dw->dw_cuname->value);
		dw->dw_cuname = atom_get(basename(tmp));
		free(tmp);

		rc = dwarf_child(dw->dw_cu, &child, &error);
		if (rc != DW_DLV_OK) {
			printf("Error in dwarf_child on CU die \n");
			continue;
		}

		dw->dw_mfgtid_last = TID_MFGTID_BASE;
		dw->dw_fwdhash = hash_new(TDESC_HASH_BUCKETS, tdesc_namehash,
		    tdesc_namecmp);
		dw->dw_enumhash = hash_new(TDESC_HASH_BUCKETS, tdesc_namehash,
		    tdesc_namecmp);

		dw->dw_void = NULL;
		dw->dw_long = NULL;
		dw->dw_maxoff = nxthdr - 1;

		dw->dw_td = tdata_new();
		die_create(dw, child);
		die_resolve(dw);

#if !defined(__APPLE__) && !defined(__linux__)
		cvt_fixups(td, dw->dw_ptrsz);
#else
		/* Ignore Solaris gore. See on-src-20080707/usr/src/tools/ctf/cvt/fixup_tdescs.c */
#endif

		cutd = tdata_new();
		debug(1, "mergeto %s to cutd\n", dw->dw_cuname->value);
		merge_into_master(&mcd, dw->dw_td, cutd, NULL, 1);

		ctfmerge_add_td(cutd, dw->dw_cuname->value);

		alist_clear(dw->dw_tidhash);
		hash_free(dw->dw_fwdhash,  NULL, NULL);
		hash_free(dw->dw_enumhash, NULL, NULL);
		tdata_free(dw->dw_td);

		cumerged++;
	}

	alist_free(dw->dw_tidhash);
	merge_cb_data_destroy(&mcd);

	debug(1, "dwarf_finish\n");
	(void) dwarf_finish(dw->dw_dw, &dw->dw_err);

	if (cumerged) {
		*mstrtd = ctfmerge_done();
		return (0);
	} else {
		return (-1);
	}
}
