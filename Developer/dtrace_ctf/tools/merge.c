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
 * This file contains routines that merge one tdata_t tree, called the child,
 * into another, called the parent.  Note that these names are used mainly for
 * convenience and to represent the direction of the merge.  They are not meant
 * to imply any relationship between the tdata_t graphs prior to the merge.
 *
 * tdata_t structures contain two main elements - a hash of iidesc_t nodes, and
 * a directed graph of tdesc_t nodes, pointed to by the iidesc_t nodes.  Simply
 * put, we merge the tdesc_t graphs, followed by the iidesc_t nodes, and then we
 * clean up loose ends.
 *
 * The algorithm is as follows:
 *
 * 1. Mapping iidesc_t nodes
 *
 * For each child iidesc_t node, we first try to map its tdesc_t subgraph
 * against the tdesc_t graph in the parent.  For each node in the child subgraph
 * that exists in the parent, a mapping between the two (between their type IDs)
 * is established.  For the child nodes that cannot be mapped onto existing
 * parent nodes, a mapping is established between the child node ID and a
 * newly-allocated ID that the node will use when it is re-created in the
 * parent.  These unmappable nodes are added to the md_tdtba (tdesc_t To Be
 * Added) hash, which tracks nodes that need to be created in the parent.
 *
 * If all of the nodes in the subgraph for an iidesc_t in the child can be
 * mapped to existing nodes in the parent, then we can try to map the child
 * iidesc_t onto an iidesc_t in the parent.  If we cannot find an equivalent
 * iidesc_t, or if we were not able to completely map the tdesc_t subgraph(s),
 * then we add this iidesc_t to the md_iitba (iidesc_t To Be Added) list.  This
 * list tracks iidesc_t nodes that are to be created in the parent.
 *
 * While visiting the tdesc_t nodes, we may discover a forward declaration (a
 * FORWARD tdesc_t) in the parent that is resolved in the child.  That is, there
 * may be a structure or union definition in the child with the same name as the
 * forward declaration in the parent.  If we find such a node, we record an
 * association in the md_fdida (Forward => Definition ID Association) list
 * between the parent ID of the forward declaration and the ID that the
 * definition will use when re-created in the parent.
 *
 * 2. Creating new tdesc_t nodes (the md_tdtba hash)
 *
 * We have now attempted to map all tdesc_t nodes from the child into the
 * parent, and have, in md_tdtba, a hash of all tdesc_t nodes that need to be
 * created (or, as we so wittily call it, conjured) in the parent.  We iterate
 * through this hash, creating the indicated tdesc_t nodes.  For a given tdesc_t
 * node, conjuring requires two steps - the copying of the common tdesc_t data
 * (name, type, etc) from the child node, and the creation of links from the
 * newly-created node to the parent equivalents of other tdesc_t nodes pointed
 * to by node being conjured.  Note that in some cases, the targets of these
 * links will be on the md_tdtba hash themselves, and may not have been created
 * yet.  As such, we can't establish the links from these new nodes into the
 * parent graph.  We therefore conjure them with links to nodes in the *child*
 * graph, and add pointers to the links to be created to the md_tdtbr (tdesc_t
 * To Be Remapped) hash.  For example, a POINTER tdesc_t that could not be
 * resolved would have its &tdesc_t->t_tdesc added to md_tdtbr.
 *
 * 3. Creating new iidesc_t nodes (the md_iitba list)
 *
 * When we have completed step 2, all tdesc_t nodes have been created (or
 * already existed) in the parent.  Some of them may have incorrect links (the
 * members of the md_tdtbr list), but they've all been created.  As such, we can
 * create all of the iidesc_t nodes, as we can attach the tdesc_t subgraph
 * pointers correctly.  We create each node, and attach the pointers to the
 * appropriate parts of the parent tdesc_t graph.
 *
 * 4. Resolving newly-created tdesc_t node links (the md_tdtbr list)
 *
 * As in step 3, we rely on the fact that all of the tdesc_t nodes have been
 * created.  Each entry in the md_tdtbr list is a pointer to where a link into
 * the parent will be established.  As saved in the md_tdtbr list, these
 * pointers point into the child tdesc_t subgraph.  We can thus get the target
 * type ID from the child, look at the ID mapping to determine the desired link
 * target, and redirect the link accordingly.
 *
 * 5. Parent => child forward declaration resolution
 *
 * If entries were made in the md_fdida list in step 1, we have forward
 * declarations in the parent that need to be resolved to their definitions
 * re-created in step 2 from the child.  Using the md_fdida list, we can locate
 * the definition for the forward declaration, and we can redirect all inbound
 * edges to the forward declaration node to the actual definition.
 *
 * A pox on the house of anyone who changes the algorithm without updating
 * this comment.
 */



#include <stdio.h>
#include <strings.h>
#include <assert.h>
#include <pthread.h>

#include "ctf_headers.h"
#include "ctftools.h"
#include "ctfmerge.h"
#include "list.h"
#include "alist.h"
#include "memory.h"
#include "traverse.h"

#if defined(__APPLE__)
#include <unistd.h>
#include <signal.h>
#endif /* __APPLE__ */

typedef struct equiv_data equiv_data_t;

/*
 * There are two traversals in this file, for equivalency and for tdesc_t
 * re-creation, that do not fit into the tdtraverse() framework.  We have our
 * own traversal mechanism and ops vector here for those two cases.
 */
typedef struct tdesc_ops {
	char *name;
	int (*equiv)(tdesc_t *, tdesc_t *, equiv_data_t *);
	tdesc_t *(*conjure)(tdesc_t *, int, merge_cb_data_t *);
} tdesc_ops_t;
extern tdesc_ops_t tdesc_ops[];

/*
 * When we first create a tdata_t from stabs data, we will have duplicate nodes.
 * Normal merges, however, assume that the child tdata_t is already self-unique,
 * and for speed reasons do not attempt to self-uniquify.  If this flag is set,
 * the merge algorithm will self-uniquify by avoiding the insertion of
 * duplicates in the md_tdtdba list.
 */
#define	MCD_F_SELFUNIQUIFY	0x1

/*
 * When we merge the CTF data for the modules, we don't want it to contain any
 * data that can be found in the reference module (usually genunix).  If this
 * flag is set, we're doing a merge between the fully merged tdata_t for this
 * module and the tdata_t for the reference module, with the data unique to this
 * module ending up in a third tdata_t.  It is this third tdata_t that will end
 * up in the .SUNW_ctf section for the module.
 */
#define	MCD_F_REFMERGE	0x2

/*
 * Mapping of child type IDs to parent type IDs
 */

static void
add_mapping(alist_t *ta, tid_t srcid, tid_t tgtid)
{
	debug(3, "Adding mapping %u => %u\n", srcid, tgtid);

	assert(!alist_find(ta, (void *)(uintptr_t)srcid, NULL));
	assert(srcid != 0 && tgtid != 0);

	alist_add(ta, (void *)(uintptr_t)srcid, (void *)(uintptr_t)tgtid);
}

static tid_t
get_mapping(alist_t *ta, int srcid)
{
	long ltgtid;

	if (alist_find(ta, (void *)(uintptr_t)srcid, (void **)(uintptr_t)&ltgtid))
		return ((int)ltgtid);
	else
		return (0);
}

/*
 * Determining equivalence of tdesc_t subgraphs
 */

struct equiv_data {
	alist_t *ed_ta;
	tdesc_t *ed_node;
	tdesc_t *ed_tgt;

	int ed_clear_mark;
	int ed_cur_mark;
	int ed_selfuniquify;
}; /* equiv_data_t */

static int equiv_node(tdesc_t *, tdesc_t *, equiv_data_t *);

/*ARGSUSED2*/
static int
equiv_intrinsic(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	intr_t *si = stdp->t_intr;
	intr_t *ti = ttdp->t_intr;

	if (si->intr_type != ti->intr_type ||
	    si->intr_signed != ti->intr_signed ||
	    si->intr_offset != ti->intr_offset ||
	    si->intr_nbits != ti->intr_nbits)
		return (0);

	if (si->intr_type == INTR_INT &&
	    si->intr_iformat != ti->intr_iformat)
		return (0);
	else if (si->intr_type == INTR_REAL &&
	    si->intr_fformat != ti->intr_fformat)
		return (0);

	return (1);
}

static int
equiv_plain(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	return (equiv_node(stdp->t_tdesc, ttdp->t_tdesc, ed));
}

static int
equiv_ptrauth(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	ptrauth_t *sp = stdp->t_ptrauth;
	ptrauth_t *tp = ttdp->t_ptrauth;

	if (sp->pta_key == tp->pta_key &&
	    sp->pta_discriminator == tp->pta_discriminator &&
	    sp->pta_discriminated == tp->pta_discriminated) {
		return equiv_node(sp->pta_type, tp->pta_type, ed);
	}
	return 0;
}

static int
equiv_function(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	fndef_t *fn1 = stdp->t_fndef, *fn2 = ttdp->t_fndef;
	int i;

	if (fn1->fn_nargs != fn2->fn_nargs ||
	    fn1->fn_vargs != fn2->fn_vargs)
		return (0);

	if (!equiv_node(fn1->fn_ret, fn2->fn_ret, ed))
		return (0);

	for (i = 0; i < fn1->fn_nargs; i++) {
		if (!equiv_node(fn1->fn_args[i], fn2->fn_args[i], ed))
			return (0);
	}

	return (1);
}

static int
equiv_array(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	ardef_t *ar1 = stdp->t_ardef, *ar2 = ttdp->t_ardef;

	if (!equiv_node(ar1->ad_contents, ar2->ad_contents, ed) ||
	    !equiv_node(ar1->ad_idxtype, ar2->ad_idxtype, ed))
		return (0);

	if (ar1->ad_nelems != ar2->ad_nelems)
		return (0);

	return (1);
}

static int
equiv_su(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	mlist_t *ml1 = stdp->t_members, *ml2 = ttdp->t_members;
	mlist_t *olm1 = NULL;

	while (ml1 && ml2) {
		if (ml1->ml_offset != ml2->ml_offset ||
		    ml1->ml_name != ml2->ml_name)
			return (0);

		/*
		 * Don't do the recursive equivalency checking more than
		 * we have to.
		 */
		if (olm1 == NULL || olm1->ml_type->t_id != ml1->ml_type->t_id) {
			if (ml1->ml_size != ml2->ml_size ||
			    !equiv_node(ml1->ml_type, ml2->ml_type, ed))
				return (0);
		}

		olm1 = ml1;
		ml1 = ml1->ml_next;
		ml2 = ml2->ml_next;
	}

	if (ml1 || ml2)
		return (0);

	return (1);
}

/*ARGSUSED2*/
static int
equiv_enum(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	elist_t *el1 = stdp->t_emem;
	elist_t *el2 = ttdp->t_emem;

	while (el1 && el2) {
		if (el1->el_number != el2->el_number ||
		    el1->el_name != el2->el_name)
			return (0);

		el1 = el1->el_next;
		el2 = el2->el_next;
	}

	if (el1 || el2)
		return (0);

	return (1);
}

/*ARGSUSED*/
static int
equiv_assert(tdesc_t *stdp, tdesc_t *ttdp, equiv_data_t *ed)
{
	/* foul, evil, and very bad - this is a "shouldn't happen" */
	assert(1 == 0);

	return (0);
}

static int
fwd_equiv(tdesc_t *ctdp, tdesc_t *mtdp)
{
	tdesc_t *defn = (ctdp->t_type == FORWARD ? mtdp : ctdp);

	return (defn->t_type == STRUCT || defn->t_type == UNION);
}

static int
equiv_node(tdesc_t *ctdp, tdesc_t *mtdp, equiv_data_t *ed)
{
	int (*equiv)(tdesc_t *ctdp, tdesc_t *mtdp, equiv_data_t *ed);
	int mapping;

	if (ctdp->t_emark > ed->ed_clear_mark ||
	    mtdp->t_emark > ed->ed_clear_mark)
		return (ctdp->t_emark == mtdp->t_emark);

	/*
	 * In normal (non-self-uniquify) mode, we don't want to do equivalency
	 * checking on a subgraph that has already been checked.  If a mapping
	 * has already been established for a given child node, we can simply
	 * compare the mapping for the child node with the ID of the parent
	 * node.  If we are in self-uniquify mode, then we're comparing two
	 * subgraphs within the child graph, and thus need to ignore any
	 * type mappings that have been created, as they are only valid into the
	 * parent.
	 */
	if ((mapping = get_mapping(ed->ed_ta, ctdp->t_id)) > 0 &&
	    mapping == mtdp->t_id && !ed->ed_selfuniquify)
		return (1);

	if (ctdp->t_name != mtdp->t_name)
		return (0);

	if (ctdp->t_type != mtdp->t_type) {
		if (ctdp->t_type == FORWARD || mtdp->t_type == FORWARD)
			return (fwd_equiv(ctdp, mtdp));
		else
			return (0);
	}

	ctdp->t_emark = ed->ed_cur_mark;
	mtdp->t_emark = ed->ed_cur_mark;
	ed->ed_cur_mark++;

	if ((equiv = tdesc_ops[ctdp->t_type].equiv) != NULL)
		return (equiv(ctdp, mtdp, ed));

	return (1);
}

/*
 * We perform an equivalency check on two subgraphs by traversing through them
 * in lockstep.  If a given node is equivalent in both the parent and the child,
 * we mark it in both subgraphs, using the t_emark field, with a monotonically
 * increasing number.  If, in the course of the traversal, we reach a node that
 * we have visited and numbered during this equivalency check, we have a cycle.
 * If the previously-visited nodes don't have the same emark, then the edges
 * that brought us to these nodes are not equivalent, and so the check ends.
 * If the emarks are the same, the edges are equivalent.  We then backtrack and
 * continue the traversal.  If we have exhausted all edges in the subgraph, and
 * have not found any inequivalent nodes, then the subgraphs are equivalent.
 */
static int
equiv_cb(void *bucket, void *arg)
{
	equiv_data_t *ed = arg;
	tdesc_t *mtdp = bucket;
	tdesc_t *ctdp = ed->ed_node;

	ed->ed_clear_mark = ed->ed_cur_mark + 1;
	ed->ed_cur_mark = ed->ed_clear_mark + 1;

	if (equiv_node(ctdp, mtdp, ed)) {
		debug(3, "equiv_node matched %d %d\n", ctdp->t_id, mtdp->t_id);
		ed->ed_tgt = mtdp;
		/* matched.  stop looking */
		return (-1);
	}

	return (0);
}

/*ARGSUSED1*/
static int
map_td_tree_pre(tdesc_t *ctdp, tdesc_t **ctdpp, void *private)
{
	merge_cb_data_t *mcd = private;

	if (get_mapping(mcd->md_ta, ctdp->t_id) > 0)
		return (0);

	return (1);
}

/*ARGSUSED1*/
static int
map_td_tree_post(tdesc_t *ctdp, tdesc_t **ctdpp, void *private)
{
	merge_cb_data_t *mcd = private;
	equiv_data_t ed;

	ed.ed_ta = mcd->md_ta;
	ed.ed_clear_mark = mcd->md_parent->td_curemark;
	ed.ed_cur_mark = mcd->md_parent->td_curemark + 1;
	ed.ed_node = ctdp;
	ed.ed_selfuniquify = 0;

	debug(3, "map_td_tree_post on %d %s\n", ctdp->t_id, tdesc_name(ctdp));

	if (hash_find_iter(mcd->md_parent->td_layouthash, ctdp,
	    equiv_cb, &ed) < 0) {
		/* We found an equivalent node */
		if (ed.ed_tgt->t_type == FORWARD && ctdp->t_type != FORWARD) {
			int id = mcd->md_tgt->td_nextid++;

			debug(3, "Creating new defn type %d\n", id);
			add_mapping(mcd->md_ta, ctdp->t_id, id);
			alist_add(mcd->md_fdida, (void *)(ulong_t)ed.ed_tgt,
			    (void *)(ulong_t)id);
			hash_add(mcd->md_tdtba, ctdp);
		} else
			add_mapping(mcd->md_ta, ctdp->t_id, ed.ed_tgt->t_id);

	} else if (debug_level > 1 && hash_iter(mcd->md_parent->td_idhash,
	    equiv_cb, &ed) < 0) {
		/*
		 * We didn't find an equivalent node by looking through the
		 * layout hash, but we somehow found it by performing an
		 * exhaustive search through the entire graph.  This usually
		 * means that the "name" hash function is broken.
		 */
		aborterr("Second pass for %d (%s) == %d\n", ctdp->t_id,
		    tdesc_name(ctdp), ed.ed_tgt->t_id);
	} else {
		int id = mcd->md_tgt->td_nextid++;

		debug(3, "Creating new type %d\n", id);
		add_mapping(mcd->md_ta, ctdp->t_id, id);
		hash_add(mcd->md_tdtba, ctdp);
	}

	mcd->md_parent->td_curemark = ed.ed_cur_mark + 1;

	return (1);
}

/*ARGSUSED1*/
static int
map_td_tree_self_post(tdesc_t *ctdp, tdesc_t **ctdpp, void *private)
{
	merge_cb_data_t *mcd = private;
	equiv_data_t ed;

	ed.ed_ta = mcd->md_ta;
	ed.ed_clear_mark = mcd->md_parent->td_curemark;
	ed.ed_cur_mark = mcd->md_parent->td_curemark + 1;
	ed.ed_node = ctdp;
	ed.ed_selfuniquify = 1;
	ed.ed_tgt = NULL;

	if (hash_find_iter(mcd->md_tdtba, ctdp, equiv_cb, &ed) < 0) {
		debug(3, "Self check found %d in %d\n", ctdp->t_id,
		    ed.ed_tgt->t_id);
		add_mapping(mcd->md_ta, ctdp->t_id,
		    get_mapping(mcd->md_ta, ed.ed_tgt->t_id));
	} else if (debug_level > 1 && hash_iter(mcd->md_tdtba,
	    equiv_cb, &ed) < 0) {
		/*
		 * We didn't find an equivalent node using the quick way (going
		 * through the hash normally), but we did find it by iterating
		 * through the entire hash.  This usually means that the hash
		 * function is broken.
		 */
		aborterr("Self-unique second pass for %d (%s) == %d\n",
		    ctdp->t_id, tdesc_name(ctdp), ed.ed_tgt->t_id);
	} else {
		int id = mcd->md_tgt->td_nextid++;

		debug(3, "Creating new type %d\n", id);
		add_mapping(mcd->md_ta, ctdp->t_id, id);
		hash_add(mcd->md_tdtba, ctdp);
	}

	mcd->md_parent->td_curemark = ed.ed_cur_mark + 1;

	return (1);
}

static tdtrav_cb_f map_pre[] = {
	NULL,
	map_td_tree_pre,	/* intrinsic */
	map_td_tree_pre,	/* pointer */
	map_td_tree_pre,	/* array */
	map_td_tree_pre,	/* function */
	map_td_tree_pre,	/* struct */
	map_td_tree_pre,	/* union */
	map_td_tree_pre,	/* enum */
	map_td_tree_pre,	/* forward */
	map_td_tree_pre,	/* typedef */
	tdtrav_assert,		/* typedef_unres */
	map_td_tree_pre,	/* volatile */
	map_td_tree_pre,	/* const */
	map_td_tree_pre,	/* restrict */
	map_td_tree_pre,	/* ptrauth */
};

static tdtrav_cb_f map_post[] = {
	NULL,
	map_td_tree_post,	/* intrinsic */
	map_td_tree_post,	/* pointer */
	map_td_tree_post,	/* array */
	map_td_tree_post,	/* function */
	map_td_tree_post,	/* struct */
	map_td_tree_post,	/* union */
	map_td_tree_post,	/* enum */
	map_td_tree_post,	/* forward */
	map_td_tree_post,	/* typedef */
	tdtrav_assert,		/* typedef_unres */
	map_td_tree_post,	/* volatile */
	map_td_tree_post,	/* const */
	map_td_tree_post,	/* restrict */
	map_td_tree_post	/* ptrauth */
};

static tdtrav_cb_f map_self_post[] = {
	NULL,
	map_td_tree_self_post,	/* intrinsic */
	map_td_tree_self_post,	/* pointer */
	map_td_tree_self_post,	/* array */
	map_td_tree_self_post,	/* function */
	map_td_tree_self_post,	/* struct */
	map_td_tree_self_post,	/* union */
	map_td_tree_self_post,	/* enum */
	map_td_tree_self_post,	/* forward */
	map_td_tree_self_post,	/* typedef */
	tdtrav_assert,		/* typedef_unres */
	map_td_tree_self_post,	/* volatile */
	map_td_tree_self_post,	/* const */
	map_td_tree_self_post,	/* restrict */
	map_td_tree_self_post	/* ptrauth */
};

/*
 * Determining equivalence of iidesc_t nodes
 */

typedef struct iifind_data {
	iidesc_t *iif_template;
	alist_t *iif_ta;
	int iif_newidx;
	int iif_refmerge;
} iifind_data_t;

/*
 * Check to see if this iidesc_t (node) - the current one on the list we're
 * iterating through - matches the target one (iif->iif_template).  Return -1
 * if it matches, to stop the iteration.
 */
static int
iidesc_match(void *data, void *arg)
{
	iidesc_t *node = data;
	iifind_data_t *iif = arg;
	int i;

	if (node->ii_type != iif->iif_template->ii_type ||
	    node->ii_name != iif->iif_template->ii_name ||
	    node->ii_dtype->t_id != iif->iif_newidx)
		return (0);

	if ((node->ii_type == II_SVAR || node->ii_type == II_SFUN) &&
	    node->ii_owner != iif->iif_template->ii_owner)
		return (0);

	if (node->ii_nargs != iif->iif_template->ii_nargs)
		return (0);

	for (i = 0; i < node->ii_nargs; i++) {
		if (get_mapping(iif->iif_ta,
		    iif->iif_template->ii_args[i]->t_id) !=
		    node->ii_args[i]->t_id)
			return (0);
	}

	if (iif->iif_refmerge) {
		switch (iif->iif_template->ii_type) {
		case II_GFUN:
		case II_SFUN:
		case II_GVAR:
		case II_SVAR:
			debug(3, "suppressing duping of %d %s from %s\n",
			    iif->iif_template->ii_type,
			    iif->iif_template->ii_name->value,
			    atom_pretty(iif->iif_template->ii_owner, "NULL"));
			return (0);
		case II_NOT:
		case II_PSYM:
		case II_SOU:
		case II_TYPE:
			break;
		}
	}

	return (-1);
}

static int
merge_type_cb(void *data, void *arg)
{
	iidesc_t *sii = data;
	merge_cb_data_t *mcd = arg;
	iifind_data_t iif;
	tdtrav_cb_f *post;

	post = (mcd->md_flags & MCD_F_SELFUNIQUIFY ? map_self_post : map_post);

	/* Map the tdesc nodes */
	(void) iitraverse(sii, &mcd->md_parent->td_curvgen, NULL, map_pre, post,
	    mcd);

	/* Map the iidesc nodes */
	iif.iif_template = sii;
	iif.iif_ta = mcd->md_ta;
	iif.iif_newidx = get_mapping(mcd->md_ta, sii->ii_dtype->t_id);
	iif.iif_refmerge = (mcd->md_flags & MCD_F_REFMERGE);

	if (hash_match(mcd->md_parent->td_iihash, sii, iidesc_match,
	    &iif) == 1)
		/* successfully mapped */
		return (1);

	debug(3, "tba %s (%d)\n", atom_pretty(sii->ii_name, "(anon)"),
	    sii->ii_type);

	array_add(&mcd->md_iitba, sii);

	return (0);
}

static int
remap_node(tdesc_t **tgtp, tdesc_t *oldtgt, int selftid, tdesc_t *newself,
    merge_cb_data_t *mcd)
{
	tdesc_t *tgt = NULL;
	tdesc_t template;
	int oldid = oldtgt->t_id;

	if (oldid == selftid) {
		*tgtp = newself;
		return (1);
	}

	if ((template.t_id = get_mapping(mcd->md_ta, oldid)) == 0)
		aborterr("failed to get mapping for tid %d\n", oldid);

	if (!hash_find(mcd->md_parent->td_idhash, (void *)&template,
	    (void *)&tgt) && (!(mcd->md_flags & MCD_F_REFMERGE) ||
	    !hash_find(mcd->md_tgt->td_idhash, (void *)&template,
	    (void *)&tgt))) {
		debug(3, "Remap couldn't find %d (from %d)\n", template.t_id,
		    oldid);
		*tgtp = oldtgt;
		array_add(&mcd->md_tdtbr, tgtp);
		return (0);
	}

	*tgtp = tgt;
	return (1);
}

static tdesc_t *
conjure_template(tdesc_t *old, int newselfid)
{
	tdesc_t *new = xcalloc(sizeof (tdesc_t));

	new->t_name = old->t_name;
	new->t_type = old->t_type;
	new->t_size = old->t_size;
	new->t_id = newselfid;
	new->t_flags = old->t_flags;

	return (new);
}

/*ARGSUSED2*/
static tdesc_t *
conjure_intrinsic(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);

	new->t_intr = xmalloc(sizeof (intr_t));
	bcopy(old->t_intr, new->t_intr, sizeof (intr_t));

	return (new);
}

static tdesc_t *
conjure_plain(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);

	(void) remap_node(&new->t_tdesc, old->t_tdesc, old->t_id, new, mcd);

	return (new);
}

static tdesc_t *
conjure_ptrauth(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);
	ptrauth_t *nptr = xmalloc(sizeof (ptrauth_t));
	ptrauth_t *optr = old->t_ptrauth;

	(void) remap_node(&nptr->pta_type, optr->pta_type, old->t_id, new,
	    mcd);

	nptr->pta_key = optr->pta_key;
	nptr->pta_discriminator = optr->pta_discriminator;
	nptr->pta_discriminated = optr->pta_discriminated;

	new->t_ptrauth = nptr;

	return (new);
}

static tdesc_t *
conjure_function(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);
	fndef_t *ofn = old->t_fndef;
	fndef_t *nfn = xmalloc(sizeof (fndef_t) + ofn->fn_nargs * sizeof(tdesc_t *));
	int i;

	(void) remap_node(&nfn->fn_ret, ofn->fn_ret, old->t_id, new, mcd);

	nfn->fn_nargs = ofn->fn_nargs;
	nfn->fn_vargs = ofn->fn_vargs;

	for (i = 0; i < ofn->fn_nargs; i++) {
		(void) remap_node(&nfn->fn_args[i], ofn->fn_args[i], old->t_id,
		    new, mcd);
	}

	new->t_fndef = nfn;

	return (new);
}

static tdesc_t *
conjure_array(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);
	ardef_t *nar = xmalloc(sizeof (ardef_t));
	ardef_t *oar = old->t_ardef;

	(void) remap_node(&nar->ad_contents, oar->ad_contents, old->t_id, new,
	    mcd);
	(void) remap_node(&nar->ad_idxtype, oar->ad_idxtype, old->t_id, new,
	    mcd);

	nar->ad_nelems = oar->ad_nelems;

	new->t_ardef = nar;

	return (new);
}

static tdesc_t *
conjure_su(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);
	mlist_t *omem, **nmemp;

	for (omem = old->t_members, nmemp = &new->t_members;
	    omem; omem = omem->ml_next, nmemp = &((*nmemp)->ml_next)) {
		*nmemp = xmalloc(sizeof (mlist_t));
		(*nmemp)->ml_offset = omem->ml_offset;
		(*nmemp)->ml_size = omem->ml_size;
		(*nmemp)->ml_name = omem->ml_name;
		(void) remap_node(&((*nmemp)->ml_type), omem->ml_type,
		    old->t_id, new, mcd);
	}
	*nmemp = NULL;

	return (new);
}

/*ARGSUSED2*/
static tdesc_t *
conjure_enum(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	tdesc_t *new = conjure_template(old, newselfid);
	elist_t *oel, **nelp;

	for (oel = old->t_emem, nelp = &new->t_emem;
	    oel; oel = oel->el_next, nelp = &((*nelp)->el_next)) {
		*nelp = xmalloc(sizeof (elist_t));
		(*nelp)->el_name = oel->el_name;
		(*nelp)->el_number = oel->el_number;
	}
	*nelp = NULL;

	return (new);
}

/*ARGSUSED2*/
static tdesc_t *
conjure_forward(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	return conjure_template(old, newselfid);
}

/*ARGSUSED*/
static tdesc_t *
conjure_assert(tdesc_t *old, int newselfid, merge_cb_data_t *mcd)
{
	assert(1 == 0);
	return (NULL);
}

static iidesc_t *
conjure_iidesc(iidesc_t *old, merge_cb_data_t *mcd)
{
	iidesc_t *new = iidesc_dup(old);
	int i;

	(void) remap_node(&new->ii_dtype, old->ii_dtype, -1, NULL, mcd);
	for (i = 0; i < new->ii_nargs; i++) {
		(void) remap_node(&new->ii_args[i], old->ii_args[i], -1, NULL,
		    mcd);
	}

	return (new);
}

static int
fwd_redir(tdesc_t *fwd, tdesc_t **fwdp, void *private)
{
	alist_t *map = private;
	tdesc_t *defn;

	if (!alist_find(map, (void *)fwd, (void **)&defn))
		return (0);

	debug(3, "Redirecting an edge to %s\n", tdesc_name(defn));

	*fwdp = defn;

	return (1);
}

static tdtrav_cb_f fwd_redir_cbs[] = {
	NULL,
	NULL,			/* intrinsic */
	NULL,			/* pointer */
	NULL,			/* array */
	NULL,			/* function */
	NULL,			/* struct */
	NULL,			/* union */
	NULL,			/* enum */
	fwd_redir,		/* forward */
	NULL,			/* typedef */
	tdtrav_assert,		/* typedef_unres */
	NULL,			/* volatile */
	NULL,			/* const */
	NULL,			/* restrict */
	NULL			/* ptrauth */
};

typedef struct redir_mstr_data {
	tdata_t *rmd_tgt;
	alist_t *rmd_map;
} redir_mstr_data_t;

static int
redir_mstr_fwd_cb(void *name, void *value, void *arg)
{
	tdesc_t *fwd = name;
	int defnid = (int)value;
	redir_mstr_data_t *rmd = arg;
	tdesc_t template;
	tdesc_t *defn;

	template.t_id = defnid;

	if (!hash_find(rmd->rmd_tgt->td_idhash, (void *)&template,
	    (void *)&defn)) {
		aborterr("Couldn't unforward %d (%s)\n", defnid,
		    tdesc_name(defn));
	}

	debug(3, "Forward map: resolved %d to %s\n", defnid, tdesc_name(defn));

	alist_add(rmd->rmd_map, (void *)fwd, (void *)defn);

	return (1);
}

static void
redir_mstr_fwds(merge_cb_data_t *mcd)
{
	redir_mstr_data_t rmd;
	alist_t *map = alist_new(ALIST_HASH_SIZE);

	rmd.rmd_tgt = mcd->md_tgt;
	rmd.rmd_map = map;

	if (alist_iter(mcd->md_fdida, redir_mstr_fwd_cb, &rmd)) {
		(void) iitraverse_hash(mcd->md_tgt->td_iihash,
		    &mcd->md_tgt->td_curvgen, fwd_redir_cbs, NULL, NULL, map);
	}

	alist_free(map);
}

static int
add_iitba_cb(void *data, void *private)
{
	merge_cb_data_t *mcd = private;
	iidesc_t *tba = data;
	iidesc_t *new;
	iifind_data_t iif;
	int newidx;

	newidx = get_mapping(mcd->md_ta, tba->ii_dtype->t_id);
	assert(newidx != -1);

	iif.iif_template = tba;
	iif.iif_ta = mcd->md_ta;
	iif.iif_newidx = newidx;
	iif.iif_refmerge = (mcd->md_flags & MCD_F_REFMERGE);

	if (hash_match(mcd->md_parent->td_iihash, tba, iidesc_match,
	    &iif) == 1) {
		debug(3, "iidesc_t %s already exists\n",
		    atom_pretty(tba->ii_name, "(anon)"));
		return (1);
	}

	new = conjure_iidesc(tba, mcd);
	hash_add(mcd->md_tgt->td_iihash, new);

	return (1);
}

static int
add_tdesc(tdesc_t *oldtdp, int newid, merge_cb_data_t *mcd)
{
	tdesc_t *newtdp;
	tdesc_t template;

	template.t_id = newid;
	assert(hash_find(mcd->md_parent->td_idhash,
	    (void *)&template, NULL) == 0);

	debug(3, "trying to conjure %d %s (%d) as %d\n",
	    oldtdp->t_type, tdesc_name(oldtdp), oldtdp->t_id, newid);

	if ((newtdp = tdesc_ops[oldtdp->t_type].conjure(oldtdp, newid,
	    mcd)) == NULL)
		/* couldn't map everything */
		return (0);

	debug(3, "succeeded\n");

	hash_add(mcd->md_tgt->td_idhash, newtdp);
	hash_add(mcd->md_tgt->td_layouthash, newtdp);

	return (1);
}

static int
add_tdtba_cb(void *data, void *arg)
{
	tdesc_t *tdp = data;
	merge_cb_data_t *mcd = arg;
	int newid;
	int rc;

	newid = get_mapping(mcd->md_ta, tdp->t_id);
	assert(newid != -1);

	if ((rc = add_tdesc(tdp, newid, mcd)))
		hash_remove(mcd->md_tdtba, (void *)tdp);

	return (rc);
}

static int
add_tdtbr_cb(void *data, void *arg)
{
	tdesc_t **tdpp = data;
	merge_cb_data_t *mcd = arg;

	debug(3, "Remapping %s (%d)\n", tdesc_name(*tdpp), (*tdpp)->t_id);

	if (!remap_node(tdpp, *tdpp, -1, NULL, mcd))
		return ARRAY_KEEP;

	return ARRAY_REMOVE;
}

static void
merge_types(hash_t *src, merge_cb_data_t *mcd)
{
	int iirc, tdrc;

	(void) hash_iter(src, merge_type_cb, mcd);

	tdrc = hash_iter(mcd->md_tdtba, add_tdtba_cb, (void *)mcd);
	debug(3, "add_tdtba_cb added %d items\n", tdrc);

	iirc = array_iter(mcd->md_iitba, add_iitba_cb, (void *)mcd);
	debug(3, "add_iitba_cb added %d items\n", iirc);

	assert(hash_count(mcd->md_tdtba) == 0);

	tdrc = array_filter(mcd->md_tdtbr, add_tdtbr_cb, (void *)mcd);
	debug(3, "add_tdtbr_cb added %d items\n", tdrc);

	if (array_count(mcd->md_tdtbr) != 0)
		aborterr("Couldn't remap all nodes\n");

	/*
	 * We now have an alist of master forwards and the ids of the new master
	 * definitions for those forwards in mcd->md_fdida.  By this point,
	 * we're guaranteed that all of the master definitions referenced in
	 * fdida have been added to the master tree.  We now traverse through
	 * the master tree, redirecting all edges inbound to forwards that have
	 * definitions to those definitions.
	 */
	if (mcd->md_parent == mcd->md_tgt) {
		redir_mstr_fwds(mcd);
	}
}

void
merge_cb_data_destroy(merge_cb_data_t *mcd)
{
	hash_free(mcd->md_tdtba, NULL, NULL);
	alist_free(mcd->md_fdida);
	alist_free(mcd->md_ta);
	array_free(&mcd->md_iitba, NULL, NULL);
	array_free(&mcd->md_tdtbr, NULL, NULL);
}

void
merge_into_master(merge_cb_data_t *mcd, tdata_t *cur, tdata_t *mstr,
    tdata_t *tgt, int selfuniquify)
{
	merge_cb_data_t mcd_buf = {0};

	cur->td_ref++;
	mstr->td_ref++;
	if (tgt)
		tgt->td_ref++;

	assert(cur->td_ref == 1 && mstr->td_ref == 1 &&
	    (tgt == NULL || tgt->td_ref == 1));

	if (mcd == NULL) {
		mcd = &mcd_buf;
	}
	mcd->md_parent = mstr;
	mcd->md_tgt = (tgt ? tgt : mstr);
	if (mcd->md_ta == NULL) { // this is an init
		mcd->md_tdtba = hash_new(TDATA_LAYOUT_HASH_SIZE, tdesc_layouthash,
		    tdesc_layoutcmp);
		mcd->md_ta = alist_new(ALIST_HASH_SIZE);
		mcd->md_fdida = alist_new(ALIST_HASH_SIZE);
	} else {
		if (hash_count(mcd->md_tdtba) != 0)
			terminate("The tdtba hash wasn't properly emptied");
		alist_clear(mcd->md_ta);
		alist_clear(mcd->md_fdida);
		array_clear(mcd->md_iitba, NULL, NULL);
		array_clear(mcd->md_tdtbr, NULL, NULL);
	}

	if (selfuniquify)
		mcd->md_flags |= MCD_F_SELFUNIQUIFY;
	if (tgt)
		mcd->md_flags |= MCD_F_REFMERGE;

	mstr->td_curvgen = MAX(mstr->td_curvgen, cur->td_curvgen);
	mstr->td_curemark = MAX(mstr->td_curemark, cur->td_curemark);

	merge_types(cur->td_iihash, mcd);

	if (debug_level >= 3) {
		debug(3, "Type association stats\n");
		alist_stats(mcd->md_ta, 0);
		debug(3, "Layout hash stats\n");
		hash_stats(mcd->md_tgt->td_layouthash, 1);
	}

	if (mcd == &mcd_buf) {
		merge_cb_data_destroy(mcd);
	}

	cur->td_ref--;
	mstr->td_ref--;
	if (tgt)
		tgt->td_ref--;
}

tdesc_ops_t tdesc_ops[] = {
	{ "ERROR! BAD tdesc TYPE", NULL, NULL },
	{ "intrinsic",		equiv_intrinsic,	conjure_intrinsic },
	{ "pointer", 		equiv_plain,		conjure_plain },
	{ "array", 		equiv_array,		conjure_array },
	{ "function", 		equiv_function,		conjure_function },
	{ "struct",		equiv_su,		conjure_su },
	{ "union",		equiv_su,		conjure_su },
	{ "enum",		equiv_enum,		conjure_enum },
	{ "forward",		NULL,			conjure_forward },
	{ "typedef",		equiv_plain,		conjure_plain },
	{ "typedef_unres",	equiv_assert,		conjure_assert },
	{ "volatile",		equiv_plain,		conjure_plain },
	{ "const", 		equiv_plain,		conjure_plain },
	{ "restrict",		equiv_plain,		conjure_plain },
	{ "ptrauth",		equiv_ptrauth,		conjure_ptrauth }
};


/*
 * Given tdata_t structures containing CTF data, merge and uniquify that data into
 * a single tdata_t.
 *
 * Merges can proceed independently.  As such, we perform the merges in parallel
 * using a worker thread model.  A given glob of CTF data (either all of the CTF
 * data from a single input file, or the result of one or more merges) can only
 * be involved in a single merge at any given time, so the process decreases in
 * parallelism, especially towards the end, as more and more files are
 * consolidated, finally resulting in a single merge of two large CTF graphs.
 * Unfortunately, the last merge is also the slowest, as the two graphs being
 * merged are each the product of merges of half of the input files.
 *
 * The algorithm consists of two phases, described in detail below.  The first
 * phase entails the merging of CTF data in groups of eight.  The second phase
 * takes the results of Phase I, and merges them two at a time.  This disparity
 * is due to an observation that the merge time increases at least quadratically
 * with the size of the CTF data being merged.  As such, merges of CTF graphs
 * newly read from input files are much faster than merges of CTF graphs that
 * are themselves the results of prior merges.
 *
 * A further complication is the need to ensure the repeatability of CTF merges.
 * That is, a merge should produce the same output every time, given the same
 * input.  In both phases, this consistency requirement is met by imposing an
 * ordering on the merge process, thus ensuring that a given set of input files
 * are merged in the same order every time.
 *
 *   Phase I
 *
 *   The main thread reads the input files one by one, transforming the CTF
 *   data they contain into tdata structures.  When a given file has been read
 *   and parsed, it is placed on the work queue for retrieval by worker threads.
 *
 *   Central to Phase I is the Work In Progress (wip) array, which is used to
 *   merge batches of files in a predictable order.  Files are read by the main
 *   thread, and are merged into wip array elements in round-robin order.  When
 *   the number of files merged into a given array slot equals the batch size,
 *   the merged CTF graph in that array is added to the done slot in order by
 *   array slot.
 *
 *   For example, consider a case where we have five input files, a batch size
 *   of two, a wip array size of two, and two worker threads (T1 and T2).
 *
 *    1. The wip array elements are assigned initial batch numbers 0 and 1.
 *    2. T1 reads an input file from the input queue (wq_queue).  This is the
 *       first input file, so it is placed into wip[0].  The second file is
 *       similarly read and placed into wip[1].  The wip array slots now contain
 *       one file each (wip_nmerged == 1).
 *    3. T1 reads the third input file, which it merges into wip[0].  The
 *       number of files in wip[0] is equal to the batch size.
 *    4. T2 reads the fourth input file, which it merges into wip[1].  wip[1]
 *       is now full too.
 *    5. T2 attempts to place the contents of wip[1] on the done queue
 *       (wq_done_queue), but it can't, since the batch ID for wip[1] is 1.
 *       Batch 0 needs to be on the done queue before batch 1 can be added, so
 *       T2 blocks on wip[1]'s cv.
 *    6. T1 attempts to place the contents of wip[0] on the done queue, and
 *       succeeds, updating wq_lastdonebatch to 0.  It clears wip[0], and sets
 *       its batch ID to 2.  T1 then signals wip[1]'s cv to awaken T2.
 *    7. T2 wakes up, notices that wq_lastdonebatch is 0, which means that
 *       batch 1 can now be added.  It adds wip[1] to the done queue, clears
 *       wip[1], and sets its batch ID to 3.  It signals wip[0]'s cv, and
 *       restarts.
 *
 *   The above process continues until all input files have been consumed.  At
 *   this point, a pair of barriers are used to allow a single thread to move
 *   any partial batches from the wip array to the done array in batch ID order.
 *   When this is complete, wq_done_queue is moved to wq_queue, and Phase II
 *   begins.
 *
 *	Locking Semantics (Phase I)
 *
 *	The input queue (wq_queue) and the done queue (wq_done_queue) are
 *	protected by separate mutexes - wq_queue_lock and wq_done_queue.  wip
 *	array slots are protected by their own mutexes, which must be grabbed
 *	before releasing the input queue lock.  The wip array lock is dropped
 *	when the thread restarts the loop.  If the array slot was full, the
 *	array lock will be held while the slot contents are added to the done
 *	queue.  The done queue lock is used to protect the wip slot cv's.
 *
 *	The pow number is protected by the queue lock.  The master batch ID
 *	and last completed batch (wq_lastdonebatch) counters are protected *in
 *	Phase I* by the done queue lock.
 *
 *   Phase II
 *
 *   When Phase II begins, the queue consists of the merged batches from the
 *   first phase.  Assume we have five batches:
 *
 *	Q:	a b c d e
 *
 *   Using the same batch ID mechanism we used in Phase I, but without the wip
 *   array, worker threads remove two entries at a time from the beginning of
 *   the queue.  These two entries are merged, and are added back to the tail
 *   of the queue, as follows:
 *
 *	Q:	a b c d e	# start
 *	Q:	c d e ab	# a, b removed, merged, added to end
 *	Q:	e ab cd		# c, d removed, merged, added to end
 *	Q:	cd eab		# e, ab removed, merged, added to end
 *	Q:	cdeab		# cd, eab removed, merged, added to end
 *
 *   When one entry remains on the queue, with no merges outstanding, Phase II
 *   finishes.  We pre-determine the stopping point by pre-calculating the
 *   number of nodes that will appear on the list.  In the example above, the
 *   number (wq_ninqueue) is 9.  When ninqueue is 1, we conclude Phase II by
 *   signaling the main thread via wq_done_cv.
 *
 *	Locking Semantics (Phase II)
 *
 *	The queue (wq_queue), ninqueue, and the master batch ID and last
 *	completed batch counters are protected by wq_queue_lock.  The done
 *	queue and corresponding lock are unused in Phase II as is the wip array.
 *
 *   Uniquification
 *
 *   We want the CTF data that goes into a given module to be as small as
 *   possible.  For example, we don't want it to contain any type data that may
 *   be present in another common module.  As such, after creating the master
 *   tdata_t for a given module, we can, if requested by the user, uniquify it
 *   against the tdata_t from another module (genunix in the case of the SunOS
 *   kernel).  We perform a merge between the tdata_t for this module and the
 *   tdata_t from genunix.  Nodes found in this module that are not present in
 *   genunix are added to a third tdata_t - the uniquified tdata_t.
 *
 *   Additive Merges
 *
 *   In some cases, for example if we are issuing a new version of a common
 *   module in a patch, we need to make sure that the CTF data already present
 *   in that module does not change.  Changes to this data would void the CTF
 *   data in any module that uniquified against the common module.  To preserve
 *   the existing data, we can perform what is known as an additive merge.  In
 *   this case, a final uniquification is performed against the CTF data in the
 *   previous version of the module.  The result will be the placement of new
 *   and changed data after the existing data, thus preserving the existing type
 *   ID space.
 *
 *   Saving the result
 *
 *   When the merges are complete, the resulting tdata_t is placed into the
 *   output file, replacing the .SUNW_ctf section (if any) already in that file.
 *
 * The person who changes the merging thread code in this file without updating
 * this comment will not live to see the stock hit five.
 */

#if !defined(__APPLE__) && !defined(__linux__)
#pragma init(bigheap)
static size_t maxpgsize = 0x400000;
#endif /* __APPLE__ */

#define	MERGE_PHASE1_BATCH_SIZE		8
#define	MERGE_PHASE1_MAX_SLOTS		5
#define	MERGE_INPUT_THROTTLE_LEN	10

#if !defined(__APPLE__) && !defined(__linux__)
static void
bigheap(void)
{
	size_t big, *size;
	int sizes;
	struct memcntl_mha mha;

	/*
	 * First, get the available pagesizes.
	 */
	if ((sizes = getpagesizes(NULL, 0)) == -1)
		return;

	if (sizes == 1 || (size = alloca(sizeof (size_t) * sizes)) == NULL)
		return;

	if (getpagesizes(size, sizes) == -1)
		return;

	while (size[sizes - 1] > maxpgsize)
		sizes--;

	/* set big to the largest allowed page size */
	big = size[sizes - 1];
	if (big & (big - 1)) {
		/*
		 * The largest page size is not a power of two for some
		 * inexplicable reason; return.
		 */
		return;
	}

	/*
	 * Now, align our break to the largest page size.
	 */
	if (brk((void *)((((uintptr_t)sbrk(0) - 1) & ~(big - 1)) + big)) != 0)
		return;

	/*
	 * set the preferred page size for the heap
	 */
	mha.mha_cmd = MHA_MAPSIZE_BSSBRK;
	mha.mha_flags = 0;
	mha.mha_pagesize = big;

	(void) memcntl(NULL, 0, MC_HAT_ADVISE, (caddr_t)&mha, 0, 0);
}
}
#else
static void
bigheap(void)
{
	/* NOOP */
}
#endif /* __APPLE__ */

static void
finalize_phase_one(workqueue_t *wq)
{
	int startslot, i;

	/*
	 * wip slots are cleared out only when maxbatchsz td's have been merged
	 * into them.  We're not guaranteed that the number of files we're
	 * merging is a multiple of maxbatchsz, so there will be some partial
	 * groups in the wip array.  Move them to the done queue in batch ID
	 * order, starting with the slot containing the next batch that would
	 * have been placed on the done queue, followed by the others.
	 * One thread will be doing this while the others wait at the barrier
	 * back in worker_thread(), so we don't need to worry about pesky things
	 * like locks.
	 */

	for (startslot = -1, i = 0; i < wq->wq_nwipslots; i++) {
		if (wq->wq_wip[i].wip_batchid == wq->wq_lastdonebatch + 1) {
			startslot = i;
			break;
		}
	}

	assert(startslot != -1);

	for (i = startslot; i < startslot + wq->wq_nwipslots; i++) {
		int slotnum = i % wq->wq_nwipslots;
		wip_t *wipslot = &wq->wq_wip[slotnum];

		if (wipslot->wip_td != NULL) {
			debug(2, "clearing slot %d (%d) (saving %d)\n",
			    slotnum, i, wipslot->wip_nmerged);
		} else
			debug(2, "clearing slot %d (%d)\n", slotnum, i);

		if (wipslot->wip_td != NULL) {
			fifo_add(wq->wq_donequeue, wipslot->wip_td);
			wq->wq_wip[slotnum].wip_td = NULL;
		}
	}

	wq->wq_lastdonebatch = wq->wq_next_batchid++;

	debug(2, "phase one done: donequeue has %d items\n",
	    fifo_len(wq->wq_donequeue));
}

static void
init_phase_two(workqueue_t *wq)
{
	int num;

	/*
	 * We're going to continually merge the first two entries on the queue,
	 * placing the result on the end, until there's nothing left to merge.
	 * At that point, everything will have been merged into one.  The
	 * initial value of ninqueue needs to be equal to the total number of
	 * entries that will show up on the queue, both at the start of the
	 * phase and as generated by merges during the phase.
	 */
	wq->wq_ninqueue = num = fifo_len(wq->wq_donequeue);
	while (num != 1) {
		wq->wq_ninqueue += num / 2;
		num = num / 2 + num % 2;
	}

	/*
	 * Move the done queue to the work queue.  We won't be using the done
	 * queue in phase 2.
	 */
	assert(fifo_len(wq->wq_queue) == 0);
	fifo_free(wq->wq_queue, NULL);
	wq->wq_queue = wq->wq_donequeue;
}

static void
wip_save_work(workqueue_t *wq, wip_t *slot, int slotnum)
{
	pthread_mutex_lock(&wq->wq_donequeue_lock);

	while (wq->wq_lastdonebatch + 1 < slot->wip_batchid)
		pthread_cond_wait(&slot->wip_cv, &wq->wq_donequeue_lock);
	assert(wq->wq_lastdonebatch + 1 == slot->wip_batchid);

	fifo_add(wq->wq_donequeue, slot->wip_td);
	wq->wq_lastdonebatch++;
	pthread_cond_signal(&wq->wq_wip[(slotnum + 1) %
	    wq->wq_nwipslots].wip_cv);

	/* reset the slot for next use */
	slot->wip_td = NULL;
	slot->wip_batchid = wq->wq_next_batchid++;

	pthread_mutex_unlock(&wq->wq_donequeue_lock);
}

static void
wip_add_work(merge_cb_data_t *mcd, wip_t *slot, tdata_t *pow)
{
	if (slot->wip_td == NULL) {
		slot->wip_td = pow;
		slot->wip_nmerged = 1;
	} else {
		debug(2, "%d: merging %p into %p\n", pthread_self(),
		    (void *)pow, (void *)slot->wip_td);

		merge_into_master(mcd, pow, slot->wip_td, NULL, 0);
		tdata_free(pow);

		slot->wip_nmerged++;
	}
}

static void
worker_runphase1(workqueue_t *wq, merge_cb_data_t *mcd)
{
	wip_t *wipslot;
	tdata_t *pow;
	int wipslotnum, pownum;

	for (;;) {
		pthread_mutex_lock(&wq->wq_queue_lock);

		while (fifo_empty(wq->wq_queue)) {
			if (wq->wq_nomorefiles == 1) {
				pthread_cond_signal(&wq->wq_work_avail);
				pthread_mutex_unlock(&wq->wq_queue_lock);

				/* on to phase 2 ... */
				return;
			}

			pthread_cond_wait(&wq->wq_work_avail,
			    &wq->wq_queue_lock);
		}

		/* there's work to be done! */
		pow = fifo_remove(wq->wq_queue);
		pownum = wq->wq_nextpownum++;
		pthread_cond_broadcast(&wq->wq_work_removed);

		assert(pow != NULL);

		/* merge it into the right slot */
		wipslotnum = pownum % wq->wq_nwipslots;
		wipslot = &wq->wq_wip[wipslotnum];

		pthread_mutex_lock(&wipslot->wip_lock);

		pthread_mutex_unlock(&wq->wq_queue_lock);

		wip_add_work(mcd, wipslot, pow);

		if (wipslot->wip_nmerged == wq->wq_maxbatchsz)
			wip_save_work(wq, wipslot, wipslotnum);

		pthread_mutex_unlock(&wipslot->wip_lock);
	}
}

static void
worker_runphase2(workqueue_t *wq, merge_cb_data_t *mcd)
{
	tdata_t *pow1, *pow2;
	int batchid;

	for (;;) {
		pthread_mutex_lock(&wq->wq_queue_lock);

		if (wq->wq_ninqueue == 1) {
			pthread_cond_signal(&wq->wq_work_avail);
			pthread_mutex_unlock(&wq->wq_queue_lock);

			debug(2, "%d: entering p2 completion barrier\n",
			    pthread_self());
			if (barrier_wait(&wq->wq_bar1)) {
				pthread_mutex_lock(&wq->wq_queue_lock);
				wq->wq_alldone = 1;
				pthread_cond_signal(&wq->wq_alldone_cv);
				pthread_mutex_unlock(&wq->wq_queue_lock);
			}

			return;
		}

		if (fifo_len(wq->wq_queue) < 2) {
			pthread_cond_wait(&wq->wq_work_avail,
			    &wq->wq_queue_lock);
			pthread_mutex_unlock(&wq->wq_queue_lock);
			continue;
		}

		/* there's work to be done! */
		pow1 = fifo_remove(wq->wq_queue);
		pow2 = fifo_remove(wq->wq_queue);
		wq->wq_ninqueue -= 2;

		batchid = wq->wq_next_batchid++;

		pthread_mutex_unlock(&wq->wq_queue_lock);

		debug(2, "%d: merging %p into %p\n", pthread_self(),
		    (void *)pow1, (void *)pow2);
		merge_into_master(mcd, pow1, pow2, NULL, 0);
		tdata_free(pow1);

		/*
		 * merging is complete.  place at the tail of the queue in
		 * proper order.
		 */
		pthread_mutex_lock(&wq->wq_queue_lock);
		while (wq->wq_lastdonebatch + 1 != batchid) {
			pthread_cond_wait(&wq->wq_done_cv,
			    &wq->wq_queue_lock);
		}

		wq->wq_lastdonebatch = batchid;

		fifo_add(wq->wq_queue, pow2);
		debug(2, "%d: added %p to queue, len now %d, ninqueue %d\n",
		    pthread_self(), (void *)pow2, fifo_len(wq->wq_queue),
		    wq->wq_ninqueue);
		pthread_cond_broadcast(&wq->wq_done_cv);
		pthread_cond_signal(&wq->wq_work_avail);
		pthread_mutex_unlock(&wq->wq_queue_lock);
	}
}

/*
 * Main loop for worker threads.
 */
static void
worker_thread(workqueue_t *wq)
{
	merge_cb_data_t mcd = {0};

	worker_runphase1(wq, &mcd);

	debug(2, "%d: entering first barrier\n", pthread_self());

	if (barrier_wait(&wq->wq_bar1)) {

		debug(2, "%d: doing work in first barrier\n", pthread_self());

		finalize_phase_one(wq);

		init_phase_two(wq);

		debug(2, "%d: ninqueue is %d, %d on queue\n", pthread_self(),
		    wq->wq_ninqueue, fifo_len(wq->wq_queue));
	}

	debug(2, "%d: entering second barrier\n", pthread_self());

	(void) barrier_wait(&wq->wq_bar2);

	debug(2, "%d: phase 1 complete\n", pthread_self());

	worker_runphase2(wq, &mcd);

	merge_cb_data_destroy(&mcd);
}

/*
 * Pass a tdata_t tree, built from an input file, off to the work queue for
 * consumption by worker threads.
 */
static int
worker_add_td(workqueue_t *wq, tdata_t *td, const char *name)
{
	debug(3, "Adding tdata %p for processing\n", (void *)td);

	pthread_mutex_lock(&wq->wq_queue_lock);
	while (fifo_len(wq->wq_queue) > wq->wq_ithrottle) {
		debug(2, "Throttling input (len = %d, throttle = %d)\n",
		    fifo_len(wq->wq_queue), wq->wq_ithrottle);
		pthread_cond_wait(&wq->wq_work_removed, &wq->wq_queue_lock);
	}

	fifo_add(wq->wq_queue, td);
	debug(1, "Thread %d announcing %s\n", pthread_self(), name);
	pthread_cond_signal(&wq->wq_work_avail);
	pthread_mutex_unlock(&wq->wq_queue_lock);

	return (1);
}

/*
 * This program is intended to be invoked from a Makefile, as part of the build.
 * As such, in the event of a failure or user-initiated interrupt (^C), we need
 * to ensure that a subsequent re-make will cause ctfmerge to be executed again.
 * Unfortunately, ctfmerge will usually be invoked directly after (and as part
 * of the same Makefile rule as) a link, and will operate on the linked file
 * in place.  If we merely exit upon receipt of a SIGINT, a subsequent make
 * will notice that the *linked* file is newer than the object files, and thus
 * will not reinvoke ctfmerge.  The only way to ensure that a subsequent make
 * reinvokes ctfmerge, is to remove the file to which we are adding CTF
 * data (confusingly named the output file).  This means that the link will need
 * to happen again, but links are generally fast, and we can't allow the merge
 * to be skipped.
 *
 * Another possibility would be to block SIGINT entirely - to always run to
 * completion.  The run time of ctfmerge can, however, be measured in minutes
 * in some cases, so this is not a valid option.
 */
static void
handle_sig(int sig)
{
	terminate("Caught signal %d - exiting\n", sig);
}


static void
wq_init(workqueue_t *wq, int nfiles)
{
	int throttle, nslots, i;

	if (getenv("CTFMERGE_MAX_SLOTS"))
		nslots = atoi(getenv("CTFMERGE_MAX_SLOTS"));
	else
		nslots = MERGE_PHASE1_MAX_SLOTS;

	if (getenv("CTFMERGE_PHASE1_BATCH_SIZE"))
		wq->wq_maxbatchsz = atoi(getenv("CTFMERGE_PHASE1_BATCH_SIZE"));
	else
		wq->wq_maxbatchsz = MERGE_PHASE1_BATCH_SIZE;

	nslots = MIN(nslots, (nfiles + wq->wq_maxbatchsz - 1) /
	    wq->wq_maxbatchsz);

	wq->wq_wip = xcalloc(sizeof (wip_t) * nslots);
	wq->wq_nwipslots = nslots;
	wq->wq_nthreads = MIN(sysconf(_SC_NPROCESSORS_ONLN) * 3 / 2, nslots);
	wq->wq_thread = xmalloc(sizeof (pthread_t) * wq->wq_nthreads);

	if (getenv("CTFMERGE_INPUT_THROTTLE"))
		throttle = atoi(getenv("CTFMERGE_INPUT_THROTTLE"));
	else
		throttle = MERGE_INPUT_THROTTLE_LEN;
	wq->wq_ithrottle = throttle * wq->wq_nthreads;

	debug(1, "Using %d slots, %d threads\n", wq->wq_nwipslots,
	    wq->wq_nthreads);

	wq->wq_next_batchid = 0;

	for (i = 0; i < nslots; i++) {
		pthread_mutex_init(&wq->wq_wip[i].wip_lock, NULL);
#if defined(__APPLE__)
		pthread_cond_init(&wq->wq_wip[i].wip_cv, NULL); /* Omitted on Solaris!?! */
#endif /* __APPLE__ */
		wq->wq_wip[i].wip_batchid = wq->wq_next_batchid++;
	}

	pthread_mutex_init(&wq->wq_queue_lock, NULL);
	wq->wq_queue = fifo_new();
	pthread_cond_init(&wq->wq_work_avail, NULL);
	pthread_cond_init(&wq->wq_work_removed, NULL);
	wq->wq_ninqueue = nfiles;
	wq->wq_nextpownum = 0;

	pthread_mutex_init(&wq->wq_donequeue_lock, NULL);
	wq->wq_donequeue = fifo_new();
	wq->wq_lastdonebatch = -1;

	pthread_cond_init(&wq->wq_done_cv, NULL);

	pthread_cond_init(&wq->wq_alldone_cv, NULL);
	wq->wq_alldone = 0;

	barrier_init(&wq->wq_bar1, wq->wq_nthreads);
	barrier_init(&wq->wq_bar2, wq->wq_nthreads);

	wq->wq_nomorefiles = 0;
}

static void
start_threads(workqueue_t *wq)
{
	sigset_t sets;
	int i;

	sigemptyset(&sets);
	sigaddset(&sets, SIGINT);
	sigaddset(&sets, SIGQUIT);
	sigaddset(&sets, SIGTERM);
	pthread_sigmask(SIG_BLOCK, &sets, NULL);

	for (i = 0; i < wq->wq_nthreads; i++) {
		pthread_create(&wq->wq_thread[i], NULL,
		    (void *(*)(void *))worker_thread, wq);
	}

	sigset(SIGINT, handle_sig);
	sigset(SIGQUIT, handle_sig);
	sigset(SIGTERM, handle_sig);
	pthread_sigmask(SIG_UNBLOCK, &sets, NULL);
}

static void
join_threads(workqueue_t *wq)
{
	int i;

	for (i = 0; i < wq->wq_nthreads; i++) {
		pthread_join(wq->wq_thread[i], NULL);
	}
}

/*
 * Core work queue structure; passed to worker threads on thread creation
 * as the main point of coordination.  Allocate as a static structure; we
 * could have put this into a local variable in main, but passing a pointer
 * into your stack to another thread is fragile at best and leads to some
 * hard-to-debug failure modes.
 */
static workqueue_t wq;

/*
 * Entry points for ctfconvert, ctfmerge
 */
void
ctfmerge_prepare(int nielems)
{
	/* Prepare for the merge */
	wq_init(&wq, nielems);
	start_threads(&wq);
}

int
ctfmerge_add_td(tdata_t *td, const char *name)
{
	return (worker_add_td(&wq, td, name));
}

tdata_t *
ctfmerge_done(void)
{
	tdata_t *mstrtd = NULL;

	pthread_mutex_lock(&wq.wq_queue_lock);
	wq.wq_nomorefiles = 1;
	pthread_cond_signal(&wq.wq_work_avail);
	pthread_mutex_unlock(&wq.wq_queue_lock);

	pthread_mutex_lock(&wq.wq_queue_lock);
	while (wq.wq_alldone == 0)
		pthread_cond_wait(&wq.wq_alldone_cv, &wq.wq_queue_lock);
	pthread_mutex_unlock(&wq.wq_queue_lock);

	join_threads(&wq);

	/*
	 * All requested files have been merged, with the resulting tree in
	 * mstrtd.
	 */
	assert(fifo_len(wq.wq_queue) == 1);
	mstrtd = fifo_remove(wq.wq_queue);

	return (mstrtd);
}
