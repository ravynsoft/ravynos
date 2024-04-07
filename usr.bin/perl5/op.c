#line 2 "op.c"
/*    op.c
 *
 *    Copyright (C) 1991, 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000,
 *    2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Larry Wall and others
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

/*
 * 'You see: Mr. Drogo, he married poor Miss Primula Brandybuck.  She was
 *  our Mr. Bilbo's first cousin on the mother's side (her mother being the
 *  youngest of the Old Took's daughters); and Mr. Drogo was his second
 *  cousin.  So Mr. Frodo is his first *and* second cousin, once removed
 *  either way, as the saying is, if you follow me.'       --the Gaffer
 *
 *     [p.23 of _The Lord of the Rings_, I/i: "A Long-Expected Party"]
 */

/* This file contains the functions that create and manipulate the OP
 * structures that hold a compiled perl program.
 *
 * Note that during the build of miniperl, a temporary copy of this file
 * is made, called opmini.c.
 *
 * A Perl program is compiled into a tree of OP nodes. Each op contains:
 *  * structural OP pointers to its children and siblings (op_sibling,
 *    op_first etc) that define the tree structure;
 *  * execution order OP pointers (op_next, plus sometimes op_other,
 *    op_lastop  etc) that define the execution sequence plus variants;
 *  * a pointer to the C "pp" function that would execute the op;
 *  * any data specific to that op.
 * For example, an OP_CONST op points to the pp_const() function and to an
 * SV containing the constant value. When pp_const() is executed, its job
 * is to push that SV onto the stack.
 *
 * OPs are mainly created by the newFOO() functions, which are mainly
 * called from the parser (in perly.y) as the code is parsed. For example
 * the Perl code $a + $b * $c would cause the equivalent of the following
 * to be called (oversimplifying a bit):
 *
 *  newBINOP(OP_ADD, flags,
 *	newSVREF($a),
 *	newBINOP(OP_MULTIPLY, flags, newSVREF($b), newSVREF($c))
 *  )
 *
 * As the parser reduces low-level rules, it creates little op subtrees;
 * as higher-level rules are resolved, these subtrees get joined together
 * as branches on a bigger subtree, until eventually a top-level rule like
 * a subroutine definition is reduced, at which point there is one large
 * parse tree left.
 *
 * The execution order pointers (op_next) are generated as the subtrees
 * are joined together. Consider this sub-expression: A*B + C/D: at the
 * point when it's just been parsed, the op tree looks like:
 *
 *   [+]
 *    |
 *   [*]------[/]
 *    |        |
 *    A---B    C---D
 *
 * with the intended execution order being:
 *
 *   [PREV] => A => B => [*] => C => D => [/] =>  [+] => [NEXT]
 *
 * At this point all the nodes' op_next pointers will have been set,
 * except that:
 *    * we don't know what the [NEXT] node will be yet;
 *    * we don't know what the [PREV] node will be yet, but when it gets
 *      created and needs its op_next set, it needs to be set to point to
 *      A, which is non-obvious.
 * To handle both those cases, we temporarily set the top node's
 * op_next to point to the first node to be executed in this subtree (A in
 * this case). This means that initially a subtree's op_next chain,
 * starting from the top node, will visit each node in execution sequence
 * then point back at the top node.
 * When we embed this subtree in a larger tree, its top op_next is used
 * to get the start node, then is set to point to its new neighbour.
 * For example the two separate [*],A,B and [/],C,D subtrees would
 * initially have had:
 *   [*] => A;  A => B;  B => [*]
 * and
 *   [/] => C;  C => D;  D => [/]
 * When these two subtrees were joined together to make the [+] subtree,
 * [+]'s op_next was set to [*]'s op_next, i.e. A; then [*]'s op_next was
 * set to point to [/]'s op_next, i.e. C.
 *
 * This op_next linking is done by the LINKLIST() macro and its underlying
 * op_linklist() function. Given a top-level op, if its op_next is
 * non-null, it's already been linked, so leave it. Otherwise link it with
 * its children as described above, possibly recursively if any of the
 * children have a null op_next.
 *
 * In summary: given a subtree, its top-level node's op_next will either
 * be:
 *   NULL: the subtree hasn't been LINKLIST()ed yet;
 *   fake: points to the start op for this subtree;
 *   real: once the subtree has been embedded into a larger tree
 */

/*

Here's an older description from Larry.

Perl's compiler is essentially a 3-pass compiler with interleaved phases:

    A bottom-up pass
    A top-down pass
    An execution-order pass

The bottom-up pass is represented by all the "newOP" routines and
the ck_ routines.  The bottom-upness is actually driven by yacc.
So at the point that a ck_ routine fires, we have no idea what the
context is, either upward in the syntax tree, or either forward or
backward in the execution order.  (The bottom-up parser builds that
part of the execution order it knows about, but if you follow the "next"
links around, you'll find it's actually a closed loop through the
top level node.)

Whenever the bottom-up parser gets to a node that supplies context to
its components, it invokes that portion of the top-down pass that applies
to that part of the subtree (and marks the top node as processed, so
if a node further up supplies context, it doesn't have to take the
plunge again).  As a particular subcase of this, as the new node is
built, it takes all the closed execution loops of its subcomponents
and links them into a new closed loop for the higher level node.  But
it's still not the real execution order.

The actual execution order is not known till we get a grammar reduction
to a top-level unit like a subroutine or file that will be called by
"name" rather than via a "next" pointer.  At that point, we can call
into peep() to do that code's portion of the 3rd pass.  It has to be
recursive, but it's recursive on basic blocks, not on tree nodes.
*/

/* To implement user lexical pragmas, there needs to be a way at run time to
   get the compile time state of %^H for that block.  Storing %^H in every
   block (or even COP) would be very expensive, so a different approach is
   taken.  The (running) state of %^H is serialised into a tree of HE-like
   structs.  Stores into %^H are chained onto the current leaf as a struct
   refcounted_he * with the key and the value.  Deletes from %^H are saved
   with a value of PL_sv_placeholder.  The state of %^H at any point can be
   turned back into a regular HV by walking back up the tree from that point's
   leaf, ignoring any key you've already seen (placeholder or not), storing
   the rest into the HV structure, then removing the placeholders. Hence
   memory is only used to store the %^H deltas from the enclosing COP, rather
   than the entire %^H on each COP.

   To cause actions on %^H to write out the serialisation records, it has
   magic type 'H'. This magic (itself) does nothing, but its presence causes
   the values to gain magic type 'h', which has entries for set and clear.
   C<Perl_magic_sethint> updates C<PL_compiling.cop_hints_hash> with a store
   record, with deletes written by C<Perl_magic_clearhint>. C<SAVEHINTS>
   saves the current C<PL_compiling.cop_hints_hash> on the save stack, so that
   it will be correctly restored when any inner compiling scope is exited.
*/

#include "EXTERN.h"
#define PERL_IN_OP_C
#include "perl.h"
#include "keywords.h"
#include "feature.h"
#include "regcomp.h"
#include "invlist_inline.h"

#define CALL_PEEP(o) PL_peepp(aTHX_ o)
#define CALL_OPFREEHOOK(o) if (PL_opfreehook) PL_opfreehook(aTHX_ o)

static const char array_passed_to_stat[] = "Array passed to stat will be coerced to a scalar";

/* remove any leading "empty" ops from the op_next chain whose first
 * node's address is stored in op_p. Store the updated address of the
 * first node in op_p.
 */

void
Perl_op_prune_chain_head(OP** op_p)
{
    PERL_ARGS_ASSERT_OP_PRUNE_CHAIN_HEAD;

    while (*op_p
        && (   (*op_p)->op_type == OP_NULL
            || (*op_p)->op_type == OP_SCOPE
            || (*op_p)->op_type == OP_SCALAR
            || (*op_p)->op_type == OP_LINESEQ)
    )
        *op_p = (*op_p)->op_next;
}


/* See the explanatory comments above struct opslab in op.h. */

#ifdef PERL_DEBUG_READONLY_OPS
#  define PERL_SLAB_SIZE 128
#  define PERL_MAX_SLAB_SIZE 4096
#  include <sys/mman.h>
#endif

#ifndef PERL_SLAB_SIZE
#  define PERL_SLAB_SIZE 64
#endif
#ifndef PERL_MAX_SLAB_SIZE
#  define PERL_MAX_SLAB_SIZE 2048
#endif

/* rounds up to nearest pointer */
#define SIZE_TO_PSIZE(x)	(((x) + sizeof(I32 *) - 1)/sizeof(I32 *))

#define DIFF(o,p)	\
    (assert(((char *)(p) - (char *)(o)) % sizeof(I32**) == 0), \
      ((size_t)((I32 **)(p) - (I32**)(o))))

/* requires double parens and aTHX_ */
#define DEBUG_S_warn(args)					       \
    DEBUG_S( 								\
        PerlIO_printf(Perl_debug_log, "%s", SvPVx_nolen(Perl_mess args)) \
    )

/* opslot_size includes the size of the slot header, and an op can't be smaller than BASEOP */
#define OPSLOT_SIZE_BASE (SIZE_TO_PSIZE(sizeof(OPSLOT)))

/* the number of bytes to allocate for a slab with sz * sizeof(I32 **) space for op */
#define OpSLABSizeBytes(sz) \
    ((sz) * sizeof(I32 *) + STRUCT_OFFSET(OPSLAB, opslab_slots))

/* malloc a new op slab (suitable for attaching to PL_compcv).
 * sz is in units of pointers from the beginning of opslab_opslots */

static OPSLAB *
S_new_slab(pTHX_ OPSLAB *head, size_t sz)
{
    OPSLAB *slab;
    size_t sz_bytes = OpSLABSizeBytes(sz);

    /* opslot_offset is only U16 */
    assert(sz < U16_MAX);
    /* room for at least one op */
    assert(sz >= OPSLOT_SIZE_BASE);

#ifdef PERL_DEBUG_READONLY_OPS
    slab = (OPSLAB *) mmap(0, sz_bytes,
                                   PROT_READ|PROT_WRITE,
                                   MAP_ANON|MAP_PRIVATE, -1, 0);
    DEBUG_m(PerlIO_printf(Perl_debug_log, "mapped %lu at %p\n",
                          (unsigned long) sz, slab));
    if (slab == MAP_FAILED) {
        perror("mmap failed");
        abort();
    }
#else
    slab = (OPSLAB *)PerlMemShared_malloc(sz_bytes);
    Zero(slab, sz_bytes, char);
#endif
    slab->opslab_size = (U16)sz;

#ifndef WIN32
    /* The context is unused in non-Windows */
    PERL_UNUSED_CONTEXT;
#endif
    slab->opslab_free_space = sz;
    slab->opslab_head = head ? head : slab;
    DEBUG_S_warn((aTHX_ "allocated new op slab sz 0x%x, %p, head slab %p",
        (unsigned int)slab->opslab_size, (void*)slab,
        (void*)(slab->opslab_head)));
    return slab;
}

#define OPSLOT_SIZE_TO_INDEX(sz) ((sz) - OPSLOT_SIZE_BASE)

#define link_freed_op(slab, o) S_link_freed_op(aTHX_ slab, o)
static void
S_link_freed_op(pTHX_ OPSLAB *slab, OP *o) {
    U16 sz = OpSLOT(o)->opslot_size;
    U16 index = OPSLOT_SIZE_TO_INDEX(sz);

    assert(sz >= OPSLOT_SIZE_BASE);
    /* make sure the array is large enough to include ops this large */
    if (!slab->opslab_freed) {
        /* we don't have a free list array yet, make a new one */
        slab->opslab_freed_size = index+1;
        slab->opslab_freed = (OP**)PerlMemShared_calloc((slab->opslab_freed_size), sizeof(OP*));

        if (!slab->opslab_freed)
            croak_no_mem();
    }
    else if (index >= slab->opslab_freed_size) {
        /* It's probably not worth doing exponential expansion here, the number of op sizes
           is small.
        */
        /* We already have a list that isn't large enough, expand it */
        size_t newsize = index+1;
        OP **p = (OP **)PerlMemShared_realloc(slab->opslab_freed, newsize * sizeof(OP*));

        if (!p)
            croak_no_mem();

        Zero(p+slab->opslab_freed_size, newsize - slab->opslab_freed_size, OP *);

        slab->opslab_freed = p;
        slab->opslab_freed_size = newsize;
    }

    o->op_next = slab->opslab_freed[index];
    slab->opslab_freed[index] = o;
}

/* Returns a sz-sized block of memory (suitable for holding an op) from
 * a free slot in the chain of op slabs attached to PL_compcv.
 * Allocates a new slab if necessary.
 * if PL_compcv isn't compiling, malloc() instead.
 */

void *
Perl_Slab_Alloc(pTHX_ size_t sz)
{
    OPSLAB *head_slab; /* first slab in the chain */
    OPSLAB *slab2;
    OPSLOT *slot;
    OP *o;
    size_t sz_in_p; /* size in pointer units, including the OPSLOT header */

    /* We only allocate ops from the slab during subroutine compilation.
       We find the slab via PL_compcv, hence that must be non-NULL. It could
       also be pointing to a subroutine which is now fully set up (CvROOT()
       pointing to the top of the optree for that sub), or a subroutine
       which isn't using the slab allocator. If our sanity checks aren't met,
       don't use a slab, but allocate the OP directly from the heap.  */
    if (!PL_compcv || CvROOT(PL_compcv)
     || (CvSTART(PL_compcv) && !CvSLABBED(PL_compcv)))
    {
        o = (OP*)PerlMemShared_calloc(1, sz);
        goto gotit;
    }

    /* While the subroutine is under construction, the slabs are accessed via
       CvSTART(), to avoid needing to expand PVCV by one pointer for something
       unneeded at runtime. Once a subroutine is constructed, the slabs are
       accessed via CvROOT(). So if CvSTART() is NULL, no slab has been
       allocated yet.  See the commit message for 8be227ab5eaa23f2 for more
       details.  */
    if (!CvSTART(PL_compcv)) {
        CvSTART(PL_compcv) =
            (OP *)(head_slab = S_new_slab(aTHX_ NULL, PERL_SLAB_SIZE));
        CvSLABBED_on(PL_compcv);
        head_slab->opslab_refcnt = 2; /* one for the CV; one for the new OP */
    }
    else ++(head_slab = (OPSLAB *)CvSTART(PL_compcv))->opslab_refcnt;

    sz_in_p = SIZE_TO_PSIZE(sz + OPSLOT_HEADER);

    /* The head slab for each CV maintains a free list of OPs. In particular, constant folding
       will free up OPs, so it makes sense to re-use them where possible. A
       freed up slot is used in preference to a new allocation.  */
    if (head_slab->opslab_freed &&
        OPSLOT_SIZE_TO_INDEX(sz_in_p) < head_slab->opslab_freed_size) {
        U16 base_index;

        /* look for a large enough size with any freed ops */
        for (base_index = OPSLOT_SIZE_TO_INDEX(sz_in_p);
             base_index < head_slab->opslab_freed_size && !head_slab->opslab_freed[base_index];
             ++base_index) {
        }

        if (base_index < head_slab->opslab_freed_size) {
            /* found a freed op */
            o = head_slab->opslab_freed[base_index];

            DEBUG_S_warn((aTHX_ "realloced  op at %p, slab %p, head slab %p",
                          (void *)o, (void *)OpMySLAB(o), (void *)head_slab));
            head_slab->opslab_freed[base_index] = o->op_next;
            Zero(o, sz, char);
            o->op_slabbed = 1;
            goto gotit;
        }
    }

#define INIT_OPSLOT(s) \
            slot->opslot_offset = DIFF(&slab2->opslab_slots, slot) ;	\
            slot->opslot_size = s;                      \
            slab2->opslab_free_space -= s;		\
            o = &slot->opslot_op;			\
            o->op_slabbed = 1

    /* The partially-filled slab is next in the chain. */
    slab2 = head_slab->opslab_next ? head_slab->opslab_next : head_slab;
    if (slab2->opslab_free_space < sz_in_p) {
        /* Remaining space is too small. */
        /* If we can fit a BASEOP, add it to the free chain, so as not
           to waste it. */
        if (slab2->opslab_free_space >= OPSLOT_SIZE_BASE) {
            slot = &slab2->opslab_slots;
            INIT_OPSLOT(slab2->opslab_free_space);
            o->op_type = OP_FREED;
            DEBUG_S_warn((aTHX_ "linked unused op space at %p, slab %p, head slab %p",
                          (void *)o, (void *)slab2, (void *)head_slab));
            link_freed_op(head_slab, o);
        }

        /* Create a new slab.  Make this one twice as big. */
        slab2 = S_new_slab(aTHX_ head_slab,
                            slab2->opslab_size  > PERL_MAX_SLAB_SIZE / 2
                                ? PERL_MAX_SLAB_SIZE
                                : slab2->opslab_size * 2);
        slab2->opslab_next = head_slab->opslab_next;
        head_slab->opslab_next = slab2;
    }
    assert(slab2->opslab_size >= sz_in_p);

    /* Create a new op slot */
    slot = OpSLOToff(slab2, slab2->opslab_free_space - sz_in_p);
    assert(slot >= &slab2->opslab_slots);
    INIT_OPSLOT(sz_in_p);
    DEBUG_S_warn((aTHX_ "allocating op at %p, slab %p, head slab %p",
        (void*)o, (void*)slab2, (void*)head_slab));

  gotit:
    /* moresib == 0, op_sibling == 0 implies a solitary unattached op */
    assert(!o->op_moresib);
    assert(!o->op_sibparent);

    return (void *)o;
}

#undef INIT_OPSLOT

#ifdef PERL_DEBUG_READONLY_OPS
void
Perl_Slab_to_ro(pTHX_ OPSLAB *slab)
{
    PERL_ARGS_ASSERT_SLAB_TO_RO;

    if (slab->opslab_readonly) return;
    slab->opslab_readonly = 1;
    for (; slab; slab = slab->opslab_next) {
        /*DEBUG_U(PerlIO_printf(Perl_debug_log,"mprotect ->ro %lu at %p\n",
                              (unsigned long) slab->opslab_size, (void *)slab));*/
        if (mprotect(slab, OpSLABSizeBytes(slab->opslab_size), PROT_READ))
            Perl_warn(aTHX_ "mprotect for %p %lu failed with %d", (void *)slab,
                             (unsigned long)slab->opslab_size, errno);
    }
}

void
Perl_Slab_to_rw(pTHX_ OPSLAB *const slab)
{
    OPSLAB *slab2;

    PERL_ARGS_ASSERT_SLAB_TO_RW;

    if (!slab->opslab_readonly) return;
    slab2 = slab;
    for (; slab2; slab2 = slab2->opslab_next) {
        /*DEBUG_U(PerlIO_printf(Perl_debug_log,"mprotect ->rw %lu at %p\n",
                              (unsigned long) size, (void *)slab2));*/
        if (mprotect((void *)slab2, OpSLABSizeBytes(slab2->opslab_size),
                     PROT_READ|PROT_WRITE)) {
            Perl_warn(aTHX_ "mprotect RW for %p %lu failed with %d", (void *)slab,
                             (unsigned long)slab2->opslab_size, errno);
        }
    }
    slab->opslab_readonly = 0;
}

#else
#  define Slab_to_rw(op)    NOOP
#endif

/* make freed ops die if they're inadvertently executed */
#ifdef DEBUGGING
static OP *
S_pp_freed(pTHX)
{
    DIE(aTHX_ "panic: freed op 0x%p called\n", PL_op);
}
#endif


/* Return the block of memory used by an op to the free list of
 * the OP slab associated with that op.
 */

void
Perl_Slab_Free(pTHX_ void *op)
{
    OP * const o = (OP *)op;
    OPSLAB *slab;

    PERL_ARGS_ASSERT_SLAB_FREE;

#ifdef DEBUGGING
    o->op_ppaddr = S_pp_freed;
#endif

    if (!o->op_slabbed) {
        if (!o->op_static)
            PerlMemShared_free(op);
        return;
    }

    slab = OpSLAB(o);
    /* If this op is already freed, our refcount will get screwy. */
    assert(o->op_type != OP_FREED);
    o->op_type = OP_FREED;
    link_freed_op(slab, o);
    DEBUG_S_warn((aTHX_ "freeing    op at %p, slab %p, head slab %p",
        (void*)o, (void *)OpMySLAB(o), (void*)slab));
    OpslabREFCNT_dec_padok(slab);
}

void
Perl_opslab_free_nopad(pTHX_ OPSLAB *slab)
{
    const bool havepad = cBOOL(PL_comppad);
    PERL_ARGS_ASSERT_OPSLAB_FREE_NOPAD;
    if (havepad) {
        ENTER;
        PAD_SAVE_SETNULLPAD();
    }
    opslab_free(slab);
    if (havepad) LEAVE;
}

/* Free a chain of OP slabs. Should only be called after all ops contained
 * in it have been freed. At this point, its reference count should be 1,
 * because OpslabREFCNT_dec() skips doing rc-- when it detects that rc == 1,
 * and just directly calls opslab_free().
 * (Note that the reference count which PL_compcv held on the slab should
 * have been removed once compilation of the sub was complete).
 *
 *
 */

void
Perl_opslab_free(pTHX_ OPSLAB *slab)
{
    OPSLAB *slab2;
    PERL_ARGS_ASSERT_OPSLAB_FREE;
    PERL_UNUSED_CONTEXT;
    DEBUG_S_warn((aTHX_ "freeing slab %p", (void*)slab));
    assert(slab->opslab_refcnt == 1);
    PerlMemShared_free(slab->opslab_freed);
    do {
        slab2 = slab->opslab_next;
#ifdef DEBUGGING
        slab->opslab_refcnt = ~(size_t)0;
#endif
#ifdef PERL_DEBUG_READONLY_OPS
        DEBUG_m(PerlIO_printf(Perl_debug_log, "Deallocate slab at %p\n",
                                               (void*)slab));
        if (munmap(slab, OpSLABSizeBytes(slab->opslab_size))) {
            perror("munmap failed");
            abort();
        }
#else
        PerlMemShared_free(slab);
#endif
        slab = slab2;
    } while (slab);
}

/* like opslab_free(), but first calls op_free() on any ops in the slab
 * not marked as OP_FREED
 */

void
Perl_opslab_force_free(pTHX_ OPSLAB *slab)
{
    OPSLAB *slab2;
#ifdef DEBUGGING
    size_t savestack_count = 0;
#endif
    PERL_ARGS_ASSERT_OPSLAB_FORCE_FREE;
    slab2 = slab;
    do {
        OPSLOT *slot = OpSLOToff(slab2, slab2->opslab_free_space);
        OPSLOT *end  = OpSLOToff(slab2, slab2->opslab_size);
        for (; slot < end;
                slot = (OPSLOT*) ((I32**)slot + slot->opslot_size) )
        {
            if (slot->opslot_op.op_type != OP_FREED
             && !(slot->opslot_op.op_savefree
#ifdef DEBUGGING
                  && ++savestack_count
#endif
                 )
            ) {
                assert(slot->opslot_op.op_slabbed);
                op_free(&slot->opslot_op);
                if (slab->opslab_refcnt == 1) goto free;
            }
        }
    } while ((slab2 = slab2->opslab_next));
    /* > 1 because the CV still holds a reference count. */
    if (slab->opslab_refcnt > 1) { /* still referenced by the savestack */
#ifdef DEBUGGING
        assert(savestack_count == slab->opslab_refcnt-1);
#endif
        /* Remove the CV’s reference count. */
        slab->opslab_refcnt--;
        return;
    }
   free:
    opslab_free(slab);
}

#ifdef PERL_DEBUG_READONLY_OPS
OP *
Perl_op_refcnt_inc(pTHX_ OP *o)
{
    if(o) {
        OPSLAB *const slab = o->op_slabbed ? OpSLAB(o) : NULL;
        if (slab && slab->opslab_readonly) {
            Slab_to_rw(slab);
            ++o->op_targ;
            Slab_to_ro(slab);
        } else {
            ++o->op_targ;
        }
    }
    return o;

}

PADOFFSET
Perl_op_refcnt_dec(pTHX_ OP *o)
{
    PADOFFSET result;
    OPSLAB *const slab = o->op_slabbed ? OpSLAB(o) : NULL;

    PERL_ARGS_ASSERT_OP_REFCNT_DEC;

    if (slab && slab->opslab_readonly) {
        Slab_to_rw(slab);
        result = --o->op_targ;
        Slab_to_ro(slab);
    } else {
        result = --o->op_targ;
    }
    return result;
}
#endif
/*
 * In the following definition, the ", (OP*)0" is just to make the compiler
 * think the expression is of the right type: croak actually does a Siglongjmp.
 */
#define CHECKOP(type,o) \
    ((PL_op_mask && PL_op_mask[type])				\
     ? ( op_free((OP*)o),					\
         Perl_croak(aTHX_ "'%s' trapped by operation mask", PL_op_desc[type]),	\
         (OP*)0 )						\
     : PL_check[type](aTHX_ (OP*)o))

#define RETURN_UNLIMITED_NUMBER (PERL_INT_MAX / 2)

STATIC OP *
S_no_fh_allowed(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_NO_FH_ALLOWED;

    yyerror(Perl_form(aTHX_ "Missing comma after first argument to %s function",
                 OP_DESC(o)));
    return o;
}

STATIC OP *
S_too_few_arguments_pv(pTHX_ OP *o, const char* name, U32 flags)
{
    PERL_ARGS_ASSERT_TOO_FEW_ARGUMENTS_PV;
    yyerror_pv(Perl_form(aTHX_ "Not enough arguments for %s", name), flags);
    return o;
}

STATIC OP *
S_too_many_arguments_pv(pTHX_ OP *o, const char *name, U32 flags)
{
    PERL_ARGS_ASSERT_TOO_MANY_ARGUMENTS_PV;

    yyerror_pv(Perl_form(aTHX_ "Too many arguments for %s", name), flags);
    return o;
}

STATIC void
S_bad_type_pv(pTHX_ I32 n, const char *t, const OP *o, const OP *kid)
{
    PERL_ARGS_ASSERT_BAD_TYPE_PV;

    yyerror_pv(Perl_form(aTHX_ "Type of arg %d to %s must be %s (not %s)",
                 (int)n, PL_op_desc[(o)->op_type], t, OP_DESC(kid)), 0);
}

STATIC void
S_bad_type_gv(pTHX_ I32 n, GV *gv, const OP *kid, const char *t)
{
    SV * const namesv = cv_name((CV *)gv, NULL, 0);
    PERL_ARGS_ASSERT_BAD_TYPE_GV;

    yyerror_pv(Perl_form(aTHX_ "Type of arg %d to %" SVf " must be %s (not %s)",
                 (int)n, SVfARG(namesv), t, OP_DESC(kid)), SvUTF8(namesv));
}

void
Perl_no_bareword_allowed(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_NO_BAREWORD_ALLOWED;

    qerror(Perl_mess(aTHX_
                     "Bareword \"%" SVf "\" not allowed while \"strict subs\" in use",
                     SVfARG(cSVOPo_sv)));
    o->op_private &= ~OPpCONST_STRICT; /* prevent warning twice about the same OP */
}

void
Perl_no_bareword_filehandle(pTHX_ const char *fhname) {
    PERL_ARGS_ASSERT_NO_BAREWORD_FILEHANDLE;

    if (strNE(fhname, "STDERR")
        && strNE(fhname, "STDOUT")
        && strNE(fhname, "STDIN")
        && strNE(fhname, "_")
        && strNE(fhname, "ARGV")
        && strNE(fhname, "ARGVOUT")
        && strNE(fhname, "DATA")) {
        qerror(Perl_mess(aTHX_ "Bareword filehandle \"%s\" not allowed under 'no feature \"bareword_filehandles\"'", fhname));
    }
}

/* "register" allocation */

PADOFFSET
Perl_allocmy(pTHX_ const char *const name, const STRLEN len, const U32 flags)
{
    PADOFFSET off;
    bool is_idfirst, is_default;
    const bool is_our = (PL_parser->in_my == KEY_our);

    PERL_ARGS_ASSERT_ALLOCMY;

    if (flags & ~SVf_UTF8)
        Perl_croak(aTHX_ "panic: allocmy illegal flag bits 0x%" UVxf,
                   (UV)flags);

    is_idfirst = flags & SVf_UTF8
        ? isIDFIRST_utf8_safe((U8*)name + 1, name + len)
        : isIDFIRST_A(name[1]);

    /* $_, @_, etc. */
    is_default = len == 2 && name[1] == '_';

    /* complain about "my $<special_var>" etc etc */
    if (!is_our && (!is_idfirst || is_default)) {
        const char * const type =
              PL_parser->in_my == KEY_sigvar ? "subroutine signature" :
              PL_parser->in_my == KEY_state  ? "\"state\""     : "\"my\"";

        if (!(flags & SVf_UTF8 && UTF8_IS_START(name[1]))
         && isASCII(name[1])
         && (!isPRINT(name[1]) || memCHRs("\t\n\r\f", name[1]))) {
            /* diag_listed_as: Can't use global %s in %s */
            yyerror(Perl_form(aTHX_ "Can't use global %c^%c%.*s in %s",
                              name[0], toCTRL(name[1]),
                              (int)(len - 2), name + 2,
                              type));
        } else {
            yyerror_pv(Perl_form(aTHX_ "Can't use global %.*s in %s",
                              (int) len, name,
                              type), flags & SVf_UTF8);
        }
    }

    /* allocate a spare slot and store the name in that slot */

    U32 addflags = 0;
    if(is_our)
        addflags |= padadd_OUR;
    else if(PL_parser->in_my == KEY_state)
        addflags |= padadd_STATE;
    else if(PL_parser->in_my == KEY_field)
        addflags |= padadd_FIELD;

    off = pad_add_name_pvn(name, len, addflags,
                    PL_parser->in_my_stash,
                    (is_our
                        /* $_ is always in main::, even with our */
                        ? (PL_curstash && !memEQs(name,len,"$_")
                            ? PL_curstash
                            : PL_defstash)
                        : NULL
                    )
    );
    /* anon sub prototypes contains state vars should always be cloned,
     * otherwise the state var would be shared between anon subs */

    if (PL_parser->in_my == KEY_state && CvANON(PL_compcv))
        CvCLONE_on(PL_compcv);

    return off;
}

/*
=for apidoc_section $optree_manipulation

=for apidoc alloccopstash

Available only under threaded builds, this function allocates an entry in
C<PL_stashpad> for the stash passed to it.

=cut
*/

#ifdef USE_ITHREADS
PADOFFSET
Perl_alloccopstash(pTHX_ HV *hv)
{
    PADOFFSET off = 0, o = 1;
    bool found_slot = FALSE;

    PERL_ARGS_ASSERT_ALLOCCOPSTASH;

    if (PL_stashpad[PL_stashpadix] == hv) return PL_stashpadix;

    for (; o < PL_stashpadmax; ++o) {
        if (PL_stashpad[o] == hv) return PL_stashpadix = o;
        if (!PL_stashpad[o] || SvTYPE(PL_stashpad[o]) != SVt_PVHV)
            found_slot = TRUE, off = o;
    }
    if (!found_slot) {
        Renew(PL_stashpad, PL_stashpadmax + 10, HV *);
        Zero(PL_stashpad + PL_stashpadmax, 10, HV *);
        off = PL_stashpadmax;
        PL_stashpadmax += 10;
    }

    PL_stashpad[PL_stashpadix = off] = hv;
    return off;
}
#endif

/* free the body of an op without examining its contents.
 * Always use this rather than FreeOp directly */

static void
S_op_destroy(pTHX_ OP *o)
{
    FreeOp(o);
}

/* Destructor */

/*
=for apidoc op_free

Free an op and its children. Only use this when an op is no longer linked
to from any optree.

Remember that any op with C<OPf_KIDS> set is expected to have a valid
C<op_first> pointer.  If you are attempting to free an op but preserve its
child op, make sure to clear that flag before calling C<op_free()>.  For
example:

    OP *kid = o->op_first; o->op_first = NULL;
    o->op_flags &= ~OPf_KIDS;
    op_free(o);

=cut
*/

void
Perl_op_free(pTHX_ OP *o)
{
    OPCODE type;
    OP *top_op = o;
    OP *next_op = o;
    bool went_up = FALSE; /* whether we reached the current node by
                            following the parent pointer from a child, and
                            so have already seen this node */

    if (!o || o->op_type == OP_FREED)
        return;

    if (o->op_private & OPpREFCOUNTED) {
        /* if base of tree is refcounted, just decrement */
        switch (o->op_type) {
        case OP_LEAVESUB:
        case OP_LEAVESUBLV:
        case OP_LEAVEEVAL:
        case OP_LEAVE:
        case OP_SCOPE:
        case OP_LEAVEWRITE:
            {
                PADOFFSET refcnt;
                OP_REFCNT_LOCK;
                refcnt = OpREFCNT_dec(o);
                OP_REFCNT_UNLOCK;
                if (refcnt) {
                    /* Need to find and remove any pattern match ops from
                     * the list we maintain for reset().  */
                    find_and_forget_pmops(o);
                    return;
                }
            }
            break;
        default:
            break;
        }
    }

    while (next_op) {
        o = next_op;

        /* free child ops before ourself, (then free ourself "on the
         * way back up") */

        /* Ensure the caller maintains the relationship between OPf_KIDS and
         * op_first != NULL when restructuring the tree
         *   https://github.com/Perl/perl5/issues/20764
         */
        assert(!(o->op_flags & OPf_KIDS) || cUNOPo->op_first);

        if (!went_up && o->op_flags & OPf_KIDS) {
            next_op = cUNOPo->op_first;
            continue;
        }

        /* find the next node to visit, *then* free the current node
         * (can't rely on o->op_* fields being valid after o has been
         * freed) */

        /* The next node to visit will be either the sibling, or the
         * parent if no siblings left, or NULL if we've worked our way
         * back up to the top node in the tree */
        next_op = (o == top_op) ? NULL : o->op_sibparent;
        went_up = cBOOL(!OpHAS_SIBLING(o)); /* parents are already visited */

        /* Now process the current node */

        /* Though ops may be freed twice, freeing the op after its slab is a
           big no-no. */
        assert(!o->op_slabbed || OpSLAB(o)->opslab_refcnt != ~(size_t)0);
        /* During the forced freeing of ops after compilation failure, kidops
           may be freed before their parents. */
        if (!o || o->op_type == OP_FREED)
            continue;

        type = o->op_type;

        /* an op should only ever acquire op_private flags that we know about.
         * If this fails, you may need to fix something in regen/op_private.
         * Don't bother testing if:
         *   * the op_ppaddr doesn't match the op; someone may have
         *     overridden the op and be doing strange things with it;
         *   * we've errored, as op flags are often left in an
         *     inconsistent state then. Note that an error when
         *     compiling the main program leaves PL_parser NULL, so
         *     we can't spot faults in the main code, only
         *     evaled/required code;
         *   * it's a banned op - we may be croaking before the op is
         *     fully formed. - see CHECKOP. */
#ifdef DEBUGGING
        if (   o->op_ppaddr == PL_ppaddr[type]
            && PL_parser
            && !PL_parser->error_count
            && !(PL_op_mask && PL_op_mask[type])
        )
        {
            assert(!(o->op_private & ~PL_op_private_valid[type]));
        }
#endif


        /* Call the op_free hook if it has been set. Do it now so that it's called
         * at the right time for refcounted ops, but still before all of the kids
         * are freed. */
        CALL_OPFREEHOOK(o);

        if (type == OP_NULL)
            type = (OPCODE)o->op_targ;

        if (o->op_slabbed)
            Slab_to_rw(OpSLAB(o));

        /* COP* is not cleared by op_clear() so that we may track line
         * numbers etc even after null() */
        if (type == OP_NEXTSTATE || type == OP_DBSTATE) {
            cop_free((COP*)o);
        }

        op_clear(o);
        FreeOp(o);
        if (PL_op == o)
            PL_op = NULL;
    }
}


/* S_op_clear_gv(): free a GV attached to an OP */

STATIC
#ifdef USE_ITHREADS
void S_op_clear_gv(pTHX_ OP *o, PADOFFSET *ixp)
#else
void S_op_clear_gv(pTHX_ OP *o, SV**svp)
#endif
{

    GV *gv = (o->op_type == OP_GV || o->op_type == OP_GVSV
            || o->op_type == OP_MULTIDEREF)
#ifdef USE_ITHREADS
                && PL_curpad
                ? ((GV*)PAD_SVl(*ixp)) : NULL;
#else
                ? (GV*)(*svp) : NULL;
#endif
    /* It's possible during global destruction that the GV is freed
       before the optree. Whilst the SvREFCNT_inc is happy to bump from
       0 to 1 on a freed SV, the corresponding SvREFCNT_dec from 1 to 0
       will trigger an assertion failure, because the entry to sv_clear
       checks that the scalar is not already freed.  A check of for
       !SvIS_FREED(gv) turns out to be invalid, because during global
       destruction the reference count can be forced down to zero
       (with SVf_BREAK set).  In which case raising to 1 and then
       dropping to 0 triggers cleanup before it should happen.  I
       *think* that this might actually be a general, systematic,
       weakness of the whole idea of SVf_BREAK, in that code *is*
       allowed to raise and lower references during global destruction,
       so any *valid* code that happens to do this during global
       destruction might well trigger premature cleanup.  */
    bool still_valid = gv && SvREFCNT(gv);

    if (still_valid)
        SvREFCNT_inc_simple_void(gv);
#ifdef USE_ITHREADS
    if (*ixp > 0) {
        pad_swipe(*ixp, TRUE);
        *ixp = 0;
    }
#else
    SvREFCNT_dec(*svp);
    *svp = NULL;
#endif
    if (still_valid) {
        int try_downgrade = SvREFCNT(gv) == 2;
        SvREFCNT_dec_NN(gv);
        if (try_downgrade)
            gv_try_downgrade(gv);
    }
}


void
Perl_op_clear(pTHX_ OP *o)
{


    PERL_ARGS_ASSERT_OP_CLEAR;

    switch (o->op_type) {
    case OP_NULL:	/* Was holding old type, if any. */
        /* FALLTHROUGH */
    case OP_ENTERTRY:
    case OP_ENTEREVAL:	/* Was holding hints. */
    case OP_ARGDEFELEM:	/* Was holding signature index. */
        o->op_targ = 0;
        break;
    default:
        if (!(o->op_flags & OPf_REF) || !OP_IS_STAT(o->op_type))
            break;
        /* FALLTHROUGH */
    case OP_GVSV:
    case OP_GV:
    case OP_AELEMFAST:
#ifdef USE_ITHREADS
            S_op_clear_gv(aTHX_ o, &(cPADOPx(o)->op_padix));
#else
            S_op_clear_gv(aTHX_ o, &(cSVOPx(o)->op_sv));
#endif
        break;
    case OP_METHOD_REDIR:
    case OP_METHOD_REDIR_SUPER:
#ifdef USE_ITHREADS
        if (cMETHOPo->op_rclass_targ) {
            pad_swipe(cMETHOPo->op_rclass_targ, 1);
            cMETHOPo->op_rclass_targ = 0;
        }
#else
        SvREFCNT_dec(cMETHOPo->op_rclass_sv);
        cMETHOPo->op_rclass_sv = NULL;
#endif
        /* FALLTHROUGH */
    case OP_METHOD_NAMED:
    case OP_METHOD_SUPER:
        SvREFCNT_dec(cMETHOPo->op_u.op_meth_sv);
        cMETHOPo->op_u.op_meth_sv = NULL;
#ifdef USE_ITHREADS
        if (o->op_targ) {
            pad_swipe(o->op_targ, 1);
            o->op_targ = 0;
        }
#endif
        break;
    case OP_CONST:
    case OP_HINTSEVAL:
        SvREFCNT_dec(cSVOPo->op_sv);
        cSVOPo->op_sv = NULL;
#ifdef USE_ITHREADS
        /** Bug #15654
          Even if op_clear does a pad_free for the target of the op,
          pad_free doesn't actually remove the sv that exists in the pad;
          instead it lives on. This results in that it could be reused as
          a target later on when the pad was reallocated.
        **/
        if(o->op_targ) {
          pad_swipe(o->op_targ,1);
          o->op_targ = 0;
        }
#endif
        break;
    case OP_DUMP:
    case OP_GOTO:
    case OP_NEXT:
    case OP_LAST:
    case OP_REDO:
        if (o->op_flags & (OPf_SPECIAL|OPf_STACKED|OPf_KIDS))
            break;
        /* FALLTHROUGH */
    case OP_TRANS:
    case OP_TRANSR:
        if (   (o->op_type == OP_TRANS || o->op_type == OP_TRANSR)
            && (o->op_private & OPpTRANS_USE_SVOP))
        {
#ifdef USE_ITHREADS
            if (cPADOPo->op_padix > 0) {
                pad_swipe(cPADOPo->op_padix, TRUE);
                cPADOPo->op_padix = 0;
            }
#else
            SvREFCNT_dec(cSVOPo->op_sv);
            cSVOPo->op_sv = NULL;
#endif
        }
        else {
            PerlMemShared_free(cPVOPo->op_pv);
            cPVOPo->op_pv = NULL;
        }
        break;
    case OP_SUBST:
        op_free(cPMOPo->op_pmreplrootu.op_pmreplroot);
        goto clear_pmop;

    case OP_SPLIT:
        if (     (o->op_private & OPpSPLIT_ASSIGN) /* @array  = split */
            && !(o->op_flags & OPf_STACKED))       /* @{expr} = split */
        {
            if (o->op_private & OPpSPLIT_LEX)
                pad_free(cPMOPo->op_pmreplrootu.op_pmtargetoff);
            else
#ifdef USE_ITHREADS
                pad_swipe(cPMOPo->op_pmreplrootu.op_pmtargetoff, TRUE);
#else
                SvREFCNT_dec(MUTABLE_SV(cPMOPo->op_pmreplrootu.op_pmtargetgv));
#endif
        }
        /* FALLTHROUGH */
    case OP_MATCH:
    case OP_QR:
    clear_pmop:
        if (!(cPMOPo->op_pmflags & PMf_CODELIST_PRIVATE))
            op_free(cPMOPo->op_code_list);
        cPMOPo->op_code_list = NULL;
        forget_pmop(cPMOPo);
        cPMOPo->op_pmreplrootu.op_pmreplroot = NULL;
        /* we use the same protection as the "SAFE" version of the PM_ macros
         * here since sv_clean_all might release some PMOPs
         * after PL_regex_padav has been cleared
         * and the clearing of PL_regex_padav needs to
         * happen before sv_clean_all
         */
#ifdef USE_ITHREADS
        if(PL_regex_pad) {        /* We could be in destruction */
            const IV offset = (cPMOPo)->op_pmoffset;
            ReREFCNT_dec(PM_GETRE(cPMOPo));
            PL_regex_pad[offset] = &PL_sv_undef;
            sv_catpvn_nomg(PL_regex_pad[0], (const char *)&offset,
                           sizeof(offset));
        }
#else
        ReREFCNT_dec(PM_GETRE(cPMOPo));
        PM_SETRE(cPMOPo, NULL);
#endif

        break;

    case OP_ARGCHECK:
        PerlMemShared_free(cUNOP_AUXo->op_aux);
        break;

    case OP_MULTICONCAT:
        {
            UNOP_AUX_item *aux = cUNOP_AUXo->op_aux;
            /* aux[PERL_MULTICONCAT_IX_PLAIN_PV] and/or
             * aux[PERL_MULTICONCAT_IX_UTF8_PV] point to plain and/or
             * utf8 shared strings */
            char *p1 = aux[PERL_MULTICONCAT_IX_PLAIN_PV].pv;
            char *p2 = aux[PERL_MULTICONCAT_IX_UTF8_PV].pv;
            if (p1)
                PerlMemShared_free(p1);
            if (p2 && p1 != p2)
                PerlMemShared_free(p2);
            PerlMemShared_free(aux);
        }
        break;

    case OP_MULTIDEREF:
        {
            UNOP_AUX_item *items = cUNOP_AUXo->op_aux;
            UV actions = items->uv;
            bool last = 0;
            bool is_hash = FALSE;

            while (!last) {
                switch (actions & MDEREF_ACTION_MASK) {

                case MDEREF_reload:
                    actions = (++items)->uv;
                    continue;

                case MDEREF_HV_padhv_helem:
                    is_hash = TRUE;
                    /* FALLTHROUGH */
                case MDEREF_AV_padav_aelem:
                    pad_free((++items)->pad_offset);
                    goto do_elem;

                case MDEREF_HV_gvhv_helem:
                    is_hash = TRUE;
                    /* FALLTHROUGH */
                case MDEREF_AV_gvav_aelem:
#ifdef USE_ITHREADS
                    S_op_clear_gv(aTHX_ o, &((++items)->pad_offset));
#else
                    S_op_clear_gv(aTHX_ o, &((++items)->sv));
#endif
                    goto do_elem;

                case MDEREF_HV_gvsv_vivify_rv2hv_helem:
                    is_hash = TRUE;
                    /* FALLTHROUGH */
                case MDEREF_AV_gvsv_vivify_rv2av_aelem:
#ifdef USE_ITHREADS
                    S_op_clear_gv(aTHX_ o, &((++items)->pad_offset));
#else
                    S_op_clear_gv(aTHX_ o, &((++items)->sv));
#endif
                    goto do_vivify_rv2xv_elem;

                case MDEREF_HV_padsv_vivify_rv2hv_helem:
                    is_hash = TRUE;
                    /* FALLTHROUGH */
                case MDEREF_AV_padsv_vivify_rv2av_aelem:
                    pad_free((++items)->pad_offset);
                    goto do_vivify_rv2xv_elem;

                case MDEREF_HV_pop_rv2hv_helem:
                case MDEREF_HV_vivify_rv2hv_helem:
                    is_hash = TRUE;
                    /* FALLTHROUGH */
                do_vivify_rv2xv_elem:
                case MDEREF_AV_pop_rv2av_aelem:
                case MDEREF_AV_vivify_rv2av_aelem:
                do_elem:
                    switch (actions & MDEREF_INDEX_MASK) {
                    case MDEREF_INDEX_none:
                        last = 1;
                        break;
                    case MDEREF_INDEX_const:
                        if (is_hash) {
#ifdef USE_ITHREADS
                            /* see RT #15654 */
                            pad_swipe((++items)->pad_offset, 1);
#else
                            SvREFCNT_dec((++items)->sv);
#endif
                        }
                        else
                            items++;
                        break;
                    case MDEREF_INDEX_padsv:
                        pad_free((++items)->pad_offset);
                        break;
                    case MDEREF_INDEX_gvsv:
#ifdef USE_ITHREADS
                        S_op_clear_gv(aTHX_ o, &((++items)->pad_offset));
#else
                        S_op_clear_gv(aTHX_ o, &((++items)->sv));
#endif
                        break;
                    }

                    if (actions & MDEREF_FLAG_last)
                        last = 1;
                    is_hash = FALSE;

                    break;

                default:
                    assert(0);
                    last = 1;
                    break;

                } /* switch */

                actions >>= MDEREF_SHIFT;
            } /* while */

            /* start of malloc is at op_aux[-1], where the length is
             * stored */
            PerlMemShared_free(cUNOP_AUXo->op_aux - 1);
        }
        break;

    case OP_METHSTART:
        {
            UNOP_AUX_item *aux = cUNOP_AUXo->op_aux;
            /* Every item in aux is a UV, so nothing in it to free */
            Safefree(aux);
        }
        break;

    case OP_INITFIELD:
        {
            UNOP_AUX_item *aux = cUNOP_AUXo->op_aux;
            /* Every item in aux is a UV, so nothing in it to free */
            Safefree(aux);
        }
        break;
    }

    if (o->op_targ > 0) {
        pad_free(o->op_targ);
        o->op_targ = 0;
    }
}

STATIC void
S_cop_free(pTHX_ COP* cop)
{
    PERL_ARGS_ASSERT_COP_FREE;

    /* If called during global destruction PL_defstash might be NULL and there
       shouldn't be any code running that will trip over the bad cop address.
       This also avoids uselessly creating the AV after it's been destroyed.
    */
    if (cop->op_type == OP_DBSTATE && PL_phase != PERL_PHASE_DESTRUCT) {
        /* Remove the now invalid op from the line number information.
           This could cause a freed memory overwrite if the debugger tried to
           set a breakpoint on this line.
        */
        AV *av = CopFILEAVn(cop);
        if (av) {
            SV * const * const svp = av_fetch(av, CopLINE(cop), FALSE);
            if (svp && *svp != &PL_sv_undef && SvIVX(*svp) == PTR2IV(cop) ) {
                (void)SvIOK_off(*svp);
                SvIV_set(*svp, 0);
            }
        }
    }
    CopFILE_free(cop);
    if (! specialWARN(cop->cop_warnings))
        cop->cop_warnings = rcpv_free(cop->cop_warnings);

    cophh_free(CopHINTHASH_get(cop));
    if (PL_curcop == cop)
       PL_curcop = NULL;
}

STATIC void
S_forget_pmop(pTHX_ PMOP *const o)
{
    HV * const pmstash = PmopSTASH(o);

    PERL_ARGS_ASSERT_FORGET_PMOP;

    if (pmstash && !SvIS_FREED(pmstash) && SvMAGICAL(pmstash)) {
        MAGIC * const mg = mg_find((const SV *)pmstash, PERL_MAGIC_symtab);
        if (mg) {
            PMOP **const array = (PMOP**) mg->mg_ptr;
            U32 count = mg->mg_len / sizeof(PMOP**);
            U32 i = count;

            while (i--) {
                if (array[i] == o) {
                    /* Found it. Move the entry at the end to overwrite it.  */
                    array[i] = array[--count];
                    mg->mg_len = count * sizeof(PMOP**);
                    /* Could realloc smaller at this point always, but probably
                       not worth it. Probably worth free()ing if we're the
                       last.  */
                    if(!count) {
                        Safefree(mg->mg_ptr);
                        mg->mg_ptr = NULL;
                    }
                    break;
                }
            }
        }
    }
    if (PL_curpm == o)
        PL_curpm = NULL;
}


STATIC void
S_find_and_forget_pmops(pTHX_ OP *o)
{
    OP* top_op = o;

    PERL_ARGS_ASSERT_FIND_AND_FORGET_PMOPS;

    while (1) {
        switch (o->op_type) {
        case OP_SUBST:
        case OP_SPLIT:
        case OP_MATCH:
        case OP_QR:
            forget_pmop(cPMOPo);
        }

        if (o->op_flags & OPf_KIDS) {
            o = cUNOPo->op_first;
            continue;
        }

        while (1) {
            if (o == top_op)
                return; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o)) {
                o = o->op_sibparent; /* process next sibling */
                break;
            }
            o = o->op_sibparent; /*try parent's next sibling */
        }
    }
}


/*
=for apidoc op_null

Neutralizes an op when it is no longer needed, but is still linked to from
other ops.

=cut
*/

void
Perl_op_null(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_OP_NULL;

    if (o->op_type == OP_NULL)
        return;
    op_clear(o);
    o->op_targ = o->op_type;
    OpTYPE_set(o, OP_NULL);
}

/*
=for apidoc op_refcnt_lock

Implements the C<OP_REFCNT_LOCK> macro which you should use instead.

=cut
*/

void
Perl_op_refcnt_lock(pTHX)
  PERL_TSA_ACQUIRE(PL_op_mutex)
{
    PERL_UNUSED_CONTEXT;
    OP_REFCNT_LOCK;
}

/*
=for apidoc op_refcnt_unlock

Implements the C<OP_REFCNT_UNLOCK> macro which you should use instead.

=cut
*/

void
Perl_op_refcnt_unlock(pTHX)
  PERL_TSA_RELEASE(PL_op_mutex)
{
    PERL_UNUSED_CONTEXT;
    OP_REFCNT_UNLOCK;
}


/*
=for apidoc op_sibling_splice

A general function for editing the structure of an existing chain of
op_sibling nodes.  By analogy with the perl-level C<splice()> function, allows
you to delete zero or more sequential nodes, replacing them with zero or
more different nodes.  Performs the necessary op_first/op_last
housekeeping on the parent node and op_sibling manipulation on the
children.  The last deleted node will be marked as the last node by
updating the op_sibling/op_sibparent or op_moresib field as appropriate.

Note that op_next is not manipulated, and nodes are not freed; that is the
responsibility of the caller.  It also won't create a new list op for an
empty list etc; use higher-level functions like op_append_elem() for that.

C<parent> is the parent node of the sibling chain. It may passed as C<NULL> if
the splicing doesn't affect the first or last op in the chain.

C<start> is the node preceding the first node to be spliced.  Node(s)
following it will be deleted, and ops will be inserted after it.  If it is
C<NULL>, the first node onwards is deleted, and nodes are inserted at the
beginning.

C<del_count> is the number of nodes to delete.  If zero, no nodes are deleted.
If -1 or greater than or equal to the number of remaining kids, all
remaining kids are deleted.

C<insert> is the first of a chain of nodes to be inserted in place of the nodes.
If C<NULL>, no nodes are inserted.

The head of the chain of deleted ops is returned, or C<NULL> if no ops were
deleted.

For example:

    action                    before      after         returns
    ------                    -----       -----         -------

                              P           P
    splice(P, A, 2, X-Y-Z)    |           |             B-C
                              A-B-C-D     A-X-Y-Z-D

                              P           P
    splice(P, NULL, 1, X-Y)   |           |             A
                              A-B-C-D     X-Y-B-C-D

                              P           P
    splice(P, NULL, 3, NULL)  |           |             A-B-C
                              A-B-C-D     D

                              P           P
    splice(P, B, 0, X-Y)      |           |             NULL
                              A-B-C-D     A-B-X-Y-C-D


For lower-level direct manipulation of C<op_sibparent> and C<op_moresib>,
see C<L</OpMORESIB_set>>, C<L</OpLASTSIB_set>>, C<L</OpMAYBESIB_set>>.

=cut
*/

OP *
Perl_op_sibling_splice(OP *parent, OP *start, int del_count, OP* insert)
{
    OP *first;
    OP *rest;
    OP *last_del = NULL;
    OP *last_ins = NULL;

    if (start)
        first = OpSIBLING(start);
    else if (!parent)
        goto no_parent;
    else
        first = cLISTOPx(parent)->op_first;

    assert(del_count >= -1);

    if (del_count && first) {
        last_del = first;
        while (--del_count && OpHAS_SIBLING(last_del))
            last_del = OpSIBLING(last_del);
        rest = OpSIBLING(last_del);
        OpLASTSIB_set(last_del, NULL);
    }
    else
        rest = first;

    if (insert) {
        last_ins = insert;
        while (OpHAS_SIBLING(last_ins))
            last_ins = OpSIBLING(last_ins);
        OpMAYBESIB_set(last_ins, rest, NULL);
    }
    else
        insert = rest;

    if (start) {
        OpMAYBESIB_set(start, insert, NULL);
    }
    else {
        assert(parent);
        cLISTOPx(parent)->op_first = insert;
        if (insert)
            parent->op_flags |= OPf_KIDS;
        else
            parent->op_flags &= ~OPf_KIDS;
    }

    if (!rest) {
        /* update op_last etc */
        U32 type;
        OP *lastop;

        if (!parent)
            goto no_parent;

        /* ought to use OP_CLASS(parent) here, but that can't handle
         * ex-foo OP_NULL ops. Also note that XopENTRYCUSTOM() can't
         * either */
        type = parent->op_type;
        if (type == OP_CUSTOM) {
            dTHX;
            type = XopENTRYCUSTOM(parent, xop_class);
        }
        else {
            if (type == OP_NULL)
                type = parent->op_targ;
            type = PL_opargs[type] & OA_CLASS_MASK;
        }

        lastop = last_ins ? last_ins : start ? start : NULL;
        if (   type == OA_BINOP
            || type == OA_LISTOP
            || type == OA_PMOP
            || type == OA_LOOP
        )
            cLISTOPx(parent)->op_last = lastop;

        if (lastop)
            OpLASTSIB_set(lastop, parent);
    }
    return last_del ? first : NULL;

  no_parent:
    Perl_croak_nocontext("panic: op_sibling_splice(): NULL parent");
}

/*
=for apidoc op_parent

Returns the parent OP of C<o>, if it has a parent. Returns C<NULL> otherwise.

=cut
*/

OP *
Perl_op_parent(OP *o)
{
    PERL_ARGS_ASSERT_OP_PARENT;
    while (OpHAS_SIBLING(o))
        o = OpSIBLING(o);
    return o->op_sibparent;
}

/* replace the sibling following start with a new UNOP, which becomes
 * the parent of the original sibling; e.g.
 *
 *  op_sibling_newUNOP(P, A, unop-args...)
 *
 *  P              P
 *  |      becomes |
 *  A-B-C          A-U-C
 *                   |
 *                   B
 *
 * where U is the new UNOP.
 *
 * parent and start args are the same as for op_sibling_splice();
 * type and flags args are as newUNOP().
 *
 * Returns the new UNOP.
 */

STATIC OP *
S_op_sibling_newUNOP(pTHX_ OP *parent, OP *start, I32 type, I32 flags)
{
    OP *kid, *newop;

    kid = op_sibling_splice(parent, start, 1, NULL);
    newop = newUNOP(type, flags, kid);
    op_sibling_splice(parent, start, 0, newop);
    return newop;
}


/* lowest-level newLOGOP-style function - just allocates and populates
 * the struct. Higher-level stuff should be done by S_new_logop() /
 * newLOGOP(). This function exists mainly to avoid op_first assignment
 * being spread throughout this file.
 */

LOGOP *
Perl_alloc_LOGOP(pTHX_ I32 type, OP *first, OP* other)
{
    LOGOP *logop;
    OP *kid = first;
    NewOp(1101, logop, 1, LOGOP);
    OpTYPE_set(logop, type);
    logop->op_first = first;
    logop->op_other = other;
    if (first)
        logop->op_flags = OPf_KIDS;
    while (kid && OpHAS_SIBLING(kid))
        kid = OpSIBLING(kid);
    if (kid)
        OpLASTSIB_set(kid, (OP*)logop);
    return logop;
}


/* Contextualizers */

/*
=for apidoc op_contextualize

Applies a syntactic context to an op tree representing an expression.
C<o> is the op tree, and C<context> must be C<G_SCALAR>, C<G_LIST>,
or C<G_VOID> to specify the context to apply.  The modified op tree
is returned.

=cut
*/

OP *
Perl_op_contextualize(pTHX_ OP *o, I32 context)
{
    PERL_ARGS_ASSERT_OP_CONTEXTUALIZE;
    switch (context) {
        case G_SCALAR: return scalar(o);
        case G_LIST:   return list(o);
        case G_VOID:   return scalarvoid(o);
        default:
            Perl_croak(aTHX_ "panic: op_contextualize bad context %ld",
                       (long) context);
    }
}

/*

=for apidoc op_linklist
This function is the implementation of the L</LINKLIST> macro.  It should
not be called directly.

=cut
*/


OP *
Perl_op_linklist(pTHX_ OP *o)
{

    OP **prevp;
    OP *kid;
    OP * top_op = o;

    PERL_ARGS_ASSERT_OP_LINKLIST;

    while (1) {
        /* Descend down the tree looking for any unprocessed subtrees to
         * do first */
        if (!o->op_next) {
            if (o->op_flags & OPf_KIDS) {
                o = cUNOPo->op_first;
                continue;
            }
            o->op_next = o; /* leaf node; link to self initially */
        }

        /* if we're at the top level, there either weren't any children
         * to process, or we've worked our way back to the top. */
        if (o == top_op)
            return o->op_next;

        /* o is now processed. Next, process any sibling subtrees */

        if (OpHAS_SIBLING(o)) {
            o = OpSIBLING(o);
            continue;
        }

        /* Done all the subtrees at this level. Go back up a level and
         * link the parent in with all its (processed) children.
         */

        o = o->op_sibparent;
        assert(!o->op_next);
        prevp = &(o->op_next);
        kid   = (o->op_flags & OPf_KIDS) ? cUNOPo->op_first : NULL;
        while (kid) {
            *prevp = kid->op_next;
            prevp = &(kid->op_next);
            kid = OpSIBLING(kid);
        }
        *prevp = o;
    }
}


static OP *
S_scalarkids(pTHX_ OP *o)
{
    if (o && o->op_flags & OPf_KIDS) {
        OP *kid;
        for (kid = cLISTOPo->op_first; kid; kid = OpSIBLING(kid))
            scalar(kid);
    }
    return o;
}

STATIC OP *
S_scalarboolean(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_SCALARBOOLEAN;

    if ((o->op_type == OP_SASSIGN && cBINOPo->op_first->op_type == OP_CONST &&
         !(cBINOPo->op_first->op_flags & OPf_SPECIAL)) ||
        (o->op_type == OP_NOT     && cUNOPo->op_first->op_type == OP_SASSIGN &&
         cBINOPx(cUNOPo->op_first)->op_first->op_type == OP_CONST &&
         !(cBINOPx(cUNOPo->op_first)->op_first->op_flags & OPf_SPECIAL))) {
        if (ckWARN(WARN_SYNTAX)) {
            const line_t oldline = CopLINE(PL_curcop);

            if (PL_parser && PL_parser->copline != NOLINE) {
                /* This ensures that warnings are reported at the first line
                   of the conditional, not the last.  */
                CopLINE_set(PL_curcop, PL_parser->copline);
            }
            Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "Found = in conditional, should be ==");
            CopLINE_set(PL_curcop, oldline);
        }
    }
    return scalar(o);
}

static SV *
S_op_varname_subscript(pTHX_ const OP *o, int subscript_type)
{
    assert(o);
    assert(o->op_type == OP_PADAV || o->op_type == OP_RV2AV ||
           o->op_type == OP_PADHV || o->op_type == OP_RV2HV);
    {
        const char funny  = o->op_type == OP_PADAV
                         || o->op_type == OP_RV2AV ? '@' : '%';
        if (o->op_type == OP_RV2AV || o->op_type == OP_RV2HV) {
            GV *gv;
            if (cUNOPo->op_first->op_type != OP_GV
             || !(gv = cGVOPx_gv(cUNOPo->op_first)))
                return NULL;
            return varname(gv, funny, 0, NULL, 0, subscript_type);
        }
        return
            varname(MUTABLE_GV(PL_compcv), funny, o->op_targ, NULL, 0, subscript_type);
    }
}

SV *
Perl_op_varname(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_OP_VARNAME;

    return S_op_varname_subscript(aTHX_ o, 1);
}

/*

Warns that an access of a single element from a named container variable in
scalar context might not be what the programmer wanted. The container
variable's (sigiled, full) name is given by C<name>, and the key to access
it is given by the C<SVOP_sv> of the C<OP_CONST> op given by C<o>.
C<is_hash> selects whether it prints using {KEY} or [KEY] brackets.

C<is_slice> selects between two different messages used in different places.
 */
void
Perl_warn_elem_scalar_context(pTHX_ const OP *o, SV *name, bool is_hash, bool is_slice)
{
    PERL_ARGS_ASSERT_WARN_ELEM_SCALAR_CONTEXT;

    SV *keysv = NULL;
    const char *keypv = NULL;

    const char lbrack = is_hash ? '{' : '[';
    const char rbrack = is_hash ? '}' : ']';

    if (o->op_type == OP_CONST) {
        keysv = cSVOPo_sv;
        if (SvPOK(keysv)) {
            SV *sv = keysv;
            keysv = sv_newmortal();
            pv_pretty(keysv, SvPVX_const(sv), SvCUR(sv), 32, NULL, NULL,
                      PERL_PV_PRETTY_DUMP |PERL_PV_ESCAPE_UNI_DETECT);
        }
        else if (!SvOK(keysv))
            keypv = "undef";
    }
    else keypv = "...";

    assert(SvPOK(name));
    sv_chop(name,SvPVX(name)+1);

    const char *msg;

    if (keypv) {
        msg = is_slice ?
            /* diag_listed_as: Scalar value @%s[%s] better written as $%s[%s] */
            PERL_DIAG_WARN_SYNTAX(
                "Scalar value @%" SVf "%c%s%c better written as $%" SVf "%c%s%c") :
            /* diag_listed_as: %%s[%s] in scalar context better written as $%s[%s] */
            PERL_DIAG_WARN_SYNTAX(
                "%%%" SVf "%c%s%c in scalar context better written as $%" SVf "%c%s%c");

        Perl_warner(aTHX_ packWARN(WARN_SYNTAX), msg,
                SVfARG(name), lbrack, keypv, rbrack,
                SVfARG(name), lbrack, keypv, rbrack);
    }
    else {
        msg = is_slice ?
            /* diag_listed_as: Scalar value @%s[%s] better written as $%s[%s] */
            PERL_DIAG_WARN_SYNTAX(
                "Scalar value @%" SVf "%c%" SVf "%c better written as $%" SVf "%c%" SVf "%c") :
            /* diag_listed_as: %%s[%s] in scalar context better written as $%s[%s] */
            PERL_DIAG_WARN_SYNTAX(
                "%%%" SVf "%c%" SVf "%c in scalar context better written as $%" SVf "%c%" SVf "%c");

        Perl_warner(aTHX_ packWARN(WARN_SYNTAX), msg,
                SVfARG(name), lbrack, SVfARG(keysv), rbrack,
                SVfARG(name), lbrack, SVfARG(keysv), rbrack);
    }
}


/* apply scalar context to the o subtree */

OP *
Perl_scalar(pTHX_ OP *o)
{
    OP * top_op = o;

    while (1) {
        OP *next_kid = NULL; /* what op (if any) to process next */
        OP *kid;

        /* assumes no premature commitment */
        if (!o || (PL_parser && PL_parser->error_count)
             || (o->op_flags & OPf_WANT)
             || o->op_type == OP_RETURN)
        {
            goto do_next;
        }

        o->op_flags = (o->op_flags & ~OPf_WANT) | OPf_WANT_SCALAR;

        switch (o->op_type) {
        case OP_REPEAT:
            scalar(cBINOPo->op_first);
            /* convert what initially looked like a list repeat into a
             * scalar repeat, e.g. $s = (1) x $n
             */
            if (o->op_private & OPpREPEAT_DOLIST) {
                kid = cLISTOPx(cUNOPo->op_first)->op_first;
                assert(kid->op_type == OP_PUSHMARK);
                if (OpHAS_SIBLING(kid) && !OpHAS_SIBLING(OpSIBLING(kid))) {
                    op_null(cLISTOPx(cUNOPo->op_first)->op_first);
                    o->op_private &=~ OPpREPEAT_DOLIST;
                }
            }
            break;

        case OP_OR:
        case OP_AND:
        case OP_COND_EXPR:
            /* impose scalar context on everything except the condition */
            next_kid = OpSIBLING(cUNOPo->op_first);
            break;

        default:
            if (o->op_flags & OPf_KIDS)
                next_kid = cUNOPo->op_first; /* do all kids */
            break;

        /* the children of these ops are usually a list of statements,
         * except the leaves, whose first child is a corresponding enter
         */
        case OP_SCOPE:
        case OP_LINESEQ:
        case OP_LIST:
            kid = cLISTOPo->op_first;
            goto do_kids;
        case OP_LEAVE:
        case OP_LEAVETRY:
            kid = cLISTOPo->op_first;
            scalar(kid);
            kid = OpSIBLING(kid);
        do_kids:
            while (kid) {
                OP *sib = OpSIBLING(kid);
                /* Apply void context to all kids except the last, which
                 * is scalar (ignoring a trailing ex-nextstate in determining
                 * if it's the last kid). E.g.
                 *      $scalar = do { void; void; scalar }
                 * Except that 'when's are always scalar, e.g.
                 *      $scalar = do { given(..) {
                    *                 when (..) { scalar }
                    *                 when (..) { scalar }
                    *                 ...
                    *                }}
                    */
                if (!sib
                     || (  !OpHAS_SIBLING(sib)
                         && sib->op_type == OP_NULL
                         && (   sib->op_targ == OP_NEXTSTATE
                             || sib->op_targ == OP_DBSTATE  )
                        )
                )
                {
                    /* tail call optimise calling scalar() on the last kid */
                    next_kid = kid;
                    goto do_next;
                }
                else if (kid->op_type == OP_LEAVEWHEN)
                    scalar(kid);
                else
                    scalarvoid(kid);
                kid = sib;
            }
            NOT_REACHED; /* NOTREACHED */
            break;

        case OP_SORT:
            Perl_ck_warner(aTHX_ packWARN(WARN_SCALAR), "Useless use of %s in scalar context", "sort");
            break;

        case OP_KVHSLICE:
        case OP_KVASLICE:
        {
            /* Warn about scalar context */
            SV *name;

            /* This warning can be nonsensical when there is a syntax error. */
            if (PL_parser && PL_parser->error_count)
                break;

            if (!ckWARN(WARN_SYNTAX)) break;

            kid = cLISTOPo->op_first;
            kid = OpSIBLING(kid); /* get past pushmark */
            assert(OpSIBLING(kid));
            name = op_varname(OpSIBLING(kid));
            if (!name) /* XS module fiddling with the op tree */
                break;
            warn_elem_scalar_context(kid, name, o->op_type == OP_KVHSLICE, false);
        }
        } /* switch */

        /* If next_kid is set, someone in the code above wanted us to process
         * that kid and all its remaining siblings.  Otherwise, work our way
         * back up the tree */
      do_next:
        while (!next_kid) {
            if (o == top_op)
                return top_op; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o))
                next_kid = o->op_sibparent;
            else {
                o = o->op_sibparent; /*try parent's next sibling */
                switch (o->op_type) {
                case OP_SCOPE:
                case OP_LINESEQ:
                case OP_LIST:
                case OP_LEAVE:
                case OP_LEAVETRY:
                    /* should really restore PL_curcop to its old value, but
                     * setting it to PL_compiling is better than do nothing */
                    PL_curcop = &PL_compiling;
                }
            }
        }
        o = next_kid;
    } /* while */
}


/* apply void context to the optree arg */

OP *
Perl_scalarvoid(pTHX_ OP *arg)
{
    OP *kid;
    SV* sv;
    OP *o = arg;

    PERL_ARGS_ASSERT_SCALARVOID;

    while (1) {
        U8 want;
        SV *useless_sv = NULL;
        const char* useless = NULL;
        OP * next_kid = NULL;

        if (o->op_type == OP_NEXTSTATE
            || o->op_type == OP_DBSTATE
            || (o->op_type == OP_NULL && (o->op_targ == OP_NEXTSTATE
                                          || o->op_targ == OP_DBSTATE)))
            PL_curcop = (COP*)o;                /* for warning below */

        /* assumes no premature commitment */
        want = o->op_flags & OPf_WANT;
        if ((want && want != OPf_WANT_SCALAR)
            || (PL_parser && PL_parser->error_count)
            || o->op_type == OP_RETURN || o->op_type == OP_REQUIRE || o->op_type == OP_LEAVEWHEN)
        {
            goto get_next_op;
        }

        if ((o->op_private & OPpTARGET_MY)
            && (PL_opargs[o->op_type] & OA_TARGLEX))/* OPp share the meaning */
        {
            /* newASSIGNOP has already applied scalar context, which we
               leave, as if this op is inside SASSIGN.  */
            goto get_next_op;
        }

        o->op_flags = (o->op_flags & ~OPf_WANT) | OPf_WANT_VOID;

        switch (o->op_type) {
        default:
            if (!(PL_opargs[o->op_type] & OA_FOLDCONST))
                break;
            /* FALLTHROUGH */
        case OP_REPEAT:
            if (o->op_flags & OPf_STACKED)
                break;
            if (o->op_type == OP_REPEAT)
                scalar(cBINOPo->op_first);
            goto func_ops;
        case OP_CONCAT:
            if ((o->op_flags & OPf_STACKED) &&
                    !(o->op_private & OPpCONCAT_NESTED))
                break;
            goto func_ops;
        case OP_SUBSTR:
            if (o->op_private == 4)
                break;
            /* FALLTHROUGH */
        case OP_WANTARRAY:
        case OP_GV:
        case OP_SMARTMATCH:
        case OP_AV2ARYLEN:
        case OP_REF:
        case OP_REFGEN:
        case OP_SREFGEN:
        case OP_ANONCODE:
        case OP_DEFINED:
        case OP_HEX:
        case OP_OCT:
        case OP_LENGTH:
        case OP_VEC:
        case OP_INDEX:
        case OP_RINDEX:
        case OP_SPRINTF:
        case OP_KVASLICE:
        case OP_KVHSLICE:
        case OP_UNPACK:
        case OP_PACK:
        case OP_JOIN:
        case OP_LSLICE:
        case OP_ANONLIST:
        case OP_ANONHASH:
        case OP_SORT:
        case OP_REVERSE:
        case OP_RANGE:
        case OP_FLIP:
        case OP_FLOP:
        case OP_CALLER:
        case OP_FILENO:
        case OP_EOF:
        case OP_TELL:
        case OP_GETSOCKNAME:
        case OP_GETPEERNAME:
        case OP_READLINK:
        case OP_TELLDIR:
        case OP_GETPPID:
        case OP_GETPGRP:
        case OP_GETPRIORITY:
        case OP_TIME:
        case OP_TMS:
        case OP_LOCALTIME:
        case OP_GMTIME:
        case OP_GHBYNAME:
        case OP_GHBYADDR:
        case OP_GHOSTENT:
        case OP_GNBYNAME:
        case OP_GNBYADDR:
        case OP_GNETENT:
        case OP_GPBYNAME:
        case OP_GPBYNUMBER:
        case OP_GPROTOENT:
        case OP_GSBYNAME:
        case OP_GSBYPORT:
        case OP_GSERVENT:
        case OP_GPWNAM:
        case OP_GPWUID:
        case OP_GGRNAM:
        case OP_GGRGID:
        case OP_GETLOGIN:
        case OP_PROTOTYPE:
        case OP_RUNCV:
        func_ops:
            useless = OP_DESC(o);
            break;

        case OP_GVSV:
        case OP_PADSV:
        case OP_PADAV:
        case OP_PADHV:
        case OP_PADANY:
        case OP_AELEM:
        case OP_AELEMFAST:
        case OP_AELEMFAST_LEX:
        case OP_ASLICE:
        case OP_HELEM:
        case OP_HSLICE:
            if (!(o->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO)))
                /* Otherwise it's "Useless use of grep iterator" */
                useless = OP_DESC(o);
            break;

        case OP_SPLIT:
            if (!(o->op_private & OPpSPLIT_ASSIGN))
                useless = OP_DESC(o);
            break;

        case OP_NOT:
            kid = cUNOPo->op_first;
            if (kid->op_type != OP_MATCH && kid->op_type != OP_SUBST &&
                kid->op_type != OP_TRANS && kid->op_type != OP_TRANSR) {
                goto func_ops;
            }
            useless = "negative pattern binding (!~)";
            break;

        case OP_SUBST:
            if (cPMOPo->op_pmflags & PMf_NONDESTRUCT)
                useless = "non-destructive substitution (s///r)";
            break;

        case OP_TRANSR:
            useless = "non-destructive transliteration (tr///r)";
            break;

        case OP_RV2GV:
        case OP_RV2SV:
        case OP_RV2AV:
        case OP_RV2HV:
            if (!(o->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO)) &&
                (!OpHAS_SIBLING(o) || OpSIBLING(o)->op_type != OP_READLINE))
                useless = "a variable";
            break;

        case OP_CONST:
            sv = cSVOPo_sv;
            if (cSVOPo->op_private & OPpCONST_STRICT)
                no_bareword_allowed(o);
            else {
                if (ckWARN(WARN_VOID)) {
                    NV nv;
                    /* don't warn on optimised away booleans, eg
                     * use constant Foo, 5; Foo || print; */
                    if (cSVOPo->op_private & OPpCONST_SHORTCIRCUIT)
                        useless = NULL;
                    /* the constants 0 and 1 are permitted as they are
                       conventionally used as dummies in constructs like
                       1 while some_condition_with_side_effects;  */
                    else if (SvNIOK(sv) && ((nv = SvNV(sv)) == 0.0 || nv == 1.0))
                        useless = NULL;
                    else if (SvPOK(sv)) {
                        SV * const dsv = newSVpvs("");
                        useless_sv
                            = Perl_newSVpvf(aTHX_
                                            "a constant (%s)",
                                            pv_pretty(dsv, SvPVX_const(sv),
                                                      SvCUR(sv), 32, NULL, NULL,
                                                      PERL_PV_PRETTY_DUMP
                                                      | PERL_PV_ESCAPE_NOCLEAR
                                                      | PERL_PV_ESCAPE_UNI_DETECT));
                        SvREFCNT_dec_NN(dsv);
                    }
                    else if (SvOK(sv)) {
                        useless_sv = Perl_newSVpvf(aTHX_ "a constant (%" SVf ")", SVfARG(sv));
                    }
                    else
                        useless = "a constant (undef)";
                }
            }
            op_null(o);         /* don't execute or even remember it */
            break;

        case OP_POSTINC:
            OpTYPE_set(o, OP_PREINC);  /* pre-increment is faster */
            break;

        case OP_POSTDEC:
            OpTYPE_set(o, OP_PREDEC);  /* pre-decrement is faster */
            break;

        case OP_I_POSTINC:
            OpTYPE_set(o, OP_I_PREINC);        /* pre-increment is faster */
            break;

        case OP_I_POSTDEC:
            OpTYPE_set(o, OP_I_PREDEC);        /* pre-decrement is faster */
            break;

        case OP_SASSIGN: {
            OP *rv2gv;
            UNOP *refgen, *rv2cv;
            LISTOP *exlist;

            if ((o->op_private & ~OPpASSIGN_BACKWARDS) != 2)
                break;

            rv2gv = cBINOPo->op_last;
            if (!rv2gv || rv2gv->op_type != OP_RV2GV)
                break;

            refgen = cUNOPx(cBINOPo->op_first);

            if (!refgen || (refgen->op_type != OP_REFGEN
                            && refgen->op_type != OP_SREFGEN))
                break;

            exlist = cLISTOPx(refgen->op_first);
            if (!exlist || exlist->op_type != OP_NULL
                || exlist->op_targ != OP_LIST)
                break;

            if (exlist->op_first->op_type != OP_PUSHMARK
                && exlist->op_first != exlist->op_last)
                break;

            rv2cv = cUNOPx(exlist->op_last);

            if (rv2cv->op_type != OP_RV2CV)
                break;

            assert ((rv2gv->op_private & OPpDONT_INIT_GV) == 0);
            assert ((o->op_private & OPpASSIGN_CV_TO_GV) == 0);
            assert ((rv2cv->op_private & OPpMAY_RETURN_CONSTANT) == 0);

            o->op_private |= OPpASSIGN_CV_TO_GV;
            rv2gv->op_private |= OPpDONT_INIT_GV;
            rv2cv->op_private |= OPpMAY_RETURN_CONSTANT;

            break;
        }

        case OP_AASSIGN: {
            inplace_aassign(o);
            break;
        }

        case OP_OR:
        case OP_AND:
            kid = cLOGOPo->op_first;
            if (kid->op_type == OP_NOT
                && (kid->op_flags & OPf_KIDS)) {
                if (o->op_type == OP_AND) {
                    OpTYPE_set(o, OP_OR);
                } else {
                    OpTYPE_set(o, OP_AND);
                }
                op_null(kid);
            }
            /* FALLTHROUGH */

        case OP_DOR:
        case OP_COND_EXPR:
        case OP_ENTERGIVEN:
        case OP_ENTERWHEN:
            next_kid = OpSIBLING(cUNOPo->op_first);
        break;

        case OP_NULL:
            if (o->op_flags & OPf_STACKED)
                break;
            /* FALLTHROUGH */
        case OP_NEXTSTATE:
        case OP_DBSTATE:
        case OP_ENTERTRY:
        case OP_ENTER:
            if (!(o->op_flags & OPf_KIDS))
                break;
            /* FALLTHROUGH */
        case OP_SCOPE:
        case OP_LEAVE:
        case OP_LEAVETRY:
        case OP_LEAVELOOP:
        case OP_LINESEQ:
        case OP_LEAVEGIVEN:
        case OP_LEAVEWHEN:
        kids:
            next_kid = cLISTOPo->op_first;
            break;
        case OP_LIST:
            /* If the first kid after pushmark is something that the padrange
               optimisation would reject, then null the list and the pushmark.
            */
            if ((kid = cLISTOPo->op_first)->op_type == OP_PUSHMARK
                && (  !(kid = OpSIBLING(kid))
                      || (  kid->op_type != OP_PADSV
                            && kid->op_type != OP_PADAV
                            && kid->op_type != OP_PADHV)
                      || kid->op_private & ~OPpLVAL_INTRO
                      || !(kid = OpSIBLING(kid))
                      || (  kid->op_type != OP_PADSV
                            && kid->op_type != OP_PADAV
                            && kid->op_type != OP_PADHV)
                      || kid->op_private & ~OPpLVAL_INTRO)
            ) {
                op_null(cUNOPo->op_first); /* NULL the pushmark */
                op_null(o); /* NULL the list */
            }
            goto kids;
        case OP_ENTEREVAL:
            scalarkids(o);
            break;
        case OP_SCALAR:
            scalar(o);
            break;
        }

        if (useless_sv) {
            /* mortalise it, in case warnings are fatal.  */
            Perl_ck_warner(aTHX_ packWARN(WARN_VOID),
                           "Useless use of %" SVf " in void context",
                           SVfARG(sv_2mortal(useless_sv)));
        }
        else if (useless) {
            Perl_ck_warner(aTHX_ packWARN(WARN_VOID),
                           "Useless use of %s in void context",
                           useless);
        }

      get_next_op:
        /* if a kid hasn't been nominated to process, continue with the
         * next sibling, or if no siblings left, go back to the parent's
         * siblings and so on
         */
        while (!next_kid) {
            if (o == arg)
                return arg; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o))
                next_kid = o->op_sibparent;
            else
                o = o->op_sibparent; /*try parent's next sibling */
        }
        o = next_kid;
    }
    NOT_REACHED;
}


static OP *
S_listkids(pTHX_ OP *o)
{
    if (o && o->op_flags & OPf_KIDS) {
        OP *kid;
        for (kid = cLISTOPo->op_first; kid; kid = OpSIBLING(kid))
            list(kid);
    }
    return o;
}


/* apply list context to the o subtree */

OP *
Perl_list(pTHX_ OP *o)
{
    OP * top_op = o;

    while (1) {
        OP *next_kid = NULL; /* what op (if any) to process next */

        OP *kid;

        /* assumes no premature commitment */
        if (!o || (o->op_flags & OPf_WANT)
             || (PL_parser && PL_parser->error_count)
             || o->op_type == OP_RETURN)
        {
            goto do_next;
        }

        if ((o->op_private & OPpTARGET_MY)
            && (PL_opargs[o->op_type] & OA_TARGLEX))/* OPp share the meaning */
        {
            goto do_next;				/* As if inside SASSIGN */
        }

        o->op_flags = (o->op_flags & ~OPf_WANT) | OPf_WANT_LIST;

        switch (o->op_type) {
        case OP_REPEAT:
            if (o->op_private & OPpREPEAT_DOLIST
             && !(o->op_flags & OPf_STACKED))
            {
                list(cBINOPo->op_first);
                kid = cBINOPo->op_last;
                /* optimise away (.....) x 1 */
                if (kid->op_type == OP_CONST && SvIOK(kSVOP_sv)
                 && SvIVX(kSVOP_sv) == 1)
                {
                    op_null(o); /* repeat */
                    op_null(cUNOPx(cBINOPo->op_first)->op_first);/* pushmark */
                    /* const (rhs): */
                    op_free(op_sibling_splice(o, cBINOPo->op_first, 1, NULL));
                }
            }
            break;

        case OP_OR:
        case OP_AND:
        case OP_COND_EXPR:
            /* impose list context on everything except the condition */
            next_kid = OpSIBLING(cUNOPo->op_first);
            break;

        default:
            if (!(o->op_flags & OPf_KIDS))
                break;
            /* possibly flatten 1..10 into a constant array */
            if (!o->op_next && cUNOPo->op_first->op_type == OP_FLOP) {
                list(cBINOPo->op_first);
                gen_constant_list(o);
                goto do_next;
            }
            next_kid = cUNOPo->op_first; /* do all kids */
            break;

        case OP_LIST:
            if (cLISTOPo->op_first->op_type == OP_PUSHMARK) {
                op_null(cUNOPo->op_first); /* NULL the pushmark */
                op_null(o); /* NULL the list */
            }
            if (o->op_flags & OPf_KIDS)
                next_kid = cUNOPo->op_first; /* do all kids */
            break;

        /* the children of these ops are usually a list of statements,
         * except the leaves, whose first child is a corresponding enter
         */
        case OP_SCOPE:
        case OP_LINESEQ:
            kid = cLISTOPo->op_first;
            goto do_kids;
        case OP_LEAVE:
        case OP_LEAVETRY:
            kid = cLISTOPo->op_first;
            list(kid);
            kid = OpSIBLING(kid);
        do_kids:
            while (kid) {
                OP *sib = OpSIBLING(kid);
                /* Apply void context to all kids except the last, which
                 * is list. E.g.
                 *      @a = do { void; void; list }
                 * Except that 'when's are always list context, e.g.
                 *      @a = do { given(..) {
                    *                 when (..) { list }
                    *                 when (..) { list }
                    *                 ...
                    *                }}
                    */
                if (!sib) {
                    /* tail call optimise calling list() on the last kid */
                    next_kid = kid;
                    goto do_next;
                }
                else if (kid->op_type == OP_LEAVEWHEN)
                    list(kid);
                else
                    scalarvoid(kid);
                kid = sib;
            }
            NOT_REACHED; /* NOTREACHED */
            break;

        }

        /* If next_kid is set, someone in the code above wanted us to process
         * that kid and all its remaining siblings.  Otherwise, work our way
         * back up the tree */
      do_next:
        while (!next_kid) {
            if (o == top_op)
                return top_op; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o))
                next_kid = o->op_sibparent;
            else {
                o = o->op_sibparent; /*try parent's next sibling */
                switch (o->op_type) {
                case OP_SCOPE:
                case OP_LINESEQ:
                case OP_LIST:
                case OP_LEAVE:
                case OP_LEAVETRY:
                    /* should really restore PL_curcop to its old value, but
                     * setting it to PL_compiling is better than do nothing */
                    PL_curcop = &PL_compiling;
                }
            }


        }
        o = next_kid;
    } /* while */
}

/* apply void context to non-final ops of a sequence */

static OP *
S_voidnonfinal(pTHX_ OP *o)
{
    if (o) {
        const OPCODE type = o->op_type;

        if (type == OP_LINESEQ || type == OP_SCOPE ||
            type == OP_LEAVE || type == OP_LEAVETRY)
        {
            OP *kid = cLISTOPo->op_first, *sib;
            if(type == OP_LEAVE) {
                /* Don't put the OP_ENTER in void context */
                assert(kid->op_type == OP_ENTER);
                kid = OpSIBLING(kid);
            }
            for (; kid; kid = sib) {
                if ((sib = OpSIBLING(kid))
                 && (  OpHAS_SIBLING(sib) || sib->op_type != OP_NULL
                    || (  sib->op_targ != OP_NEXTSTATE
                       && sib->op_targ != OP_DBSTATE  )))
                {
                    scalarvoid(kid);
                }
            }
            PL_curcop = &PL_compiling;
        }
        o->op_flags &= ~OPf_PARENS;
        if (PL_hints & HINT_BLOCK_SCOPE)
            o->op_flags |= OPf_PARENS;
    }
    else
        o = newOP(OP_STUB, 0);
    return o;
}

STATIC OP *
S_modkids(pTHX_ OP *o, I32 type)
{
    if (o && o->op_flags & OPf_KIDS) {
        OP *kid;
        for (kid = cLISTOPo->op_first; kid; kid = OpSIBLING(kid))
            op_lvalue(kid, type);
    }
    return o;
}


/* for a helem/hslice/kvslice, if its a fixed hash, croak on invalid
 * const fields. Also, convert CONST keys to HEK-in-SVs.
 * rop    is the op that retrieves the hash;
 * key_op is the first key
 * real   if false, only check (and possibly croak); don't update op
 */

void
Perl_check_hash_fields_and_hekify(pTHX_ UNOP *rop, SVOP *key_op, int real)
{
    PADNAME *lexname;
    GV **fields;
    bool check_fields;

    /* find the padsv corresponding to $lex->{} or @{$lex}{} */
    if (rop) {
        if (rop->op_first->op_type == OP_PADSV)
            /* @$hash{qw(keys here)} */
            rop = cUNOPx(rop->op_first);
        else {
            /* @{$hash}{qw(keys here)} */
            if (rop->op_first->op_type == OP_SCOPE
                && cLISTOPx(rop->op_first)->op_last->op_type == OP_PADSV)
                {
                    rop = cUNOPx(cLISTOPx(rop->op_first)->op_last);
                }
            else
                rop = NULL;
        }
    }

    lexname = NULL; /* just to silence compiler warnings */
    fields  = NULL; /* just to silence compiler warnings */

    check_fields =
            rop
         && (lexname = padnamelist_fetch(PL_comppad_name, rop->op_targ),
             PadnameHasTYPE(lexname))
         && (fields = (GV**)hv_fetchs(PadnameTYPE(lexname), "FIELDS", FALSE))
         && isGV(*fields) && GvHV(*fields);

    for (; key_op; key_op = cSVOPx(OpSIBLING(key_op))) {
        SV **svp, *sv;
        if (key_op->op_type != OP_CONST)
            continue;
        svp = cSVOPx_svp(key_op);

        /* make sure it's not a bareword under strict subs */
        if (key_op->op_private & OPpCONST_BARE &&
            key_op->op_private & OPpCONST_STRICT)
        {
            no_bareword_allowed((OP*)key_op);
        }

        /* Make the CONST have a shared SV */
        if (   !SvIsCOW_shared_hash(sv = *svp)
            && SvTYPE(sv) < SVt_PVMG
            && SvOK(sv)
            && !SvROK(sv)
            && real)
        {
            SSize_t keylen;
            const char * const key = SvPV_const(sv, *(STRLEN*)&keylen);
            SV *nsv = newSVpvn_share(key, SvUTF8(sv) ? -keylen : keylen, 0);
            SvREFCNT_dec_NN(sv);
            *svp = nsv;
        }

        if (   check_fields
            && !hv_fetch_ent(GvHV(*fields), *svp, FALSE, 0))
        {
            Perl_croak(aTHX_ "No such class field \"%" SVf "\" "
                        "in variable %" PNf " of type %" HEKf,
                        SVfARG(*svp), PNfARG(lexname),
                        HEKfARG(HvNAME_HEK(PadnameTYPE(lexname))));
        }
    }
}


/* do all the final processing on an optree (e.g. running the peephole
 * optimiser on it), then attach it to cv (if cv is non-null)
 */

static void
S_process_optree(pTHX_ CV *cv, OP *optree, OP* start)
{
    OP **startp;

    /* XXX for some reason, evals, require and main optrees are
     * never attached to their CV; instead they just hang off
     * PL_main_root + PL_main_start or PL_eval_root + PL_eval_start
     * and get manually freed when appropriate */
    if (cv)
        startp = &CvSTART(cv);
    else
        startp = PL_in_eval? &PL_eval_start : &PL_main_start;

    *startp = start;
    optree->op_private |= OPpREFCOUNTED;
    OpREFCNT_set(optree, 1);
    optimize_optree(optree);
    CALL_PEEP(*startp);
    finalize_optree(optree);
    op_prune_chain_head(startp);

    if (cv) {
        /* now that optimizer has done its work, adjust pad values */
        pad_tidy(optree->op_type == OP_LEAVEWRITE ? padtidy_FORMAT
                 : CvCLONE(cv) ? padtidy_SUBCLONE : padtidy_SUB);
    }
}

#ifdef USE_ITHREADS
/* Relocate sv to the pad for thread safety.
 * Despite being a "constant", the SV is written to,
 * for reference counts, sv_upgrade() etc. */
void
Perl_op_relocate_sv(pTHX_ SV** svp, PADOFFSET* targp)
{
    PADOFFSET ix;
    PERL_ARGS_ASSERT_OP_RELOCATE_SV;
    if (!*svp) return;
    ix = pad_alloc(OP_CONST, SVf_READONLY);
    SvREFCNT_dec(PAD_SVl(ix));
    PAD_SETSV(ix, *svp);
    /* XXX I don't know how this isn't readonly already. */
    if (!SvIsCOW(PAD_SVl(ix))) SvREADONLY_on(PAD_SVl(ix));
    *svp = NULL;
    *targp = ix;
}
#endif

static void
S_mark_padname_lvalue(pTHX_ PADNAME *pn)
{
    CV *cv = PL_compcv;
    PadnameLVALUE_on(pn);
    while (PadnameOUTER(pn) && PARENT_PAD_INDEX(pn)) {
        cv = CvOUTSIDE(cv);
        /* RT #127786: cv can be NULL due to an eval within the DB package
         * called from an anon sub - anon subs don't have CvOUTSIDE() set
         * unless they contain an eval, but calling eval within DB
         * pretends the eval was done in the caller's scope.
         */
        if (!cv)
            break;
        assert(CvPADLIST(cv));
        pn =
           PadlistNAMESARRAY(CvPADLIST(cv))[PARENT_PAD_INDEX(pn)];
        assert(PadnameLEN(pn));
        PadnameLVALUE_on(pn);
    }
}

static bool
S_vivifies(const OPCODE type)
{
    switch(type) {
    case OP_RV2AV:     case   OP_ASLICE:
    case OP_RV2HV:     case OP_KVASLICE:
    case OP_RV2SV:     case   OP_HSLICE:
    case OP_AELEMFAST: case OP_KVHSLICE:
    case OP_HELEM:
    case OP_AELEM:
        return 1;
    }
    return 0;
}


/* apply lvalue reference (aliasing) context to the optree o.
 * E.g. in
 *     \($x,$y) = (...)
 * o would be the list ($x,$y) and type would be OP_AASSIGN.
 * It may descend and apply this to children too, for example in
 * \( $cond ? $x, $y) = (...)
 */

static void
S_lvref(pTHX_ OP *o, I32 type)
{
    OP *kid;
    OP * top_op = o;

    while (1) {
        switch (o->op_type) {
        case OP_COND_EXPR:
            o = OpSIBLING(cUNOPo->op_first);
            continue;

        case OP_PUSHMARK:
            goto do_next;

        case OP_RV2AV:
            if (cUNOPo->op_first->op_type != OP_GV) goto badref;
            o->op_flags |= OPf_STACKED;
            if (o->op_flags & OPf_PARENS) {
                if (o->op_private & OPpLVAL_INTRO) {
                     yyerror(Perl_form(aTHX_ "Can't modify reference to "
                          "localized parenthesized array in list assignment"));
                    goto do_next;
                }
              slurpy:
                OpTYPE_set(o, OP_LVAVREF);
                o->op_private &= OPpLVAL_INTRO|OPpPAD_STATE;
                o->op_flags |= OPf_MOD|OPf_REF;
                goto do_next;
            }
            o->op_private |= OPpLVREF_AV;
            goto checkgv;

        case OP_RV2CV:
            kid = cUNOPo->op_first;
            if (kid->op_type == OP_NULL)
                kid = cUNOPx(OpSIBLING(kUNOP->op_first))
                    ->op_first;
            o->op_private = OPpLVREF_CV;
            if (kid->op_type == OP_GV)
                o->op_flags |= OPf_STACKED;
            else if (kid->op_type == OP_PADCV) {
                o->op_targ = kid->op_targ;
                kid->op_targ = 0;
                op_free(cUNOPo->op_first);
                cUNOPo->op_first = NULL;
                o->op_flags &=~ OPf_KIDS;
            }
            else goto badref;
            break;

        case OP_RV2HV:
            if (o->op_flags & OPf_PARENS) {
              parenhash:
                yyerror(Perl_form(aTHX_ "Can't modify reference to "
                                     "parenthesized hash in list assignment"));
                    goto do_next;
            }
            o->op_private |= OPpLVREF_HV;
            /* FALLTHROUGH */
        case OP_RV2SV:
          checkgv:
            if (cUNOPo->op_first->op_type != OP_GV) goto badref;
            o->op_flags |= OPf_STACKED;
            break;

        case OP_PADHV:
            if (o->op_flags & OPf_PARENS) goto parenhash;
            o->op_private |= OPpLVREF_HV;
            /* FALLTHROUGH */
        case OP_PADSV:
            PAD_COMPNAME_GEN_set(o->op_targ, PERL_INT_MAX);
            break;

        case OP_PADAV:
            PAD_COMPNAME_GEN_set(o->op_targ, PERL_INT_MAX);
            if (o->op_flags & OPf_PARENS) goto slurpy;
            o->op_private |= OPpLVREF_AV;
            break;

        case OP_AELEM:
        case OP_HELEM:
            o->op_private |= OPpLVREF_ELEM;
            o->op_flags   |= OPf_STACKED;
            break;

        case OP_ASLICE:
        case OP_HSLICE:
            OpTYPE_set(o, OP_LVREFSLICE);
            o->op_private &= OPpLVAL_INTRO;
            goto do_next;

        case OP_NULL:
            if (o->op_flags & OPf_SPECIAL)		/* do BLOCK */
                goto badref;
            else if (!(o->op_flags & OPf_KIDS))
                goto do_next;

            /* the code formerly only recursed into the first child of
             * a non ex-list OP_NULL. if we ever encounter such a null op with
             * more than one child, need to decide whether its ok to process
             * *all* its kids or not */
            assert(o->op_targ == OP_LIST
                    || !(OpHAS_SIBLING(cBINOPo->op_first)));
            /* FALLTHROUGH */
        case OP_LIST:
            o = cLISTOPo->op_first;
            continue;

        case OP_STUB:
            if (o->op_flags & OPf_PARENS)
                goto do_next;
            /* FALLTHROUGH */
        default:
          badref:
            /* diag_listed_as: Can't modify reference to %s in %s assignment */
            yyerror(Perl_form(aTHX_ "Can't modify reference to %s in %s",
                         o->op_type == OP_NULL && o->op_flags & OPf_SPECIAL
                          ? "do block"
                          : OP_DESC(o),
                         PL_op_desc[type]));
            goto do_next;
        }

        OpTYPE_set(o, OP_LVREF);
        o->op_private &=
            OPpLVAL_INTRO|OPpLVREF_ELEM|OPpLVREF_TYPE|OPpPAD_STATE;
        if (type == OP_ENTERLOOP)
            o->op_private |= OPpLVREF_ITER;

      do_next:
        while (1) {
            if (o == top_op)
                return; /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o)) {
                o = o->op_sibparent;
                break;
            }
            o = o->op_sibparent; /*try parent's next sibling */
        }
    } /* while */
}


PERL_STATIC_INLINE bool
S_potential_mod_type(I32 type)
{
    /* Types that only potentially result in modification.  */
    return type == OP_GREPSTART || type == OP_ENTERSUB
        || type == OP_REFGEN    || type == OP_LEAVESUBLV;
}


/*
=for apidoc op_lvalue

Propagate lvalue ("modifiable") context to an op and its children.
C<type> represents the context type, roughly based on the type of op that
would do the modifying, although C<local()> is represented by C<OP_NULL>,
because it has no op type of its own (it is signalled by a flag on
the lvalue op).

This function detects things that can't be modified, such as C<$x+1>, and
generates errors for them.  For example, C<$x+1 = 2> would cause it to be
called with an op of type C<OP_ADD> and a C<type> argument of C<OP_SASSIGN>.

It also flags things that need to behave specially in an lvalue context,
such as C<$$x = 5> which might have to vivify a reference in C<$x>.

=cut

Perl_op_lvalue_flags() is a non-API lower-level interface to
op_lvalue().  The flags param has these bits:
    OP_LVALUE_NO_CROAK:  return rather than croaking on error

*/

OP *
Perl_op_lvalue_flags(pTHX_ OP *o, I32 type, U32 flags)
{
    OP *top_op = o;

    if (!o || (PL_parser && PL_parser->error_count))
        return o;

    while (1) {
    OP *kid;
    /* -1 = error on localize, 0 = ignore localize, 1 = ok to localize */
    int localize = -1;
    OP *next_kid = NULL;

    if ((o->op_private & OPpTARGET_MY)
        && (PL_opargs[o->op_type] & OA_TARGLEX))/* OPp share the meaning */
    {
        goto do_next;
    }

    /* elements of a list might be in void context because the list is
       in scalar context or because they are attribute sub calls */
    if ((o->op_flags & OPf_WANT) == OPf_WANT_VOID)
        goto do_next;

    if (type == OP_PRTF || type == OP_SPRINTF) type = OP_ENTERSUB;

    switch (o->op_type) {
    case OP_UNDEF:
        if (type == OP_SASSIGN)
            goto nomod;
        PL_modcount++;
        goto do_next;

    case OP_STUB:
        if ((o->op_flags & OPf_PARENS))
            break;
        goto nomod;

    case OP_ENTERSUB:
        if ((type == OP_UNDEF || type == OP_REFGEN || type == OP_LOCK) &&
            !(o->op_flags & OPf_STACKED)) {
            OpTYPE_set(o, OP_RV2CV);		/* entersub => rv2cv */
            assert(cUNOPo->op_first->op_type == OP_NULL);
            op_null(cLISTOPx(cUNOPo->op_first)->op_first);/* disable pushmark */
            break;
        }
        else {				/* lvalue subroutine call */
            o->op_private |= OPpLVAL_INTRO;
            PL_modcount = RETURN_UNLIMITED_NUMBER;
            if (S_potential_mod_type(type)) {
                o->op_private |= OPpENTERSUB_INARGS;
                break;
            }
            else {                      /* Compile-time error message: */
                OP *kid = cUNOPo->op_first;
                CV *cv;
                GV *gv;
                SV *namesv;

                if (kid->op_type != OP_PUSHMARK) {
                    if (kid->op_type != OP_NULL || kid->op_targ != OP_LIST)
                        Perl_croak(aTHX_
                                "panic: unexpected lvalue entersub "
                                "args: type/targ %ld:%" UVuf,
                                (long)kid->op_type, (UV)kid->op_targ);
                    kid = kLISTOP->op_first;
                }
                while (OpHAS_SIBLING(kid))
                    kid = OpSIBLING(kid);
                if (!(kid->op_type == OP_NULL && kid->op_targ == OP_RV2CV)) {
                    break;	/* Postpone until runtime */
                }

                kid = kUNOP->op_first;
                if (kid->op_type == OP_NULL && kid->op_targ == OP_RV2SV)
                    kid = kUNOP->op_first;
                if (kid->op_type == OP_NULL)
                    Perl_croak(aTHX_
                               "panic: unexpected constant lvalue entersub "
                               "entry via type/targ %ld:%" UVuf,
                               (long)kid->op_type, (UV)kid->op_targ);
                if (kid->op_type != OP_GV) {
                    break;
                }

                gv = kGVOP_gv;
                cv = isGV(gv)
                    ? GvCV(gv)
                    : SvROK(gv) && SvTYPE(SvRV(gv)) == SVt_PVCV
                        ? MUTABLE_CV(SvRV(gv))
                        : NULL;
                if (!cv)
                    break;
                if (CvLVALUE(cv))
                    break;
                if (flags & OP_LVALUE_NO_CROAK)
                    return NULL;

                namesv = cv_name(cv, NULL, 0);
                yyerror_pv(Perl_form(aTHX_ "Can't modify non-lvalue "
                                     "subroutine call of &%" SVf " in %s",
                                     SVfARG(namesv), PL_op_desc[type]),
                           SvUTF8(namesv));
                goto do_next;
            }
        }
        /* FALLTHROUGH */
    default:
      nomod:
        if (flags & OP_LVALUE_NO_CROAK) return NULL;
        /* grep, foreach, subcalls, refgen */
        if (S_potential_mod_type(type))
            break;
        yyerror(Perl_form(aTHX_ "Can't modify %s in %s",
                     (o->op_type == OP_NULL && (o->op_flags & OPf_SPECIAL)
                      ? "do block"
                      : OP_DESC(o)),
                     type ? PL_op_desc[type] : "local"));
        goto do_next;

    case OP_PREINC:
    case OP_PREDEC:
    case OP_POW:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_MODULO:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_CONCAT:
    case OP_LEFT_SHIFT:
    case OP_RIGHT_SHIFT:
    case OP_BIT_AND:
    case OP_BIT_XOR:
    case OP_BIT_OR:
    case OP_I_MULTIPLY:
    case OP_I_DIVIDE:
    case OP_I_MODULO:
    case OP_I_ADD:
    case OP_I_SUBTRACT:
        if (!(o->op_flags & OPf_STACKED))
            goto nomod;
        PL_modcount++;
        break;

    case OP_REPEAT:
        if (o->op_flags & OPf_STACKED) {
            PL_modcount++;
            break;
        }
        if (!(o->op_private & OPpREPEAT_DOLIST))
            goto nomod;
        else {
            const I32 mods = PL_modcount;
            /* we recurse rather than iterate here because we need to
             * calculate and use the delta applied to PL_modcount by the
             * first child. So in something like
             *     ($x, ($y) x 3) = split;
             * split knows that 4 elements are wanted
             */
            modkids(cBINOPo->op_first, type);
            if (type != OP_AASSIGN)
                goto nomod;
            kid = cBINOPo->op_last;
            if (kid->op_type == OP_CONST && SvIOK(kSVOP_sv)) {
                const IV iv = SvIV(kSVOP_sv);
                if (PL_modcount != RETURN_UNLIMITED_NUMBER)
                    PL_modcount =
                        mods + (PL_modcount - mods) * (iv < 0 ? 0 : iv);
            }
            else
                PL_modcount = RETURN_UNLIMITED_NUMBER;
        }
        break;

    case OP_COND_EXPR:
        localize = 1;
        next_kid = OpSIBLING(cUNOPo->op_first);
        break;

    case OP_RV2AV:
    case OP_RV2HV:
        if (type == OP_REFGEN && o->op_flags & OPf_PARENS) {
           PL_modcount = RETURN_UNLIMITED_NUMBER;
           /* Treat \(@foo) like ordinary list, but still mark it as modi-
              fiable since some contexts need to know.  */
           o->op_flags |= OPf_MOD;
           goto do_next;
        }
        /* FALLTHROUGH */
    case OP_RV2GV:
        if (scalar_mod_type(o, type))
            goto nomod;
        ref(cUNOPo->op_first, o->op_type);
        /* FALLTHROUGH */
    case OP_ASLICE:
    case OP_HSLICE:
        localize = 1;
        /* FALLTHROUGH */
    case OP_AASSIGN:
        /* Do not apply the lvsub flag for rv2[ah]v in scalar context.  */
        if (type == OP_LEAVESUBLV && (
                (o->op_type != OP_RV2AV && o->op_type != OP_RV2HV)
             || (o->op_flags & OPf_WANT) != OPf_WANT_SCALAR
           ))
            o->op_private |= OPpMAYBE_LVSUB;
        /* FALLTHROUGH */
    case OP_NEXTSTATE:
    case OP_DBSTATE:
       PL_modcount = RETURN_UNLIMITED_NUMBER;
        break;

    case OP_KVHSLICE:
    case OP_KVASLICE:
    case OP_AKEYS:
        if (type == OP_LEAVESUBLV)
            o->op_private |= OPpMAYBE_LVSUB;
        goto nomod;

    case OP_AVHVSWITCH:
        if (type == OP_LEAVESUBLV
         && (o->op_private & OPpAVHVSWITCH_MASK) + OP_EACH == OP_KEYS)
            o->op_private |= OPpMAYBE_LVSUB;
        goto nomod;

    case OP_AV2ARYLEN:
        PL_hints |= HINT_BLOCK_SCOPE;
        if (type == OP_LEAVESUBLV)
            o->op_private |= OPpMAYBE_LVSUB;
        PL_modcount++;
        break;

    case OP_RV2SV:
        ref(cUNOPo->op_first, o->op_type);
        localize = 1;
        /* FALLTHROUGH */
    case OP_GV:
        PL_hints |= HINT_BLOCK_SCOPE;
        /* FALLTHROUGH */
    case OP_SASSIGN:
    case OP_ANDASSIGN:
    case OP_ORASSIGN:
    case OP_DORASSIGN:
        PL_modcount++;
        break;

    case OP_AELEMFAST:
    case OP_AELEMFAST_LEX:
        localize = -1;
        PL_modcount++;
        break;

    case OP_PADAV:
    case OP_PADHV:
       PL_modcount = RETURN_UNLIMITED_NUMBER;
        if (type == OP_REFGEN && o->op_flags & OPf_PARENS)
        {
           /* Treat \(@foo) like ordinary list, but still mark it as modi-
              fiable since some contexts need to know.  */
            o->op_flags |= OPf_MOD;
            goto do_next;
        }
        if (scalar_mod_type(o, type))
            goto nomod;
        if ((o->op_flags & OPf_WANT) != OPf_WANT_SCALAR
          && type == OP_LEAVESUBLV)
            o->op_private |= OPpMAYBE_LVSUB;
        /* FALLTHROUGH */
    case OP_PADSV:
        PL_modcount++;
        if (!type) /* local() */
            Perl_croak(aTHX_ "Can't localize lexical variable %" PNf,
                              PNfARG(PAD_COMPNAME(o->op_targ)));
        if (!(o->op_private & OPpLVAL_INTRO)
         || (  type != OP_SASSIGN && type != OP_AASSIGN
            && PadnameIsSTATE(PAD_COMPNAME_SV(o->op_targ))  ))
            S_mark_padname_lvalue(aTHX_ PAD_COMPNAME_SV(o->op_targ));
        break;

    case OP_PUSHMARK:
        localize = 0;
        break;

    case OP_KEYS:
        if (type != OP_LEAVESUBLV && !scalar_mod_type(NULL, type))
            goto nomod;
        goto lvalue_func;
    case OP_SUBSTR:
        if (o->op_private == 4) /* don't allow 4 arg substr as lvalue */
            goto nomod;
        /* FALLTHROUGH */
    case OP_POS:
    case OP_VEC:
      lvalue_func:
        if (type == OP_LEAVESUBLV)
            o->op_private |= OPpMAYBE_LVSUB;
        if (o->op_flags & OPf_KIDS && OpHAS_SIBLING(cBINOPo->op_first)) {
            /* we recurse rather than iterate here because the child
             * needs to be processed with a different 'type' parameter */

            /* substr and vec */
            /* If this op is in merely potential (non-fatal) modifiable
               context, then apply OP_ENTERSUB context to
               the kid op (to avoid croaking).  Other-
               wise pass this op’s own type so the correct op is mentioned
               in error messages.  */
            op_lvalue(OpSIBLING(cBINOPo->op_first),
                      S_potential_mod_type(type)
                        ? (I32)OP_ENTERSUB
                        : o->op_type);
        }
        break;

    case OP_AELEM:
    case OP_HELEM:
        ref(cBINOPo->op_first, o->op_type);
        if (type == OP_ENTERSUB &&
             !(o->op_private & (OPpLVAL_INTRO | OPpDEREF)))
            o->op_private |= OPpLVAL_DEFER;
        if (type == OP_LEAVESUBLV)
            o->op_private |= OPpMAYBE_LVSUB;
        localize = 1;
        PL_modcount++;
        break;

    case OP_LEAVE:
    case OP_LEAVELOOP:
        o->op_private |= OPpLVALUE;
        /* FALLTHROUGH */
    case OP_SCOPE:
    case OP_ENTER:
    case OP_LINESEQ:
        localize = 0;
        if (o->op_flags & OPf_KIDS)
            next_kid = cLISTOPo->op_last;
        break;

    case OP_NULL:
        localize = 0;
        if (o->op_flags & OPf_SPECIAL)		/* do BLOCK */
            goto nomod;
        else if (!(o->op_flags & OPf_KIDS))
            break;

        if (o->op_targ != OP_LIST) {
            OP *sib = OpSIBLING(cLISTOPo->op_first);
            /* OP_TRANS and OP_TRANSR with argument have a weird optree
             * that looks like
             *
             *   null
             *      arg
             *      trans
             *
             * compared with things like OP_MATCH which have the argument
             * as a child:
             *
             *   match
             *      arg
             *
             * so handle specially to correctly get "Can't modify" croaks etc
             */

            if (sib && (sib->op_type == OP_TRANS || sib->op_type == OP_TRANSR))
            {
                /* this should trigger a "Can't modify transliteration" err */
                op_lvalue(sib, type);
            }
            next_kid = cBINOPo->op_first;
            /* we assume OP_NULLs which aren't ex-list have no more than 2
             * children. If this assumption is wrong, increase the scan
             * limit below */
            assert(   !OpHAS_SIBLING(next_kid)
                   || !OpHAS_SIBLING(OpSIBLING(next_kid)));
            break;
        }
        /* FALLTHROUGH */
    case OP_LIST:
        localize = 0;
        next_kid = cLISTOPo->op_first;
        break;

    case OP_COREARGS:
        goto do_next;

    case OP_AND:
    case OP_OR:
        if (type == OP_LEAVESUBLV
         || !S_vivifies(cLOGOPo->op_first->op_type))
            next_kid = cLOGOPo->op_first;
        else if (type == OP_LEAVESUBLV
         || !S_vivifies(OpSIBLING(cLOGOPo->op_first)->op_type))
            next_kid = OpSIBLING(cLOGOPo->op_first);
        goto nomod;

    case OP_SREFGEN:
        if (type == OP_NULL) { /* local */
          local_refgen:
            if (!FEATURE_MYREF_IS_ENABLED)
                Perl_croak(aTHX_ "The experimental declared_refs "
                                 "feature is not enabled");
            Perl_ck_warner_d(aTHX_
                     packWARN(WARN_EXPERIMENTAL__DECLARED_REFS),
                    "Declaring references is experimental");
            next_kid = cUNOPo->op_first;
            goto do_next;
        }
        if (type != OP_AASSIGN && type != OP_SASSIGN
         && type != OP_ENTERLOOP)
            goto nomod;
        /* Don’t bother applying lvalue context to the ex-list.  */
        kid = cUNOPx(cUNOPo->op_first)->op_first;
        assert (!OpHAS_SIBLING(kid));
        goto kid_2lvref;
    case OP_REFGEN:
        if (type == OP_NULL) /* local */
            goto local_refgen;
        if (type != OP_AASSIGN) goto nomod;
        kid = cUNOPo->op_first;
      kid_2lvref:
        {
            const U8 ec = PL_parser ? PL_parser->error_count : 0;
            S_lvref(aTHX_ kid, type);
            if (!PL_parser || PL_parser->error_count == ec) {
                if (!FEATURE_REFALIASING_IS_ENABLED)
                    Perl_croak(aTHX_
                       "Experimental aliasing via reference not enabled");
                Perl_ck_warner_d(aTHX_
                                 packWARN(WARN_EXPERIMENTAL__REFALIASING),
                                "Aliasing via reference is experimental");
            }
        }
        if (o->op_type == OP_REFGEN)
            op_null(cUNOPx(cUNOPo->op_first)->op_first); /* pushmark */
        op_null(o);
        goto do_next;

    case OP_SPLIT:
        if ((o->op_private & OPpSPLIT_ASSIGN)) {
            /* This is actually @array = split.  */
            PL_modcount = RETURN_UNLIMITED_NUMBER;
            break;
        }
        goto nomod;

    case OP_SCALAR:
        op_lvalue(cUNOPo->op_first, OP_ENTERSUB);
        goto nomod;

    case OP_ANONCODE:
        /* If we were to set OPf_REF on this and it was constructed by XS
         * code as a child of an OP_REFGEN then we'd end up generating a
         * double-ref when executed. We don't want to do that, so don't
         * set flag here.
         *   See also https://github.com/Perl/perl5/issues/20384
         */

        // Perl always sets OPf_REF as of 5.37.5.
        //
        if (LIKELY(o->op_flags & OPf_REF)) goto nomod;

        // If we got here, then our op came from an XS module that predates
        // 5.37.5’s change to the op tree, which we have to handle a bit
        // diffrently to preserve backward compatibility.
        //
        goto do_next;
    }

    /* [20011101.069 (#7861)] File test operators interpret OPf_REF to mean that
       their argument is a filehandle; thus \stat(".") should not set
       it. AMS 20011102 */
    if (type == OP_REFGEN && OP_IS_STAT(o->op_type))
        goto do_next;

    if (type != OP_LEAVESUBLV)
        o->op_flags |= OPf_MOD;

    if (type == OP_AASSIGN || type == OP_SASSIGN)
        o->op_flags |= o->op_type == OP_ENTERSUB ? 0 : OPf_SPECIAL|OPf_REF;
    else if (!type) { /* local() */
        switch (localize) {
        case 1:
            o->op_private |= OPpLVAL_INTRO;
            o->op_flags &= ~OPf_SPECIAL;
            PL_hints |= HINT_BLOCK_SCOPE;
            break;
        case 0:
            break;
        case -1:
            Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                           "Useless localization of %s", OP_DESC(o));
        }
    }
    else if (type != OP_GREPSTART && type != OP_ENTERSUB
             && type != OP_LEAVESUBLV && o->op_type != OP_ENTERSUB)
        o->op_flags |= OPf_REF;

  do_next:
    while (!next_kid) {
        if (o == top_op)
            return top_op; /* at top; no parents/siblings to try */
        if (OpHAS_SIBLING(o)) {
            next_kid = o->op_sibparent;
            if (!OpHAS_SIBLING(next_kid)) {
                /* a few node types don't recurse into their second child */
                OP *parent = next_kid->op_sibparent;
                I32 ptype  = parent->op_type;
                if (   (ptype == OP_NULL && parent->op_targ != OP_LIST)
                    || (   (ptype == OP_AND || ptype == OP_OR)
                        && (type != OP_LEAVESUBLV
                            && S_vivifies(next_kid->op_type))
                       )
                )  {
                    /*try parent's next sibling */
                    o = parent;
                    next_kid =  NULL;
                }
            }
        }
        else
            o = o->op_sibparent; /*try parent's next sibling */

    }
    o = next_kid;

    } /* while */

}


STATIC bool
S_scalar_mod_type(const OP *o, I32 type)
{
    switch (type) {
    case OP_POS:
    case OP_SASSIGN:
        if (o && o->op_type == OP_RV2GV)
            return FALSE;
        /* FALLTHROUGH */
    case OP_PREINC:
    case OP_PREDEC:
    case OP_POSTINC:
    case OP_POSTDEC:
    case OP_I_PREINC:
    case OP_I_PREDEC:
    case OP_I_POSTINC:
    case OP_I_POSTDEC:
    case OP_POW:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_MODULO:
    case OP_REPEAT:
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_I_MULTIPLY:
    case OP_I_DIVIDE:
    case OP_I_MODULO:
    case OP_I_ADD:
    case OP_I_SUBTRACT:
    case OP_LEFT_SHIFT:
    case OP_RIGHT_SHIFT:
    case OP_BIT_AND:
    case OP_BIT_XOR:
    case OP_BIT_OR:
    case OP_NBIT_AND:
    case OP_NBIT_XOR:
    case OP_NBIT_OR:
    case OP_SBIT_AND:
    case OP_SBIT_XOR:
    case OP_SBIT_OR:
    case OP_CONCAT:
    case OP_SUBST:
    case OP_TRANS:
    case OP_TRANSR:
    case OP_READ:
    case OP_SYSREAD:
    case OP_RECV:
    case OP_ANDASSIGN:
    case OP_ORASSIGN:
    case OP_DORASSIGN:
    case OP_VEC:
    case OP_SUBSTR:
        return TRUE;
    default:
        return FALSE;
    }
}

STATIC bool
S_is_handle_constructor(const OP *o, I32 numargs)
{
    PERL_ARGS_ASSERT_IS_HANDLE_CONSTRUCTOR;

    switch (o->op_type) {
    case OP_PIPE_OP:
    case OP_SOCKPAIR:
        if (numargs == 2)
            return TRUE;
        /* FALLTHROUGH */
    case OP_SYSOPEN:
    case OP_OPEN:
    case OP_SELECT:		/* XXX c.f. SelectSaver.pm */
    case OP_SOCKET:
    case OP_OPEN_DIR:
    case OP_ACCEPT:
        if (numargs == 1)
            return TRUE;
        /* FALLTHROUGH */
    default:
        return FALSE;
    }
}

static OP *
S_refkids(pTHX_ OP *o, I32 type)
{
    if (o && o->op_flags & OPf_KIDS) {
        OP *kid;
        for (kid = cLISTOPo->op_first; kid; kid = OpSIBLING(kid))
            ref(kid, type);
    }
    return o;
}


/* Apply reference (autovivification) context to the subtree at o.
 * For example in
 *     push @{expression}, ....;
 * o will be the head of 'expression' and type will be OP_RV2AV.
 * It marks the op o (or a suitable child) as autovivifying, e.g. by
 * setting  OPf_MOD.
 * For OP_RV2AV/OP_PADAV and OP_RV2HV/OP_PADHV sets OPf_REF too if
 * set_op_ref is true.
 *
 * Also calls scalar(o).
 */

OP *
Perl_doref(pTHX_ OP *o, I32 type, bool set_op_ref)
{
    OP * top_op = o;

    PERL_ARGS_ASSERT_DOREF;

    if (PL_parser && PL_parser->error_count)
        return o;

    while (1) {
        switch (o->op_type) {
        case OP_ENTERSUB:
            if ((type == OP_EXISTS || type == OP_DEFINED) &&
                !(o->op_flags & OPf_STACKED)) {
                OpTYPE_set(o, OP_RV2CV);             /* entersub => rv2cv */
                assert(cUNOPo->op_first->op_type == OP_NULL);
                /* disable pushmark */
                op_null(cLISTOPx(cUNOPo->op_first)->op_first);
                o->op_flags |= OPf_SPECIAL;
            }
            else if (type == OP_RV2SV || type == OP_RV2AV || type == OP_RV2HV){
                o->op_private |= (type == OP_RV2AV ? OPpDEREF_AV
                                  : type == OP_RV2HV ? OPpDEREF_HV
                                  : OPpDEREF_SV);
                o->op_flags |= OPf_MOD;
            }

            break;

        case OP_COND_EXPR:
            o = OpSIBLING(cUNOPo->op_first);
            continue;

        case OP_RV2SV:
            if (type == OP_DEFINED)
                o->op_flags |= OPf_SPECIAL;		/* don't create GV */
            /* FALLTHROUGH */
        case OP_PADSV:
            if (type == OP_RV2SV || type == OP_RV2AV || type == OP_RV2HV) {
                o->op_private |= (type == OP_RV2AV ? OPpDEREF_AV
                                  : type == OP_RV2HV ? OPpDEREF_HV
                                  : OPpDEREF_SV);
                o->op_flags |= OPf_MOD;
            }
            if (o->op_flags & OPf_KIDS) {
                type = o->op_type;
                o = cUNOPo->op_first;
                continue;
            }
            break;

        case OP_RV2AV:
        case OP_RV2HV:
            if (set_op_ref)
                o->op_flags |= OPf_REF;
            /* FALLTHROUGH */
        case OP_RV2GV:
            if (type == OP_DEFINED)
                o->op_flags |= OPf_SPECIAL;		/* don't create GV */
            type = o->op_type;
            o = cUNOPo->op_first;
            continue;

        case OP_PADAV:
        case OP_PADHV:
            if (set_op_ref)
                o->op_flags |= OPf_REF;
            break;

        case OP_SCALAR:
        case OP_NULL:
            if (!(o->op_flags & OPf_KIDS) || type == OP_DEFINED)
                break;
             o = cBINOPo->op_first;
            continue;

        case OP_AELEM:
        case OP_HELEM:
            if (type == OP_RV2SV || type == OP_RV2AV || type == OP_RV2HV) {
                o->op_private |= (type == OP_RV2AV ? OPpDEREF_AV
                                  : type == OP_RV2HV ? OPpDEREF_HV
                                  : OPpDEREF_SV);
                o->op_flags |= OPf_MOD;
            }
            type = o->op_type;
            o = cBINOPo->op_first;
            continue;;

        case OP_SCOPE:
        case OP_LEAVE:
            set_op_ref = FALSE;
            /* FALLTHROUGH */
        case OP_ENTER:
        case OP_LIST:
            if (!(o->op_flags & OPf_KIDS))
                break;
            o = cLISTOPo->op_last;
            continue;

        default:
            break;
        } /* switch */

        while (1) {
            if (o == top_op)
                return scalar(top_op); /* at top; no parents/siblings to try */
            if (OpHAS_SIBLING(o)) {
                o = o->op_sibparent;
                /* Normally skip all siblings and go straight to the parent;
                 * the only op that requires two children to be processed
                 * is OP_COND_EXPR */
                if (!OpHAS_SIBLING(o)
                        && o->op_sibparent->op_type == OP_COND_EXPR)
                    break;
                continue;
            }
            o = o->op_sibparent; /*try parent's next sibling */
        }
    } /* while */
}


STATIC OP *
S_dup_attrlist(pTHX_ OP *o)
{
    OP *rop;

    PERL_ARGS_ASSERT_DUP_ATTRLIST;

    /* An attrlist is either a simple OP_CONST or an OP_LIST with kids,
     * where the first kid is OP_PUSHMARK and the remaining ones
     * are OP_CONST.  We need to push the OP_CONST values.
     */
    if (o->op_type == OP_CONST)
        rop = newSVOP(OP_CONST, o->op_flags, SvREFCNT_inc_NN(cSVOPo->op_sv));
    else {
        assert((o->op_type == OP_LIST) && (o->op_flags & OPf_KIDS));
        rop = NULL;
        for (o = cLISTOPo->op_first; o; o = OpSIBLING(o)) {
            if (o->op_type == OP_CONST)
                rop = op_append_elem(OP_LIST, rop,
                                  newSVOP(OP_CONST, o->op_flags,
                                          SvREFCNT_inc_NN(cSVOPo->op_sv)));
        }
    }
    return rop;
}

STATIC void
S_apply_attrs(pTHX_ HV *stash, SV *target, OP *attrs)
{
    PERL_ARGS_ASSERT_APPLY_ATTRS;
    {
        SV * const stashsv = newSVhek(HvNAME_HEK(stash));

        /* fake up C<use attributes $pkg,$rv,@attrs> */

#define ATTRSMODULE "attributes"
#define ATTRSMODULE_PM "attributes.pm"

        Perl_load_module(
          aTHX_ PERL_LOADMOD_IMPORT_OPS,
          newSVpvs(ATTRSMODULE),
          NULL,
          op_prepend_elem(OP_LIST,
                          newSVOP(OP_CONST, 0, stashsv),
                          op_prepend_elem(OP_LIST,
                                          newSVOP(OP_CONST, 0,
                                                  newRV(target)),
                                          dup_attrlist(attrs))));
    }
}

STATIC void
S_apply_attrs_my(pTHX_ HV *stash, OP *target, OP *attrs, OP **imopsp)
{
    OP *pack, *imop, *arg;
    SV *meth, *stashsv, **svp;

    PERL_ARGS_ASSERT_APPLY_ATTRS_MY;

    if (!attrs)
        return;

    assert(target->op_type == OP_PADSV ||
           target->op_type == OP_PADHV ||
           target->op_type == OP_PADAV);

    /* Ensure that attributes.pm is loaded. */
    /* Don't force the C<use> if we don't need it. */
    svp = hv_fetchs(GvHVn(PL_incgv), ATTRSMODULE_PM, FALSE);
    if (svp && *svp != &PL_sv_undef)
        NOOP;	/* already in %INC */
    else
        Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT,
                               newSVpvs(ATTRSMODULE), NULL);

    /* Need package name for method call. */
    pack = newSVOP(OP_CONST, 0, newSVpvs(ATTRSMODULE));

    /* Build up the real arg-list. */
    stashsv = newSVhek(HvNAME_HEK(stash));

    arg = newPADxVOP(OP_PADSV, 0, target->op_targ);
    arg = op_prepend_elem(OP_LIST,
                       newSVOP(OP_CONST, 0, stashsv),
                       op_prepend_elem(OP_LIST,
                                    newUNOP(OP_REFGEN, 0,
                                            arg),
                                    dup_attrlist(attrs)));

    /* Fake up a method call to import */
    meth = newSVpvs_share("import");
    imop = op_convert_list(OP_ENTERSUB, OPf_STACKED|OPf_WANT_VOID,
                   op_append_elem(OP_LIST,
                               op_prepend_elem(OP_LIST, pack, arg),
                               newMETHOP_named(OP_METHOD_NAMED, 0, meth)));

    /* Combine the ops. */
    *imopsp = op_append_elem(OP_LIST, *imopsp, imop);
}

/*
=notfor apidoc apply_attrs_string

Attempts to apply a list of attributes specified by the C<attrstr> and
C<len> arguments to the subroutine identified by the C<cv> argument which
is expected to be associated with the package identified by the C<stashpv>
argument (see L<attributes>).  It gets this wrong, though, in that it
does not correctly identify the boundaries of the individual attribute
specifications within C<attrstr>.  This is not really intended for the
public API, but has to be listed here for systems such as AIX which
need an explicit export list for symbols.  (It's called from XS code
in support of the C<ATTRS:> keyword from F<xsubpp>.)  Patches to fix it
to respect attribute syntax properly would be welcome.

=cut
*/

void
Perl_apply_attrs_string(pTHX_ const char *stashpv, CV *cv,
                        const char *attrstr, STRLEN len)
{
    OP *attrs = NULL;

    PERL_ARGS_ASSERT_APPLY_ATTRS_STRING;

    if (!len) {
        len = strlen(attrstr);
    }

    while (len) {
        for (; isSPACE(*attrstr) && len; --len, ++attrstr) ;
        if (len) {
            const char * const sstr = attrstr;
            for (; !isSPACE(*attrstr) && len; --len, ++attrstr) ;
            attrs = op_append_elem(OP_LIST, attrs,
                                newSVOP(OP_CONST, 0,
                                        newSVpvn(sstr, attrstr-sstr)));
        }
    }

    Perl_load_module(aTHX_ PERL_LOADMOD_IMPORT_OPS,
                     newSVpvs(ATTRSMODULE),
                     NULL, op_prepend_elem(OP_LIST,
                                  newSVOP(OP_CONST, 0, newSVpv(stashpv,0)),
                                  op_prepend_elem(OP_LIST,
                                               newSVOP(OP_CONST, 0,
                                                       newRV(MUTABLE_SV(cv))),
                                               attrs)));
}

STATIC void
S_move_proto_attr(pTHX_ OP **proto, OP **attrs, const GV * name,
                        bool curstash)
{
    OP *new_proto = NULL;
    STRLEN pvlen;
    char *pv;
    OP *o;

    PERL_ARGS_ASSERT_MOVE_PROTO_ATTR;

    if (!*attrs)
        return;

    o = *attrs;
    if (o->op_type == OP_CONST) {
        pv = SvPV(cSVOPo_sv, pvlen);
        if (memBEGINs(pv, pvlen, "prototype(")) {
            SV * const tmpsv = newSVpvn_flags(pv + 10, pvlen - 11, SvUTF8(cSVOPo_sv));
            SV ** const tmpo = cSVOPx_svp(o);
            SvREFCNT_dec(cSVOPo_sv);
            *tmpo = tmpsv;
            new_proto = o;
            *attrs = NULL;
        }
    } else if (o->op_type == OP_LIST) {
        OP * lasto;
        assert(o->op_flags & OPf_KIDS);
        lasto = cLISTOPo->op_first;
        assert(lasto->op_type == OP_PUSHMARK);
        for (o = OpSIBLING(lasto); o; o = OpSIBLING(o)) {
            if (o->op_type == OP_CONST) {
                pv = SvPV(cSVOPo_sv, pvlen);
                if (memBEGINs(pv, pvlen, "prototype(")) {
                    SV * const tmpsv = newSVpvn_flags(pv + 10, pvlen - 11, SvUTF8(cSVOPo_sv));
                    SV ** const tmpo = cSVOPx_svp(o);
                    SvREFCNT_dec(cSVOPo_sv);
                    *tmpo = tmpsv;
                    if (new_proto && ckWARN(WARN_MISC)) {
                        STRLEN new_len;
                        const char * newp = SvPV(cSVOPo_sv, new_len);
                        Perl_warner(aTHX_ packWARN(WARN_MISC),
                            "Attribute prototype(%" UTF8f ") discards earlier prototype attribute in same sub",
                            UTF8fARG(SvUTF8(cSVOPo_sv), new_len, newp));
                        op_free(new_proto);
                    }
                    else if (new_proto)
                        op_free(new_proto);
                    new_proto = o;
                    /* excise new_proto from the list */
                    op_sibling_splice(*attrs, lasto, 1, NULL);
                    o = lasto;
                    continue;
                }
            }
            lasto = o;
        }
        /* If the list is now just the PUSHMARK, scrap the whole thing; otherwise attributes.xs
           would get pulled in with no real need */
        if (!OpHAS_SIBLING(cLISTOPx(*attrs)->op_first)) {
            op_free(*attrs);
            *attrs = NULL;
        }
    }

    if (new_proto) {
        SV *svname;
        if (isGV(name)) {
            svname = sv_newmortal();
            gv_efullname3(svname, name, NULL);
        }
        else if (SvPOK(name) && *SvPVX((SV *)name) == '&')
            svname = newSVpvn_flags(SvPVX((SV *)name)+1, SvCUR(name)-1, SvUTF8(name)|SVs_TEMP);
        else
            svname = (SV *)name;
        if (ckWARN(WARN_ILLEGALPROTO))
            (void)validate_proto(svname, cSVOPx_sv(new_proto), TRUE,
                                 curstash);
        if (*proto && ckWARN(WARN_PROTOTYPE)) {
            STRLEN old_len, new_len;
            const char * oldp = SvPV(cSVOPx_sv(*proto), old_len);
            const char * newp = SvPV(cSVOPx_sv(new_proto), new_len);

            if (curstash && svname == (SV *)name
             && !memchr(SvPVX(svname), ':', SvCUR(svname))) {
                svname = sv_2mortal(newSVsv(PL_curstname));
                sv_catpvs(svname, "::");
                sv_catsv(svname, (SV *)name);
            }

            Perl_warner(aTHX_ packWARN(WARN_PROTOTYPE),
                "Prototype '%" UTF8f "' overridden by attribute 'prototype(%" UTF8f ")'"
                " in %" SVf,
                UTF8fARG(SvUTF8(cSVOPx_sv(*proto)), old_len, oldp),
                UTF8fARG(SvUTF8(cSVOPx_sv(new_proto)), new_len, newp),
                SVfARG(svname));
        }
        if (*proto)
            op_free(*proto);
        *proto = new_proto;
    }
}

static void
S_cant_declare(pTHX_ OP *o)
{
    if (o->op_type == OP_NULL
     && (o->op_flags & (OPf_SPECIAL|OPf_KIDS)) == OPf_KIDS)
        o = cUNOPo->op_first;
    yyerror(Perl_form(aTHX_ "Can't declare %s in \"%s\"",
                             o->op_type == OP_NULL
                               && o->op_flags & OPf_SPECIAL
                                 ? "do block"
                                 : OP_DESC(o),
                             PL_parser->in_my == KEY_our   ? "our"   :
                             PL_parser->in_my == KEY_state ? "state" :
                                                             "my"));
}

STATIC OP *
S_my_kid(pTHX_ OP *o, OP *attrs, OP **imopsp)
{
    I32 type;
    const bool stately = PL_parser && PL_parser->in_my == KEY_state;

    PERL_ARGS_ASSERT_MY_KID;

    if (!o || (PL_parser && PL_parser->error_count))
        return o;

    type = o->op_type;

    if (OP_TYPE_IS_OR_WAS(o, OP_LIST)) {
        OP *kid;
        for (kid = cLISTOPo->op_first; kid; kid = OpSIBLING(kid))
            my_kid(kid, attrs, imopsp);
        return o;
    } else if (type == OP_UNDEF || type == OP_STUB) {
        return o;
    } else if (type == OP_RV2SV ||	/* "our" declaration */
               type == OP_RV2AV ||
               type == OP_RV2HV) {
        if (cUNOPo->op_first->op_type != OP_GV) { /* MJD 20011224 */
            S_cant_declare(aTHX_ o);
        } else if (attrs) {
            GV * const gv = cGVOPx_gv(cUNOPo->op_first);
            assert(PL_parser);
            PL_parser->in_my = FALSE;
            PL_parser->in_my_stash = NULL;
            apply_attrs(GvSTASH(gv),
                        (type == OP_RV2SV ? GvSVn(gv) :
                         type == OP_RV2AV ? MUTABLE_SV(GvAVn(gv)) :
                         type == OP_RV2HV ? MUTABLE_SV(GvHVn(gv)) : MUTABLE_SV(gv)),
                        attrs);
        }
        o->op_private |= OPpOUR_INTRO;
        return o;
    }
    else if (type == OP_REFGEN || type == OP_SREFGEN) {
        if (!FEATURE_MYREF_IS_ENABLED)
            Perl_croak(aTHX_ "The experimental declared_refs "
                             "feature is not enabled");
        Perl_ck_warner_d(aTHX_
             packWARN(WARN_EXPERIMENTAL__DECLARED_REFS),
            "Declaring references is experimental");
        /* Kid is a nulled OP_LIST, handled above.  */
        my_kid(cUNOPo->op_first, attrs, imopsp);
        return o;
    }
    else if (type != OP_PADSV &&
             type != OP_PADAV &&
             type != OP_PADHV &&
             type != OP_PUSHMARK)
    {
        S_cant_declare(aTHX_ o);
        return o;
    }
    else if (attrs && type != OP_PUSHMARK) {
        HV *stash;

        assert(PL_parser);
        PL_parser->in_my = FALSE;
        PL_parser->in_my_stash = NULL;

        /* check for C<my Dog $spot> when deciding package */
        stash = PAD_COMPNAME_TYPE(o->op_targ);
        if (!stash)
            stash = PL_curstash;
        apply_attrs_my(stash, o, attrs, imopsp);
    }
    o->op_flags |= OPf_MOD;
    o->op_private |= OPpLVAL_INTRO;
    if (stately)
        o->op_private |= OPpPAD_STATE;
    return o;
}

OP *
Perl_my_attrs(pTHX_ OP *o, OP *attrs)
{
    OP *rops;
    int maybe_scalar = 0;

    PERL_ARGS_ASSERT_MY_ATTRS;

/* [perl #17376]: this appears to be premature, and results in code such as
   C< our(%x); > executing in list mode rather than void mode */
#if 0
    if (o->op_flags & OPf_PARENS)
        list(o);
    else
        maybe_scalar = 1;
#else
    maybe_scalar = 1;
#endif
    if (attrs)
        SAVEFREEOP(attrs);
    rops = NULL;
    o = my_kid(o, attrs, &rops);
    if (rops) {
        if (maybe_scalar && o->op_type == OP_PADSV) {
            o = scalar(op_append_list(OP_LIST, rops, o));
            o->op_private |= OPpLVAL_INTRO;
        }
        else {
            /* The listop in rops might have a pushmark at the beginning,
               which will mess up list assignment. */
            LISTOP * const lrops = cLISTOPx(rops); /* for brevity */
            if (rops->op_type == OP_LIST &&
                lrops->op_first && lrops->op_first->op_type == OP_PUSHMARK)
            {
                OP * const pushmark = lrops->op_first;
                /* excise pushmark */
                op_sibling_splice(rops, NULL, 1, NULL);
                op_free(pushmark);
            }
            o = op_append_list(OP_LIST, o, rops);
        }
    }
    PL_parser->in_my = FALSE;
    PL_parser->in_my_stash = NULL;
    return o;
}

OP *
Perl_sawparens(pTHX_ OP *o)
{
    PERL_UNUSED_CONTEXT;
    if (o)
        o->op_flags |= OPf_PARENS;
    return o;
}

OP *
Perl_bind_match(pTHX_ I32 type, OP *left, OP *right)
{
    OP *o;
    bool ismatchop = 0;
    const OPCODE ltype = left->op_type;
    const OPCODE rtype = right->op_type;

    PERL_ARGS_ASSERT_BIND_MATCH;

    if ( (ltype == OP_RV2AV || ltype == OP_RV2HV || ltype == OP_PADAV
          || ltype == OP_PADHV) && ckWARN(WARN_MISC))
    {
      const char * const desc
          = PL_op_desc[(
                          rtype == OP_SUBST || rtype == OP_TRANS
                       || rtype == OP_TRANSR
                       )
                       ? (int)rtype : OP_MATCH];
      const bool isary = ltype == OP_RV2AV || ltype == OP_PADAV;
      SV * const name = op_varname(left);
      if (name)
        Perl_warner(aTHX_ packWARN(WARN_MISC),
             "Applying %s to %" SVf " will act on scalar(%" SVf ")",
             desc, SVfARG(name), SVfARG(name));
      else {
        const char * const sample = (isary
             ? "@array" : "%hash");
        Perl_warner(aTHX_ packWARN(WARN_MISC),
             "Applying %s to %s will act on scalar(%s)",
             desc, sample, sample);
      }
    }

    if (rtype == OP_CONST &&
        cSVOPx(right)->op_private & OPpCONST_BARE &&
        cSVOPx(right)->op_private & OPpCONST_STRICT)
    {
        no_bareword_allowed(right);
    }

    /* !~ doesn't make sense with /r, so error on it for now */
    if (rtype == OP_SUBST && (cPMOPx(right)->op_pmflags & PMf_NONDESTRUCT) &&
        type == OP_NOT)
        /* diag_listed_as: Using !~ with %s doesn't make sense */
        yyerror("Using !~ with s///r doesn't make sense");
    if (rtype == OP_TRANSR && type == OP_NOT)
        /* diag_listed_as: Using !~ with %s doesn't make sense */
        yyerror("Using !~ with tr///r doesn't make sense");

    ismatchop = (rtype == OP_MATCH ||
                 rtype == OP_SUBST ||
                 rtype == OP_TRANS || rtype == OP_TRANSR)
             && !(right->op_flags & OPf_SPECIAL);
    if (ismatchop && right->op_private & OPpTARGET_MY) {
        right->op_targ = 0;
        right->op_private &= ~OPpTARGET_MY;
    }
    if (!(right->op_flags & OPf_STACKED) && !right->op_targ && ismatchop) {
        if (left->op_type == OP_PADSV
         && !(left->op_private & OPpLVAL_INTRO))
        {
            right->op_targ = left->op_targ;
            op_free(left);
            o = right;
        }
        else {
            right->op_flags |= OPf_STACKED;
            if (rtype != OP_MATCH && rtype != OP_TRANSR &&
            ! (rtype == OP_TRANS &&
               right->op_private & OPpTRANS_IDENTICAL) &&
            ! (rtype == OP_SUBST &&
               (cPMOPx(right)->op_pmflags & PMf_NONDESTRUCT)))
                left = op_lvalue(left, rtype);
            if (right->op_type == OP_TRANS || right->op_type == OP_TRANSR)
                o = newBINOP(OP_NULL, OPf_STACKED, scalar(left), right);
            else
                o = op_prepend_elem(rtype, scalar(left), right);
        }
        if (type == OP_NOT)
            return newUNOP(OP_NOT, 0, scalar(o));
        return o;
    }
    else
        return bind_match(type, left,
                pmruntime(newPMOP(OP_MATCH, 0), right, NULL, 0, 0));
}

OP *
Perl_invert(pTHX_ OP *o)
{
    if (!o)
        return NULL;
    return newUNOP(OP_NOT, OPf_SPECIAL, scalar(o));
}

OP *
Perl_cmpchain_start(pTHX_ I32 type, OP *left, OP *right)
{
    BINOP *bop;
    OP *op;

    if (!left)
        left = newOP(OP_NULL, 0);
    if (!right)
        right = newOP(OP_NULL, 0);
    scalar(left);
    scalar(right);
    NewOp(0, bop, 1, BINOP);
    op = (OP*)bop;
    ASSUME((PL_opargs[type] & OA_CLASS_MASK) == OA_BINOP);
    OpTYPE_set(op, type);
    cBINOPx(op)->op_flags = OPf_KIDS;
    cBINOPx(op)->op_private = 2;
    cBINOPx(op)->op_first = left;
    cBINOPx(op)->op_last = right;
    OpMORESIB_set(left, right);
    OpLASTSIB_set(right, op);
    return op;
}

OP *
Perl_cmpchain_extend(pTHX_ I32 type, OP *ch, OP *right)
{
    BINOP *bop;
    OP *op;

    PERL_ARGS_ASSERT_CMPCHAIN_EXTEND;
    if (!right)
        right = newOP(OP_NULL, 0);
    scalar(right);
    NewOp(0, bop, 1, BINOP);
    op = (OP*)bop;
    ASSUME((PL_opargs[type] & OA_CLASS_MASK) == OA_BINOP);
    OpTYPE_set(op, type);
    if (ch->op_type != OP_NULL) {
        UNOP *lch;
        OP *nch, *cleft, *cright;
        NewOp(0, lch, 1, UNOP);
        nch = (OP*)lch;
        OpTYPE_set(nch, OP_NULL);
        nch->op_flags = OPf_KIDS;
        cleft = cBINOPx(ch)->op_first;
        cright = cBINOPx(ch)->op_last;
        cBINOPx(ch)->op_first = NULL;
        cBINOPx(ch)->op_last = NULL;
        cBINOPx(ch)->op_private = 0;
        cBINOPx(ch)->op_flags = 0;
        cUNOPx(nch)->op_first = cright;
        OpMORESIB_set(cright, ch);
        OpMORESIB_set(ch, cleft);
        OpLASTSIB_set(cleft, nch);
        ch = nch;
    }
    OpMORESIB_set(right, op);
    OpMORESIB_set(op, cUNOPx(ch)->op_first);
    cUNOPx(ch)->op_first = right;
    return ch;
}

OP *
Perl_cmpchain_finish(pTHX_ OP *ch)
{

    PERL_ARGS_ASSERT_CMPCHAIN_FINISH;
    if (ch->op_type != OP_NULL) {
        OPCODE cmpoptype = ch->op_type;
        ch = CHECKOP(cmpoptype, ch);
        if(!ch->op_next && ch->op_type == cmpoptype)
            ch = fold_constants(op_integerize(op_std_init(ch)));
        return ch;
    } else {
        OP *condop = NULL;
        OP *rightarg = cUNOPx(ch)->op_first;
        cUNOPx(ch)->op_first = OpSIBLING(rightarg);
        OpLASTSIB_set(rightarg, NULL);
        while (1) {
            OP *cmpop = cUNOPx(ch)->op_first;
            OP *leftarg = OpSIBLING(cmpop);
            OPCODE cmpoptype = cmpop->op_type;
            OP *nextrightarg;
            bool is_last;
            is_last = !(cUNOPx(ch)->op_first = OpSIBLING(leftarg));
            OpLASTSIB_set(cmpop, NULL);
            OpLASTSIB_set(leftarg, NULL);
            if (is_last) {
                ch->op_flags = 0;
                op_free(ch);
                nextrightarg = NULL;
            } else {
                nextrightarg = newUNOP(OP_CMPCHAIN_DUP, 0, leftarg);
                leftarg = newOP(OP_NULL, 0);
            }
            cBINOPx(cmpop)->op_first = leftarg;
            cBINOPx(cmpop)->op_last = rightarg;
            OpMORESIB_set(leftarg, rightarg);
            OpLASTSIB_set(rightarg, cmpop);
            cmpop->op_flags = OPf_KIDS;
            cmpop->op_private = 2;
            cmpop = CHECKOP(cmpoptype, cmpop);
            if(!cmpop->op_next && cmpop->op_type == cmpoptype)
                cmpop = op_integerize(op_std_init(cmpop));
            condop = condop ? newLOGOP(OP_CMPCHAIN_AND, 0, cmpop, condop) :
                        cmpop;
            if (!nextrightarg)
                return condop;
            rightarg = nextrightarg;
        }
    }
}

/*
=for apidoc op_scope

Wraps up an op tree with some additional ops so that at runtime a dynamic
scope will be created.  The original ops run in the new dynamic scope,
and then, provided that they exit normally, the scope will be unwound.
The additional ops used to create and unwind the dynamic scope will
normally be an C<enter>/C<leave> pair, but a C<scope> op may be used
instead if the ops are simple enough to not need the full dynamic scope
structure.

=cut
*/

OP *
Perl_op_scope(pTHX_ OP *o)
{
    if (o) {
        if (o->op_flags & OPf_PARENS || PERLDB_NOOPT || TAINTING_get) {
            o = op_prepend_elem(OP_LINESEQ,
                    newOP(OP_ENTER, (o->op_flags & OPf_WANT)), o);
            OpTYPE_set(o, OP_LEAVE);
        }
        else if (o->op_type == OP_LINESEQ) {
            OP *kid;
            OpTYPE_set(o, OP_SCOPE);
            kid = cLISTOPo->op_first;
            if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE) {
                op_null(kid);

                /* The following deals with things like 'do {1 for 1}' */
                kid = OpSIBLING(kid);
                if (kid &&
                    (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE))
                    op_null(kid);
            }
        }
        else
            o = newLISTOP(OP_SCOPE, 0, o, NULL);
    }
    return o;
}

OP *
Perl_op_unscope(pTHX_ OP *o)
{
    if (o && o->op_type == OP_LINESEQ) {
        OP *kid = cLISTOPo->op_first;
        for(; kid; kid = OpSIBLING(kid))
            if (kid->op_type == OP_NEXTSTATE || kid->op_type == OP_DBSTATE)
                op_null(kid);
    }
    return o;
}

/*
=for apidoc block_start

Handles compile-time scope entry.
Arranges for hints to be restored on block
exit and also handles pad sequence numbers to make lexical variables scope
right.  Returns a savestack index for use with C<block_end>.

=cut
*/

int
Perl_block_start(pTHX_ int full)
{
    const int retval = PL_savestack_ix;

    PL_compiling.cop_seq = PL_cop_seqmax;
    COP_SEQMAX_INC;
    pad_block_start(full);
    SAVEHINTS();
    PL_hints &= ~HINT_BLOCK_SCOPE;
    SAVECOMPILEWARNINGS();
    PL_compiling.cop_warnings = DUP_WARNINGS(PL_compiling.cop_warnings);
    SAVEI32(PL_compiling.cop_seq);
    PL_compiling.cop_seq = 0;

    CALL_BLOCK_HOOKS(bhk_start, full);

    return retval;
}

/*
=for apidoc block_end

Handles compile-time scope exit.  C<floor>
is the savestack index returned by
C<block_start>, and C<seq> is the body of the block.  Returns the block,
possibly modified.

=cut
*/

OP*
Perl_block_end(pTHX_ I32 floor, OP *seq)
{
    const int needblockscope = PL_hints & HINT_BLOCK_SCOPE;
    OP* retval = voidnonfinal(seq);
    OP *o;

    /* XXX Is the null PL_parser check necessary here? */
    assert(PL_parser); /* Let’s find out under debugging builds.  */
    if (PL_parser && PL_parser->parsed_sub) {
        o = newSTATEOP(0, NULL, NULL);
        op_null(o);
        retval = op_append_elem(OP_LINESEQ, retval, o);
    }

    CALL_BLOCK_HOOKS(bhk_pre_end, &retval);

    LEAVE_SCOPE(floor);
    if (needblockscope)
        PL_hints |= HINT_BLOCK_SCOPE; /* propagate out */
    o = pad_leavemy();

    if (o) {
        /* pad_leavemy has created a sequence of introcv ops for all my
           subs declared in the block.  We have to replicate that list with
           clonecv ops, to deal with this situation:

               sub {
                   my sub s1;
                   my sub s2;
                   sub s1 { state sub foo { \&s2 } }
               }->()

           Originally, I was going to have introcv clone the CV and turn
           off the stale flag.  Since &s1 is declared before &s2, the
           introcv op for &s1 is executed (on sub entry) before the one for
           &s2.  But the &foo sub inside &s1 (which is cloned when &s1 is
           cloned, since it is a state sub) closes over &s2 and expects
           to see it in its outer CV’s pad.  If the introcv op clones &s1,
           then &s2 is still marked stale.  Since &s1 is not active, and
           &foo closes over &s1’s implicit entry for &s2, we get a ‘Varia-
           ble will not stay shared’ warning.  Because it is the same stub
           that will be used when the introcv op for &s2 is executed, clos-
           ing over it is safe.  Hence, we have to turn off the stale flag
           on all lexical subs in the block before we clone any of them.
           Hence, having introcv clone the sub cannot work.  So we create a
           list of ops like this:

               lineseq
                  |
                  +-- introcv
                  |
                  +-- introcv
                  |
                  +-- introcv
                  |
                  .
                  .
                  .
                  |
                  +-- clonecv
                  |
                  +-- clonecv
                  |
                  +-- clonecv
                  |
                  .
                  .
                  .
         */
        OP *kid = o->op_flags & OPf_KIDS ? cLISTOPo->op_first : o;
        OP * const last = o->op_flags & OPf_KIDS ? cLISTOPo->op_last : o;
        for (;; kid = OpSIBLING(kid)) {
            OP *newkid = newOP(OP_CLONECV, 0);
            newkid->op_targ = kid->op_targ;
            o = op_append_elem(OP_LINESEQ, o, newkid);
            if (kid == last) break;
        }
        retval = op_prepend_elem(OP_LINESEQ, o, retval);
    }

    CALL_BLOCK_HOOKS(bhk_post_end, &retval);

    return retval;
}

/*
=for apidoc_section $scope

=for apidoc blockhook_register

Register a set of hooks to be called when the Perl lexical scope changes
at compile time.  See L<perlguts/"Compile-time scope hooks">.

=cut
*/

void
Perl_blockhook_register(pTHX_ BHK *hk)
{
    PERL_ARGS_ASSERT_BLOCKHOOK_REGISTER;

    Perl_av_create_and_push(aTHX_ &PL_blockhooks, newSViv(PTR2IV(hk)));
}

void
Perl_newPROG(pTHX_ OP *o)
{
    OP *start;

    PERL_ARGS_ASSERT_NEWPROG;

    if (PL_in_eval) {
        PERL_CONTEXT *cx;
        I32 i;
        if (PL_eval_root)
                return;
        PL_eval_root = newUNOP(OP_LEAVEEVAL,
                               ((PL_in_eval & EVAL_KEEPERR)
                                ? OPf_SPECIAL : 0), o);

        cx = CX_CUR();
        assert(CxTYPE(cx) == CXt_EVAL);

        if ((cx->blk_gimme & G_WANT) == G_VOID)
            scalarvoid(PL_eval_root);
        else if ((cx->blk_gimme & G_WANT) == G_LIST)
            list(PL_eval_root);
        else
            scalar(PL_eval_root);

        start = op_linklist(PL_eval_root);
        PL_eval_root->op_next = 0;
        i = PL_savestack_ix;
        SAVEFREEOP(o);
        ENTER;
        S_process_optree(aTHX_ NULL, PL_eval_root, start);
        LEAVE;
        PL_savestack_ix = i;
    }
    else {
        if (o->op_type == OP_STUB) {
            /* This block is entered if nothing is compiled for the main
               program. This will be the case for an genuinely empty main
               program, or one which only has BEGIN blocks etc, so already
               run and freed.

               Historically (5.000) the guard above was !o. However, commit
               f8a08f7b8bd67b28 (Jun 2001), integrated to blead as
               c71fccf11fde0068, changed perly.y so that newPROG() is now
               called with the output of block_end(), which returns a new
               OP_STUB for the case of an empty optree. ByteLoader (and
               maybe other things) also take this path, because they set up
               PL_main_start and PL_main_root directly, without generating an
               optree.

               If the parsing the main program aborts (due to parse errors,
               or due to BEGIN or similar calling exit), then newPROG()
               isn't even called, and hence this code path and its cleanups
               are skipped. This shouldn't make a make a difference:
               * a non-zero return from perl_parse is a failure, and
                 perl_destruct() should be called immediately.
               * however, if exit(0) is called during the parse, then
                 perl_parse() returns 0, and perl_run() is called. As
                 PL_main_start will be NULL, perl_run() will return
                 promptly, and the exit code will remain 0.
            */

            PL_comppad_name = 0;
            PL_compcv = 0;
            S_op_destroy(aTHX_ o);
            return;
        }
        PL_main_root = op_scope(sawparens(scalarvoid(o)));
        PL_curcop = &PL_compiling;
        start = LINKLIST(PL_main_root);
        PL_main_root->op_next = 0;
        S_process_optree(aTHX_ NULL, PL_main_root, start);
        if (!PL_parser->error_count)
            /* on error, leave CV slabbed so that ops left lying around
             * will eb cleaned up. Else unslab */
            cv_forget_slab(PL_compcv);
        PL_compcv = 0;

        /* Register with debugger */
        if (PERLDB_INTER) {
            CV * const cv = get_cvs("DB::postponed", 0);
            if (cv) {
                dSP;
                PUSHMARK(SP);
                XPUSHs(MUTABLE_SV(CopFILEGV(&PL_compiling)));
                PUTBACK;
                call_sv(MUTABLE_SV(cv), G_DISCARD);
            }
        }
    }
}

OP *
Perl_localize(pTHX_ OP *o, I32 lex)
{
    PERL_ARGS_ASSERT_LOCALIZE;

    if (o->op_flags & OPf_PARENS)
/* [perl #17376]: this appears to be premature, and results in code such as
   C< our(%x); > executing in list mode rather than void mode */
#if 0
        list(o);
#else
        NOOP;
#endif
    else {
        if ( PL_parser->bufptr > PL_parser->oldbufptr
            && PL_parser->bufptr[-1] == ','
            && ckWARN(WARN_PARENTHESIS))
        {
            char *s = PL_parser->bufptr;
            bool sigil = FALSE;

            /* some heuristics to detect a potential error */
            while (*s && (memCHRs(", \t\n", *s)))
                s++;

            while (1) {
                if (*s && (memCHRs("@$%", *s) || (!lex && *s == '*'))
                       && *++s
                       && (isWORDCHAR(*s) || UTF8_IS_CONTINUED(*s))) {
                    s++;
                    sigil = TRUE;
                    while (*s && (isWORDCHAR(*s) || UTF8_IS_CONTINUED(*s)))
                        s++;
                    while (*s && (memCHRs(", \t\n", *s)))
                        s++;
                }
                else
                    break;
            }
            if (sigil && (*s == ';' || *s == '=')) {
                Perl_warner(aTHX_ packWARN(WARN_PARENTHESIS),
                                "Parentheses missing around \"%s\" list",
                                lex
                                    ? (PL_parser->in_my == KEY_our
                                        ? "our"
                                        : PL_parser->in_my == KEY_state
                                            ? "state"
                                            : "my")
                                    : "local");
            }
        }
    }
    if (lex)
        o = my(o);
    else
        o = op_lvalue(o, OP_NULL);		/* a bit kludgey */
    PL_parser->in_my = FALSE;
    PL_parser->in_my_stash = NULL;
    return o;
}

OP *
Perl_jmaybe(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_JMAYBE;

    if (o->op_type == OP_LIST) {
        if (FEATURE_MULTIDIMENSIONAL_IS_ENABLED) {
            OP * const o2
                = newSVREF(newGVOP(OP_GV, 0, gv_fetchpvs(";", GV_ADD|GV_NOTQUAL, SVt_PV)));
            o = op_convert_list(OP_JOIN, 0, op_prepend_elem(OP_LIST, o2, o));
        }
        else {
            /* If the user disables this, then a warning might not be enough to alert
               them to a possible change of behaviour here, so throw an exception.
            */
            yyerror("Multidimensional hash lookup is disabled");
        }
    }
    return o;
}

PERL_STATIC_INLINE OP *
S_op_std_init(pTHX_ OP *o)
{
    I32 type = o->op_type;

    PERL_ARGS_ASSERT_OP_STD_INIT;

    if (PL_opargs[type] & OA_RETSCALAR)
        scalar(o);
    if (PL_opargs[type] & OA_TARGET && !o->op_targ)
        o->op_targ = pad_alloc(type, SVs_PADTMP);

    return o;
}

PERL_STATIC_INLINE OP *
S_op_integerize(pTHX_ OP *o)
{
    I32 type = o->op_type;

    PERL_ARGS_ASSERT_OP_INTEGERIZE;

    /* integerize op. */
    if ((PL_opargs[type] & OA_OTHERINT) && (PL_hints & HINT_INTEGER))
    {
        o->op_ppaddr = PL_ppaddr[++(o->op_type)];
    }

    if (type == OP_NEGATE)
        /* XXX might want a ck_negate() for this */
        cUNOPo->op_first->op_private &= ~OPpCONST_STRICT;

    return o;
}

/* This function exists solely to provide a scope to limit
   setjmp/longjmp() messing with auto variables.  It cannot be inlined because
   it uses setjmp
 */
STATIC int
S_fold_constants_eval(pTHX) {
    int ret = 0;
    dJMPENV;

    JMPENV_PUSH(ret);

    if (ret == 0) {
        CALLRUNOPS(aTHX);
    }

    JMPENV_POP;

    return ret;
}

static OP *
S_fold_constants(pTHX_ OP *const o)
{
    OP *curop;
    OP *newop;
    I32 type = o->op_type;
    bool is_stringify;
    SV *sv = NULL;
    int ret = 0;
    OP *old_next;
    SV * const oldwarnhook = PL_warnhook;
    SV * const olddiehook  = PL_diehook;
    COP not_compiling;
    U8 oldwarn = PL_dowarn;
    I32 old_cxix;

    PERL_ARGS_ASSERT_FOLD_CONSTANTS;

    if (!(PL_opargs[type] & OA_FOLDCONST))
        goto nope;

    switch (type) {
    case OP_UCFIRST:
    case OP_LCFIRST:
    case OP_UC:
    case OP_LC:
    case OP_FC:
#ifdef USE_LOCALE_CTYPE
        if (IN_LC_COMPILETIME(LC_CTYPE))
            goto nope;
#endif
        break;
    case OP_SLT:
    case OP_SGT:
    case OP_SLE:
    case OP_SGE:
    case OP_SCMP:
#ifdef USE_LOCALE_COLLATE
        if (IN_LC_COMPILETIME(LC_COLLATE))
            goto nope;
#endif
        break;
    case OP_SPRINTF:
        /* XXX what about the numeric ops? */
#ifdef USE_LOCALE_NUMERIC
        if (IN_LC_COMPILETIME(LC_NUMERIC))
            goto nope;
#endif
        break;
    case OP_PACK:
        if (!OpHAS_SIBLING(cLISTOPo->op_first)
          || OpSIBLING(cLISTOPo->op_first)->op_type != OP_CONST)
            goto nope;
        {
            SV * const sv = cSVOPx_sv(OpSIBLING(cLISTOPo->op_first));
            if (!SvPOK(sv) || SvGMAGICAL(sv)) goto nope;
            {
                const char *s = SvPVX_const(sv);
                while (s < SvEND(sv)) {
                    if (isALPHA_FOLD_EQ(*s, 'p')) goto nope;
                    s++;
                }
            }
        }
        break;
    case OP_REPEAT:
        if (o->op_private & OPpREPEAT_DOLIST) goto nope;
        break;
    case OP_SREFGEN:
        if (cUNOPx(cUNOPo->op_first)->op_first->op_type != OP_CONST
         || SvPADTMP(cSVOPx_sv(cUNOPx(cUNOPo->op_first)->op_first)))
            goto nope;
    }

    if (PL_parser && PL_parser->error_count)
        goto nope;		/* Don't try to run w/ errors */

    for (curop = LINKLIST(o); curop != o; curop = LINKLIST(curop)) {
        switch (curop->op_type) {
        case OP_CONST:
            if (   (curop->op_private & OPpCONST_BARE)
                && (curop->op_private & OPpCONST_STRICT)) {
                no_bareword_allowed(curop);
                goto nope;
            }
            /* FALLTHROUGH */
        case OP_LIST:
        case OP_SCALAR:
        case OP_NULL:
        case OP_PUSHMARK:
            /* Foldable; move to next op in list */
            break;

        default:
            /* No other op types are considered foldable */
            goto nope;
        }
    }

    curop = LINKLIST(o);
    old_next = o->op_next;
    o->op_next = 0;
    PL_op = curop;

    old_cxix = cxstack_ix;
    create_eval_scope(NULL, G_FAKINGEVAL);

    /* Verify that we don't need to save it:  */
    assert(PL_curcop == &PL_compiling);
    StructCopy(&PL_compiling, &not_compiling, COP);
    PL_curcop = &not_compiling;
    /* The above ensures that we run with all the correct hints of the
       currently compiling COP, but that IN_PERL_RUNTIME is true. */
    assert(IN_PERL_RUNTIME);
    PL_warnhook = PERL_WARNHOOK_FATAL;
    PL_diehook  = NULL;

    /* Effective $^W=1.  */
    if ( ! (PL_dowarn & G_WARN_ALL_MASK))
        PL_dowarn |= G_WARN_ON;

    ret = S_fold_constants_eval(aTHX);

    switch (ret) {
    case 0:
        sv = *(PL_stack_sp--);
        if (o->op_targ && sv == PAD_SV(o->op_targ)) {	/* grab pad temp? */
            pad_swipe(o->op_targ,  FALSE);
        }
        else if (SvTEMP(sv)) {			/* grab mortal temp? */
            SvREFCNT_inc_simple_void(sv);
            SvTEMP_off(sv);
        }
        else { assert(SvIMMORTAL(sv)); }
        break;
    case 3:
        /* Something tried to die.  Abandon constant folding.  */
        /* Pretend the error never happened.  */
        CLEAR_ERRSV();
        o->op_next = old_next;
        break;
    default:
        /* Don't expect 1 (setjmp failed) or 2 (something called my_exit)  */
        PL_warnhook = oldwarnhook;
        PL_diehook  = olddiehook;
        /* XXX note that this croak may fail as we've already blown away
         * the stack - eg any nested evals */
        Perl_croak(aTHX_ "panic: fold_constants JMPENV_PUSH returned %d", ret);
    }
    PL_dowarn   = oldwarn;
    PL_warnhook = oldwarnhook;
    PL_diehook  = olddiehook;
    PL_curcop = &PL_compiling;

    /* if we croaked, depending on how we croaked the eval scope
     * may or may not have already been popped */
    if (cxstack_ix > old_cxix) {
        assert(cxstack_ix == old_cxix + 1);
        assert(CxTYPE(CX_CUR()) == CXt_EVAL);
        delete_eval_scope();
    }
    if (ret)
        goto nope;

    /* OP_STRINGIFY and constant folding are used to implement qq.
       Here the constant folding is an implementation detail that we
       want to hide.  If the stringify op is itself already marked
       folded, however, then it is actually a folded join.  */
    is_stringify = type == OP_STRINGIFY && !o->op_folded;
    op_free(o);
    assert(sv);
    if (is_stringify)
        SvPADTMP_off(sv);
    else if (!SvIMMORTAL(sv)) {
        SvPADTMP_on(sv);
        SvREADONLY_on(sv);
    }
    newop = newSVOP(OP_CONST, 0, MUTABLE_SV(sv));
    if (!is_stringify) newop->op_folded = 1;
    return newop;

 nope:
    return o;
}

/* convert a constant range in list context into an OP_RV2AV, OP_CONST pair;
 * the constant value being an AV holding the flattened range.
 */

static void
S_gen_constant_list(pTHX_ OP *o)
{
    OP *curop, *old_next;
    SV * const oldwarnhook = PL_warnhook;
    SV * const olddiehook  = PL_diehook;
    COP *old_curcop;
    U8 oldwarn = PL_dowarn;
    SV **svp;
    AV *av;
    I32 old_cxix;
    COP not_compiling;
    int ret = 0;
    dJMPENV;
    bool op_was_null;

    list(o);
    if (PL_parser && PL_parser->error_count)
        return;		/* Don't attempt to run with errors */

    curop = LINKLIST(o);
    old_next = o->op_next;
    o->op_next = 0;
    op_was_null = o->op_type == OP_NULL;
    if (op_was_null) /* b3698342565fb462291fba4b432cfcd05b6eb4e1 */
        o->op_type = OP_CUSTOM;
    CALL_PEEP(curop);
    if (op_was_null)
        o->op_type = OP_NULL;
    op_prune_chain_head(&curop);
    PL_op = curop;

    old_cxix = cxstack_ix;
    create_eval_scope(NULL, G_FAKINGEVAL);

    old_curcop = PL_curcop;
    StructCopy(old_curcop, &not_compiling, COP);
    PL_curcop = &not_compiling;
    /* The above ensures that we run with all the correct hints of the
       current COP, but that IN_PERL_RUNTIME is true. */
    assert(IN_PERL_RUNTIME);
    PL_warnhook = PERL_WARNHOOK_FATAL;
    PL_diehook  = NULL;
    JMPENV_PUSH(ret);

    /* Effective $^W=1.  */
    if ( ! (PL_dowarn & G_WARN_ALL_MASK))
        PL_dowarn |= G_WARN_ON;

    switch (ret) {
    case 0:
#if defined DEBUGGING && !defined DEBUGGING_RE_ONLY
        PL_curstackinfo->si_stack_hwm = 0; /* stop valgrind complaining */
#endif
        Perl_pp_pushmark(aTHX);
        CALLRUNOPS(aTHX);
        PL_op = curop;
        assert (!(curop->op_flags & OPf_SPECIAL));
        assert(curop->op_type == OP_RANGE);
        Perl_pp_anonlist(aTHX);
        break;
    case 3:
        CLEAR_ERRSV();
        o->op_next = old_next;
        break;
    default:
        JMPENV_POP;
        PL_warnhook = oldwarnhook;
        PL_diehook = olddiehook;
        Perl_croak(aTHX_ "panic: gen_constant_list JMPENV_PUSH returned %d",
            ret);
    }

    JMPENV_POP;
    PL_dowarn = oldwarn;
    PL_warnhook = oldwarnhook;
    PL_diehook = olddiehook;
    PL_curcop = old_curcop;

    if (cxstack_ix > old_cxix) {
        assert(cxstack_ix == old_cxix + 1);
        assert(CxTYPE(CX_CUR()) == CXt_EVAL);
        delete_eval_scope();
    }
    if (ret)
        return;

    OpTYPE_set(o, OP_RV2AV);
    o->op_flags &= ~OPf_REF;	/* treat \(1..2) like an ordinary list */
    o->op_flags |= OPf_PARENS;	/* and flatten \(1..2,3) */
    o->op_opt = 0;		/* needs to be revisited in rpeep() */
    av = (AV *)SvREFCNT_inc_NN(*PL_stack_sp--);

    /* replace subtree with an OP_CONST */
    curop = cUNOPo->op_first;
    op_sibling_splice(o, NULL, -1, newSVOP(OP_CONST, 0, (SV *)av));
    op_free(curop);

    if (AvFILLp(av) != -1)
        for (svp = AvARRAY(av) + AvFILLp(av); svp >= AvARRAY(av); --svp)
        {
            SvPADTMP_on(*svp);
            SvREADONLY_on(*svp);
        }
    LINKLIST(o);
    list(o);
    return;
}

/*
=for apidoc_section $optree_manipulation
*/

enum {
    FORBID_LOOPEX_DEFAULT = (1<<0),
};

static void walk_ops_find_labels(pTHX_ OP *o, HV *gotolabels)
{
    switch(o->op_type) {
        case OP_NEXTSTATE:
        case OP_DBSTATE:
            {
                STRLEN label_len;
                U32 label_flags;
                const char *label_pv = CopLABEL_len_flags((COP *)o, &label_len, &label_flags);
                if(!label_pv)
                    break;

                SV *labelsv = newSVpvn_flags(label_pv, label_len, label_flags);
                SAVEFREESV(labelsv);

                sv_inc(HeVAL(hv_fetch_ent(gotolabels, labelsv, TRUE, 0)));
                break;
            }
    }

    if(!(o->op_flags & OPf_KIDS))
        return;

    OP *kid = cUNOPo->op_first;
    while(kid) {
        walk_ops_find_labels(aTHX_ kid, gotolabels);
        kid = OpSIBLING(kid);
    }
}

static void walk_ops_forbid(pTHX_ OP *o, U32 flags, HV *permittedloops, HV *permittedgotos, const char *blockname)
{
    bool is_loop = FALSE;
    SV *labelsv = NULL;

    switch(o->op_type) {
        case OP_NEXTSTATE:
        case OP_DBSTATE:
            PL_curcop = (COP *)o;
            return;

        case OP_RETURN:
            goto forbid;

        case OP_GOTO:
            {
                /* OPf_STACKED means either dynamically computed label or `goto &sub` */
                if(o->op_flags & OPf_STACKED)
                    goto forbid;

                SV *target = newSVpvn_utf8(cPVOPo->op_pv, strlen(cPVOPo->op_pv),
                        cPVOPo->op_private & OPpPV_IS_UTF8);
                SAVEFREESV(target);

                if(hv_fetch_ent(permittedgotos, target, FALSE, 0))
                    break;

                goto forbid;
            }

        case OP_NEXT:
        case OP_LAST:
        case OP_REDO:
            {
                /* OPf_SPECIAL means this is a default loopex */
                if(o->op_flags & OPf_SPECIAL) {
                    if(flags & FORBID_LOOPEX_DEFAULT)
                        goto forbid;

                    break;
                }
                /* OPf_STACKED means it's a dynamically computed label */
                if(o->op_flags & OPf_STACKED)
                    goto forbid;

                SV *target = newSVpv(cPVOPo->op_pv, strlen(cPVOPo->op_pv));
                if(cPVOPo->op_private & OPpPV_IS_UTF8)
                    SvUTF8_on(target);
                SAVEFREESV(target);

                if(hv_fetch_ent(permittedloops, target, FALSE, 0))
                    break;

                goto forbid;
            }

        case OP_LEAVELOOP:
            {
                STRLEN label_len;
                U32 label_flags;
                const char *label_pv = CopLABEL_len_flags(PL_curcop, &label_len, &label_flags);

                if(label_pv) {
                    labelsv = newSVpvn(label_pv, label_len);
                    if(label_flags & SVf_UTF8)
                        SvUTF8_on(labelsv);
                    SAVEFREESV(labelsv);

                    sv_inc(HeVAL(hv_fetch_ent(permittedloops, labelsv, TRUE, 0)));
                }

                is_loop = TRUE;
                break;
            }

forbid:
            /* diag_listed_as: Can't "%s" out of a "defer" block */
            /* diag_listed_as: Can't "%s" out of a "finally" block */
            croak("Can't \"%s\" out of %s", PL_op_name[o->op_type], blockname);

        default:
            break;
    }

    if(!(o->op_flags & OPf_KIDS))
        return;

    OP *kid = cUNOPo->op_first;
    while(kid) {
        walk_ops_forbid(aTHX_ kid, flags, permittedloops, permittedgotos, blockname);
        kid = OpSIBLING(kid);

        if(is_loop) {
            /* Now in the body of the loop; we can permit loopex default */
            flags &= ~FORBID_LOOPEX_DEFAULT;
        }
    }

    if(is_loop && labelsv) {
        HE *he = hv_fetch_ent(permittedloops, labelsv, FALSE, 0);
        if(SvIV(HeVAL(he)) > 1)
            sv_dec(HeVAL(he));
        else
            hv_delete_ent(permittedloops, labelsv, 0, 0);
    }
}

/*
=for apidoc forbid_outofblock_ops

Checks an optree that implements a block, to ensure there are no control-flow
ops that attempt to leave the block.  Any C<OP_RETURN> is forbidden, as is any
C<OP_GOTO>. Loops are analysed, so any LOOPEX op (C<OP_NEXT>, C<OP_LAST> or
C<OP_REDO>) that affects a loop that contains it within the block are
permitted, but those that do not are forbidden.

If any of these forbidden constructions are detected, an exception is thrown
by using the op name and the blockname argument to construct a suitable
message.

This function alone is not sufficient to ensure the optree does not perform
any of these forbidden activities during runtime, as it might call a different
function that performs a non-local LOOPEX, or a string-eval() that performs a
C<goto>, or various other things. It is intended purely as a compile-time
check for those that could be detected statically. Additional runtime checks
may be required depending on the circumstance it is used for.

Note currently that I<all> C<OP_GOTO> ops are forbidden, even in cases where
they might otherwise be safe to execute.  This may be permitted in a later
version.

=cut
*/

void
Perl_forbid_outofblock_ops(pTHX_ OP *o, const char *blockname)
{
    PERL_ARGS_ASSERT_FORBID_OUTOFBLOCK_OPS;

    ENTER;
    SAVEVPTR(PL_curcop);

    HV *looplabels = newHV();
    SAVEFREESV((SV *)looplabels);

    HV *gotolabels = newHV();
    SAVEFREESV((SV *)gotolabels);

    walk_ops_find_labels(aTHX_ o, gotolabels);

    walk_ops_forbid(aTHX_ o, FORBID_LOOPEX_DEFAULT, looplabels, gotolabels, blockname);

    LEAVE;
}

/* List constructors */

/*
=for apidoc op_append_elem

Append an item to the list of ops contained directly within a list-type
op, returning the lengthened list.  C<first> is the list-type op,
and C<last> is the op to append to the list.  C<optype> specifies the
intended opcode for the list.  If C<first> is not already a list of the
right type, it will be upgraded into one.  If either C<first> or C<last>
is null, the other is returned unchanged.

=cut
*/

OP *
Perl_op_append_elem(pTHX_ I32 type, OP *first, OP *last)
{
    if (!first)
        return last;

    if (!last)
        return first;

    if (first->op_type != (unsigned)type
        || (type == OP_LIST && (first->op_flags & OPf_PARENS)))
    {
        return newLISTOP(type, 0, first, last);
    }

    op_sibling_splice(first, cLISTOPx(first)->op_last, 0, last);
    first->op_flags |= OPf_KIDS;
    return first;
}

/*
=for apidoc op_append_list

Concatenate the lists of ops contained directly within two list-type ops,
returning the combined list.  C<first> and C<last> are the list-type ops
to concatenate.  C<optype> specifies the intended opcode for the list.
If either C<first> or C<last> is not already a list of the right type,
it will be upgraded into one.  If either C<first> or C<last> is null,
the other is returned unchanged.

=cut
*/

OP *
Perl_op_append_list(pTHX_ I32 type, OP *first, OP *last)
{
    if (!first)
        return last;

    if (!last)
        return first;

    if (first->op_type != (unsigned)type)
        return op_prepend_elem(type, first, last);

    if (last->op_type != (unsigned)type)
        return op_append_elem(type, first, last);

    OpMORESIB_set(cLISTOPx(first)->op_last, cLISTOPx(last)->op_first);
    cLISTOPx(first)->op_last = cLISTOPx(last)->op_last;
    OpLASTSIB_set(cLISTOPx(first)->op_last, first);
    first->op_flags |= (last->op_flags & OPf_KIDS);

    S_op_destroy(aTHX_ last);

    return first;
}

/*
=for apidoc op_prepend_elem

Prepend an item to the list of ops contained directly within a list-type
op, returning the lengthened list.  C<first> is the op to prepend to the
list, and C<last> is the list-type op.  C<optype> specifies the intended
opcode for the list.  If C<last> is not already a list of the right type,
it will be upgraded into one.  If either C<first> or C<last> is null,
the other is returned unchanged.

=cut
*/

OP *
Perl_op_prepend_elem(pTHX_ I32 type, OP *first, OP *last)
{
    if (!first)
        return last;

    if (!last)
        return first;

    if (last->op_type == (unsigned)type) {
        if (type == OP_LIST) {	/* already a PUSHMARK there */
            /* insert 'first' after pushmark */
            op_sibling_splice(last, cLISTOPx(last)->op_first, 0, first);
            if (!(first->op_flags & OPf_PARENS))
                last->op_flags &= ~OPf_PARENS;
        }
        else
            op_sibling_splice(last, NULL, 0, first);
        last->op_flags |= OPf_KIDS;
        return last;
    }

    return newLISTOP(type, 0, first, last);
}

/*
=for apidoc op_convert_list

Converts C<o> into a list op if it is not one already, and then converts it
into the specified C<type>, calling its check function, allocating a target if
it needs one, and folding constants.

A list-type op is usually constructed one kid at a time via C<newLISTOP>,
C<op_prepend_elem> and C<op_append_elem>.  Then finally it is passed to
C<op_convert_list> to make it the right type.

=cut
*/

OP *
Perl_op_convert_list(pTHX_ I32 type, I32 flags, OP *o)
{
    if (type < 0) type = -type, flags |= OPf_SPECIAL;
    if (type == OP_RETURN) {
        if (FEATURE_MODULE_TRUE_IS_ENABLED)
            flags |= OPf_SPECIAL;
    }
    if (!o || o->op_type != OP_LIST)
        o = force_list(o, FALSE);
    else
    {
        o->op_flags &= ~OPf_WANT;
        o->op_private &= ~OPpLVAL_INTRO;
    }

    if (!(PL_opargs[type] & OA_MARK))
        op_null(cLISTOPo->op_first);
    else {
        OP * const kid2 = OpSIBLING(cLISTOPo->op_first);
        if (kid2 && kid2->op_type == OP_COREARGS) {
            op_null(cLISTOPo->op_first);
            kid2->op_private |= OPpCOREARGS_PUSHMARK;
        }
    }

    if (type != OP_SPLIT)
        /* At this point o is a LISTOP, but OP_SPLIT is a PMOP; let
         * ck_split() create a real PMOP and leave the op's type as listop
         * for now. Otherwise op_free() etc will crash.
         */
        OpTYPE_set(o, type);

    o->op_flags |= flags;
    if (flags & OPf_FOLDED)
        o->op_folded = 1;

    o = CHECKOP(type, o);
    if (o->op_type != (unsigned)type)
        return o;

    return fold_constants(op_integerize(op_std_init(o)));
}

/* Constructors */


/*
=for apidoc_section $optree_construction

=for apidoc newNULLLIST

Constructs, checks, and returns a new C<stub> op, which represents an
empty list expression.

=cut
*/

OP *
Perl_newNULLLIST(pTHX)
{
    return newOP(OP_STUB, 0);
}

/* promote o and any siblings to be a list if its not already; i.e.
 *
 *  o - A - B
 *
 * becomes
 *
 *  list
 *    |
 *  pushmark - o - A - B
 *
 * If nullit it true, the list op is nulled.
 */

static OP *
S_force_list(pTHX_ OP *o, bool nullit)
{
    if (!o || o->op_type != OP_LIST) {
        OP *rest = NULL;
        if (o) {
            /* manually detach any siblings then add them back later */
            rest = OpSIBLING(o);
            OpLASTSIB_set(o, NULL);
        }
        o = newLISTOP(OP_LIST, 0, o, NULL);
        if (rest)
            op_sibling_splice(o, cLISTOPo->op_last, 0, rest);
    }
    if (nullit)
        op_null(o);
    return o;
}

/*
=for apidoc op_force_list

Promotes o and any siblings to be an C<OP_LIST> if it is not already. If
a new C<OP_LIST> op was created, its first child will be C<OP_PUSHMARK>.
The returned node itself will be nulled, leaving only its children.

This is often what you want to do before putting the optree into list
context; as

    o = op_contextualize(op_force_list(o), G_LIST);

=cut
*/

OP *
Perl_op_force_list(pTHX_ OP *o)
{
    return force_list(o, TRUE);
}

/*
=for apidoc newLISTOP

Constructs, checks, and returns an op of any list type.  C<type> is
the opcode.  C<flags> gives the eight bits of C<op_flags>, except that
C<OPf_KIDS> will be set automatically if required.  C<first> and C<last>
supply up to two ops to be direct children of the list op; they are
consumed by this function and become part of the constructed op tree.

For most list operators, the check function expects all the kid ops to be
present already, so calling C<newLISTOP(OP_JOIN, ...)> (e.g.) is not
appropriate.  What you want to do in that case is create an op of type
C<OP_LIST>, append more children to it, and then call L</op_convert_list>.
See L</op_convert_list> for more information.

=cut
*/

OP *
Perl_newLISTOP(pTHX_ I32 type, I32 flags, OP *first, OP *last)
{
    LISTOP *listop;
    /* Note that allocating an OP_PUSHMARK can die under Safe.pm if
     * pushmark is banned. So do it now while existing ops are in a
     * consistent state, in case they suddenly get freed */
    OP* const pushop = type == OP_LIST ? newOP(OP_PUSHMARK, 0) : NULL;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_LISTOP
        || type == OP_CUSTOM);

    NewOp(1101, listop, 1, LISTOP);
    OpTYPE_set(listop, type);
    if (first || last)
        flags |= OPf_KIDS;
    listop->op_flags = (U8)flags;

    if (!last && first)
        last = first;
    else if (!first && last)
        first = last;
    else if (first)
        OpMORESIB_set(first, last);
    listop->op_first = first;
    listop->op_last = last;

    if (pushop) {
        OpMORESIB_set(pushop, first);
        listop->op_first = pushop;
        listop->op_flags |= OPf_KIDS;
        if (!last)
            listop->op_last = pushop;
    }
    if (listop->op_last)
        OpLASTSIB_set(listop->op_last, (OP*)listop);

    return CHECKOP(type, listop);
}

/*
=for apidoc newOP

Constructs, checks, and returns an op of any base type (any type that
has no extra fields).  C<type> is the opcode.  C<flags> gives the
eight bits of C<op_flags>, and, shifted up eight bits, the eight bits
of C<op_private>.

=cut
*/

OP *
Perl_newOP(pTHX_ I32 type, I32 flags)
{
    OP *o;

    if (type == -OP_ENTEREVAL) {
        type = OP_ENTEREVAL;
        flags |= OPpEVAL_BYTES<<8;
    }

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_BASEOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_BASEOP_OR_UNOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_FILESTATOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_LOOPEXOP);

    NewOp(1101, o, 1, OP);
    OpTYPE_set(o, type);
    o->op_flags = (U8)flags;

    o->op_next = o;
    o->op_private = (U8)(0 | (flags >> 8));
    if (PL_opargs[type] & OA_RETSCALAR)
        scalar(o);
    if (PL_opargs[type] & OA_TARGET)
        o->op_targ = pad_alloc(type, SVs_PADTMP);
    return CHECKOP(type, o);
}

/*
=for apidoc newUNOP

Constructs, checks, and returns an op of any unary type.  C<type> is
the opcode.  C<flags> gives the eight bits of C<op_flags>, except that
C<OPf_KIDS> will be set automatically if required, and, shifted up eight
bits, the eight bits of C<op_private>, except that the bit with value 1
is automatically set.  C<first> supplies an optional op to be the direct
child of the unary op; it is consumed by this function and become part
of the constructed op tree.

=for apidoc Amnh||OPf_KIDS

=cut
*/

OP *
Perl_newUNOP(pTHX_ I32 type, I32 flags, OP *first)
{
    UNOP *unop;

    if (type == -OP_ENTEREVAL) {
        type = OP_ENTEREVAL;
        flags |= OPpEVAL_BYTES<<8;
    }

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_UNOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_BASEOP_OR_UNOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_FILESTATOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_LOOPEXOP
        || type == OP_SASSIGN
        || type == OP_ENTERTRY
        || type == OP_ENTERTRYCATCH
        || type == OP_CUSTOM
        || type == OP_NULL );

    if (!first)
        first = newOP(OP_STUB, 0);
    if (PL_opargs[type] & OA_MARK)
        first = op_force_list(first);

    NewOp(1101, unop, 1, UNOP);
    OpTYPE_set(unop, type);
    unop->op_first = first;
    unop->op_flags = (U8)(flags | OPf_KIDS);
    unop->op_private = (U8)(1 | (flags >> 8));

    if (!OpHAS_SIBLING(first)) /* true unless weird syntax error */
        OpLASTSIB_set(first, (OP*)unop);

    unop = (UNOP*) CHECKOP(type, unop);
    if (unop->op_next)
        return (OP*)unop;

    return fold_constants(op_integerize(op_std_init((OP *) unop)));
}

/*
=for apidoc newUNOP_AUX

Similar to C<newUNOP>, but creates an C<UNOP_AUX> struct instead, with C<op_aux>
initialised to C<aux>

=cut
*/

OP *
Perl_newUNOP_AUX(pTHX_ I32 type, I32 flags, OP *first, UNOP_AUX_item *aux)
{
    UNOP_AUX *unop;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_UNOP_AUX
        || type == OP_CUSTOM);

    NewOp(1101, unop, 1, UNOP_AUX);
    unop->op_type = (OPCODE)type;
    unop->op_ppaddr = PL_ppaddr[type];
    unop->op_first = first;
    unop->op_flags = (U8)(flags | (first ? OPf_KIDS : 0));
    unop->op_private = (U8)((first ? 1 : 0) | (flags >> 8));
    unop->op_aux = aux;

    if (first && !OpHAS_SIBLING(first)) /* true unless weird syntax error */
        OpLASTSIB_set(first, (OP*)unop);

    unop = (UNOP_AUX*) CHECKOP(type, unop);

    return op_std_init((OP *) unop);
}

/*
=for apidoc newMETHOP

Constructs, checks, and returns an op of method type with a method name
evaluated at runtime.  C<type> is the opcode.  C<flags> gives the eight
bits of C<op_flags>, except that C<OPf_KIDS> will be set automatically,
and, shifted up eight bits, the eight bits of C<op_private>, except that
the bit with value 1 is automatically set.  C<dynamic_meth> supplies an
op which evaluates method name; it is consumed by this function and
become part of the constructed op tree.
Supported optypes: C<OP_METHOD>.

=cut
*/

static OP*
S_newMETHOP_internal(pTHX_ I32 type, I32 flags, OP* dynamic_meth, SV* const_meth) {
    METHOP *methop;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_METHOP
        || type == OP_CUSTOM);

    NewOp(1101, methop, 1, METHOP);
    if (dynamic_meth) {
        if (PL_opargs[type] & OA_MARK) dynamic_meth = op_force_list(dynamic_meth);
        methop->op_flags = (U8)(flags | OPf_KIDS);
        methop->op_u.op_first = dynamic_meth;
        methop->op_private = (U8)(1 | (flags >> 8));

        if (!OpHAS_SIBLING(dynamic_meth))
            OpLASTSIB_set(dynamic_meth, (OP*)methop);
    }
    else {
        assert(const_meth);
        methop->op_flags = (U8)(flags & ~OPf_KIDS);
        methop->op_u.op_meth_sv = const_meth;
        methop->op_private = (U8)(0 | (flags >> 8));
        methop->op_next = (OP*)methop;
    }

#ifdef USE_ITHREADS
    methop->op_rclass_targ = 0;
#else
    methop->op_rclass_sv = NULL;
#endif

    OpTYPE_set(methop, type);
    return CHECKOP(type, methop);
}

OP *
Perl_newMETHOP (pTHX_ I32 type, I32 flags, OP* dynamic_meth) {
    PERL_ARGS_ASSERT_NEWMETHOP;
    return newMETHOP_internal(type, flags, dynamic_meth, NULL);
}

/*
=for apidoc newMETHOP_named

Constructs, checks, and returns an op of method type with a constant
method name.  C<type> is the opcode.  C<flags> gives the eight bits of
C<op_flags>, and, shifted up eight bits, the eight bits of
C<op_private>.  C<const_meth> supplies a constant method name;
it must be a shared COW string.
Supported optypes: C<OP_METHOD_NAMED>.

=cut
*/

OP *
Perl_newMETHOP_named (pTHX_ I32 type, I32 flags, SV* const_meth) {
    PERL_ARGS_ASSERT_NEWMETHOP_NAMED;
    return newMETHOP_internal(type, flags, NULL, const_meth);
}

/*
=for apidoc newBINOP

Constructs, checks, and returns an op of any binary type.  C<type>
is the opcode.  C<flags> gives the eight bits of C<op_flags>, except
that C<OPf_KIDS> will be set automatically, and, shifted up eight bits,
the eight bits of C<op_private>, except that the bit with value 1 or
2 is automatically set as required.  C<first> and C<last> supply up to
two ops to be the direct children of the binary op; they are consumed
by this function and become part of the constructed op tree.

=cut
*/

OP *
Perl_newBINOP(pTHX_ I32 type, I32 flags, OP *first, OP *last)
{
    BINOP *binop;

    ASSUME((PL_opargs[type] & OA_CLASS_MASK) == OA_BINOP
        || type == OP_NULL || type == OP_CUSTOM);

    NewOp(1101, binop, 1, BINOP);

    if (!first)
        first = newOP(OP_NULL, 0);

    OpTYPE_set(binop, type);
    binop->op_first = first;
    binop->op_flags = (U8)(flags | OPf_KIDS);
    if (!last) {
        last = first;
        binop->op_private = (U8)(1 | (flags >> 8));
    }
    else {
        binop->op_private = (U8)(2 | (flags >> 8));
        OpMORESIB_set(first, last);
    }

    if (!OpHAS_SIBLING(last)) /* true unless weird syntax error */
        OpLASTSIB_set(last, (OP*)binop);

    binop->op_last = OpSIBLING(binop->op_first);
    if (binop->op_last)
        OpLASTSIB_set(binop->op_last, (OP*)binop);

    binop = (BINOP*) CHECKOP(type, binop);
    if (binop->op_next || binop->op_type != (OPCODE)type)
        return (OP*)binop;

    return fold_constants(op_integerize(op_std_init((OP *)binop)));
}

void
Perl_invmap_dump(pTHX_ SV* invlist, UV *map)
{
    const char indent[] = "    ";

    UV len = _invlist_len(invlist);
    UV * array = invlist_array(invlist);
    UV i;

    PERL_ARGS_ASSERT_INVMAP_DUMP;

    for (i = 0; i < len; i++) {
        UV start = array[i];
        UV end   = (i + 1 < len) ? array[i+1] - 1 : IV_MAX;

        PerlIO_printf(Perl_debug_log, "%s[%" UVuf "] 0x%04" UVXf, indent, i, start);
        if (end == IV_MAX) {
            PerlIO_printf(Perl_debug_log, " .. INFTY");
        }
        else if (end != start) {
            PerlIO_printf(Perl_debug_log, " .. 0x%04" UVXf, end);
        }
        else {
            PerlIO_printf(Perl_debug_log, "            ");
        }

        PerlIO_printf(Perl_debug_log, "\t");

        if (map[i] == TR_UNLISTED) {
            PerlIO_printf(Perl_debug_log, "TR_UNLISTED\n");
        }
        else if (map[i] == TR_SPECIAL_HANDLING) {
            PerlIO_printf(Perl_debug_log, "TR_SPECIAL_HANDLING\n");
        }
        else {
            PerlIO_printf(Perl_debug_log, "0x%04" UVXf "\n", map[i]);
        }
    }
}

/* Given an OP_TRANS / OP_TRANSR op o, plus OP_CONST ops expr and repl
 * containing the search and replacement strings, assemble into
 * a translation table attached as o->op_pv.
 * Free expr and repl.
 * It expects the toker to have already set the
 *   OPpTRANS_COMPLEMENT
 *   OPpTRANS_SQUASH
 *   OPpTRANS_DELETE
 * flags as appropriate; this function may add
 *   OPpTRANS_USE_SVOP
 *   OPpTRANS_CAN_FORCE_UTF8
 *   OPpTRANS_IDENTICAL
 *   OPpTRANS_GROWS
 * flags
 */

static OP *
S_pmtrans(pTHX_ OP *o, OP *expr, OP *repl)
{
    /* This function compiles a tr///, from data gathered from toke.c, into a
     * form suitable for use by do_trans() in doop.c at runtime.
     *
     * It first normalizes the data, while discarding extraneous inputs; then
     * writes out the compiled data.  The normalization allows for complete
     * analysis, and avoids some false negatives and positives earlier versions
     * of this code had.
     *
     * The normalization form is an inversion map (described below in detail).
     * This is essentially the compiled form for tr///'s that require UTF-8,
     * and its easy to use it to write the 257-byte table for tr///'s that
     * don't need UTF-8.  That table is identical to what's been in use for
     * many perl versions, except that it doesn't handle some edge cases that
     * it used to, involving code points above 255.  The UTF-8 form now handles
     * these.  (This could be changed with extra coding should it shown to be
     * desirable.)
     *
     * If the complement (/c) option is specified, the lhs string (tstr) is
     * parsed into an inversion list.  Complementing these is trivial.  Then a
     * complemented tstr is built from that, and used thenceforth.  This hides
     * the fact that it was complemented from almost all successive code.
     *
     * One of the important characteristics to know about the input is whether
     * the transliteration may be done in place, or does a temporary need to be
     * allocated, then copied.  If the replacement for every character in every
     * possible string takes up no more bytes than the character it
     * replaces, then it can be edited in place.  Otherwise the replacement
     * could overwrite a byte we are about to read, depending on the strings
     * being processed.  The comments and variable names here refer to this as
     * "growing".  Some inputs won't grow, and might even shrink under /d, but
     * some inputs could grow, so we have to assume any given one might grow.
     * On very long inputs, the temporary could eat up a lot of memory, so we
     * want to avoid it if possible.  For non-UTF-8 inputs, everything is
     * single-byte, so can be edited in place, unless there is something in the
     * pattern that could force it into UTF-8.  The inversion map makes it
     * feasible to determine this.  Previous versions of this code pretty much
     * punted on determining if UTF-8 could be edited in place.  Now, this code
     * is rigorous in making that determination.
     *
     * Another characteristic we need to know is whether the lhs and rhs are
     * identical.  If so, and no other flags are present, the only effect of
     * the tr/// is to count the characters present in the input that are
     * mentioned in the lhs string.  The implementation of that is easier and
     * runs faster than the more general case.  Normalizing here allows for
     * accurate determination of this.  Previously there were false negatives
     * possible.
     *
     * Instead of 'transliterated', the comments here use 'unmapped' for the
     * characters that are left unchanged by the operation; otherwise they are
     * 'mapped'
     *
     * The lhs of the tr/// is here referred to as the t side.
     * The rhs of the tr/// is here referred to as the r side.
     */

    SV * const tstr = cSVOPx(expr)->op_sv;
    SV * const rstr = cSVOPx(repl)->op_sv;
    STRLEN tlen;
    STRLEN rlen;
    const U8 * t0 = (U8*)SvPV_const(tstr, tlen);
    const U8 * r0 = (U8*)SvPV_const(rstr, rlen);
    const U8 * t = t0;
    const U8 * r = r0;
    UV t_count = 0, r_count = 0;  /* Number of characters in search and
                                         replacement lists */

    /* khw thinks some of the private flags for this op are quaintly named.
     * OPpTRANS_GROWS for example is TRUE if the replacement for some lhs
     * character when represented in UTF-8 is longer than the original
     * character's UTF-8 representation */
    const bool complement = cBOOL(o->op_private & OPpTRANS_COMPLEMENT);
    const bool squash     = cBOOL(o->op_private & OPpTRANS_SQUASH);
    const bool del        = cBOOL(o->op_private & OPpTRANS_DELETE);

    /* Set to true if there is some character < 256 in the lhs that maps to
     * above 255.  If so, a non-UTF-8 match string can be forced into being in
     * UTF-8 by a tr/// operation. */
    bool can_force_utf8 = FALSE;

    /* What is the maximum expansion factor in UTF-8 transliterations.  If a
     * 2-byte UTF-8 encoded character is to be replaced by a 3-byte one, its
     * expansion factor is 1.5.  This number is used at runtime to calculate
     * how much space to allocate for non-inplace transliterations.  Without
     * this number, the worst case is 14, which is extremely unlikely to happen
     * in real life, and could require significant memory overhead. */
    NV max_expansion = 1.;

    UV t_range_count, r_range_count, min_range_count;
    UV* t_array;
    SV* t_invlist;
    UV* r_map;
    UV r_cp = 0, t_cp = 0;
    UV t_cp_end = (UV) -1;
    UV r_cp_end;
    Size_t len;
    AV* invmap;
    UV final_map = TR_UNLISTED;    /* The final character in the replacement
                                      list, updated as we go along.  Initialize
                                      to something illegal */

    bool rstr_utf8 = cBOOL(SvUTF8(rstr));
    bool tstr_utf8 = cBOOL(SvUTF8(tstr));

    const U8* tend = t + tlen;
    const U8* rend = r + rlen;

    SV * inverted_tstr = NULL;

    Size_t i;
    unsigned int pass2;

    /* This routine implements detection of a transliteration having a longer
     * UTF-8 representation than its source, by partitioning all the possible
     * code points of the platform into equivalence classes of the same UTF-8
     * byte length in the first pass.  As it constructs the mappings, it carves
     * these up into smaller chunks, but doesn't merge any together.  This
     * makes it easy to find the instances it's looking for.  A second pass is
     * done after this has been determined which merges things together to
     * shrink the table for runtime.  The table below is used for both ASCII
     * and EBCDIC platforms.  On EBCDIC, the byte length is not monotonically
     * increasing for code points below 256.  To correct for that, the macro
     * CP_ADJUST defined below converts those code points to ASCII in the first
     * pass, and we use the ASCII partition values.  That works because the
     * growth factor will be unaffected, which is all that is calculated during
     * the first pass. */
    UV PL_partition_by_byte_length[] = {
        0,
        0x80,   /* Below this is 1 byte representations */
        (32 * (1UL << (    UTF_ACCUMULATION_SHIFT))),   /* 2 bytes below this */
        (16 * (1UL << (2 * UTF_ACCUMULATION_SHIFT))),   /* 3 bytes below this */
        ( 8 * (1UL << (3 * UTF_ACCUMULATION_SHIFT))),   /* 4 bytes below this */
        ( 4 * (1UL << (4 * UTF_ACCUMULATION_SHIFT))),   /* 5 bytes below this */
        ( 2 * (1UL << (5 * UTF_ACCUMULATION_SHIFT)))    /* 6 bytes below this */

#  ifdef UV_IS_QUAD
                                                    ,
        ( ((UV) 1U << (6 * UTF_ACCUMULATION_SHIFT)))    /* 7 bytes below this */
#  endif

    };

    PERL_ARGS_ASSERT_PMTRANS;

    PL_hints |= HINT_BLOCK_SCOPE;

    /* If /c, the search list is sorted and complemented.  This is now done by
     * creating an inversion list from it, and then trivially inverting that.
     * The previous implementation used qsort, but creating the list
     * automatically keeps it sorted as we go along */
    if (complement) {
        UV start, end;
        SV * inverted_tlist = _new_invlist(tlen);
        Size_t temp_len;

        DEBUG_y(PerlIO_printf(Perl_debug_log,
                    "%s: %d: tstr before inversion=\n%s\n",
                    __FILE__, __LINE__, _byte_dump_string(t, tend - t, 0)));

        while (t < tend) {

            /* Non-utf8 strings don't have ranges, so each character is listed
             * out */
            if (! tstr_utf8) {
                inverted_tlist = add_cp_to_invlist(inverted_tlist, *t);
                t++;
            }
            else {  /* But UTF-8 strings have been parsed in toke.c to have
                 * ranges if appropriate. */
                UV t_cp;
                Size_t t_char_len;

                /* Get the first character */
                t_cp = valid_utf8_to_uvchr(t, &t_char_len);
                t += t_char_len;

                /* If the next byte indicates that this wasn't the first
                 * element of a range, the range is just this one */
                if (t >= tend || *t != RANGE_INDICATOR) {
                    inverted_tlist = add_cp_to_invlist(inverted_tlist, t_cp);
                }
                else { /* Otherwise, ignore the indicator byte, and get the
                          final element, and add the whole range */
                    t++;
                    t_cp_end = valid_utf8_to_uvchr(t, &t_char_len);
                    t += t_char_len;

                    inverted_tlist = _add_range_to_invlist(inverted_tlist,
                                                      t_cp, t_cp_end);
                }
            }
        } /* End of parse through tstr */

        /* The inversion list is done; now invert it */
        _invlist_invert(inverted_tlist);

        /* Now go through the inverted list and create a new tstr for the rest
         * of the routine to use.  Since the UTF-8 version can have ranges, and
         * can be much more compact than the non-UTF-8 version, we create the
         * string in UTF-8 even if not necessary.  (This is just an intermediate
         * value that gets thrown away anyway.) */
        invlist_iterinit(inverted_tlist);
        inverted_tstr = newSVpvs("");
        while (invlist_iternext(inverted_tlist, &start, &end)) {
            U8 temp[UTF8_MAXBYTES];
            U8 * temp_end_pos;

            /* IV_MAX keeps things from going out of bounds */
            start = MIN(IV_MAX, start);
            end   = MIN(IV_MAX, end);

            temp_end_pos = uvchr_to_utf8(temp, start);
            sv_catpvn(inverted_tstr, (char *) temp, temp_end_pos - temp);

            if (start != end) {
                Perl_sv_catpvf(aTHX_ inverted_tstr, "%c", RANGE_INDICATOR);
                temp_end_pos = uvchr_to_utf8(temp, end);
                sv_catpvn(inverted_tstr, (char *) temp, temp_end_pos - temp);
            }
        }

        /* Set up so the remainder of the routine uses this complement, instead
         * of the actual input */
        t0 = t = (U8*)SvPV_const(inverted_tstr, temp_len);
        tend = t0 + temp_len;
        tstr_utf8 = TRUE;

        SvREFCNT_dec_NN(inverted_tlist);
    }

    /* For non-/d, an empty rhs means to use the lhs */
    if (rlen == 0 && ! del) {
        r0 = t0;
        rend = tend;
        rstr_utf8  = tstr_utf8;
    }

    t_invlist = _new_invlist(1);

    /* Initialize to a single range */
    t_invlist = _add_range_to_invlist(t_invlist, 0, UV_MAX);

    /* Below, we parse the (potentially adjusted) input, creating the inversion
     * map.  This is done in two passes.  The first pass is just to determine
     * if the transliteration can be done in-place.  It can be done in place if
     * no possible inputs result in the replacement taking up more bytes than
     * the input.  To figure that out, in the first pass we start with all the
     * possible code points partitioned into ranges so that every code point in
     * a range occupies the same number of UTF-8 bytes as every other code
     * point in the range.  Constructing the inversion map doesn't merge ranges
     * together, but can split them into multiple ones.  Given the starting
     * partition, the ending state will also have the same characteristic,
     * namely that each code point in each partition requires the same number
     * of UTF-8 bytes to represent as every other code point in the same
     * partition.
     *
     * This partitioning has been pre-compiled.  Copy it to initialize */
    len = C_ARRAY_LENGTH(PL_partition_by_byte_length);
    invlist_extend(t_invlist, len);
    t_array = invlist_array(t_invlist);
    Copy(PL_partition_by_byte_length, t_array, len, UV);
    invlist_set_len(t_invlist, len, *(get_invlist_offset_addr(t_invlist)));
    Newx(r_map, len + 1, UV);

    /* The inversion map the first pass creates could be used as-is, but
     * generally would be larger and slower to run than the output of the
     * second pass.  */

    for (pass2 = 0; pass2 < 2; pass2++) {
        if (pass2) {
            /* In the second pass, we start with a single range */
            t_invlist = _add_range_to_invlist(t_invlist, 0, UV_MAX);
            len = 1;
            t_array = invlist_array(t_invlist);
        }

/* As noted earlier, we convert EBCDIC code points to Unicode in the first pass
 * so as to get the well-behaved length 1 vs length 2 boundary.  Only code
 * points below 256 differ between the two character sets in this regard.  For
 * these, we also can't have any ranges, as they have to be individually
 * converted. */
#ifdef EBCDIC
#  define CP_ADJUST(x)          ((pass2) ? (x) : NATIVE_TO_UNI(x))
#  define FORCE_RANGE_LEN_1(x)  ((pass2) ? 0 : ((x) < 256))
#  define CP_SKIP(x)            ((pass2) ? UVCHR_SKIP(x) : OFFUNISKIP(x))
#else
#  define CP_ADJUST(x)          (x)
#  define FORCE_RANGE_LEN_1(x)  0
#  define CP_SKIP(x)            UVCHR_SKIP(x)
#endif

        /* And the mapping of each of the ranges is initialized.  Initially,
         * everything is TR_UNLISTED. */
        for (i = 0; i < len; i++) {
            r_map[i] = TR_UNLISTED;
        }

        t = t0;
        t_count = 0;
        r = r0;
        r_count = 0;
        t_range_count = r_range_count = 0;

        DEBUG_y(PerlIO_printf(Perl_debug_log, "%s: %d:\ntstr=%s\n",
                    __FILE__, __LINE__, _byte_dump_string(t, tend - t, 0)));
        DEBUG_y(PerlIO_printf(Perl_debug_log, "rstr=%s\n",
                                        _byte_dump_string(r, rend - r, 0)));
        DEBUG_y(PerlIO_printf(Perl_debug_log, "/c=%d; /s=%d; /d=%d\n",
                                                  complement, squash, del));
        DEBUG_y(invmap_dump(t_invlist, r_map));

        /* Now go through the search list constructing an inversion map.  The
         * input is not necessarily in any particular order.  Making it an
         * inversion map orders it, potentially simplifying, and makes it easy
         * to deal with at run time.  This is the only place in core that
         * generates an inversion map; if others were introduced, it might be
         * better to create general purpose routines to handle them.
         * (Inversion maps are created in perl in other places.)
         *
         * An inversion map consists of two parallel arrays.  One is
         * essentially an inversion list: an ordered list of code points such
         * that each element gives the first code point of a range of
         * consecutive code points that map to the element in the other array
         * that has the same index as this one (in other words, the
         * corresponding element).  Thus the range extends up to (but not
         * including) the code point given by the next higher element.  In a
         * true inversion map, the corresponding element in the other array
         * gives the mapping of the first code point in the range, with the
         * understanding that the next higher code point in the inversion
         * list's range will map to the next higher code point in the map.
         *
         * So if at element [i], let's say we have:
         *
         *     t_invlist  r_map
         * [i]    A         a
         *
         * This means that A => a, B => b, C => c....  Let's say that the
         * situation is such that:
         *
         * [i+1]  L        -1
         *
         * This means the sequence that started at [i] stops at K => k.  This
         * illustrates that you need to look at the next element to find where
         * a sequence stops.  Except, the highest element in the inversion list
         * begins a range that is understood to extend to the platform's
         * infinity.
         *
         * This routine modifies traditional inversion maps to reserve two
         * mappings:
         *
         *  TR_UNLISTED (or -1) indicates that no code point in the range
         *      is listed in the tr/// searchlist.  At runtime, these are
         *      always passed through unchanged.  In the inversion map, all
         *      points in the range are mapped to -1, instead of increasing,
         *      like the 'L' in the example above.
         *
         *      We start the parse with every code point mapped to this, and as
         *      we parse and find ones that are listed in the search list, we
         *      carve out ranges as we go along that override that.
         *
         *  TR_SPECIAL_HANDLING (or -2) indicates that every code point in the
         *      range needs special handling.  Again, all code points in the
         *      range are mapped to -2, instead of increasing.
         *
         *      Under /d this value means the code point should be deleted from
         *      the transliteration when encountered.
         *
         *      Otherwise, it marks that every code point in the range is to
         *      map to the final character in the replacement list.  This
         *      happens only when the replacement list is shorter than the
         *      search one, so there are things in the search list that have no
         *      correspondence in the replacement list.  For example, in
         *      tr/a-z/A/, 'A' is the final value, and the inversion map
         *      generated for this would be like this:
         *          \0  =>  -1
         *          a   =>   A
         *          b-z =>  -2
         *          z+1 =>  -1
         *      'A' appears once, then the remainder of the range maps to -2.
         *      The use of -2 isn't strictly necessary, as an inversion map is
         *      capable of representing this situation, but not nearly so
         *      compactly, and this is actually quite commonly encountered.
         *      Indeed, the original design of this code used a full inversion
         *      map for this.  But things like
         *          tr/\0-\x{FFFF}/A/
         *      generated huge data structures, slowly, and the execution was
         *      also slow.  So the current scheme was implemented.
         *
         *  So, if the next element in our example is:
         *
         * [i+2]  Q        q
         *
         * Then all of L, M, N, O, and P map to TR_UNLISTED.  If the next
         * elements are
         *
         * [i+3]  R        z
         * [i+4]  S       TR_UNLISTED
         *
         * Then Q => q; R => z; and S => TR_UNLISTED.  If [i+4] (the 'S') is
         * the final element in the arrays, every code point from S to infinity
         * maps to TR_UNLISTED.
         *
         */
                           /* Finish up range started in what otherwise would
                            * have been the final iteration */
        while (t < tend || t_range_count > 0) {
            bool adjacent_to_range_above = FALSE;
            bool adjacent_to_range_below = FALSE;

            bool merge_with_range_above = FALSE;
            bool merge_with_range_below = FALSE;

            UV span, invmap_range_length_remaining;
            SSize_t j;
            Size_t i;

            /* If we are in the middle of processing a range in the 'target'
             * side, the previous iteration has set us up.  Otherwise, look at
             * the next character in the search list */
            if (t_range_count <= 0) {
                if (! tstr_utf8) {

                    /* Here, not in the middle of a range, and not UTF-8.  The
                     * next code point is the single byte where we're at */
                    t_cp = CP_ADJUST(*t);
                    t_range_count = 1;
                    t++;
                }
                else {
                    Size_t t_char_len;

                    /* Here, not in the middle of a range, and is UTF-8.  The
                     * next code point is the next UTF-8 char in the input.  We
                     * know the input is valid, because the toker constructed
                     * it */
                    t_cp = CP_ADJUST(valid_utf8_to_uvchr(t, &t_char_len));
                    t += t_char_len;

                    /* UTF-8 strings (only) have been parsed in toke.c to have
                     * ranges.  See if the next byte indicates that this was
                     * the first element of a range.  If so, get the final
                     * element and calculate the range size.  If not, the range
                     * size is 1 */
                    if (   t < tend && *t == RANGE_INDICATOR
                        && ! FORCE_RANGE_LEN_1(t_cp))
                    {
                        t++;
                        t_range_count = valid_utf8_to_uvchr(t, &t_char_len)
                                      - t_cp + 1;
                        t += t_char_len;
                    }
                    else {
                        t_range_count = 1;
                    }
                }

                /* Count the total number of listed code points * */
                t_count += t_range_count;
            }

            /* Similarly, get the next character in the replacement list */
            if (r_range_count <= 0) {
                if (r >= rend) {

                    /* But if we've exhausted the rhs, there is nothing to map
                     * to, except the special handling one, and we make the
                     * range the same size as the lhs one. */
                    r_cp = TR_SPECIAL_HANDLING;
                    r_range_count = t_range_count;

                    if (! del) {
                        DEBUG_yv(PerlIO_printf(Perl_debug_log,
                                        "final_map =%" UVXf "\n", final_map));
                    }
                }
                else {
                    if (! rstr_utf8) {
                        r_cp = CP_ADJUST(*r);
                        r_range_count = 1;
                        r++;
                    }
                    else {
                        Size_t r_char_len;

                        r_cp = CP_ADJUST(valid_utf8_to_uvchr(r, &r_char_len));
                        r += r_char_len;
                        if (   r < rend && *r == RANGE_INDICATOR
                            && ! FORCE_RANGE_LEN_1(r_cp))
                        {
                            r++;
                            r_range_count = valid_utf8_to_uvchr(r,
                                                    &r_char_len) - r_cp + 1;
                            r += r_char_len;
                        }
                        else {
                            r_range_count = 1;
                        }
                    }

                    if (r_cp == TR_SPECIAL_HANDLING) {
                        r_range_count = t_range_count;
                    }

                    /* This is the final character so far */
                    final_map = r_cp + r_range_count - 1;

                    r_count += r_range_count;
                }
            }

            /* Here, we have the next things ready in both sides.  They are
             * potentially ranges.  We try to process as big a chunk as
             * possible at once, but the lhs and rhs must be synchronized, so
             * things like tr/A-Z/a-ij-z/ will need to be processed in 2 chunks
             * */
            min_range_count = MIN(t_range_count, r_range_count);

            /* Search the inversion list for the entry that contains the input
             * code point <cp>.  The inversion map was initialized to cover the
             * entire range of possible inputs, so this should not fail.  So
             * the return value is the index into the list's array of the range
             * that contains <cp>, that is, 'i' such that array[i] <= cp <
             * array[i+1] */
            j = _invlist_search(t_invlist, t_cp);
            assert(j >= 0);
            i = j;

            /* Here, the data structure might look like:
             *
             * index    t   r     Meaning
             * [i-1]    J   j   # J-L => j-l
             * [i]      M  -1   # M => default; as do N, O, P, Q
             * [i+1]    R   x   # R => x, S => x+1, T => x+2
             * [i+2]    U   y   # U => y, V => y+1, ...
             * ...
             * [-1]     Z  -1   # Z => default; as do Z+1, ... infinity
             *
             * where 'x' and 'y' above are not to be taken literally.
             *
             * The maximum chunk we can handle in this loop iteration, is the
             * smallest of the three components: the lhs 't_', the rhs 'r_',
             * and the remainder of the range in element [i].  (In pass 1, that
             * range will have everything in it be of the same class; we can't
             * cross into another class.)  'min_range_count' already contains
             * the smallest of the first two values.  The final one is
             * irrelevant if the map is to the special indicator */

            invmap_range_length_remaining = (i + 1 < len)
                                            ? t_array[i+1] - t_cp
                                            : IV_MAX - t_cp;
            span = MAX(1, MIN(min_range_count, invmap_range_length_remaining));

            /* The end point of this chunk is where we are, plus the span, but
             * never larger than the platform's infinity */
            t_cp_end = MIN(IV_MAX, t_cp + span - 1);

            if (r_cp == TR_SPECIAL_HANDLING) {

                /* If unmatched lhs code points map to the final map, use that
                 * value.  This being set to TR_SPECIAL_HANDLING indicates that
                 * we don't have a final map: unmatched lhs code points are
                 * simply deleted */
                r_cp_end = (del) ? TR_SPECIAL_HANDLING : final_map;
            }
            else {
                r_cp_end = MIN(IV_MAX, r_cp + span - 1);

                /* If something on the lhs is below 256, and something on the
                 * rhs is above, there is a potential mapping here across that
                 * boundary.  Indeed the only way there isn't is if both sides
                 * start at the same point.  That means they both cross at the
                 * same time.  But otherwise one crosses before the other */
                if (t_cp < 256 && r_cp_end > 255 && r_cp != t_cp) {
                    can_force_utf8 = TRUE;
                }
            }

            /* If a character appears in the search list more than once, the
             * 2nd and succeeding occurrences are ignored, so only do this
             * range if haven't already processed this character.  (The range
             * has been set up so that all members in it will be of the same
             * ilk) */
            if (r_map[i] == TR_UNLISTED) {
                DEBUG_yv(PerlIO_printf(Perl_debug_log,
                    "Processing %" UVxf "-%" UVxf " => %" UVxf "-%" UVxf "\n",
                    t_cp, t_cp_end, r_cp, r_cp_end));

                /* This is the first definition for this chunk, hence is valid
                 * and needs to be processed.  Here and in the comments below,
                 * we use the above sample data.  The t_cp chunk must be any
                 * contiguous subset of M, N, O, P, and/or Q.
                 *
                 * In the first pass, calculate if there is any possible input
                 * string that has a character whose transliteration will be
                 * longer than it.  If none, the transliteration may be done
                 * in-place, as it can't write over a so-far unread byte.
                 * Otherwise, a copy must first be made.  This could be
                 * expensive for long inputs.
                 *
                 * In the first pass, the t_invlist has been partitioned so
                 * that all elements in any single range have the same number
                 * of bytes in their UTF-8 representations.  And the r space is
                 * either a single byte, or a range of strictly monotonically
                 * increasing code points.  So the final element in the range
                 * will be represented by no fewer bytes than the initial one.
                 * That means that if the final code point in the t range has
                 * at least as many bytes as the final code point in the r,
                 * then all code points in the t range have at least as many
                 * bytes as their corresponding r range element.  But if that's
                 * not true, the transliteration of at least the final code
                 * point grows in length.  As an example, suppose we had
                 *      tr/\x{fff0}-\x{fff1}/\x{ffff}-\x{10000}/
                 * The UTF-8 for all but 10000 occupies 3 bytes on ASCII
                 * platforms.  We have deliberately set up the data structure
                 * so that any range in the lhs gets split into chunks for
                 * processing, such that every code point in a chunk has the
                 * same number of UTF-8 bytes.  We only have to check the final
                 * code point in the rhs against any code point in the lhs. */
                if ( ! pass2
                    && r_cp_end != TR_SPECIAL_HANDLING
                    && CP_SKIP(t_cp_end) < CP_SKIP(r_cp_end))
                {
                    /* Here, we will need to make a copy of the input string
                     * before doing the transliteration.  The worst possible
                     * case is an expansion ratio of 14:1. This is rare, and
                     * we'd rather allocate only the necessary amount of extra
                     * memory for that copy.  We can calculate the worst case
                     * for this particular transliteration is by keeping track
                     * of the expansion factor for each range.
                     *
                     * Consider tr/\xCB/\X{E000}/.  The maximum expansion
                     * factor is 1 byte going to 3 if the target string is not
                     * UTF-8, but 2 bytes going to 3 if it is in UTF-8.  We
                     * could pass two different values so doop could choose
                     * based on the UTF-8ness of the target.  But khw thinks
                     * (perhaps wrongly) that is overkill.  It is used only to
                     * make sure we malloc enough space.
                     *
                     * If no target string can force the result to be UTF-8,
                     * then we don't have to worry about the case of the target
                     * string not being UTF-8 */
                    NV t_size = (can_force_utf8 && t_cp < 256)
                                ? 1
                                : CP_SKIP(t_cp_end);
                    NV ratio = CP_SKIP(r_cp_end) / t_size;

                    o->op_private |= OPpTRANS_GROWS;

                    /* Now that we know it grows, we can keep track of the
                     * largest ratio */
                    if (ratio > max_expansion) {
                        max_expansion = ratio;
                        DEBUG_y(PerlIO_printf(Perl_debug_log,
                                        "New expansion factor: %" NVgf "\n",
                                        max_expansion));
                    }
                }

                /* The very first range is marked as adjacent to the
                 * non-existent range below it, as it causes things to "just
                 * work" (TradeMark)
                 *
                 * If the lowest code point in this chunk is M, it adjoins the
                 * J-L range */
                if (t_cp == t_array[i]) {
                    adjacent_to_range_below = TRUE;

                    /* And if the map has the same offset from the beginning of
                     * the range as does this new code point (or both are for
                     * TR_SPECIAL_HANDLING), this chunk can be completely
                     * merged with the range below.  EXCEPT, in the first pass,
                     * we don't merge ranges whose UTF-8 byte representations
                     * have different lengths, so that we can more easily
                     * detect if a replacement is longer than the source, that
                     * is if it 'grows'.  But in the 2nd pass, there's no
                     * reason to not merge */
                    if (   (i > 0 && (   pass2
                                      || CP_SKIP(t_array[i-1])
                                                            == CP_SKIP(t_cp)))
                        && (   (   r_cp == TR_SPECIAL_HANDLING
                                && r_map[i-1] == TR_SPECIAL_HANDLING)
                            || (   r_cp != TR_SPECIAL_HANDLING
                                && r_cp - r_map[i-1] == t_cp - t_array[i-1])))
                    {
                        merge_with_range_below = TRUE;
                    }
                }

                /* Similarly, if the highest code point in this chunk is 'Q',
                 * it adjoins the range above, and if the map is suitable, can
                 * be merged with it */
                if (    t_cp_end >= IV_MAX - 1
                    || (   i + 1 < len
                        && t_cp_end + 1 == t_array[i+1]))
                {
                    adjacent_to_range_above = TRUE;
                    if (i + 1 < len)
                    if (    (   pass2
                             || CP_SKIP(t_cp) == CP_SKIP(t_array[i+1]))
                        && (   (   r_cp == TR_SPECIAL_HANDLING
                                && r_map[i+1] == (UV) TR_SPECIAL_HANDLING)
                            || (   r_cp != TR_SPECIAL_HANDLING
                                && r_cp_end == r_map[i+1] - 1)))
                    {
                        merge_with_range_above = TRUE;
                    }
                }

                if (merge_with_range_below && merge_with_range_above) {

                    /* Here the new chunk looks like M => m, ... Q => q; and
                     * the range above is like R => r, ....  Thus, the [i-1]
                     * and [i+1] ranges should be seamlessly melded so the
                     * result looks like
                     *
                     * [i-1]    J   j   # J-T => j-t
                     * [i]      U   y   # U => y, V => y+1, ...
                     * ...
                     * [-1]     Z  -1   # Z => default; as do Z+1, ... infinity
                     */
                    Move(t_array + i + 2, t_array + i, len - i - 2, UV);
                    Move(r_map   + i + 2, r_map   + i, len - i - 2, UV);
                    len -= 2;
                    invlist_set_len(t_invlist,
                                    len,
                                    *(get_invlist_offset_addr(t_invlist)));
                }
                else if (merge_with_range_below) {

                    /* Here the new chunk looks like M => m, .... But either
                     * (or both) it doesn't extend all the way up through Q; or
                     * the range above doesn't start with R => r. */
                    if (! adjacent_to_range_above) {

                        /* In the first case, let's say the new chunk extends
                         * through O.  We then want:
                         *
                         * [i-1]    J   j   # J-O => j-o
                         * [i]      P  -1   # P => -1, Q => -1
                         * [i+1]    R   x   # R => x, S => x+1, T => x+2
                         * [i+2]    U   y   # U => y, V => y+1, ...
                         * ...
                         * [-1]     Z  -1   # Z => default; as do Z+1, ...
                         *                                            infinity
                         */
                        t_array[i] = t_cp_end + 1;
                        r_map[i] = TR_UNLISTED;
                    }
                    else { /* Adjoins the range above, but can't merge with it
                              (because 'x' is not the next map after q) */
                        /*
                         * [i-1]    J   j   # J-Q => j-q
                         * [i]      R   x   # R => x, S => x+1, T => x+2
                         * [i+1]    U   y   # U => y, V => y+1, ...
                         * ...
                         * [-1]     Z  -1   # Z => default; as do Z+1, ...
                         *                                          infinity
                         */

                        Move(t_array + i + 1, t_array + i, len - i - 1, UV);
                        Move(r_map + i + 1, r_map + i, len - i - 1, UV);
                        len--;
                        invlist_set_len(t_invlist, len,
                                        *(get_invlist_offset_addr(t_invlist)));
                    }
                }
                else if (merge_with_range_above) {

                    /* Here the new chunk ends with Q => q, and the range above
                     * must start with R => r, so the two can be merged. But
                     * either (or both) the new chunk doesn't extend all the
                     * way down to M; or the mapping of the final code point
                     * range below isn't m */
                    if (! adjacent_to_range_below) {

                        /* In the first case, let's assume the new chunk starts
                         * with P => p.  Then, because it's merge-able with the
                         * range above, that range must be R => r.  We want:
                         *
                         * [i-1]    J   j   # J-L => j-l
                         * [i]      M  -1   # M => -1, N => -1
                         * [i+1]    P   p   # P-T => p-t
                         * [i+2]    U   y   # U => y, V => y+1, ...
                         * ...
                         * [-1]     Z  -1   # Z => default; as do Z+1, ...
                         *                                          infinity
                         */
                        t_array[i+1] = t_cp;
                        r_map[i+1] = r_cp;
                    }
                    else { /* Adjoins the range below, but can't merge with it
                            */
                        /*
                         * [i-1]    J   j   # J-L => j-l
                         * [i]      M   x   # M-T => x-5 .. x+2
                         * [i+1]    U   y   # U => y, V => y+1, ...
                         * ...
                         * [-1]     Z  -1   # Z => default; as do Z+1, ...
                         *                                          infinity
                         */
                        Move(t_array + i + 1, t_array + i, len - i - 1, UV);
                        Move(r_map   + i + 1, r_map   + i, len - i - 1, UV);
                        len--;
                        t_array[i] = t_cp;
                        r_map[i] = r_cp;
                        invlist_set_len(t_invlist, len,
                                        *(get_invlist_offset_addr(t_invlist)));
                    }
                }
                else if (adjacent_to_range_below && adjacent_to_range_above) {
                    /* The new chunk completely fills the gap between the
                     * ranges on either side, but can't merge with either of
                     * them.
                     *
                     * [i-1]    J   j   # J-L => j-l
                     * [i]      M   z   # M => z, N => z+1 ... Q => z+4
                     * [i+1]    R   x   # R => x, S => x+1, T => x+2
                     * [i+2]    U   y   # U => y, V => y+1, ...
                     * ...
                     * [-1]     Z  -1   # Z => default; as do Z+1, ... infinity
                     */
                    r_map[i] = r_cp;
                }
                else if (adjacent_to_range_below) {
                    /* The new chunk adjoins the range below, but not the range
                     * above, and can't merge.  Let's assume the chunk ends at
                     * O.
                     *
                     * [i-1]    J   j   # J-L => j-l
                     * [i]      M   z   # M => z, N => z+1, O => z+2
                     * [i+1]    P   -1  # P => -1, Q => -1
                     * [i+2]    R   x   # R => x, S => x+1, T => x+2
                     * [i+3]    U   y   # U => y, V => y+1, ...
                     * ...
                     * [-w]     Z  -1   # Z => default; as do Z+1, ... infinity
                     */
                    invlist_extend(t_invlist, len + 1);
                    t_array = invlist_array(t_invlist);
                    Renew(r_map, len + 1, UV);

                    Move(t_array + i + 1, t_array + i + 2, len - i - 1, UV);
                    Move(r_map + i + 1,   r_map   + i + 2, len - i - 1, UV);
                    r_map[i] = r_cp;
                    t_array[i+1] = t_cp_end + 1;
                    r_map[i+1] = TR_UNLISTED;
                    len++;
                    invlist_set_len(t_invlist, len,
                                    *(get_invlist_offset_addr(t_invlist)));
                }
                else if (adjacent_to_range_above) {
                    /* The new chunk adjoins the range above, but not the range
                     * below, and can't merge.  Let's assume the new chunk
                     * starts at O
                     *
                     * [i-1]    J   j   # J-L => j-l
                     * [i]      M  -1   # M => default, N => default
                     * [i+1]    O   z   # O => z, P => z+1, Q => z+2
                     * [i+2]    R   x   # R => x, S => x+1, T => x+2
                     * [i+3]    U   y   # U => y, V => y+1, ...
                     * ...
                     * [-1]     Z  -1   # Z => default; as do Z+1, ... infinity
                     */
                    invlist_extend(t_invlist, len + 1);
                    t_array = invlist_array(t_invlist);
                    Renew(r_map, len + 1, UV);

                    Move(t_array + i + 1, t_array + i + 2, len - i - 1, UV);
                    Move(r_map   + i + 1, r_map   + i + 2, len - i - 1, UV);
                    t_array[i+1] = t_cp;
                    r_map[i+1] = r_cp;
                    len++;
                    invlist_set_len(t_invlist, len,
                                    *(get_invlist_offset_addr(t_invlist)));
                }
                else {
                    /* The new chunk adjoins neither the range above, nor the
                     * range below.  Lets assume it is N..P => n..p
                     *
                     * [i-1]    J   j   # J-L => j-l
                     * [i]      M  -1   # M => default
                     * [i+1]    N   n   # N..P => n..p
                     * [i+2]    Q  -1   # Q => default
                     * [i+3]    R   x   # R => x, S => x+1, T => x+2
                     * [i+4]    U   y   # U => y, V => y+1, ...
                     * ...
                     * [-1]     Z  -1   # Z => default; as do Z+1, ... infinity
                     */

                    DEBUG_yv(PerlIO_printf(Perl_debug_log,
                                        "Before fixing up: len=%d, i=%d\n",
                                        (int) len, (int) i));
                    DEBUG_yv(invmap_dump(t_invlist, r_map));

                    invlist_extend(t_invlist, len + 2);
                    t_array = invlist_array(t_invlist);
                    Renew(r_map, len + 2, UV);

                    Move(t_array + i + 1,
                         t_array + i + 2 + 1, len - i - (2 - 1), UV);
                    Move(r_map   + i + 1,
                         r_map   + i + 2 + 1, len - i - (2 - 1), UV);

                    len += 2;
                    invlist_set_len(t_invlist, len,
                                    *(get_invlist_offset_addr(t_invlist)));

                    t_array[i+1] = t_cp;
                    r_map[i+1] = r_cp;

                    t_array[i+2] = t_cp_end + 1;
                    r_map[i+2] = TR_UNLISTED;
                }
                DEBUG_yv(PerlIO_printf(Perl_debug_log,
                          "After iteration: span=%" UVuf ", t_range_count=%"
                          UVuf " r_range_count=%" UVuf "\n",
                          span, t_range_count, r_range_count));
                DEBUG_yv(invmap_dump(t_invlist, r_map));
            } /* End of this chunk needs to be processed */

            /* Done with this chunk. */
            t_cp += span;
            if (t_cp >= IV_MAX) {
                break;
            }
            t_range_count -= span;
            if (r_cp != TR_SPECIAL_HANDLING) {
                r_cp += span;
                r_range_count -= span;
            }
            else {
                r_range_count = 0;
            }

        } /* End of loop through the search list */

        /* We don't need an exact count, but we do need to know if there is
         * anything left over in the replacement list.  So, just assume it's
         * one byte per character */
        if (rend > r) {
            r_count++;
        }
    } /* End of passes */

    SvREFCNT_dec(inverted_tstr);

    DEBUG_y(PerlIO_printf(Perl_debug_log, "After everything: \n"));
    DEBUG_y(invmap_dump(t_invlist, r_map));

    /* We now have normalized the input into an inversion map.
     *
     * See if the lhs and rhs are equivalent.  If so, this tr/// is a no-op
     * except for the count, and streamlined runtime code can be used */
    if (!del && !squash) {

        /* They are identical if they point to the same address, or if
         * everything maps to UNLISTED or to itself.  This catches things that
         * not looking at the normalized inversion map doesn't catch, like
         * tr/aa/ab/ or tr/\x{100}-\x{104}/\x{100}-\x{102}\x{103}-\x{104}  */
        if (r0 != t0) {
            for (i = 0; i < len; i++) {
                if (r_map[i] != TR_UNLISTED && r_map[i] != t_array[i]) {
                    goto done_identical_check;
                }
            }
        }

        /* Here have gone through entire list, and didn't find any
         * non-identical mappings */
        o->op_private |= OPpTRANS_IDENTICAL;

      done_identical_check: ;
    }

    t_array = invlist_array(t_invlist);

    /* If has components above 255, we generally need to use the inversion map
     * implementation */
    if (   can_force_utf8
        || (   len > 0
            && t_array[len-1] > 255
                 /* If the final range is 0x100-INFINITY and is a special
                  * mapping, the table implementation can handle it */
            && ! (   t_array[len-1] == 256
                  && (   r_map[len-1] == TR_UNLISTED
                      || r_map[len-1] == TR_SPECIAL_HANDLING))))
    {
        SV* r_map_sv;
        SV* temp_sv;

        /* A UTF-8 op is generated, indicated by this flag.  This op is an
         * sv_op */
        o->op_private |= OPpTRANS_USE_SVOP;

        if (can_force_utf8) {
            o->op_private |= OPpTRANS_CAN_FORCE_UTF8;
        }

        /* The inversion map is pushed; first the list. */
        invmap = MUTABLE_AV(newAV());

        SvREADONLY_on(t_invlist);
        av_push(invmap, t_invlist);

        /* 2nd is the mapping */
        r_map_sv = newSVpvn((char *) r_map, len * sizeof(UV));
        SvREADONLY_on(r_map_sv);
        av_push(invmap, r_map_sv);

        /* 3rd is the max possible expansion factor */
        temp_sv = newSVnv(max_expansion);
        SvREADONLY_on(temp_sv);
        av_push(invmap, temp_sv);

        /* Characters that are in the search list, but not in the replacement
         * list are mapped to the final character in the replacement list */
        if (! del && r_count < t_count) {
            temp_sv = newSVuv(final_map);
            SvREADONLY_on(temp_sv);
            av_push(invmap, temp_sv);
        }

#ifdef USE_ITHREADS
        cPADOPo->op_padix = pad_alloc(OP_TRANS, SVf_READONLY);
        SvREFCNT_dec(PAD_SVl(cPADOPo->op_padix));
        PAD_SETSV(cPADOPo->op_padix, (SV *) invmap);
        SvPADTMP_on(invmap);
        SvREADONLY_on(invmap);
#else
        cSVOPo->op_sv = (SV *) invmap;
#endif

    }
    else {
        OPtrans_map *tbl;
        unsigned short i;

        /* The OPtrans_map struct already contains one slot; hence the -1. */
        SSize_t struct_size = sizeof(OPtrans_map)
                            + (256 - 1 + 1)*sizeof(short);

        /* Non-utf8 case: set o->op_pv to point to a simple 256+ entry lookup
         * table. Entries with the value TR_UNMAPPED indicate chars not to be
         * translated, while TR_DELETE indicates a search char without a
         * corresponding replacement char under /d.
         *
         * In addition, an extra slot at the end is used to store the final
         * repeating char, or TR_R_EMPTY under an empty replacement list, or
         * TR_DELETE under /d; which makes the runtime code easier. */

        /* Indicate this is an op_pv */
        o->op_private &= ~OPpTRANS_USE_SVOP;

        tbl = (OPtrans_map*)PerlMemShared_calloc(struct_size, 1);
        tbl->size = 256;
        cPVOPo->op_pv = (char*)tbl;

        for (i = 0; i < len; i++) {
            STATIC_ASSERT_DECL(TR_SPECIAL_HANDLING == TR_DELETE);
            short upper = i >= len - 1 ? 256 : (short) t_array[i+1];
            short to = (short) r_map[i];
            short j;
            bool do_increment = TRUE;

            /* Any code points above our limit should be irrelevant */
            if (t_array[i] >= tbl->size) break;

            /* Set up the map */
            if (to == (short) TR_SPECIAL_HANDLING && ! del) {
                to = (short) final_map;
                do_increment = FALSE;
            }
            else if (to < 0) {
                do_increment = FALSE;
            }

            /* Create a map for everything in this range.  The value increases
             * except for the special cases */
            for (j = (short) t_array[i]; j < upper; j++) {
                tbl->map[j] = to;
                if (do_increment) to++;
            }
        }

        tbl->map[tbl->size] = del
                              ? (short) TR_DELETE
                              : (short) rlen
                                ? (short) final_map
                                : (short) TR_R_EMPTY;
        DEBUG_y(PerlIO_printf(Perl_debug_log,"%s: %d\n", __FILE__, __LINE__));
        for (i = 0; i < tbl->size; i++) {
            if (tbl->map[i] < 0) {
                DEBUG_y(PerlIO_printf(Perl_debug_log," %02x=>%d",
                                                (unsigned) i, tbl->map[i]));
            }
            else {
                DEBUG_y(PerlIO_printf(Perl_debug_log," %02x=>%02x",
                                                (unsigned) i, tbl->map[i]));
            }
            if ((i+1) % 8 == 0 || i + 1 == (short) tbl->size) {
                DEBUG_y(PerlIO_printf(Perl_debug_log,"\n"));
            }
        }
        DEBUG_y(PerlIO_printf(Perl_debug_log,"Final map 0x%x=>%02x\n",
                                (unsigned) tbl->size, tbl->map[tbl->size]));

        SvREFCNT_dec(t_invlist);

#if 0   /* code that added excess above-255 chars at the end of the table, in
           case we ever want to not use the inversion map implementation for
           this */

        ASSUME(j <= rlen);
        excess = rlen - j;

        if (excess) {
            /* More replacement chars than search chars:
             * store excess replacement chars at end of main table.
             */

            struct_size += excess;
            tbl = (OPtrans_map*)PerlMemShared_realloc(tbl,
                        struct_size + excess * sizeof(short));
            tbl->size += excess;
            cPVOPo->op_pv = (char*)tbl;

            for (i = 0; i < excess; i++)
                tbl->map[i + 256] = r[j+i];
        }
        else {
            /* no more replacement chars than search chars */
        }
#endif

    }

    DEBUG_y(PerlIO_printf(Perl_debug_log,
            "/d=%d, /s=%d, /c=%d, identical=%d, grows=%d,"
            " use_svop=%d, can_force_utf8=%d,\nexpansion=%" NVgf "\n",
            del, squash, complement,
            cBOOL(o->op_private & OPpTRANS_IDENTICAL),
            cBOOL(o->op_private & OPpTRANS_USE_SVOP),
            cBOOL(o->op_private & OPpTRANS_GROWS),
            cBOOL(o->op_private & OPpTRANS_CAN_FORCE_UTF8),
            max_expansion));

    Safefree(r_map);

    if(del && rlen != 0 && r_count == t_count) {
        Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Useless use of /d modifier in transliteration operator");
    } else if(r_count > t_count) {
        Perl_ck_warner(aTHX_ packWARN(WARN_MISC), "Replacement list is longer than search list");
    }

    op_free(expr);
    op_free(repl);

    return o;
}


/*
=for apidoc newPMOP

Constructs, checks, and returns an op of any pattern matching type.
C<type> is the opcode.  C<flags> gives the eight bits of C<op_flags>
and, shifted up eight bits, the eight bits of C<op_private>.

=cut
*/

OP *
Perl_newPMOP(pTHX_ I32 type, I32 flags)
{
    PMOP *pmop;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_PMOP
        || type == OP_CUSTOM);

    NewOp(1101, pmop, 1, PMOP);
    OpTYPE_set(pmop, type);
    pmop->op_flags = (U8)flags;
    pmop->op_private = (U8)(0 | (flags >> 8));
    if (PL_opargs[type] & OA_RETSCALAR)
        scalar((OP *)pmop);

    if (PL_hints & HINT_RE_TAINT)
        pmop->op_pmflags |= PMf_RETAINT;
#ifdef USE_LOCALE_CTYPE
    if (IN_LC_COMPILETIME(LC_CTYPE)) {
        set_regex_charset(&(pmop->op_pmflags), REGEX_LOCALE_CHARSET);
    }
    else
#endif
         if (IN_UNI_8_BIT) {
        set_regex_charset(&(pmop->op_pmflags), REGEX_UNICODE_CHARSET);
    }
    if (PL_hints & HINT_RE_FLAGS) {
        SV *reflags = Perl_refcounted_he_fetch_pvn(aTHX_
         PL_compiling.cop_hints_hash, STR_WITH_LEN("reflags"), 0, 0
        );
        if (reflags && SvOK(reflags)) pmop->op_pmflags |= SvIV(reflags);
        reflags = Perl_refcounted_he_fetch_pvn(aTHX_
         PL_compiling.cop_hints_hash, STR_WITH_LEN("reflags_charset"), 0, 0
        );
        if (reflags && SvOK(reflags)) {
            set_regex_charset(&(pmop->op_pmflags), (regex_charset)SvIV(reflags));
        }
    }


#ifdef USE_ITHREADS
    assert(SvPOK(PL_regex_pad[0]));
    if (SvCUR(PL_regex_pad[0])) {
        /* Pop off the "packed" IV from the end.  */
        SV *const repointer_list = PL_regex_pad[0];
        const char *p = SvEND(repointer_list) - sizeof(IV);
        const IV offset = *((IV*)p);

        assert(SvCUR(repointer_list) % sizeof(IV) == 0);

        SvEND_set(repointer_list, p);

        pmop->op_pmoffset = offset;
        /* This slot should be free, so assert this:  */
        assert(PL_regex_pad[offset] == &PL_sv_undef);
    } else {
        SV * const repointer = &PL_sv_undef;
        av_push(PL_regex_padav, repointer);
        pmop->op_pmoffset = av_top_index(PL_regex_padav);
        PL_regex_pad = AvARRAY(PL_regex_padav);
    }
#endif

    return CHECKOP(type, pmop);
}

static void
S_set_haseval(pTHX)
{
    PADOFFSET i = 1;
    PL_cv_has_eval = 1;
    /* Any pad names in scope are potentially lvalues.  */
    for (; i < PadnamelistMAXNAMED(PL_comppad_name); i++) {
        PADNAME *pn = PAD_COMPNAME_SV(i);
        if (!pn || !PadnameLEN(pn))
            continue;
        if (PadnameOUTER(pn) || PadnameIN_SCOPE(pn, PL_cop_seqmax))
            S_mark_padname_lvalue(aTHX_ pn);
    }
}

/* Given some sort of match op o, and an expression expr containing a
 * pattern, either compile expr into a regex and attach it to o (if it's
 * constant), or convert expr into a runtime regcomp op sequence (if it's
 * not)
 *
 * Flags currently has 2 bits of meaning:
 * 1: isreg indicates that the pattern is part of a regex construct, eg
 *      $x =~ /pattern/ or split /pattern/, as opposed to $x =~ $pattern or
 *      split "pattern", which aren't. In the former case, expr will be a list
 *      if the pattern contains more than one term (eg /a$b/).
 * 2: The pattern is for a split.
 *
 * When the pattern has been compiled within a new anon CV (for
 * qr/(?{...})/ ), then floor indicates the savestack level just before
 * the new sub was created
 *
 * tr/// is also handled.
 */

OP *
Perl_pmruntime(pTHX_ OP *o, OP *expr, OP *repl, UV flags, I32 floor)
{
    PMOP *pm;
    LOGOP *rcop;
    I32 repl_has_vars = 0;
    bool is_trans = (o->op_type == OP_TRANS || o->op_type == OP_TRANSR);
    bool is_compiletime;
    bool has_code;
    bool isreg    = cBOOL(flags & 1);
    bool is_split = cBOOL(flags & 2);

    PERL_ARGS_ASSERT_PMRUNTIME;

    if (is_trans) {
        return pmtrans(o, expr, repl);
    }

    /* find whether we have any runtime or code elements;
     * at the same time, temporarily set the op_next of each DO block;
     * then when we LINKLIST, this will cause the DO blocks to be excluded
     * from the op_next chain (and from having LINKLIST recursively
     * applied to them). We fix up the DOs specially later */

    is_compiletime = 1;
    has_code = 0;
    if (expr->op_type == OP_LIST) {
        OP *child;
        for (child = cLISTOPx(expr)->op_first; child; child = OpSIBLING(child)) {
            if (child->op_type == OP_NULL && (child->op_flags & OPf_SPECIAL)) {
                has_code = 1;
                assert(!child->op_next);
                if (UNLIKELY(!OpHAS_SIBLING(child))) {
                    assert(PL_parser && PL_parser->error_count);
                    /* This can happen with qr/ (?{(^{})/.  Just fake up
                       the op we were expecting to see, to avoid crashing
                       elsewhere.  */
                    op_sibling_splice(expr, child, 0,
                              newSVOP(OP_CONST, 0, &PL_sv_no));
                }
                child->op_next = OpSIBLING(child);
            }
            else if (child->op_type != OP_CONST && child->op_type != OP_PUSHMARK)
            is_compiletime = 0;
        }
    }
    else if (expr->op_type != OP_CONST)
        is_compiletime = 0;

    LINKLIST(expr);

    /* fix up DO blocks; treat each one as a separate little sub;
     * also, mark any arrays as LIST/REF */

    if (expr->op_type == OP_LIST) {
        OP *child;
        for (child = cLISTOPx(expr)->op_first; child; child = OpSIBLING(child)) {

            if (child->op_type == OP_PADAV || child->op_type == OP_RV2AV) {
                assert( !(child->op_flags  & OPf_WANT));
                /* push the array rather than its contents. The regex
                 * engine will retrieve and join the elements later */
                child->op_flags |= (OPf_WANT_LIST | OPf_REF);
                continue;
            }

            if (!(child->op_type == OP_NULL && (child->op_flags & OPf_SPECIAL)))
                continue;
            child->op_next = NULL; /* undo temporary hack from above */
            scalar(child);
            LINKLIST(child);
            if (cLISTOPx(child)->op_first->op_type == OP_LEAVE) {
                LISTOP *leaveop = cLISTOPx(cLISTOPx(child)->op_first);
                /* skip ENTER */
                assert(leaveop->op_first->op_type == OP_ENTER);
                assert(OpHAS_SIBLING(leaveop->op_first));
                child->op_next = OpSIBLING(leaveop->op_first);
                /* skip leave */
                assert(leaveop->op_flags & OPf_KIDS);
                assert(leaveop->op_last->op_next == (OP*)leaveop);
                leaveop->op_next = NULL; /* stop on last op */
                op_null((OP*)leaveop);
            }
            else {
                /* skip SCOPE */
                OP *scope = cLISTOPx(child)->op_first;
                assert(scope->op_type == OP_SCOPE);
                assert(scope->op_flags & OPf_KIDS);
                scope->op_next = NULL; /* stop on last op */
                op_null(scope);
            }

            /* XXX optimize_optree() must be called on o before
             * CALL_PEEP(), as currently S_maybe_multiconcat() can't
             * currently cope with a peephole-optimised optree.
             * Calling optimize_optree() here ensures that condition
             * is met, but may mean optimize_optree() is applied
             * to the same optree later (where hopefully it won't do any
             * harm as it can't convert an op to multiconcat if it's
             * already been converted */
            optimize_optree(child);

            /* have to peep the DOs individually as we've removed it from
             * the op_next chain */
            CALL_PEEP(child);
            op_prune_chain_head(&(child->op_next));
            if (is_compiletime)
                /* runtime finalizes as part of finalizing whole tree */
                finalize_optree(child);
        }
    }
    else if (expr->op_type == OP_PADAV || expr->op_type == OP_RV2AV) {
        assert( !(expr->op_flags  & OPf_WANT));
        /* push the array rather than its contents. The regex
         * engine will retrieve and join the elements later */
        expr->op_flags |= (OPf_WANT_LIST | OPf_REF);
    }

    PL_hints |= HINT_BLOCK_SCOPE;
    pm = cPMOPo;
    assert(floor==0 || (pm->op_pmflags & PMf_HAS_CV));

    if (is_compiletime) {
        U32 rx_flags = pm->op_pmflags & RXf_PMf_COMPILETIME;
        regexp_engine const *eng = current_re_engine();

        if (is_split) {
            /* make engine handle split ' ' specially */
            pm->op_pmflags |= PMf_SPLIT;
            rx_flags |= RXf_SPLIT;
        }

        if (!has_code || !eng->op_comp) {
            /* compile-time simple constant pattern */

            if ((pm->op_pmflags & PMf_HAS_CV) && !has_code) {
                /* whoops! we guessed that a qr// had a code block, but we
                 * were wrong (e.g. /[(?{}]/ ). Throw away the PL_compcv
                 * that isn't required now. Note that we have to be pretty
                 * confident that nothing used that CV's pad while the
                 * regex was parsed, except maybe op targets for \Q etc.
                 * If there were any op targets, though, they should have
                 * been stolen by constant folding.
                 */
#ifdef DEBUGGING
                SSize_t i = 0;
                assert(PadnamelistMAXNAMED(PL_comppad_name) == 0);
                while (++i <= AvFILLp(PL_comppad)) {
#  ifdef USE_PAD_RESET
                    /* under USE_PAD_RESET, pad swipe replaces a swiped
                     * folded constant with a fresh padtmp */
                    assert(!PL_curpad[i] || SvPADTMP(PL_curpad[i]));
#  else
                    assert(!PL_curpad[i]);
#  endif
                }
#endif
                /* This LEAVE_SCOPE will restore PL_compcv to point to the
                 * outer CV (the one whose slab holds the pm op). The
                 * inner CV (which holds expr) will be freed later, once
                 * all the entries on the parse stack have been popped on
                 * return from this function. Which is why its safe to
                 * call op_free(expr) below.
                 */
                LEAVE_SCOPE(floor);
                pm->op_pmflags &= ~PMf_HAS_CV;
            }

            /* Skip compiling if parser found an error for this pattern */
            if (pm->op_pmflags & PMf_HAS_ERROR) {
                return o;
            }

            PM_SETRE(pm,
                eng->op_comp
                    ? eng->op_comp(aTHX_ NULL, 0, expr, eng, NULL, NULL,
                                        rx_flags, pm->op_pmflags)
                    : Perl_re_op_compile(aTHX_ NULL, 0, expr, eng, NULL, NULL,
                                        rx_flags, pm->op_pmflags)
            );
            op_free(expr);
        }
        else {
            /* compile-time pattern that includes literal code blocks */

            REGEXP* re;

            /* Skip compiling if parser found an error for this pattern */
            if (pm->op_pmflags & PMf_HAS_ERROR) {
                return o;
            }

            re = eng->op_comp(aTHX_ NULL, 0, expr, eng, NULL, NULL,
                        rx_flags,
                        (pm->op_pmflags |
                            ((PL_hints & HINT_RE_EVAL) ? PMf_USE_RE_EVAL : 0))
                    );
            PM_SETRE(pm, re);
            if (pm->op_pmflags & PMf_HAS_CV) {
                CV *cv;
                /* this QR op (and the anon sub we embed it in) is never
                 * actually executed. It's just a placeholder where we can
                 * squirrel away expr in op_code_list without the peephole
                 * optimiser etc processing it for a second time */
                OP *qr = newPMOP(OP_QR, 0);
                cPMOPx(qr)->op_code_list = expr;

                /* handle the implicit sub{} wrapped round the qr/(?{..})/ */
                SvREFCNT_inc_simple_void(PL_compcv);
                cv = newATTRSUB(floor, 0, NULL, NULL, qr);
                ReANY(re)->qr_anoncv = cv;

                /* attach the anon CV to the pad so that
                 * pad_fixup_inner_anons() can find it */
                (void)pad_add_anon(cv, o->op_type);
                SvREFCNT_inc_simple_void(cv);
            }
            else {
                pm->op_code_list = expr;
            }
        }
    }
    else {
        /* runtime pattern: build chain of regcomp etc ops */
        bool reglist;
        PADOFFSET cv_targ = 0;

        reglist = isreg && expr->op_type == OP_LIST;
        if (reglist)
            op_null(expr);

        if (has_code) {
            pm->op_code_list = expr;
            /* don't free op_code_list; its ops are embedded elsewhere too */
            pm->op_pmflags |= PMf_CODELIST_PRIVATE;
        }

        if (is_split)
            /* make engine handle split ' ' specially */
            pm->op_pmflags |= PMf_SPLIT;

        /* the OP_REGCMAYBE is a placeholder in the non-threaded case
         * to allow its op_next to be pointed past the regcomp and
         * preceding stacking ops;
         * OP_REGCRESET is there to reset taint before executing the
         * stacking ops */
        if (pm->op_pmflags & PMf_KEEP || TAINTING_get)
            expr = newUNOP((TAINTING_get ? OP_REGCRESET : OP_REGCMAYBE),0,expr);

        if (pm->op_pmflags & PMf_HAS_CV) {
            /* we have a runtime qr with literal code. This means
             * that the qr// has been wrapped in a new CV, which
             * means that runtime consts, vars etc will have been compiled
             * against a new pad. So... we need to execute those ops
             * within the environment of the new CV. So wrap them in a call
             * to a new anon sub. i.e. for
             *
             *     qr/a$b(?{...})/,
             *
             * we build an anon sub that looks like
             *
             *     sub { "a", $b, '(?{...})' }
             *
             * and call it, passing the returned list to regcomp.
             * Or to put it another way, the list of ops that get executed
             * are:
             *
             *     normal              PMf_HAS_CV
             *     ------              -------------------
             *                         pushmark (for regcomp)
             *                         pushmark (for entersub)
             *                         anoncode
             *                         entersub
             *     regcreset                  regcreset
             *     pushmark                   pushmark
             *     const("a")                 const("a")
             *     gvsv(b)                    gvsv(b)
             *     const("(?{...})")          const("(?{...})")
             *                                leavesub
             *     regcomp             regcomp
             */

            SvREFCNT_inc_simple_void(PL_compcv);
            CvLVALUE_on(PL_compcv);
            /* these lines are just an unrolled newANONATTRSUB */
            expr = newSVOP(OP_ANONCODE, OPf_REF,
                    MUTABLE_SV(newATTRSUB(floor, 0, NULL, NULL, expr)));
            cv_targ = expr->op_targ;

            expr = list(op_force_list(newUNOP(OP_ENTERSUB, 0, scalar(expr))));
        }

        rcop = alloc_LOGOP(OP_REGCOMP, scalar(expr), o);
        rcop->op_flags |=  ((PL_hints & HINT_RE_EVAL) ? OPf_SPECIAL : 0)
                           | (reglist ? OPf_STACKED : 0);
        rcop->op_targ = cv_targ;

        /* /$x/ may cause an eval, since $x might be qr/(?{..})/  */
        if (PL_hints & HINT_RE_EVAL)
            S_set_haseval(aTHX);

        /* establish postfix order */
        if (expr->op_type == OP_REGCRESET || expr->op_type == OP_REGCMAYBE) {
            LINKLIST(expr);
            rcop->op_next = expr;
            cUNOPx(expr)->op_first->op_next = (OP*)rcop;
        }
        else {
            rcop->op_next = LINKLIST(expr);
            expr->op_next = (OP*)rcop;
        }

        op_prepend_elem(o->op_type, scalar((OP*)rcop), o);
    }

    if (repl) {
        OP *curop = repl;
        bool konst;
        /* If we are looking at s//.../e with a single statement, get past
           the implicit do{}. */
        if (curop->op_type == OP_NULL && curop->op_flags & OPf_KIDS
             && cUNOPx(curop)->op_first->op_type == OP_SCOPE
             && cUNOPx(curop)->op_first->op_flags & OPf_KIDS)
         {
            OP *sib;
            OP *kid = cUNOPx(cUNOPx(curop)->op_first)->op_first;
            if (kid->op_type == OP_NULL && (sib = OpSIBLING(kid))
             && !OpHAS_SIBLING(sib))
                curop = sib;
        }
        if (curop->op_type == OP_CONST)
            konst = TRUE;
        else if (( (curop->op_type == OP_RV2SV ||
                    curop->op_type == OP_RV2AV ||
                    curop->op_type == OP_RV2HV ||
                    curop->op_type == OP_RV2GV)
                   && cUNOPx(curop)->op_first
                   && cUNOPx(curop)->op_first->op_type == OP_GV )
                || curop->op_type == OP_PADSV
                || curop->op_type == OP_PADAV
                || curop->op_type == OP_PADHV
                || curop->op_type == OP_PADANY) {
            repl_has_vars = 1;
            konst = TRUE;
        }
        else konst = FALSE;
        if (konst
            && !(repl_has_vars
                 && (!PM_GETRE(pm)
                     || !RX_PRELEN(PM_GETRE(pm))
                     || RX_EXTFLAGS(PM_GETRE(pm)) & RXf_EVAL_SEEN)))
        {
            pm->op_pmflags |= PMf_CONST;	/* const for long enough */
            op_prepend_elem(o->op_type, scalar(repl), o);
        }
        else {
            rcop = alloc_LOGOP(OP_SUBSTCONT, scalar(repl), o);
            rcop->op_private = 1;

            /* establish postfix order */
            rcop->op_next = LINKLIST(repl);
            repl->op_next = (OP*)rcop;

            pm->op_pmreplrootu.op_pmreplroot = scalar((OP*)rcop);
            assert(!(pm->op_pmflags & PMf_ONCE));
            pm->op_pmstashstartu.op_pmreplstart = LINKLIST(rcop);
            rcop->op_next = 0;
        }
    }

    return (OP*)pm;
}

/*
=for apidoc newSVOP

Constructs, checks, and returns an op of any type that involves an
embedded SV.  C<type> is the opcode.  C<flags> gives the eight bits
of C<op_flags>.  C<sv> gives the SV to embed in the op; this function
takes ownership of one reference to it.

=cut
*/

OP *
Perl_newSVOP(pTHX_ I32 type, I32 flags, SV *sv)
{
    SVOP *svop;

    PERL_ARGS_ASSERT_NEWSVOP;

    /* OP_RUNCV is allowed specially so rpeep has room to convert it into an
     * OP_CONST */
    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_SVOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_PVOP_OR_SVOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_FILESTATOP
        || type == OP_RUNCV
        || type == OP_CUSTOM);

    NewOp(1101, svop, 1, SVOP);
    OpTYPE_set(svop, type);
    svop->op_sv = sv;
    svop->op_next = (OP*)svop;
    svop->op_flags = (U8)flags;
    svop->op_private = (U8)(0 | (flags >> 8));
    if (PL_opargs[type] & OA_RETSCALAR)
        scalar((OP*)svop);
    if (PL_opargs[type] & OA_TARGET)
        svop->op_targ = pad_alloc(type, SVs_PADTMP);
    return CHECKOP(type, svop);
}

/*
=for apidoc newDEFSVOP

Constructs and returns an op to access C<$_>.

=cut
*/

OP *
Perl_newDEFSVOP(pTHX)
{
        return newSVREF(newGVOP(OP_GV, 0, PL_defgv));
}

#ifdef USE_ITHREADS

/*
=for apidoc newPADOP

Constructs, checks, and returns an op of any type that involves a
reference to a pad element.  C<type> is the opcode.  C<flags> gives the
eight bits of C<op_flags>.  A pad slot is automatically allocated, and
is populated with C<sv>; this function takes ownership of one reference
to it.

This function only exists if Perl has been compiled to use ithreads.

=cut
*/

OP *
Perl_newPADOP(pTHX_ I32 type, I32 flags, SV *sv)
{
    PADOP *padop;

    PERL_ARGS_ASSERT_NEWPADOP;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_SVOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_PVOP_OR_SVOP
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_FILESTATOP
        || type == OP_CUSTOM);

    NewOp(1101, padop, 1, PADOP);
    OpTYPE_set(padop, type);
    padop->op_padix =
        pad_alloc(type, isGV(sv) ? SVf_READONLY : SVs_PADTMP);
    SvREFCNT_dec(PAD_SVl(padop->op_padix));
    PAD_SETSV(padop->op_padix, sv);
    assert(sv);
    padop->op_next = (OP*)padop;
    padop->op_flags = (U8)flags;
    if (PL_opargs[type] & OA_RETSCALAR)
        scalar((OP*)padop);
    if (PL_opargs[type] & OA_TARGET)
        padop->op_targ = pad_alloc(type, SVs_PADTMP);
    return CHECKOP(type, padop);
}

#endif /* USE_ITHREADS */

/*
=for apidoc newGVOP

Constructs, checks, and returns an op of any type that involves an
embedded reference to a GV.  C<type> is the opcode.  C<flags> gives the
eight bits of C<op_flags>.  C<gv> identifies the GV that the op should
reference; calling this function does not transfer ownership of any
reference to it.

=cut
*/

OP *
Perl_newGVOP(pTHX_ I32 type, I32 flags, GV *gv)
{
    PERL_ARGS_ASSERT_NEWGVOP;

#ifdef USE_ITHREADS
    return newPADOP(type, flags, SvREFCNT_inc_simple_NN(gv));
#else
    return newSVOP(type, flags, SvREFCNT_inc_simple_NN(gv));
#endif
}

/*
=for apidoc newPVOP

Constructs, checks, and returns an op of any type that involves an
embedded C-level pointer (PV).  C<type> is the opcode.  C<flags> gives
the eight bits of C<op_flags>.  C<pv> supplies the C-level pointer.
Depending on the op type, the memory referenced by C<pv> may be freed
when the op is destroyed.  If the op is of a freeing type, C<pv> must
have been allocated using C<PerlMemShared_malloc>.

=cut
*/

OP *
Perl_newPVOP(pTHX_ I32 type, I32 flags, char *pv)
{
    const bool utf8 = cBOOL(flags & SVf_UTF8);
    PVOP *pvop;

    flags &= ~SVf_UTF8;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_PVOP_OR_SVOP
        || type == OP_CUSTOM
        || (PL_opargs[type] & OA_CLASS_MASK) == OA_LOOPEXOP);

    NewOp(1101, pvop, 1, PVOP);
    OpTYPE_set(pvop, type);
    pvop->op_pv = pv;
    pvop->op_next = (OP*)pvop;
    pvop->op_flags = (U8)flags;
    pvop->op_private = utf8 ? OPpPV_IS_UTF8 : 0;
    if (PL_opargs[type] & OA_RETSCALAR)
        scalar((OP*)pvop);
    if (PL_opargs[type] & OA_TARGET)
        pvop->op_targ = pad_alloc(type, SVs_PADTMP);
    return CHECKOP(type, pvop);
}

void
Perl_package(pTHX_ OP *o)
{
    SV *const sv = cSVOPo->op_sv;

    PERL_ARGS_ASSERT_PACKAGE;

    SAVEGENERICSV(PL_curstash);
    save_item(PL_curstname);

    PL_curstash = (HV *)SvREFCNT_inc(gv_stashsv(sv, GV_ADD));

    sv_setsv(PL_curstname, sv);

    PL_hints |= HINT_BLOCK_SCOPE;
    PL_parser->copline = NOLINE;

    op_free(o);
}

void
Perl_package_version( pTHX_ OP *v )
{
    U32 savehints = PL_hints;
    PERL_ARGS_ASSERT_PACKAGE_VERSION;
    PL_hints &= ~HINT_STRICT_VARS;
    sv_setsv( GvSV(gv_fetchpvs("VERSION", GV_ADDMULTI, SVt_PV)), cSVOPx(v)->op_sv );
    PL_hints = savehints;
    op_free(v);
}

/* Extract the first two components of a "version" object as two 8bit integers
 * and return them packed into a single U16 in the format of PL_prevailing_version.
 * This function only ever has to cope with version objects already known
 * bounded by the current perl version, so we know its components will fit
 * (Up until we reach perl version 5.256 anyway) */
static U16 S_extract_shortver(pTHX_ SV *sv)
{
    SV *rv;
    if(!SvRV(sv) || !SvOBJECT(rv = SvRV(sv)) || !sv_derived_from(sv, "version"))
        return 0;

    AV *av = MUTABLE_AV(SvRV(*hv_fetchs(MUTABLE_HV(rv), "version", 0)));

    U16 shortver = 0;

    IV major = av_count(av) > 0 ? SvIV(*av_fetch(av, 0, false)) : 0;
    if(major > 255)
        shortver |= 255 << 8;
    else
        shortver |= major << 8;

    IV minor = av_count(av) > 1 ? SvIV(*av_fetch(av, 1, false)) : 0;
    if(minor > 255)
        shortver |= 255;
    else
        shortver |= minor;

    return shortver;
}
#define SHORTVER(maj,min) ((maj << 8) | min)

void
Perl_utilize(pTHX_ int aver, I32 floor, OP *version, OP *idop, OP *arg)
{
    OP *pack;
    OP *imop;
    OP *veop;
    SV *use_version = NULL;

    PERL_ARGS_ASSERT_UTILIZE;

    if (idop->op_type != OP_CONST)
        Perl_croak(aTHX_ "Module name must be constant");

    veop = NULL;

    if (version) {
        SV * const vesv = cSVOPx(version)->op_sv;

        if (!arg && !SvNIOKp(vesv)) {
            arg = version;
        }
        else {
            OP *pack;
            SV *meth;

            if (version->op_type != OP_CONST || !SvNIOKp(vesv))
                Perl_croak(aTHX_ "Version number must be a constant number");

            /* Make copy of idop so we don't free it twice */
            pack = newSVOP(OP_CONST, 0, newSVsv(cSVOPx(idop)->op_sv));

            /* Fake up a method call to VERSION */
            meth = newSVpvs_share("VERSION");
            veop = op_convert_list(OP_ENTERSUB, OPf_STACKED,
                            op_append_elem(OP_LIST,
                                        op_prepend_elem(OP_LIST, pack, version),
                                        newMETHOP_named(OP_METHOD_NAMED, 0, meth)));
        }
    }

    /* Fake up an import/unimport */
    if (arg && arg->op_type == OP_STUB) {
        imop = arg;		/* no import on explicit () */
    }
    else if (SvNIOKp(cSVOPx(idop)->op_sv)) {
        imop = NULL;		/* use 5.0; */
        if (aver)
            use_version = cSVOPx(idop)->op_sv;
        else
            idop->op_private |= OPpCONST_NOVER;
    }
    else {
        SV *meth;

        /* Make copy of idop so we don't free it twice */
        pack = newSVOP(OP_CONST, 0, newSVsv(cSVOPx(idop)->op_sv));

        /* Fake up a method call to import/unimport */
        meth = aver
            ? newSVpvs_share("import") : newSVpvs_share("unimport");
        imop = op_convert_list(OP_ENTERSUB, OPf_STACKED,
                       op_append_elem(OP_LIST,
                                   op_prepend_elem(OP_LIST, pack, arg),
                                   newMETHOP_named(OP_METHOD_NAMED, 0, meth)
                       ));
    }

    /* Fake up the BEGIN {}, which does its thing immediately. */
    newATTRSUB(floor,
        newSVOP(OP_CONST, 0, newSVpvs_share("BEGIN")),
        NULL,
        NULL,
        op_append_elem(OP_LINESEQ,
            op_append_elem(OP_LINESEQ,
                newSTATEOP(0, NULL, newUNOP(OP_REQUIRE, 0, idop)),
                newSTATEOP(0, NULL, veop)),
            newSTATEOP(0, NULL, imop) ));

    if (use_version) {
        /* Enable the
         * feature bundle that corresponds to the required version. */
        use_version = sv_2mortal(new_version(use_version));
        S_enable_feature_bundle(aTHX_ use_version);

        U16 shortver = S_extract_shortver(aTHX_ use_version);

        /* If a version >= 5.11.0 is requested, strictures are on by default! */
        if (shortver >= SHORTVER(5, 11)) {
            if (!(PL_hints & HINT_EXPLICIT_STRICT_REFS))
                PL_hints |= HINT_STRICT_REFS;
            if (!(PL_hints & HINT_EXPLICIT_STRICT_SUBS))
                PL_hints |= HINT_STRICT_SUBS;
            if (!(PL_hints & HINT_EXPLICIT_STRICT_VARS))
                PL_hints |= HINT_STRICT_VARS;

            if (shortver >= SHORTVER(5, 35))
                free_and_set_cop_warnings(&PL_compiling, pWARN_ALL);
        }
        /* otherwise they are off */
        else {
            if(PL_prevailing_version >= SHORTVER(5, 11))
                deprecate_fatal_in(WARN_DEPRECATED__VERSION_DOWNGRADE, "5.40",
                    "Downgrading a use VERSION declaration to below v5.11");

            if (!(PL_hints & HINT_EXPLICIT_STRICT_REFS))
                PL_hints &= ~HINT_STRICT_REFS;
            if (!(PL_hints & HINT_EXPLICIT_STRICT_SUBS))
                PL_hints &= ~HINT_STRICT_SUBS;
            if (!(PL_hints & HINT_EXPLICIT_STRICT_VARS))
                PL_hints &= ~HINT_STRICT_VARS;
        }

        PL_prevailing_version = shortver;
    }

    /* The "did you use incorrect case?" warning used to be here.
     * The problem is that on case-insensitive filesystems one
     * might get false positives for "use" (and "require"):
     * "use Strict" or "require CARP" will work.  This causes
     * portability problems for the script: in case-strict
     * filesystems the script will stop working.
     *
     * The "incorrect case" warning checked whether "use Foo"
     * imported "Foo" to your namespace, but that is wrong, too:
     * there is no requirement nor promise in the language that
     * a Foo.pm should or would contain anything in package "Foo".
     *
     * There is very little Configure-wise that can be done, either:
     * the case-sensitivity of the build filesystem of Perl does not
     * help in guessing the case-sensitivity of the runtime environment.
     */

    PL_hints |= HINT_BLOCK_SCOPE;
    PL_parser->copline = NOLINE;
    COP_SEQMAX_INC; /* Purely for B::*'s benefit */
}

/*
=for apidoc_section $embedding

=for apidoc      load_module
=for apidoc_item load_module_nocontext

These load the module whose name is pointed to by the string part of C<name>.
Note that the actual module name, not its filename, should be given.
Eg, "Foo::Bar" instead of "Foo/Bar.pm". ver, if specified and not NULL,
provides version semantics similar to C<use Foo::Bar VERSION>. The optional
trailing arguments can be used to specify arguments to the module's C<import()>
method, similar to C<use Foo::Bar VERSION LIST>; their precise handling depends
on the flags. The flags argument is a bitwise-ORed collection of any of
C<PERL_LOADMOD_DENY>, C<PERL_LOADMOD_NOIMPORT>, or C<PERL_LOADMOD_IMPORT_OPS>
(or 0 for no flags).

If C<PERL_LOADMOD_NOIMPORT> is set, the module is loaded as if with an empty
import list, as in C<use Foo::Bar ()>; this is the only circumstance in which
the trailing optional arguments may be omitted entirely. Otherwise, if
C<PERL_LOADMOD_IMPORT_OPS> is set, the trailing arguments must consist of
exactly one C<OP*>, containing the op tree that produces the relevant import
arguments. Otherwise, the trailing arguments must all be C<SV*> values that
will be used as import arguments; and the list must be terminated with C<(SV*)
NULL>. If neither C<PERL_LOADMOD_NOIMPORT> nor C<PERL_LOADMOD_IMPORT_OPS> is
set, the trailing C<NULL> pointer is needed even if no import arguments are
desired. The reference count for each specified C<SV*> argument is
decremented. In addition, the C<name> argument is modified.

If C<PERL_LOADMOD_DENY> is set, the module is loaded as if with C<no> rather
than C<use>.

C<load_module> and C<load_module_nocontext> have the same apparent signature,
but the former hides the fact that it is accessing a thread context parameter.
So use the latter when you get a compilation error about C<pTHX>.

=for apidoc Amnh||PERL_LOADMOD_DENY
=for apidoc Amnh||PERL_LOADMOD_NOIMPORT
=for apidoc Amnh||PERL_LOADMOD_IMPORT_OPS

=for apidoc vload_module
Like C<L</load_module>> but the arguments are an encapsulated argument list.

=cut */

void
Perl_load_module(pTHX_ U32 flags, SV *name, SV *ver, ...)
{
    va_list args;

    PERL_ARGS_ASSERT_LOAD_MODULE;

    va_start(args, ver);
    vload_module(flags, name, ver, &args);
    va_end(args);
}

#ifdef MULTIPLICITY
void
Perl_load_module_nocontext(U32 flags, SV *name, SV *ver, ...)
{
    dTHX;
    va_list args;
    PERL_ARGS_ASSERT_LOAD_MODULE_NOCONTEXT;
    va_start(args, ver);
    vload_module(flags, name, ver, &args);
    va_end(args);
}
#endif

void
Perl_vload_module(pTHX_ U32 flags, SV *name, SV *ver, va_list *args)
{
    OP *veop, *imop;
    OP * modname;
    I32 floor;

    PERL_ARGS_ASSERT_VLOAD_MODULE;

    /* utilize() fakes up a BEGIN { require ..; import ... }, so make sure
     * that it has a PL_parser to play with while doing that, and also
     * that it doesn't mess with any existing parser, by creating a tmp
     * new parser with lex_start(). This won't actually be used for much,
     * since pp_require() will create another parser for the real work.
     * The ENTER/LEAVE pair protect callers from any side effects of use.
     *
     * start_subparse() creates a new PL_compcv. This means that any ops
     * allocated below will be allocated from that CV's op slab, and so
     * will be automatically freed if the utilise() fails
     */

    ENTER;
    SAVEVPTR(PL_curcop);
    lex_start(NULL, NULL, LEX_START_SAME_FILTER);
    floor = start_subparse(FALSE, 0);

    modname = newSVOP(OP_CONST, 0, name);
    modname->op_private |= OPpCONST_BARE;
    if (ver) {
        veop = newSVOP(OP_CONST, 0, ver);
    }
    else
        veop = NULL;
    if (flags & PERL_LOADMOD_NOIMPORT) {
        imop = sawparens(newNULLLIST());
    }
    else if (flags & PERL_LOADMOD_IMPORT_OPS) {
        imop = va_arg(*args, OP*);
    }
    else {
        SV *sv;
        imop = NULL;
        sv = va_arg(*args, SV*);
        while (sv) {
            imop = op_append_elem(OP_LIST, imop, newSVOP(OP_CONST, 0, sv));
            sv = va_arg(*args, SV*);
        }
    }

    utilize(!(flags & PERL_LOADMOD_DENY), floor, veop, modname, imop);
    LEAVE;
}

PERL_STATIC_INLINE OP *
S_new_entersubop(pTHX_ GV *gv, OP *arg)
{
    return newUNOP(OP_ENTERSUB, OPf_STACKED,
                   newLISTOP(OP_LIST, 0, arg,
                             newUNOP(OP_RV2CV, 0,
                                     newGVOP(OP_GV, 0, gv))));
}

OP *
Perl_dofile(pTHX_ OP *term, I32 force_builtin)
{
    OP *doop;
    GV *gv;

    PERL_ARGS_ASSERT_DOFILE;

    if (!force_builtin && (gv = gv_override("do", 2))) {
        doop = S_new_entersubop(aTHX_ gv, term);
    }
    else {
        doop = newUNOP(OP_DOFILE, 0, scalar(term));
    }
    return doop;
}

/*
=for apidoc_section $optree_construction

=for apidoc newSLICEOP

Constructs, checks, and returns an C<lslice> (list slice) op.  C<flags>
gives the eight bits of C<op_flags>, except that C<OPf_KIDS> will
be set automatically, and, shifted up eight bits, the eight bits of
C<op_private>, except that the bit with value 1 or 2 is automatically
set as required.  C<listval> and C<subscript> supply the parameters of
the slice; they are consumed by this function and become part of the
constructed op tree.

=cut
*/

OP *
Perl_newSLICEOP(pTHX_ I32 flags, OP *subscript, OP *listval)
{
    return newBINOP(OP_LSLICE, flags,
            list(op_force_list(subscript)),
            list(op_force_list(listval)));
}

#define ASSIGN_SCALAR 0
#define ASSIGN_LIST   1
#define ASSIGN_REF    2

/* given the optree o on the LHS of an assignment, determine whether its:
 *  ASSIGN_SCALAR   $x  = ...
 *  ASSIGN_LIST    ($x) = ...
 *  ASSIGN_REF     \$x  = ...
 */

STATIC I32
S_assignment_type(pTHX_ const OP *o)
{
    unsigned type;
    U8 flags;
    U8 ret;

    if (!o)
        return ASSIGN_LIST;

    if (o->op_type == OP_SREFGEN)
    {
        OP * const kid = cUNOPx(cUNOPo->op_first)->op_first;
        type = kid->op_type;
        flags = o->op_flags | kid->op_flags;
        if (!(flags & OPf_PARENS)
          && (kid->op_type == OP_RV2AV || kid->op_type == OP_PADAV ||
              kid->op_type == OP_RV2HV || kid->op_type == OP_PADHV ))
            return ASSIGN_REF;
        ret = ASSIGN_REF;
    } else {
        if ((o->op_type == OP_NULL) && (o->op_flags & OPf_KIDS))
            o = cUNOPo->op_first;
        flags = o->op_flags;
        type = o->op_type;
        ret = ASSIGN_SCALAR;
    }

    if (type == OP_COND_EXPR) {
        OP * const sib = OpSIBLING(cLOGOPo->op_first);
        const I32 t = assignment_type(sib);
        const I32 f = assignment_type(OpSIBLING(sib));

        if (t == ASSIGN_LIST && f == ASSIGN_LIST)
            return ASSIGN_LIST;
        if ((t == ASSIGN_LIST) ^ (f == ASSIGN_LIST))
            yyerror("Assignment to both a list and a scalar");
        return ASSIGN_SCALAR;
    }

    if (type == OP_LIST &&
        (flags & OPf_WANT) == OPf_WANT_SCALAR &&
        o->op_private & OPpLVAL_INTRO)
        return ret;

    if (type == OP_LIST || flags & OPf_PARENS ||
        type == OP_RV2AV || type == OP_RV2HV ||
        type == OP_ASLICE || type == OP_HSLICE ||
        type == OP_KVASLICE || type == OP_KVHSLICE || type == OP_REFGEN)
        return ASSIGN_LIST;

    if (type == OP_PADAV || type == OP_PADHV)
        return ASSIGN_LIST;

    if (type == OP_RV2SV)
        return ret;

    return ret;
}

static OP *
S_newONCEOP(pTHX_ OP *initop, OP *padop)
{
    const PADOFFSET target = padop->op_targ;
    OP *const other = newOP(OP_PADSV,
                            padop->op_flags
                            | ((padop->op_private & ~OPpLVAL_INTRO) << 8));
    OP *const first = newOP(OP_NULL, 0);
    OP *const nullop = newCONDOP(0, first, initop, other);
    /* XXX targlex disabled for now; see ticket #124160
        newCONDOP(0, first, S_maybe_targlex(aTHX_ initop), other);
     */
    OP *const condop = first->op_next;

    OpTYPE_set(condop, OP_ONCE);
    other->op_targ = target;
    nullop->op_flags |= OPf_WANT_SCALAR;

    /* Store the initializedness of state vars in a separate
       pad entry.  */
    condop->op_targ =
      pad_add_name_pvn("$",1,padadd_NO_DUP_CHECK|padadd_STATE,0,0);
    /* hijacking PADSTALE for uninitialized state variables */
    SvPADSTALE_on(PAD_SVl(condop->op_targ));

    return nullop;
}

/*
=for apidoc newARGDEFELEMOP

Constructs and returns a new C<OP_ARGDEFELEM> op which provides a defaulting
expression given by C<expr> for the signature parameter at the index given
by C<argindex>. The expression optree is consumed by this function and
becomes part of the returned optree.

=cut
*/

OP *
Perl_newARGDEFELEMOP(pTHX_ I32 flags, OP *expr, I32 argindex)
{
    PERL_ARGS_ASSERT_NEWARGDEFELEMOP;

    OP *o = (OP *)alloc_LOGOP(OP_ARGDEFELEM, expr, LINKLIST(expr));
    o->op_flags |= (U8)(flags);
    o->op_private = 1 | (U8)(flags >> 8);

    /* re-purpose op_targ to hold @_ index */
    o->op_targ = (PADOFFSET)(argindex);

    return o;
}

/*
=for apidoc newASSIGNOP

Constructs, checks, and returns an assignment op.  C<left> and C<right>
supply the parameters of the assignment; they are consumed by this
function and become part of the constructed op tree.

If C<optype> is C<OP_ANDASSIGN>, C<OP_ORASSIGN>, or C<OP_DORASSIGN>, then
a suitable conditional optree is constructed.  If C<optype> is the opcode
of a binary operator, such as C<OP_BIT_OR>, then an op is constructed that
performs the binary operation and assigns the result to the left argument.
Either way, if C<optype> is non-zero then C<flags> has no effect.

If C<optype> is zero, then a plain scalar or list assignment is
constructed.  Which type of assignment it is is automatically determined.
C<flags> gives the eight bits of C<op_flags>, except that C<OPf_KIDS>
will be set automatically, and, shifted up eight bits, the eight bits
of C<op_private>, except that the bit with value 1 or 2 is automatically
set as required.

=cut
*/

OP *
Perl_newASSIGNOP(pTHX_ I32 flags, OP *left, I32 optype, OP *right)
{
    OP *o;
    I32 assign_type;

    switch (optype) {
        case 0: break;
        case OP_ANDASSIGN:
        case OP_ORASSIGN:
        case OP_DORASSIGN:
            right = scalar(right);
            return newLOGOP(optype, 0,
                op_lvalue(scalar(left), optype),
                newBINOP(OP_SASSIGN, OPpASSIGN_BACKWARDS<<8, right, right));
        default:
            return newBINOP(optype, OPf_STACKED,
                op_lvalue(scalar(left), optype), scalar(right));
    }

    if ((assign_type = assignment_type(left)) == ASSIGN_LIST) {
        OP *state_var_op = NULL;
        static const char no_list_state[] = "Initialization of state variables"
            " in list currently forbidden";
        OP *curop;

        if (left->op_type == OP_ASLICE || left->op_type == OP_HSLICE)
            left->op_private &= ~ OPpSLICEWARNING;

        PL_modcount = 0;
        left = op_lvalue(left, OP_AASSIGN);
        curop = list(op_force_list(left));
        o = newBINOP(OP_AASSIGN, flags, list(op_force_list(right)), curop);
        o->op_private = (U8)(0 | (flags >> 8));

        if (OP_TYPE_IS_OR_WAS(left, OP_LIST))
        {
            OP *lop = cLISTOPx(left)->op_first, *vop, *eop;
            if (!(left->op_flags & OPf_PARENS) &&
                    lop->op_type == OP_PUSHMARK &&
                    (vop = OpSIBLING(lop)) &&
                    (vop->op_type == OP_PADAV || vop->op_type == OP_PADHV) &&
                    !(vop->op_flags & OPf_PARENS) &&
                    (vop->op_private & (OPpLVAL_INTRO|OPpPAD_STATE)) ==
                        (OPpLVAL_INTRO|OPpPAD_STATE) &&
                    (eop = OpSIBLING(vop)) &&
                    eop->op_type == OP_ENTERSUB &&
                    !OpHAS_SIBLING(eop)) {
                state_var_op = vop;
            } else {
                while (lop) {
                    if ((lop->op_type == OP_PADSV ||
                         lop->op_type == OP_PADAV ||
                         lop->op_type == OP_PADHV ||
                         lop->op_type == OP_PADANY)
                      && (lop->op_private & OPpPAD_STATE)
                    )
                        yyerror(no_list_state);
                    lop = OpSIBLING(lop);
                }
            }
        }
        else if (  (left->op_private & OPpLVAL_INTRO)
                && (left->op_private & OPpPAD_STATE)
                && (   left->op_type == OP_PADSV
                    || left->op_type == OP_PADAV
                    || left->op_type == OP_PADHV
                    || left->op_type == OP_PADANY)
        ) {
                /* All single variable list context state assignments, hence
                   state ($a) = ...
                   (state $a) = ...
                   state @a = ...
                   state (@a) = ...
                   (state @a) = ...
                   state %a = ...
                   state (%a) = ...
                   (state %a) = ...
                */
                if (left->op_flags & OPf_PARENS)
                    yyerror(no_list_state);
                else
                    state_var_op = left;
        }

        /* optimise @a = split(...) into:
        * @{expr}:              split(..., @{expr}) (where @a is not flattened)
        * @a, my @a, local @a:  split(...)          (where @a is attached to
        *                                            the split op itself)
        */

        if (   right
            && right->op_type == OP_SPLIT
            /* don't do twice, e.g. @b = (@a = split) */
            && !(right->op_private & OPpSPLIT_ASSIGN))
        {
            OP *gvop = NULL;

            if (   (  left->op_type == OP_RV2AV
                   && (gvop=cUNOPx(left)->op_first)->op_type==OP_GV)
                || left->op_type == OP_PADAV)
            {
                /* @pkg or @lex or local @pkg' or 'my @lex' */
                OP *tmpop;
                if (gvop) {
#ifdef USE_ITHREADS
                    cPMOPx(right)->op_pmreplrootu.op_pmtargetoff
                        = cPADOPx(gvop)->op_padix;
                    cPADOPx(gvop)->op_padix = 0;	/* steal it */
#else
                    cPMOPx(right)->op_pmreplrootu.op_pmtargetgv
                        = MUTABLE_GV(cSVOPx(gvop)->op_sv);
                    cSVOPx(gvop)->op_sv = NULL;	/* steal it */
#endif
                    right->op_private |=
                        left->op_private & OPpOUR_INTRO;
                }
                else {
                    cPMOPx(right)->op_pmreplrootu.op_pmtargetoff = left->op_targ;
                    left->op_targ = 0;	/* steal it */
                    right->op_private |= OPpSPLIT_LEX;
                }
                right->op_private |= left->op_private & OPpLVAL_INTRO;

              detach_split:
                tmpop = cUNOPo->op_first;	/* to list (nulled) */
                tmpop = cUNOPx(tmpop)->op_first; /* to pushmark */
                assert(OpSIBLING(tmpop) == right);
                assert(!OpHAS_SIBLING(right));
                /* detach the split subtreee from the o tree,
                 * then free the residual o tree */
                op_sibling_splice(cUNOPo->op_first, tmpop, 1, NULL);
                op_free(o);			/* blow off assign */
                right->op_private |= OPpSPLIT_ASSIGN;
                right->op_flags &= ~OPf_WANT;
                        /* "I don't know and I don't care." */
                return right;
            }
            else if (left->op_type == OP_RV2AV) {
                /* @{expr} */

                OP *pushop = cUNOPx(cBINOPo->op_last)->op_first;
                assert(OpSIBLING(pushop) == left);
                /* Detach the array ...  */
                op_sibling_splice(cBINOPo->op_last, pushop, 1, NULL);
                /* ... and attach it to the split.  */
                op_sibling_splice(right, cLISTOPx(right)->op_last,
                                  0, left);
                right->op_flags |= OPf_STACKED;
                /* Detach split and expunge aassign as above.  */
                goto detach_split;
            }
            else if (PL_modcount < RETURN_UNLIMITED_NUMBER &&
                    cLISTOPx(right)->op_last->op_type == OP_CONST)
            {
                /* convert split(...,0) to split(..., PL_modcount+1) */
                SV ** const svp =
                    &cSVOPx(cLISTOPx(right)->op_last)->op_sv;
                SV * const sv = *svp;
                if (SvIOK(sv) && SvIVX(sv) == 0)
                {
                  if (right->op_private & OPpSPLIT_IMPLIM) {
                    /* our own SV, created in ck_split */
                    SvREADONLY_off(sv);
                    sv_setiv(sv, PL_modcount+1);
                  }
                  else {
                    /* SV may belong to someone else */
                    SvREFCNT_dec(sv);
                    *svp = newSViv(PL_modcount+1);
                  }
                }
            }
        }

        if (state_var_op)
            o = S_newONCEOP(aTHX_ o, state_var_op);
        return o;
    }
    if (assign_type == ASSIGN_REF)
        return newBINOP(OP_REFASSIGN, flags, scalar(right), left);
    if (!right)
        right = newOP(OP_UNDEF, 0);
    if (right->op_type == OP_READLINE) {
        right->op_flags |= OPf_STACKED;
        return newBINOP(OP_NULL, flags, op_lvalue(scalar(left), OP_SASSIGN),
                scalar(right));
    }
    else {
        o = newBINOP(OP_SASSIGN, flags,
            scalar(right), op_lvalue(scalar(left), OP_SASSIGN) );
    }
    return o;
}

/*
=for apidoc newSTATEOP

Constructs a state op (COP).  The state op is normally a C<nextstate> op,
but will be a C<dbstate> op if debugging is enabled for currently-compiled
code.  The state op is populated from C<PL_curcop> (or C<PL_compiling>).
If C<label> is non-null, it supplies the name of a label to attach to
the state op; this function takes ownership of the memory pointed at by
C<label>, and will free it.  C<flags> gives the eight bits of C<op_flags>
for the state op.

If C<o> is null, the state op is returned.  Otherwise the state op is
combined with C<o> into a C<lineseq> list op, which is returned.  C<o>
is consumed by this function and becomes part of the returned op tree.

=cut
*/

OP *
Perl_newSTATEOP(pTHX_ I32 flags, char *label, OP *o)
{
    const U32 seq = intro_my();
    const U32 utf8 = flags & SVf_UTF8;
    COP *cop;

    assert(PL_parser);
    PL_parser->parsed_sub = 0;

    flags &= ~SVf_UTF8;

    NewOp(1101, cop, 1, COP);
    if (PERLDB_LINE && CopLINE(PL_curcop) && PL_curstash != PL_debstash) {
        OpTYPE_set(cop, OP_DBSTATE);
    }
    else {
        OpTYPE_set(cop, OP_NEXTSTATE);
    }
    cop->op_flags = (U8)flags;
    CopHINTS_set(cop, PL_hints);
#ifdef VMS
    if (VMSISH_HUSHED) cop->op_private |= OPpHUSH_VMSISH;
#endif
    cop->op_next = (OP*)cop;

    cop->cop_seq = seq;
    cop->cop_warnings = DUP_WARNINGS(PL_curcop->cop_warnings);
    CopHINTHASH_set(cop, cophh_copy(CopHINTHASH_get(PL_curcop)));
    CopFEATURES_setfrom(cop, PL_curcop);
    if (label) {
        Perl_cop_store_label(aTHX_ cop, label, strlen(label), utf8);

        PL_hints |= HINT_BLOCK_SCOPE;
        /* It seems that we need to defer freeing this pointer, as other parts
           of the grammar end up wanting to copy it after this op has been
           created. */
        SAVEFREEPV(label);
    }

    if (PL_parser->preambling != NOLINE) {
        CopLINE_set(cop, PL_parser->preambling);
        PL_parser->copline = NOLINE;
    }
    else if (PL_parser->copline == NOLINE)
        CopLINE_set(cop, CopLINE(PL_curcop));
    else {
        CopLINE_set(cop, PL_parser->copline);
        PL_parser->copline = NOLINE;
    }
#ifdef USE_ITHREADS
    CopFILE_copy(cop, PL_curcop);
#else
    CopFILEGV_set(cop, CopFILEGV(PL_curcop));
#endif
    CopSTASH_set(cop, PL_curstash);

    if (cop->op_type == OP_DBSTATE) {
        /* this line can have a breakpoint - store the cop in IV */
        AV *av = CopFILEAVx(PL_curcop);
        if (av) {
            SV * const * const svp = av_fetch(av, CopLINE(cop), FALSE);
            if (svp && *svp != &PL_sv_undef ) {
                (void)SvIOK_on(*svp);
                SvIV_set(*svp, PTR2IV(cop));
            }
        }
    }

    if (flags & OPf_SPECIAL)
        op_null((OP*)cop);
    return op_prepend_elem(OP_LINESEQ, (OP*)cop, o);
}

/*
=for apidoc newLOGOP

Constructs, checks, and returns a logical (flow control) op.  C<type>
is the opcode.  C<flags> gives the eight bits of C<op_flags>, except
that C<OPf_KIDS> will be set automatically, and, shifted up eight bits,
the eight bits of C<op_private>, except that the bit with value 1 is
automatically set.  C<first> supplies the expression controlling the
flow, and C<other> supplies the side (alternate) chain of ops; they are
consumed by this function and become part of the constructed op tree.

=cut
*/

OP *
Perl_newLOGOP(pTHX_ I32 type, I32 flags, OP *first, OP *other)
{
    PERL_ARGS_ASSERT_NEWLOGOP;

    return new_logop(type, flags, &first, &other);
}


/* See if the optree o contains a single OP_CONST (plus possibly
 * surrounding enter/nextstate/null etc). If so, return it, else return
 * NULL.
 */

STATIC OP *
S_search_const(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_SEARCH_CONST;

  redo:
    switch (o->op_type) {
        case OP_CONST:
            return o;
        case OP_NULL:
            if (o->op_flags & OPf_KIDS) {
                o = cUNOPo->op_first;
                goto redo;
            }
            break;
        case OP_LEAVE:
        case OP_SCOPE:
        case OP_LINESEQ:
        {
            OP *kid;
            if (!(o->op_flags & OPf_KIDS))
                return NULL;
            kid = cLISTOPo->op_first;

            do {
                switch (kid->op_type) {
                    case OP_ENTER:
                    case OP_NULL:
                    case OP_NEXTSTATE:
                        kid = OpSIBLING(kid);
                        break;
                    default:
                        if (kid != cLISTOPo->op_last)
                            return NULL;
                        goto last;
                }
            } while (kid);

            if (!kid)
                kid = cLISTOPo->op_last;
          last:
             o = kid;
             goto redo;
        }
    }

    return NULL;
}


STATIC OP *
S_new_logop(pTHX_ I32 type, I32 flags, OP** firstp, OP** otherp)
{
    LOGOP *logop;
    OP *o;
    OP *first;
    OP *other;
    OP *cstop = NULL;
    int prepend_not = 0;

    PERL_ARGS_ASSERT_NEW_LOGOP;

    first = *firstp;
    other = *otherp;

    /* [perl #59802]: Warn about things like "return $a or $b", which
       is parsed as "(return $a) or $b" rather than "return ($a or
       $b)".  NB: This also applies to xor, which is why we do it
       here.
     */
    switch (first->op_type) {
    case OP_NEXT:
    case OP_LAST:
    case OP_REDO:
        /* XXX: Perhaps we should emit a stronger warning for these.
           Even with the high-precedence operator they don't seem to do
           anything sensible.

           But until we do, fall through here.
         */
    case OP_RETURN:
    case OP_EXIT:
    case OP_DIE:
    case OP_GOTO:
        /* XXX: Currently we allow people to "shoot themselves in the
           foot" by explicitly writing "(return $a) or $b".

           Warn unless we are looking at the result from folding or if
           the programmer explicitly grouped the operators like this.
           The former can occur with e.g.

                use constant FEATURE => ( $] >= ... );
                sub { not FEATURE and return or do_stuff(); }
         */
        if (!first->op_folded && !(first->op_flags & OPf_PARENS))
            Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                           "Possible precedence issue with control flow operator");
        /* XXX: Should we optimze this to "return $a;" (i.e. remove
           the "or $b" part)?
        */
        break;
    }

    if (type == OP_XOR)		/* Not short circuit, but here by precedence. */
        return newBINOP(type, flags, scalar(first), scalar(other));

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_LOGOP
        || type == OP_CUSTOM);

    scalarboolean(first);

    /* search for a constant op that could let us fold the test */
    if ((cstop = search_const(first))) {
        if (cstop->op_private & OPpCONST_STRICT)
            no_bareword_allowed(cstop);
        else if ((cstop->op_private & OPpCONST_BARE))
                Perl_ck_warner(aTHX_ packWARN(WARN_BAREWORD), "Bareword found in conditional");
        if ((type == OP_AND &&  SvTRUE(cSVOPx(cstop)->op_sv)) ||
            (type == OP_OR  && !SvTRUE(cSVOPx(cstop)->op_sv)) ||
            (type == OP_DOR && !SvOK(cSVOPx(cstop)->op_sv))) {
            /* Elide the (constant) lhs, since it can't affect the outcome */
            *firstp = NULL;
            if (other->op_type == OP_CONST)
                other->op_private |= OPpCONST_SHORTCIRCUIT;
            op_free(first);
            if (other->op_type == OP_LEAVE)
                other = newUNOP(OP_NULL, OPf_SPECIAL, other);
            else if (other->op_type == OP_MATCH
                  || other->op_type == OP_SUBST
                  || other->op_type == OP_TRANSR
                  || other->op_type == OP_TRANS)
                /* Mark the op as being unbindable with =~ */
                other->op_flags |= OPf_SPECIAL;

            other->op_folded = 1;
            return other;
        }
        else {
            /* Elide the rhs, since the outcome is entirely determined by
             * the (constant) lhs */

            /* check for C<my $x if 0>, or C<my($x,$y) if 0> */
            const OP *o2 = other;
            if ( ! (o2->op_type == OP_LIST
                    && (( o2 = cUNOPx(o2)->op_first))
                    && o2->op_type == OP_PUSHMARK
                    && (( o2 = OpSIBLING(o2))) )
            )
                o2 = other;
            if ((o2->op_type == OP_PADSV || o2->op_type == OP_PADAV
                        || o2->op_type == OP_PADHV)
                && o2->op_private & OPpLVAL_INTRO
                && !(o2->op_private & OPpPAD_STATE))
            {
        Perl_croak(aTHX_ "This use of my() in false conditional is "
                          "no longer allowed");
            }

            *otherp = NULL;
            if (cstop->op_type == OP_CONST)
                cstop->op_private |= OPpCONST_SHORTCIRCUIT;
            op_free(other);
            return first;
        }
    }
    else if ((first->op_flags & OPf_KIDS) && type != OP_DOR
        && ckWARN(WARN_MISC)) /* [#24076] Don't warn for <FH> err FOO. */
    {
        const OP * const k1 = cUNOPx(first)->op_first;
        const OP * const k2 = OpSIBLING(k1);
        OPCODE warnop = 0;
        switch (first->op_type)
        {
        case OP_NULL:
            if (k2 && k2->op_type == OP_READLINE
                  && (k2->op_flags & OPf_STACKED)
                  && ((k1->op_flags & OPf_WANT) == OPf_WANT_SCALAR))
            {
                warnop = k2->op_type;
            }
            break;

        case OP_SASSIGN:
            if (k1->op_type == OP_READDIR
                  || k1->op_type == OP_GLOB
                  || (k1->op_type == OP_NULL && k1->op_targ == OP_GLOB)
                 || k1->op_type == OP_EACH
                 || k1->op_type == OP_AEACH)
            {
                warnop = ((k1->op_type == OP_NULL)
                          ? (OPCODE)k1->op_targ : k1->op_type);
            }
            break;
        }
        if (warnop) {
            const line_t oldline = CopLINE(PL_curcop);
            /* This ensures that warnings are reported at the first line
               of the construction, not the last.  */
            CopLINE_set(PL_curcop, PL_parser->copline);
            Perl_warner(aTHX_ packWARN(WARN_MISC),
                 "Value of %s%s can be \"0\"; test with defined()",
                 PL_op_desc[warnop],
                 ((warnop == OP_READLINE || warnop == OP_GLOB)
                  ? " construct" : "() operator"));
            CopLINE_set(PL_curcop, oldline);
        }
    }

    /* optimize AND and OR ops that have NOTs as children */
    if (first->op_type == OP_NOT
        && (first->op_flags & OPf_KIDS)
        && ((first->op_flags & OPf_SPECIAL) /* unless ($x) { } */
            || (other->op_type == OP_NOT))  /* if (!$x && !$y) { } */
        ) {
        if (type == OP_AND || type == OP_OR) {
            if (type == OP_AND)
                type = OP_OR;
            else
                type = OP_AND;
            op_null(first);
            if (other->op_type == OP_NOT) { /* !a AND|OR !b => !(a OR|AND b) */
                op_null(other);
                prepend_not = 1; /* prepend a NOT op later */
            }
        }
    }

    logop = alloc_LOGOP(type, first, LINKLIST(other));
    logop->op_flags |= (U8)flags;
    logop->op_private = (U8)(1 | (flags >> 8));

    /* establish postfix order */
    logop->op_next = LINKLIST(first);
    first->op_next = (OP*)logop;
    assert(!OpHAS_SIBLING(first));
    op_sibling_splice((OP*)logop, first, 0, other);

    CHECKOP(type,logop);

    o = newUNOP(prepend_not ? OP_NOT : OP_NULL,
                PL_opargs[type] & OA_RETSCALAR ? OPf_WANT_SCALAR : 0,
                (OP*)logop);
    other->op_next = o;

    return o;
}

/*
=for apidoc newCONDOP

Constructs, checks, and returns a conditional-expression (C<cond_expr>)
op.  C<flags> gives the eight bits of C<op_flags>, except that C<OPf_KIDS>
will be set automatically, and, shifted up eight bits, the eight bits of
C<op_private>, except that the bit with value 1 is automatically set.
C<first> supplies the expression selecting between the two branches,
and C<trueop> and C<falseop> supply the branches; they are consumed by
this function and become part of the constructed op tree.

=cut
*/

OP *
Perl_newCONDOP(pTHX_ I32 flags, OP *first, OP *trueop, OP *falseop)
{
    LOGOP *logop;
    OP *start;
    OP *o;
    OP *cstop;

    PERL_ARGS_ASSERT_NEWCONDOP;

    if (!falseop)
        return newLOGOP(OP_AND, 0, first, trueop);
    if (!trueop)
        return newLOGOP(OP_OR, 0, first, falseop);

    scalarboolean(first);
    if ((cstop = search_const(first))) {
        /* Left or right arm of the conditional?  */
        const bool left = SvTRUE(cSVOPx(cstop)->op_sv);
        OP *live = left ? trueop : falseop;
        OP *const dead = left ? falseop : trueop;
        if (cstop->op_private & OPpCONST_BARE &&
            cstop->op_private & OPpCONST_STRICT) {
            no_bareword_allowed(cstop);
        }
        op_free(first);
        op_free(dead);
        if (live->op_type == OP_LEAVE)
            live = newUNOP(OP_NULL, OPf_SPECIAL, live);
        else if (live->op_type == OP_MATCH || live->op_type == OP_SUBST
              || live->op_type == OP_TRANS || live->op_type == OP_TRANSR)
            /* Mark the op as being unbindable with =~ */
            live->op_flags |= OPf_SPECIAL;
        live->op_folded = 1;
        return live;
    }
    logop = alloc_LOGOP(OP_COND_EXPR, first, LINKLIST(trueop));
    logop->op_flags |= (U8)flags;
    logop->op_private = (U8)(1 | (flags >> 8));
    logop->op_next = LINKLIST(falseop);

    CHECKOP(OP_COND_EXPR, /* that's logop->op_type */
            logop);

    /* establish postfix order */
    start = LINKLIST(first);
    first->op_next = (OP*)logop;

    /* make first, trueop, falseop siblings */
    op_sibling_splice((OP*)logop, first,  0, trueop);
    op_sibling_splice((OP*)logop, trueop, 0, falseop);

    o = newUNOP(OP_NULL, 0, (OP*)logop);

    trueop->op_next = falseop->op_next = o;

    o->op_next = start;
    return o;
}

/*
=for apidoc newTRYCATCHOP

Constructs and returns a conditional execution statement that implements
the C<try>/C<catch> semantics.  First the op tree in C<tryblock> is executed,
inside a context that traps exceptions.  If an exception occurs then the
optree in C<catchblock> is executed, with the trapped exception set into the
lexical variable given by C<catchvar> (which must be an op of type
C<OP_PADSV>).  All the optrees are consumed by this function and become part
of the returned op tree.

The C<flags> argument is currently ignored.

=cut
 */

OP *
Perl_newTRYCATCHOP(pTHX_ I32 flags, OP *tryblock, OP *catchvar, OP *catchblock)
{
    OP *o, *catchop;

    PERL_ARGS_ASSERT_NEWTRYCATCHOP;
    assert(catchvar->op_type == OP_PADSV);

    PERL_UNUSED_ARG(flags);

    /* The returned optree is shaped as:
     *   LISTOP leavetrycatch
     *       LOGOP entertrycatch
     *       LISTOP poptry
     *           $tryblock here
     *       LOGOP catch
     *           $catchblock here
     */

    if(tryblock->op_type != OP_LINESEQ)
        tryblock = op_convert_list(OP_LINESEQ, 0, tryblock);
    OpTYPE_set(tryblock, OP_POPTRY);

    /* Manually construct a naked LOGOP.
     * Normally if we call newLOGOP the returned value is a UNOP(OP_NULL)
     * containing the LOGOP we wanted as its op_first */
    catchop = (OP *)alloc_LOGOP(OP_CATCH, newOP(OP_NULL, 0), catchblock);
    OpMORESIB_set(cUNOPx(catchop)->op_first, catchblock);
    OpLASTSIB_set(catchblock, catchop);

    /* Inject the catchvar's pad offset into the OP_CATCH targ */
    cLOGOPx(catchop)->op_targ = catchvar->op_targ;
    op_free(catchvar);

    /* Build the optree structure */
    o = newLISTOP(OP_LIST, 0, tryblock, catchop);
    o = op_convert_list(OP_ENTERTRYCATCH, 0, o);

    return o;
}

/*
=for apidoc newRANGE

Constructs and returns a C<range> op, with subordinate C<flip> and
C<flop> ops.  C<flags> gives the eight bits of C<op_flags> for the
C<flip> op and, shifted up eight bits, the eight bits of C<op_private>
for both the C<flip> and C<range> ops, except that the bit with value
1 is automatically set.  C<left> and C<right> supply the expressions
controlling the endpoints of the range; they are consumed by this function
and become part of the constructed op tree.

=cut
*/

OP *
Perl_newRANGE(pTHX_ I32 flags, OP *left, OP *right)
{
    LOGOP *range;
    OP *flip;
    OP *flop;
    OP *leftstart;
    OP *o;

    PERL_ARGS_ASSERT_NEWRANGE;

    range = alloc_LOGOP(OP_RANGE, left, LINKLIST(right));
    range->op_flags = OPf_KIDS;
    leftstart = LINKLIST(left);
    range->op_private = (U8)(1 | (flags >> 8));

    /* make left and right siblings */
    op_sibling_splice((OP*)range, left, 0, right);

    range->op_next = (OP*)range;
    flip = newUNOP(OP_FLIP, flags, (OP*)range);
    flop = newUNOP(OP_FLOP, 0, flip);
    o = newUNOP(OP_NULL, 0, flop);
    LINKLIST(flop);
    range->op_next = leftstart;

    left->op_next = flip;
    right->op_next = flop;

    range->op_targ =
        pad_add_name_pvn("$", 1, padadd_NO_DUP_CHECK|padadd_STATE, 0, 0);
    sv_upgrade(PAD_SV(range->op_targ), SVt_PVNV);
    flip->op_targ =
        pad_add_name_pvn("$", 1, padadd_NO_DUP_CHECK|padadd_STATE, 0, 0);;
    sv_upgrade(PAD_SV(flip->op_targ), SVt_PVNV);
    SvPADTMP_on(PAD_SV(flip->op_targ));

    flip->op_private =  left->op_type == OP_CONST ? OPpFLIP_LINENUM : 0;
    flop->op_private = right->op_type == OP_CONST ? OPpFLIP_LINENUM : 0;

    /* check barewords before they might be optimized away */
    if (flip->op_private && cSVOPx(left)->op_private & OPpCONST_STRICT)
        no_bareword_allowed(left);
    if (flop->op_private && cSVOPx(right)->op_private & OPpCONST_STRICT)
        no_bareword_allowed(right);

    flip->op_next = o;
    if (!flip->op_private || !flop->op_private)
        LINKLIST(o);		/* blow off optimizer unless constant */

    return o;
}

/*
=for apidoc newLOOPOP

Constructs, checks, and returns an op tree expressing a loop.  This is
only a loop in the control flow through the op tree; it does not have
the heavyweight loop structure that allows exiting the loop by C<last>
and suchlike.  C<flags> gives the eight bits of C<op_flags> for the
top-level op, except that some bits will be set automatically as required.
C<expr> supplies the expression controlling loop iteration, and C<block>
supplies the body of the loop; they are consumed by this function and
become part of the constructed op tree.  C<debuggable> is currently
unused and should always be 1.

=cut
*/

OP *
Perl_newLOOPOP(pTHX_ I32 flags, I32 debuggable, OP *expr, OP *block)
{
    PERL_ARGS_ASSERT_NEWLOOPOP;

    OP* listop;
    OP* o;
    const bool once = block && block->op_flags & OPf_SPECIAL &&
                      block->op_type == OP_NULL;

    PERL_UNUSED_ARG(debuggable);

    if (once && (
          (expr->op_type == OP_CONST && !SvTRUE(cSVOPx(expr)->op_sv))
       || (  expr->op_type == OP_NOT
          && cUNOPx(expr)->op_first->op_type == OP_CONST
          && SvTRUE(cSVOPx_sv(cUNOPx(expr)->op_first))
          )
       ))
        /* Return the block now, so that S_new_logop does not try to
           fold it away. */
    {
        op_free(expr);
        return block;	/* do {} while 0 does once */
    }

    if (expr->op_type == OP_READLINE
        || expr->op_type == OP_READDIR
        || expr->op_type == OP_GLOB
        || expr->op_type == OP_EACH || expr->op_type == OP_AEACH
        || (expr->op_type == OP_NULL && expr->op_targ == OP_GLOB)) {
        expr = newUNOP(OP_DEFINED, 0,
            newASSIGNOP(0, newDEFSVOP(), 0, expr) );
    } else if (expr->op_flags & OPf_KIDS) {
        const OP * const k1 = cUNOPx(expr)->op_first;
        const OP * const k2 = k1 ? OpSIBLING(k1) : NULL;
        switch (expr->op_type) {
          case OP_NULL:
            if (k2 && (k2->op_type == OP_READLINE || k2->op_type == OP_READDIR)
                  && (k2->op_flags & OPf_STACKED)
                  && ((k1->op_flags & OPf_WANT) == OPf_WANT_SCALAR))
                expr = newUNOP(OP_DEFINED, 0, expr);
            break;

          case OP_SASSIGN:
            if (k1 && (k1->op_type == OP_READDIR
                  || k1->op_type == OP_GLOB
                  || (k1->op_type == OP_NULL && k1->op_targ == OP_GLOB)
                  || k1->op_type == OP_EACH
                  || k1->op_type == OP_AEACH))
                expr = newUNOP(OP_DEFINED, 0, expr);
            break;
        }
    }

    /* if block is null, the next op_append_elem() would put UNSTACK, a scalar
     * op, in listop. This is wrong. [perl #27024] */
    if (!block)
        block = newOP(OP_NULL, 0);
    listop = op_append_elem(OP_LINESEQ, block, newOP(OP_UNSTACK, 0));
    o = new_logop(OP_AND, 0, &expr, &listop);

    if (once) {
        ASSUME(listop);
    }

    if (listop)
        cLISTOPx(listop)->op_last->op_next = LINKLIST(o);

    if (once && o != listop)
    {
        assert(cUNOPo->op_first->op_type == OP_AND
            || cUNOPo->op_first->op_type == OP_OR);
        o->op_next = cLOGOPx(cUNOPo->op_first)->op_other;
    }

    if (o == listop)
        o = newUNOP(OP_NULL, 0, o);	/* or do {} while 1 loses outer block */

    o->op_flags |= flags;
    o = op_scope(o);
    o->op_flags |= OPf_SPECIAL;	/* suppress cx_popblock() curpm restoration*/
    return o;
}

/*
=for apidoc newWHILEOP

Constructs, checks, and returns an op tree expressing a C<while> loop.
This is a heavyweight loop, with structure that allows exiting the loop
by C<last> and suchlike.

C<loop> is an optional preconstructed C<enterloop> op to use in the
loop; if it is null then a suitable op will be constructed automatically.
C<expr> supplies the loop's controlling expression.  C<block> supplies the
main body of the loop, and C<cont> optionally supplies a C<continue> block
that operates as a second half of the body.  All of these optree inputs
are consumed by this function and become part of the constructed op tree.

C<flags> gives the eight bits of C<op_flags> for the C<leaveloop>
op and, shifted up eight bits, the eight bits of C<op_private> for
the C<leaveloop> op, except that (in both cases) some bits will be set
automatically.  C<debuggable> is currently unused and should always be 1.
C<has_my> can be supplied as true to force the
loop body to be enclosed in its own scope.

=cut
*/

OP *
Perl_newWHILEOP(pTHX_ I32 flags, I32 debuggable, LOOP *loop,
        OP *expr, OP *block, OP *cont, I32 has_my)
{
    OP *redo;
    OP *next = NULL;
    OP *listop;
    OP *o;
    U8 loopflags = 0;

    PERL_UNUSED_ARG(debuggable);

    if (expr) {
        if (expr->op_type == OP_READLINE
         || expr->op_type == OP_READDIR
         || expr->op_type == OP_GLOB
         || expr->op_type == OP_EACH || expr->op_type == OP_AEACH
                     || (expr->op_type == OP_NULL && expr->op_targ == OP_GLOB)) {
            expr = newUNOP(OP_DEFINED, 0,
                newASSIGNOP(0, newDEFSVOP(), 0, expr) );
        } else if (expr->op_flags & OPf_KIDS) {
            const OP * const k1 = cUNOPx(expr)->op_first;
            const OP * const k2 = (k1) ? OpSIBLING(k1) : NULL;
            switch (expr->op_type) {
              case OP_NULL:
                if (k2 && (k2->op_type == OP_READLINE || k2->op_type == OP_READDIR)
                      && (k2->op_flags & OPf_STACKED)
                      && ((k1->op_flags & OPf_WANT) == OPf_WANT_SCALAR))
                    expr = newUNOP(OP_DEFINED, 0, expr);
                break;

              case OP_SASSIGN:
                if (k1 && (k1->op_type == OP_READDIR
                      || k1->op_type == OP_GLOB
                      || (k1->op_type == OP_NULL && k1->op_targ == OP_GLOB)
                     || k1->op_type == OP_EACH
                     || k1->op_type == OP_AEACH))
                    expr = newUNOP(OP_DEFINED, 0, expr);
                break;
            }
        }
    }

    if (!block)
        block = newOP(OP_NULL, 0);
    else if (cont || has_my) {
        block = op_scope(block);
    }

    if (cont) {
        next = LINKLIST(cont);
    }
    if (expr) {
        OP * const unstack = newOP(OP_UNSTACK, 0);
        if (!next)
            next = unstack;
        cont = op_append_elem(OP_LINESEQ, cont, unstack);
    }

    assert(block);
    listop = op_append_list(OP_LINESEQ, block, cont);
    assert(listop);
    redo = LINKLIST(listop);

    if (expr) {
        scalar(listop);
        o = new_logop(OP_AND, 0, &expr, &listop);
        if (o == expr && o->op_type == OP_CONST && !SvTRUE(cSVOPo->op_sv)) {
            op_free((OP*)loop);
            return expr;		/* listop already freed by new_logop */
        }
        if (listop)
            cLISTOPx(listop)->op_last->op_next =
                (o == listop ? redo : LINKLIST(o));
    }
    else
        o = listop;

    if (!loop) {
        NewOp(1101,loop,1,LOOP);
        OpTYPE_set(loop, OP_ENTERLOOP);
        loop->op_private = 0;
        loop->op_next = (OP*)loop;
    }

    o = newBINOP(OP_LEAVELOOP, 0, (OP*)loop, o);

    loop->op_redoop = redo;
    loop->op_lastop = o;
    o->op_private |= loopflags;

    if (next)
        loop->op_nextop = next;
    else
        loop->op_nextop = o;

    o->op_flags |= flags;
    o->op_private |= (flags >> 8);
    return o;
}

/*
=for apidoc newFOROP

Constructs, checks, and returns an op tree expressing a C<foreach>
loop (iteration through a list of values).  This is a heavyweight loop,
with structure that allows exiting the loop by C<last> and suchlike.

C<sv> optionally supplies the variable(s) that will be aliased to each
item in turn; if null, it defaults to C<$_>.
C<expr> supplies the list of values to iterate over.  C<block> supplies
the main body of the loop, and C<cont> optionally supplies a C<continue>
block that operates as a second half of the body.  All of these optree
inputs are consumed by this function and become part of the constructed
op tree.

C<flags> gives the eight bits of C<op_flags> for the C<leaveloop>
op and, shifted up eight bits, the eight bits of C<op_private> for
the C<leaveloop> op, except that (in both cases) some bits will be set
automatically.

=cut
*/

OP *
Perl_newFOROP(pTHX_ I32 flags, OP *sv, OP *expr, OP *block, OP *cont)
{
    LOOP *loop;
    OP *iter;
    PADOFFSET padoff = 0;
    PADOFFSET how_many_more = 0;
    I32 iterflags = 0;
    I32 iterpflags = 0;
    bool parens = 0;

    PERL_ARGS_ASSERT_NEWFOROP;

    if (sv) {
        if (sv->op_type == OP_RV2SV) {	/* symbol table variable */
            iterpflags = sv->op_private & OPpOUR_INTRO; /* for our $x () */
            OpTYPE_set(sv, OP_RV2GV);

            /* The op_type check is needed to prevent a possible segfault
             * if the loop variable is undeclared and 'strict vars' is in
             * effect. This is illegal but is nonetheless parsed, so we
             * may reach this point with an OP_CONST where we're expecting
             * an OP_GV.
             */
            if (cUNOPx(sv)->op_first->op_type == OP_GV
             && cGVOPx_gv(cUNOPx(sv)->op_first) == PL_defgv)
                iterpflags |= OPpITER_DEF;
        }
        else if (sv->op_type == OP_PADSV) { /* private variable */
            if (sv->op_flags & OPf_PARENS) {
                /* handle degenerate 1-var form of "for my ($x, ...)" */
                sv->op_private |= OPpLVAL_INTRO;
                parens = 1;
            }
            iterpflags = sv->op_private & OPpLVAL_INTRO; /* for my $x () */
            padoff = sv->op_targ;
            sv->op_targ = 0;
            op_free(sv);
            sv = NULL;
            PAD_COMPNAME_GEN_set(padoff, PERL_INT_MAX);
        }
        else if (sv->op_type == OP_NULL && sv->op_targ == OP_SREFGEN)
            NOOP;
        else if (sv->op_type == OP_LIST) {
            LISTOP *list = cLISTOPx(sv);
            OP *pushmark = list->op_first;
            OP *first_padsv;
            UNOP *padsv;
            PADOFFSET i;

            iterpflags = OPpLVAL_INTRO; /* for my ($k, $v) () */
            parens = 1;

            if (!pushmark || pushmark->op_type != OP_PUSHMARK) {
                Perl_croak(aTHX_ "panic: newFORLOOP, found %s, expecting pushmark",
                           pushmark ? PL_op_desc[pushmark->op_type] : "NULL");
            }
            first_padsv = OpSIBLING(pushmark);
            if (!first_padsv || first_padsv->op_type != OP_PADSV) {
                Perl_croak(aTHX_ "panic: newFORLOOP, found %s, expecting padsv",
                           first_padsv ? PL_op_desc[first_padsv->op_type] : "NULL");
            }
            padoff = first_padsv->op_targ;

            /* There should be at least one more PADSV to find, and the ops
               should have consecutive values in targ: */
            padsv = cUNOPx(OpSIBLING(first_padsv));
            do {
                if (!padsv || padsv->op_type != OP_PADSV) {
                    Perl_croak(aTHX_ "panic: newFORLOOP, found %s at %zd, expecting padsv",
                               padsv ? PL_op_desc[padsv->op_type] : "NULL",
                               how_many_more);
                }
                ++how_many_more;
                if (padsv->op_targ != padoff + how_many_more) {
                    Perl_croak(aTHX_ "panic: newFORLOOP, padsv at %zd targ is %zd, not %zd",
                               how_many_more, padsv->op_targ, padoff + how_many_more);
                }

                padsv = cUNOPx(OpSIBLING(padsv));
            } while (padsv);

            /* OK, this optree has the shape that we expected. So now *we*
               "claim" the Pad slots: */
            first_padsv->op_targ = 0;
            PAD_COMPNAME_GEN_set(padoff, PERL_INT_MAX);

            i = padoff;

            padsv = cUNOPx(OpSIBLING(first_padsv));
            do {
                ++i;
                padsv->op_targ = 0;
                PAD_COMPNAME_GEN_set(i, PERL_INT_MAX);

                padsv = cUNOPx(OpSIBLING(padsv));
            } while (padsv);

            op_free(sv);
            sv = NULL;
        }
        else
            Perl_croak(aTHX_ "Can't use %s for loop variable", PL_op_desc[sv->op_type]);
        if (padoff) {
            PADNAME * const pn = PAD_COMPNAME(padoff);
            const char * const name = PadnamePV(pn);

            if (PadnameLEN(pn) == 2 && name[0] == '$' && name[1] == '_')
                iterpflags |= OPpITER_DEF;
        }
    }
    else {
        sv = newGVOP(OP_GV, 0, PL_defgv);
        iterpflags |= OPpITER_DEF;
    }

    if (expr->op_type == OP_RV2AV || expr->op_type == OP_PADAV) {
        expr = op_lvalue(op_force_list(scalar(ref(expr, OP_ITER))), OP_GREPSTART);
        iterflags |= OPf_STACKED;
    }
    else if (expr->op_type == OP_NULL &&
             (expr->op_flags & OPf_KIDS) &&
             cBINOPx(expr)->op_first->op_type == OP_FLOP)
    {
        /* Basically turn for($x..$y) into the same as for($x,$y), but we
         * set the STACKED flag to indicate that these values are to be
         * treated as min/max values by 'pp_enteriter'.
         */
        const UNOP* const flip = cUNOPx(cUNOPx(cBINOPx(expr)->op_first)->op_first);
        LOGOP* const range = cLOGOPx(flip->op_first);
        OP* const left  = range->op_first;
        OP* const right = OpSIBLING(left);
        LISTOP* listop;

        range->op_flags &= ~OPf_KIDS;
        /* detach range's children */
        op_sibling_splice((OP*)range, NULL, -1, NULL);

        listop = cLISTOPx(newLISTOP(OP_LIST, 0, left, right));
        listop->op_first->op_next = range->op_next;
        left->op_next = range->op_other;
        right->op_next = (OP*)listop;
        listop->op_next = listop->op_first;

        op_free(expr);
        expr = (OP*)(listop);
        op_null(expr);
        iterflags |= OPf_STACKED;
    }
    else {
        expr = op_lvalue(op_force_list(expr), OP_GREPSTART);
    }

    loop = (LOOP*)op_convert_list(OP_ENTERITER, iterflags,
                                  op_append_elem(OP_LIST, list(expr),
                                                 scalar(sv)));
    assert(!loop->op_next);
    /* for my  $x () sets OPpLVAL_INTRO;
     * for our $x () sets OPpOUR_INTRO */
    loop->op_private = (U8)iterpflags;

    /* upgrade loop from a LISTOP to a LOOPOP;
     * keep it in-place if there's space */
    if (loop->op_slabbed
        &&    OpSLOT(loop)->opslot_size
            < SIZE_TO_PSIZE(sizeof(LOOP) + OPSLOT_HEADER))
    {
        /* no space; allocate new op */
        LOOP *tmp;
        NewOp(1234,tmp,1,LOOP);
        Copy(loop,tmp,1,LISTOP);
        assert(loop->op_last->op_sibparent == (OP*)loop);
        OpLASTSIB_set(loop->op_last, (OP*)tmp); /*point back to new parent */
        S_op_destroy(aTHX_ (OP*)loop);
        loop = tmp;
    }
    else if (!loop->op_slabbed)
    {
        /* loop was malloc()ed */
        loop = (LOOP*)PerlMemShared_realloc(loop, sizeof(LOOP));
        OpLASTSIB_set(loop->op_last, (OP*)loop);
    }
    loop->op_targ = padoff;
    if (parens)
        /* hint to deparser that this:  for my (...) ... */
        loop->op_flags |= OPf_PARENS;
    iter = newOP(OP_ITER, 0);
    iter->op_targ = how_many_more;
    return newWHILEOP(flags, 1, loop, iter, block, cont, 0);
}

/*
=for apidoc newLOOPEX

Constructs, checks, and returns a loop-exiting op (such as C<goto>
or C<last>).  C<type> is the opcode.  C<label> supplies the parameter
determining the target of the op; it is consumed by this function and
becomes part of the constructed op tree.

=cut
*/

OP*
Perl_newLOOPEX(pTHX_ I32 type, OP *label)
{
    OP *o = NULL;

    PERL_ARGS_ASSERT_NEWLOOPEX;

    assert((PL_opargs[type] & OA_CLASS_MASK) == OA_LOOPEXOP
        || type == OP_CUSTOM);

    if (type != OP_GOTO) {
        /* "last()" means "last" */
        if (label->op_type == OP_STUB && (label->op_flags & OPf_PARENS)) {
            o = newOP(type, OPf_SPECIAL);
        }
    }
    else {
        /* Check whether it's going to be a goto &function */
        if (label->op_type == OP_ENTERSUB
                && !(label->op_flags & OPf_STACKED))
            label = newUNOP(OP_REFGEN, 0, op_lvalue(label, OP_REFGEN));
    }

    /* Check for a constant argument */
    if (label->op_type == OP_CONST) {
            SV * const sv = cSVOPx(label)->op_sv;
            STRLEN l;
            const char *s = SvPV_const(sv,l);
            if (l == strlen(s)) {
                o = newPVOP(type,
                            SvUTF8(cSVOPx(label)->op_sv),
                            savesharedpv(
                                SvPV_nolen_const(cSVOPx(label)->op_sv)));
            }
    }

    /* If we have already created an op, we do not need the label. */
    if (o)
                op_free(label);
    else o = newUNOP(type, OPf_STACKED, label);

    PL_hints |= HINT_BLOCK_SCOPE;
    return o;
}

/* if the condition is a literal array or hash
   (or @{ ... } etc), make a reference to it.
 */
STATIC OP *
S_ref_array_or_hash(pTHX_ OP *cond)
{
    if (cond
    && (cond->op_type == OP_RV2AV
    ||  cond->op_type == OP_PADAV
    ||  cond->op_type == OP_RV2HV
    ||  cond->op_type == OP_PADHV))

        return newUNOP(OP_REFGEN, 0, op_lvalue(cond, OP_REFGEN));

    else if(cond
    && (cond->op_type == OP_ASLICE
    ||  cond->op_type == OP_KVASLICE
    ||  cond->op_type == OP_HSLICE
    ||  cond->op_type == OP_KVHSLICE)) {

        /* anonlist now needs a list from this op, was previously used in
         * scalar context */
        cond->op_flags &= ~(OPf_WANT_SCALAR | OPf_REF);
        cond->op_flags |= OPf_WANT_LIST;

        return newANONLIST(op_lvalue(cond, OP_ANONLIST));
    }

    else
        return cond;
}

/* These construct the optree fragments representing given()
   and when() blocks.

   entergiven and enterwhen are LOGOPs; the op_other pointer
   points up to the associated leave op. We need this so we
   can put it in the context and make break/continue work.
   (Also, of course, pp_enterwhen will jump straight to
   op_other if the match fails.)
 */

STATIC OP *
S_newGIVWHENOP(pTHX_ OP *cond, OP *block,
                   I32 enter_opcode, I32 leave_opcode,
                   PADOFFSET entertarg)
{
    LOGOP *enterop;
    OP *o;

    PERL_ARGS_ASSERT_NEWGIVWHENOP;
    PERL_UNUSED_ARG(entertarg); /* used to indicate targ of lexical $_ */

    enterop = alloc_LOGOP(enter_opcode, block, NULL);
    enterop->op_targ = 0;
    enterop->op_private = 0;

    o = newUNOP(leave_opcode, 0, (OP *) enterop);

    if (cond) {
        /* prepend cond if we have one */
        op_sibling_splice((OP*)enterop, NULL, 0, scalar(cond));

        o->op_next = LINKLIST(cond);
        cond->op_next = (OP *) enterop;
    }
    else {
        /* This is a default {} block */
        enterop->op_flags |= OPf_SPECIAL;
        o      ->op_flags |= OPf_SPECIAL;

        o->op_next = (OP *) enterop;
    }

    CHECKOP(enter_opcode, enterop); /* Currently does nothing, since
                                       entergiven and enterwhen both
                                       use ck_null() */

    enterop->op_next = LINKLIST(block);
    block->op_next = enterop->op_other = o;

    return o;
}


/* For the purposes of 'when(implied_smartmatch)'
 *              versus 'when(boolean_expression)',
 * does this look like a boolean operation? For these purposes
   a boolean operation is:
     - a subroutine call [*]
     - a logical connective
     - a comparison operator
     - a filetest operator, with the exception of -s -M -A -C
     - defined(), exists() or eof()
     - /$re/ or $foo =~ /$re/

   [*] possibly surprising
 */
STATIC bool
S_looks_like_bool(pTHX_ const OP *o)
{
    PERL_ARGS_ASSERT_LOOKS_LIKE_BOOL;

    switch(o->op_type) {
        case OP_OR:
        case OP_DOR:
            return looks_like_bool(cLOGOPo->op_first);

        case OP_AND:
        {
            OP* sibl = OpSIBLING(cLOGOPo->op_first);
            ASSUME(sibl);
            return (
                looks_like_bool(cLOGOPo->op_first)
             && looks_like_bool(sibl));
        }

        case OP_NULL:
        case OP_SCALAR:
            return (
                o->op_flags & OPf_KIDS
            && looks_like_bool(cUNOPo->op_first));

        case OP_ENTERSUB:

        case OP_NOT:	case OP_XOR:

        case OP_EQ:	case OP_NE:	case OP_LT:
        case OP_GT:	case OP_LE:	case OP_GE:

        case OP_I_EQ:	case OP_I_NE:	case OP_I_LT:
        case OP_I_GT:	case OP_I_LE:	case OP_I_GE:

        case OP_SEQ:	case OP_SNE:	case OP_SLT:
        case OP_SGT:	case OP_SLE:	case OP_SGE:

        case OP_SMARTMATCH:

        case OP_FTRREAD:  case OP_FTRWRITE: case OP_FTREXEC:
        case OP_FTEREAD:  case OP_FTEWRITE: case OP_FTEEXEC:
        case OP_FTIS:     case OP_FTEOWNED: case OP_FTROWNED:
        case OP_FTZERO:   case OP_FTSOCK:   case OP_FTCHR:
        case OP_FTBLK:    case OP_FTFILE:   case OP_FTDIR:
        case OP_FTPIPE:   case OP_FTLINK:   case OP_FTSUID:
        case OP_FTSGID:   case OP_FTSVTX:   case OP_FTTTY:
        case OP_FTTEXT:   case OP_FTBINARY:

        case OP_DEFINED: case OP_EXISTS:
        case OP_MATCH:	 case OP_EOF:

        case OP_FLOP:

            return TRUE;

        case OP_INDEX:
        case OP_RINDEX:
            /* optimised-away (index() != -1) or similar comparison */
            if (o->op_private & OPpTRUEBOOL)
                return TRUE;
            return FALSE;

        case OP_CONST:
            /* Detect comparisons that have been optimized away */
            if (cSVOPo->op_sv == &PL_sv_yes
            ||  cSVOPo->op_sv == &PL_sv_no)

                return TRUE;
            else
                return FALSE;
        /* FALLTHROUGH */
        default:
            return FALSE;
    }
}


/*
=for apidoc newGIVENOP

Constructs, checks, and returns an op tree expressing a C<given> block.
C<cond> supplies the expression to whose value C<$_> will be locally
aliased, and C<block> supplies the body of the C<given> construct; they
are consumed by this function and become part of the constructed op tree.
C<defsv_off> must be zero (it used to identity the pad slot of lexical $_).

=cut
*/

OP *
Perl_newGIVENOP(pTHX_ OP *cond, OP *block, PADOFFSET defsv_off)
{
    PERL_ARGS_ASSERT_NEWGIVENOP;
    PERL_UNUSED_ARG(defsv_off);

    assert(!defsv_off);
    return newGIVWHENOP(
        ref_array_or_hash(cond),
        block,
        OP_ENTERGIVEN, OP_LEAVEGIVEN,
        0);
}

/*
=for apidoc newWHENOP

Constructs, checks, and returns an op tree expressing a C<when> block.
C<cond> supplies the test expression, and C<block> supplies the block
that will be executed if the test evaluates to true; they are consumed
by this function and become part of the constructed op tree.  C<cond>
will be interpreted DWIMically, often as a comparison against C<$_>,
and may be null to generate a C<default> block.

=cut
*/

OP *
Perl_newWHENOP(pTHX_ OP *cond, OP *block)
{
    const bool cond_llb = (!cond || looks_like_bool(cond));
    OP *cond_op;

    PERL_ARGS_ASSERT_NEWWHENOP;

    if (cond_llb)
        cond_op = cond;
    else {
        cond_op = newBINOP(OP_SMARTMATCH, OPf_SPECIAL,
                newDEFSVOP(),
                scalar(ref_array_or_hash(cond)));
    }

    return newGIVWHENOP(cond_op, block, OP_ENTERWHEN, OP_LEAVEWHEN, 0);
}

/*
=for apidoc newDEFEROP

Constructs and returns a deferred-block statement that implements the
C<defer> semantics.  The C<block> optree is consumed by this function and
becomes part of the returned optree.

The C<flags> argument carries additional flags to set on the returned op,
including the C<op_private> field.

=cut
 */

OP *
Perl_newDEFEROP(pTHX_ I32 flags, OP *block)
{
    OP *o, *start, *blockfirst;

    PERL_ARGS_ASSERT_NEWDEFEROP;

    forbid_outofblock_ops(block,
        (flags & (OPpDEFER_FINALLY << 8)) ? "a \"finally\" block" : "a \"defer\" block");

    start = LINKLIST(block);

    /* Hide the block inside an OP_NULL with no execution */
    block = newUNOP(OP_NULL, 0, block);
    block->op_next = block;

    o = (OP *)alloc_LOGOP(OP_PUSHDEFER, block, start);
    o->op_flags |= OPf_WANT_VOID | (U8)(flags);
    o->op_private = (U8)(flags >> 8);

    /* Terminate the block */
    blockfirst = cUNOPx(block)->op_first;
    assert(blockfirst->op_type == OP_SCOPE || blockfirst->op_type == OP_LEAVE);
    blockfirst->op_next = NULL;

    return o;
}

/*
=for apidoc op_wrap_finally

Wraps the given C<block> optree fragment in its own scoped block, arranging
for the C<finally> optree fragment to be invoked when leaving that block for
any reason. Both optree fragments are consumed and the combined result is
returned.

=cut
*/

OP *
Perl_op_wrap_finally(pTHX_ OP *block, OP *finally)
{
    PERL_ARGS_ASSERT_OP_WRAP_FINALLY;

    /* TODO: If block is already an ENTER/LEAVE-wrapped line sequence we can
     * just splice the DEFEROP in at the top, for efficiency.
     */

    OP *o = newLISTOP(OP_LINESEQ, 0, newDEFEROP((OPpDEFER_FINALLY << 8), finally), block);
    o = op_prepend_elem(OP_LINESEQ, newOP(OP_ENTER, 0), o);
    OpTYPE_set(o, OP_LEAVE);

    return o;
}

/* must not conflict with SVf_UTF8 */
#define CV_CKPROTO_CURSTASH	0x1

void
Perl_cv_ckproto_len_flags(pTHX_ const CV *cv, const GV *gv, const char *p,
                    const STRLEN len, const U32 flags)
{
    SV *name = NULL, *msg;
    const char * cvp = SvROK(cv)
                        ? SvTYPE(SvRV_const(cv)) == SVt_PVCV
                           ? (cv = (const CV *)SvRV_const(cv), CvPROTO(cv))
                           : ""
                        : CvPROTO(cv);
    STRLEN clen = CvPROTOLEN(cv), plen = len;

    PERL_ARGS_ASSERT_CV_CKPROTO_LEN_FLAGS;

    if (p == NULL && cvp == NULL)
        return;

    if (!ckWARN_d(WARN_PROTOTYPE))
        return;

    if (p && cvp) {
        p = S_strip_spaces(aTHX_ p, &plen);
        cvp = S_strip_spaces(aTHX_ cvp, &clen);
        if ((flags & SVf_UTF8) == SvUTF8(cv)) {
            if (plen == clen && memEQ(cvp, p, plen))
                return;
        } else {
            if (flags & SVf_UTF8) {
                if (bytes_cmp_utf8((const U8 *)cvp, clen, (const U8 *)p, plen) == 0)
                    return;
            }
            else {
                if (bytes_cmp_utf8((const U8 *)p, plen, (const U8 *)cvp, clen) == 0)
                    return;
            }
        }
    }

    msg = sv_newmortal();

    if (gv)
    {
        if (isGV(gv))
            gv_efullname3(name = sv_newmortal(), gv, NULL);
        else if (SvPOK(gv) && *SvPVX((SV *)gv) == '&')
            name = newSVpvn_flags(SvPVX((SV *)gv)+1, SvCUR(gv)-1, SvUTF8(gv)|SVs_TEMP);
        else if (flags & CV_CKPROTO_CURSTASH || SvROK(gv)) {
            name = newSVhek_mortal(HvNAME_HEK(PL_curstash));
            sv_catpvs(name, "::");
            if (SvROK(gv)) {
                assert (SvTYPE(SvRV_const(gv)) == SVt_PVCV);
                assert (CvNAMED(SvRV_const(gv)));
                sv_cathek(name, CvNAME_HEK(MUTABLE_CV(SvRV_const(gv))));
            }
            else sv_catsv(name, (SV *)gv);
        }
        else name = (SV *)gv;
    }
    sv_setpvs(msg, "Prototype mismatch:");
    if (name)
        Perl_sv_catpvf(aTHX_ msg, " sub %" SVf, SVfARG(name));
    if (cvp)
        Perl_sv_catpvf(aTHX_ msg, " (%" UTF8f ")",
            UTF8fARG(SvUTF8(cv),clen,cvp)
        );
    else
        sv_catpvs(msg, ": none");
    sv_catpvs(msg, " vs ");
    if (p)
        Perl_sv_catpvf(aTHX_ msg, "(%" UTF8f ")", UTF8fARG(flags & SVf_UTF8,len,p));
    else
        sv_catpvs(msg, "none");
    Perl_warner(aTHX_ packWARN(WARN_PROTOTYPE), "%" SVf, SVfARG(msg));
}

static void const_sv_xsub(pTHX_ CV* cv);
static void const_av_xsub(pTHX_ CV* cv);

/*

=for apidoc_section $optree_manipulation

=for apidoc cv_const_sv

If C<cv> is a constant sub eligible for inlining, returns the constant
value returned by the sub.  Otherwise, returns C<NULL>.

Constant subs can be created with C<newCONSTSUB> or as described in
L<perlsub/"Constant Functions">.

=cut
*/
SV *
Perl_cv_const_sv(const CV *const cv)
{
    SV *sv;
    if (!cv)
        return NULL;
    if (!(SvTYPE(cv) == SVt_PVCV || SvTYPE(cv) == SVt_PVFM))
        return NULL;
    sv = CvCONST(cv) ? MUTABLE_SV(CvXSUBANY(cv).any_ptr) : NULL;
    if (sv && SvTYPE(sv) == SVt_PVAV) return NULL;
    return sv;
}

SV *
Perl_cv_const_sv_or_av(const CV * const cv)
{
    if (!cv)
        return NULL;
    if (SvROK(cv)) return SvRV((SV *)cv);
    assert (SvTYPE(cv) == SVt_PVCV || SvTYPE(cv) == SVt_PVFM);
    return CvCONST(cv) ? MUTABLE_SV(CvXSUBANY(cv).any_ptr) : NULL;
}

/* op_const_sv:  examine an optree to determine whether it's in-lineable.
 * Can be called in 2 ways:
 *
 * !allow_lex
 * 	look for a single OP_CONST with attached value: return the value
 *
 * allow_lex && !CvCONST(cv);
 *
 * 	examine the clone prototype, and if contains only a single
 * 	OP_CONST, return the value; or if it contains a single PADSV ref-
 * 	erencing an outer lexical, turn on CvCONST to indicate the CV is
 * 	a candidate for "constizing" at clone time, and return NULL.
 */

static SV *
S_op_const_sv(pTHX_ const OP *o, CV *cv, bool allow_lex)
{
    SV *sv = NULL;
    bool padsv = FALSE;

    assert(o);
    assert(cv);

    for (; o; o = o->op_next) {
        const OPCODE type = o->op_type;

        if (type == OP_NEXTSTATE || type == OP_LINESEQ
             || type == OP_NULL
             || type == OP_PUSHMARK)
                continue;
        if (type == OP_DBSTATE)
                continue;
        if (type == OP_LEAVESUB)
            break;
        if (sv)
            return NULL;
        if (type == OP_CONST && cSVOPo->op_sv)
            sv = cSVOPo->op_sv;
        else if (type == OP_UNDEF && !o->op_private) {
            sv = newSV_type(SVt_NULL);
            SAVEFREESV(sv);
        }
        else if (allow_lex && type == OP_PADSV) {
                if (PAD_COMPNAME_FLAGS(o->op_targ) & PADNAMEf_OUTER)
                {
                    sv = &PL_sv_undef; /* an arbitrary non-null value */
                    padsv = TRUE;
                }
                else
                    return NULL;
        }
        else {
            return NULL;
        }
    }
    if (padsv) {
        CvCONST_on(cv);
        return NULL;
    }
    return sv;
}

static void
S_already_defined(pTHX_ CV *const cv, OP * const block, OP * const o,
                        PADNAME * const name, SV ** const const_svp)
{
    assert (cv);
    assert (o || name);
    assert (const_svp);
    if (!block) {
        if (CvFLAGS(PL_compcv)) {
            /* might have had built-in attrs applied */
            const bool pureperl = !CvISXSUB(cv) && CvROOT(cv);
            if (CvLVALUE(PL_compcv) && ! CvLVALUE(cv) && pureperl
             && ckWARN(WARN_MISC))
            {
                /* protect against fatal warnings leaking compcv */
                SAVEFREESV(PL_compcv);
                Perl_warner(aTHX_ packWARN(WARN_MISC), "lvalue attribute ignored after the subroutine has been defined");
                SvREFCNT_inc_simple_void_NN(PL_compcv);
            }
            CvFLAGS(cv) |=
                (CvFLAGS(PL_compcv) & CVf_BUILTIN_ATTRS
                  & ~(CVf_LVALUE * pureperl));
        }
        return;
    }

    /* redundant check for speed: */
    if (CvCONST(cv) || ckWARN(WARN_REDEFINE)) {
        const line_t oldline = CopLINE(PL_curcop);
        SV *namesv = o
            ? cSVOPo->op_sv
            : newSVpvn_flags( PadnamePV(name)+1,PadnameLEN(name)-1,
               (PadnameUTF8(name)) ? SVf_UTF8|SVs_TEMP : SVs_TEMP
              );
        if (PL_parser && PL_parser->copline != NOLINE)
            /* This ensures that warnings are reported at the first
               line of a redefinition, not the last.  */
            CopLINE_set(PL_curcop, PL_parser->copline);
        /* protect against fatal warnings leaking compcv */
        SAVEFREESV(PL_compcv);
        report_redefined_cv(namesv, cv, const_svp);
        SvREFCNT_inc_simple_void_NN(PL_compcv);
        CopLINE_set(PL_curcop, oldline);
    }
    SAVEFREESV(cv);
    return;
}

CV *
Perl_newMYSUB(pTHX_ I32 floor, OP *o, OP *proto, OP *attrs, OP *block)
{
    CV **spot;
    SV **svspot;
    const char *ps;
    STRLEN ps_len = 0; /* init it to avoid false uninit warning from icc */
    U32 ps_utf8 = 0;
    CV *cv = NULL;
    CV *compcv = PL_compcv;
    SV *const_sv;
    PADNAME *name;
    PADOFFSET pax = o->op_targ;
    CV *outcv = CvOUTSIDE(PL_compcv);
    CV *clonee = NULL;
    HEK *hek = NULL;
    bool reusable = FALSE;
    OP *start = NULL;
#ifdef PERL_DEBUG_READONLY_OPS
    OPSLAB *slab = NULL;
#endif

    PERL_ARGS_ASSERT_NEWMYSUB;

    PL_hints |= HINT_BLOCK_SCOPE;

    /* Find the pad slot for storing the new sub.
       We cannot use PL_comppad, as it is the pad owned by the new sub.  We
       need to look in CvOUTSIDE and find the pad belonging to the enclos-
       ing sub.  And then we need to dig deeper if this is a lexical from
       outside, as in:
           my sub foo; sub { sub foo { } }
     */
  redo:
    name = PadlistNAMESARRAY(CvPADLIST(outcv))[pax];
    if (PadnameOUTER(name) && PARENT_PAD_INDEX(name)) {
        pax = PARENT_PAD_INDEX(name);
        outcv = CvOUTSIDE(outcv);
        assert(outcv);
        goto redo;
    }
    svspot =
        &PadARRAY(PadlistARRAY(CvPADLIST(outcv))
                        [CvDEPTH(outcv) ? CvDEPTH(outcv) : 1])[pax];
    spot = (CV **)svspot;

    if (!(PL_parser && PL_parser->error_count))
        move_proto_attr(&proto, &attrs, (GV *)PadnameSV(name), 0);

    if (proto) {
        assert(proto->op_type == OP_CONST);
        ps = SvPV_const(cSVOPx(proto)->op_sv, ps_len);
        ps_utf8 = SvUTF8(cSVOPx(proto)->op_sv);
    }
    else
        ps = NULL;

    if (proto)
        SAVEFREEOP(proto);
    if (attrs)
        SAVEFREEOP(attrs);

    if (PL_parser && PL_parser->error_count) {
        op_free(block);
        SvREFCNT_dec(PL_compcv);
        PL_compcv = 0;
        goto done;
    }

    if (CvDEPTH(outcv) && CvCLONE(compcv)) {
        cv = *spot;
        svspot = (SV **)(spot = &clonee);
    }
    else if (PadnameIsSTATE(name) || CvDEPTH(outcv))
        cv = *spot;
    else {
        assert (SvTYPE(*spot) == SVt_PVCV);
        if (CvNAMED(*spot))
            hek = CvNAME_HEK(*spot);
        else {
            U32 hash;
            PERL_HASH(hash, PadnamePV(name)+1, PadnameLEN(name)-1);
            CvNAME_HEK_set(*spot, hek =
                share_hek(
                    PadnamePV(name)+1,
                    (PadnameLEN(name)-1) * (PadnameUTF8(name) ? -1 : 1),
                    hash
                )
            );
            CvLEXICAL_on(*spot);
        }
        cv = PadnamePROTOCV(name);
        svspot = (SV **)(spot = &PadnamePROTOCV(name));
    }

    if (block) {
        /* This makes sub {}; work as expected.  */
        if (block->op_type == OP_STUB) {
            const line_t l = PL_parser->copline;
            op_free(block);
            block = newSTATEOP(0, NULL, 0);
            PL_parser->copline = l;
        }
        block = CvLVALUE(compcv)
             || (cv && CvLVALUE(cv) && !CvROOT(cv) && !CvXSUB(cv))
                   ? newUNOP(OP_LEAVESUBLV, 0,
                             op_lvalue(voidnonfinal(block), OP_LEAVESUBLV))
                   : newUNOP(OP_LEAVESUB, 0, voidnonfinal(block));
        start = LINKLIST(block);
        block->op_next = 0;
        if (ps && !*ps && !attrs && !CvLVALUE(compcv))
            const_sv = S_op_const_sv(aTHX_ start, compcv, FALSE);
        else
            const_sv = NULL;
    }
    else
        const_sv = NULL;

    if (cv) {
        const bool exists = CvROOT(cv) || CvXSUB(cv);

        /* if the subroutine doesn't exist and wasn't pre-declared
         * with a prototype, assume it will be AUTOLOADed,
         * skipping the prototype check
         */
        if (exists || SvPOK(cv))
            cv_ckproto_len_flags(cv, (GV *)PadnameSV(name), ps, ps_len,
                                 ps_utf8);
        /* already defined? */
        if (exists) {
            S_already_defined(aTHX_ cv, block, NULL, name, &const_sv);
            if (block)
                cv = NULL;
            else {
                if (attrs)
                    goto attrs;
                /* just a "sub foo;" when &foo is already defined */
                SAVEFREESV(compcv);
                goto done;
            }
        }
        else if (CvDEPTH(outcv) && CvCLONE(compcv)) {
            cv = NULL;
            reusable = TRUE;
        }
    }

    if (const_sv) {
        SvREFCNT_inc_simple_void_NN(const_sv);
        SvFLAGS(const_sv) |= SVs_PADTMP;
        if (cv) {
            assert(!CvROOT(cv) && !CvCONST(cv));
            cv_forget_slab(cv);
        }
        else {
            cv = MUTABLE_CV(newSV_type(SVt_PVCV));
            CvFILE_set_from_cop(cv, PL_curcop);
            CvSTASH_set(cv, PL_curstash);
            *spot = cv;
        }
        SvPVCLEAR(MUTABLE_SV(cv));  /* prototype is "" */
        CvXSUBANY(cv).any_ptr = const_sv;
        CvXSUB(cv) = const_sv_xsub;
        CvCONST_on(cv);
        CvISXSUB_on(cv);
        PoisonPADLIST(cv);
        CvFLAGS(cv) |= CvNOWARN_AMBIGUOUS(compcv);
        op_free(block);
        SvREFCNT_dec(compcv);
        PL_compcv = NULL;
        goto setname;
    }

    /* Checking whether outcv is CvOUTSIDE(compcv) is not sufficient to
       determine whether this sub definition is in the same scope as its
       declaration.  If this sub definition is inside an inner named pack-
       age sub (my sub foo; sub bar { sub foo { ... } }), outcv points to
       the package sub.  So check PadnameOUTER(name) too.
     */
    if (outcv == CvOUTSIDE(compcv) && !PadnameOUTER(name)) {
        assert(!CvWEAKOUTSIDE(compcv));
        SvREFCNT_dec(CvOUTSIDE(compcv));
        CvWEAKOUTSIDE_on(compcv);
    }
    /* XXX else do we have a circular reference? */

    if (cv) {	/* must reuse cv in case stub is referenced elsewhere */
        /* transfer PL_compcv to cv */
        if (block) {
            bool free_file = CvFILE(cv) && CvDYNFILE(cv);
            cv_flags_t preserved_flags =
                CvFLAGS(cv) & (CVf_BUILTIN_ATTRS|CVf_NAMED);
            PADLIST *const temp_padl = CvPADLIST(cv);
            CV *const temp_cv = CvOUTSIDE(cv);
            const cv_flags_t other_flags =
                CvFLAGS(cv) & (CVf_SLABBED|CVf_WEAKOUTSIDE);
            OP * const cvstart = CvSTART(cv);

            SvPOK_off(cv);
            CvFLAGS(cv) =
                CvFLAGS(compcv) | preserved_flags;
            CvOUTSIDE(cv) = CvOUTSIDE(compcv);
            CvOUTSIDE_SEQ(cv) = CvOUTSIDE_SEQ(compcv);
            CvPADLIST_set(cv, CvPADLIST(compcv));
            CvOUTSIDE(compcv) = temp_cv;
            CvPADLIST_set(compcv, temp_padl);
            CvSTART(cv) = CvSTART(compcv);
            CvSTART(compcv) = cvstart;
            CvFLAGS(compcv) &= ~(CVf_SLABBED|CVf_WEAKOUTSIDE);
            CvFLAGS(compcv) |= other_flags;

            if (free_file) {
                Safefree(CvFILE(cv));
                CvFILE(cv) = NULL;
            }

            /* inner references to compcv must be fixed up ... */
            pad_fixup_inner_anons(CvPADLIST(cv), compcv, cv);
            if (PERLDB_INTER)/* Advice debugger on the new sub. */
                ++PL_sub_generation;
        }
        else {
            /* Might have had built-in attributes applied -- propagate them. */
            CvFLAGS(cv) |= (CvFLAGS(compcv) & CVf_BUILTIN_ATTRS);
        }
        /* ... before we throw it away */
        SvREFCNT_dec(compcv);
        PL_compcv = compcv = cv;
    }
    else {
        cv = compcv;
        *spot = cv;
    }

  setname:
    CvLEXICAL_on(cv);
    if (!CvNAME_HEK(cv)) {
        if (hek) (void)share_hek_hek(hek);
        else {
            U32 hash;
            PERL_HASH(hash, PadnamePV(name)+1, PadnameLEN(name)-1);
            hek = share_hek(PadnamePV(name)+1,
                      (PadnameLEN(name)-1) * (PadnameUTF8(name) ? -1 : 1),
                      hash);
        }
        CvNAME_HEK_set(cv, hek);
    }

    if (const_sv)
        goto clone;

    if (CvFILE(cv) && CvDYNFILE(cv))
        Safefree(CvFILE(cv));
    CvFILE_set_from_cop(cv, PL_curcop);
    CvSTASH_set(cv, PL_curstash);

    if (ps) {
        sv_setpvn(MUTABLE_SV(cv), ps, ps_len);
        if (ps_utf8)
            SvUTF8_on(MUTABLE_SV(cv));
    }

    if (block) {
        /* If we assign an optree to a PVCV, then we've defined a
         * subroutine that the debugger could be able to set a breakpoint
         * in, so signal to pp_entereval that it should not throw away any
         * saved lines at scope exit.  */

        PL_breakable_sub_gen++;
        CvROOT(cv) = block;
        /* The cv no longer needs to hold a refcount on the slab, as CvROOT
           itself has a refcount. */
        CvSLABBED_off(cv);
        OpslabREFCNT_dec_padok((OPSLAB *)CvSTART(cv));
#ifdef PERL_DEBUG_READONLY_OPS
        slab = (OPSLAB *)CvSTART(cv);
#endif
        S_process_optree(aTHX_ cv, block, start);
    }

  attrs:
    if (attrs) {
        /* Need to do a C<use attributes $stash_of_cv,\&cv,@attrs>. */
        apply_attrs(PL_curstash, MUTABLE_SV(cv), attrs);
    }

    if (block) {
        if (PERLDB_SUBLINE && PL_curstash != PL_debstash) {
            SV * const tmpstr = sv_newmortal();
            GV * const db_postponed = gv_fetchpvs("DB::postponed",
                                                  GV_ADDMULTI, SVt_PVHV);
            HV *hv;
            SV * const sv = Perl_newSVpvf(aTHX_ "%s:%" LINE_Tf "-%" LINE_Tf,
                                          CopFILE(PL_curcop),
                                          (line_t)PL_subline,
                                          CopLINE(PL_curcop));
            if (HvNAME_HEK(PL_curstash)) {
                sv_sethek(tmpstr, HvNAME_HEK(PL_curstash));
                sv_catpvs(tmpstr, "::");
            }
            else
                sv_setpvs(tmpstr, "__ANON__::");

            sv_catpvn_flags(tmpstr, PadnamePV(name)+1, PadnameLEN(name)-1,
                            PadnameUTF8(name) ? SV_CATUTF8 : SV_CATBYTES);
            (void)hv_store_ent(GvHV(PL_DBsub), tmpstr, sv, 0);
            hv = GvHVn(db_postponed);
            if (HvTOTALKEYS(hv) > 0 && hv_exists_ent(hv, tmpstr, 0)) {
                CV * const pcv = GvCV(db_postponed);
                if (pcv) {
                    dSP;
                    PUSHMARK(SP);
                    XPUSHs(tmpstr);
                    PUTBACK;
                    call_sv(MUTABLE_SV(pcv), G_DISCARD);
                }
            }
        }
    }

  clone:
    if (clonee) {
        assert(CvDEPTH(outcv));
        spot = (CV **)
            &PadARRAY(PadlistARRAY(CvPADLIST(outcv))[CvDEPTH(outcv)])[pax];
        if (reusable)
            cv_clone_into(clonee, *spot);
        else *spot = cv_clone(clonee);
        SvREFCNT_dec_NN(clonee);
        cv = *spot;
    }

    if (CvDEPTH(outcv) && !reusable && PadnameIsSTATE(name)) {
        PADOFFSET depth = CvDEPTH(outcv);
        while (--depth) {
            SV *oldcv;
            svspot = &PadARRAY(PadlistARRAY(CvPADLIST(outcv))[depth])[pax];
            oldcv = *svspot;
            *svspot = SvREFCNT_inc_simple_NN(cv);
            SvREFCNT_dec(oldcv);
        }
    }

  done:
    if (PL_parser)
        PL_parser->copline = NOLINE;
    LEAVE_SCOPE(floor);
#ifdef PERL_DEBUG_READONLY_OPS
    if (slab)
        Slab_to_ro(slab);
#endif
    op_free(o);
    return cv;
}

/*
=for apidoc newATTRSUB_x

Construct a Perl subroutine, also performing some surrounding jobs.

This function is expected to be called in a Perl compilation context,
and some aspects of the subroutine are taken from global variables
associated with compilation.  In particular, C<PL_compcv> represents
the subroutine that is currently being compiled.  It must be non-null
when this function is called, and some aspects of the subroutine being
constructed are taken from it.  The constructed subroutine may actually
be a reuse of the C<PL_compcv> object, but will not necessarily be so.

If C<block> is null then the subroutine will have no body, and for the
time being it will be an error to call it.  This represents a forward
subroutine declaration such as S<C<sub foo ($$);>>.  If C<block> is
non-null then it provides the Perl code of the subroutine body, which
will be executed when the subroutine is called.  This body includes
any argument unwrapping code resulting from a subroutine signature or
similar.  The pad use of the code must correspond to the pad attached
to C<PL_compcv>.  The code is not expected to include a C<leavesub> or
C<leavesublv> op; this function will add such an op.  C<block> is consumed
by this function and will become part of the constructed subroutine.

C<proto> specifies the subroutine's prototype, unless one is supplied
as an attribute (see below).  If C<proto> is null, then the subroutine
will not have a prototype.  If C<proto> is non-null, it must point to a
C<const> op whose value is a string, and the subroutine will have that
string as its prototype.  If a prototype is supplied as an attribute, the
attribute takes precedence over C<proto>, but in that case C<proto> should
preferably be null.  In any case, C<proto> is consumed by this function.

C<attrs> supplies attributes to be applied the subroutine.  A handful of
attributes take effect by built-in means, being applied to C<PL_compcv>
immediately when seen.  Other attributes are collected up and attached
to the subroutine by this route.  C<attrs> may be null to supply no
attributes, or point to a C<const> op for a single attribute, or point
to a C<list> op whose children apart from the C<pushmark> are C<const>
ops for one or more attributes.  Each C<const> op must be a string,
giving the attribute name optionally followed by parenthesised arguments,
in the manner in which attributes appear in Perl source.  The attributes
will be applied to the sub by this function.  C<attrs> is consumed by
this function.

If C<o_is_gv> is false and C<o> is null, then the subroutine will
be anonymous.  If C<o_is_gv> is false and C<o> is non-null, then C<o>
must point to a C<const> OP, which will be consumed by this function,
and its string value supplies a name for the subroutine.  The name may
be qualified or unqualified, and if it is unqualified then a default
stash will be selected in some manner.  If C<o_is_gv> is true, then C<o>
doesn't point to an C<OP> at all, but is instead a cast pointer to a C<GV>
by which the subroutine will be named.

If there is already a subroutine of the specified name, then the new
sub will either replace the existing one in the glob or be merged with
the existing one.  A warning may be generated about redefinition.

If the subroutine has one of a few special names, such as C<BEGIN> or
C<END>, then it will be claimed by the appropriate queue for automatic
running of phase-related subroutines.  In this case the relevant glob will
be left not containing any subroutine, even if it did contain one before.
In the case of C<BEGIN>, the subroutine will be executed and the reference
to it disposed of before this function returns.

The function returns a pointer to the constructed subroutine.  If the sub
is anonymous then ownership of one counted reference to the subroutine
is transferred to the caller.  If the sub is named then the caller does
not get ownership of a reference.  In most such cases, where the sub
has a non-phase name, the sub will be alive at the point it is returned
by virtue of being contained in the glob that names it.  A phase-named
subroutine will usually be alive by virtue of the reference owned by the
phase's automatic run queue.  But a C<BEGIN> subroutine, having already
been executed, will quite likely have been destroyed already by the
time this function returns, making it erroneous for the caller to make
any use of the returned pointer.  It is the caller's responsibility to
ensure that it knows which of these situations applies.

=for apidoc newATTRSUB
Construct a Perl subroutine, also performing some surrounding jobs.

This is the same as L<perlintern/C<newATTRSUB_x>> with its C<o_is_gv> parameter set to
FALSE.  This means that if C<o> is null, the new sub will be anonymous; otherwise
the name will be derived from C<o> in the way described (as with all other
details) in L<perlintern/C<newATTRSUB_x>>.

=for apidoc newSUB
Like C<L</newATTRSUB>>, but without attributes.

=cut
*/

/* _x = extended */
CV *
Perl_newATTRSUB_x(pTHX_ I32 floor, OP *o, OP *proto, OP *attrs,
                            OP *block, bool o_is_gv)
{
    GV *gv;
    const char *ps;
    STRLEN ps_len = 0; /* init it to avoid false uninit warning from icc */
    U32 ps_utf8 = 0;
    CV *cv = NULL;     /* the previous CV with this name, if any */
    SV *const_sv;
    const bool ec = PL_parser && PL_parser->error_count;
    /* If the subroutine has no body, no attributes, and no builtin attributes
       then it's just a sub declaration, and we may be able to get away with
       storing with a placeholder scalar in the symbol table, rather than a
       full CV.  If anything is present then it will take a full CV to
       store it.  */
    const I32 gv_fetch_flags
        = ec ? GV_NOADD_NOINIT :
        (block || attrs || (CvFLAGS(PL_compcv) & CVf_BUILTIN_ATTRS))
        ? GV_ADDMULTI : GV_ADDMULTI | GV_NOINIT;
    STRLEN namlen = 0;
    const char * const name =
         o ? SvPV_const(o_is_gv ? (SV *)o : cSVOPo->op_sv, namlen) : NULL;
    bool has_name;
    bool name_is_utf8 = o && !o_is_gv && SvUTF8(cSVOPo->op_sv);
    bool evanescent = FALSE;
    bool isBEGIN = FALSE;
    OP *start = NULL;
#ifdef PERL_DEBUG_READONLY_OPS
    OPSLAB *slab = NULL;
#endif

    if (o_is_gv) {
        gv = (GV*)o;
        o = NULL;
        has_name = TRUE;
    } else if (name) {
        /* Try to optimise and avoid creating a GV.  Instead, the CV’s name
           hek and CvSTASH pointer together can imply the GV.  If the name
           contains a package name, then GvSTASH(CvGV(cv)) may differ from
           CvSTASH, so forego the optimisation if we find any.
           Also, we may be called from load_module at run time, so
           PL_curstash (which sets CvSTASH) may not point to the stash the
           sub is stored in.  */
        /* XXX This optimization is currently disabled for packages other
               than main, since there was too much CPAN breakage.  */
        const I32 flags =
           ec ? GV_NOADD_NOINIT
              :   (IN_PERL_RUNTIME && PL_curstash != CopSTASH(PL_curcop))
               || PL_curstash != PL_defstash
               || memchr(name, ':', namlen) || memchr(name, '\'', namlen)
                    ? gv_fetch_flags
                    : GV_ADDMULTI | GV_NOINIT | GV_NOTQUAL;
        gv = gv_fetchsv(cSVOPo->op_sv, flags, SVt_PVCV);
        has_name = TRUE;
    } else if (PERLDB_NAMEANON && CopLINE(PL_curcop)) {
        SV * const sv = sv_newmortal();
        Perl_sv_setpvf(aTHX_ sv, "%s[%s:%" LINE_Tf "]",
                       PL_curstash ? "__ANON__" : "__ANON__::__ANON__",
                       CopFILE(PL_curcop), CopLINE(PL_curcop));
        gv = gv_fetchsv(sv, gv_fetch_flags, SVt_PVCV);
        has_name = TRUE;
    } else if (PL_curstash) {
        gv = gv_fetchpvs("__ANON__", gv_fetch_flags, SVt_PVCV);
        has_name = FALSE;
    } else {
        gv = gv_fetchpvs("__ANON__::__ANON__", gv_fetch_flags, SVt_PVCV);
        has_name = FALSE;
    }

    if (!ec) {
        if (isGV(gv)) {
            move_proto_attr(&proto, &attrs, gv, 0);
        } else {
            assert(cSVOPo);
            move_proto_attr(&proto, &attrs, (GV *)cSVOPo->op_sv, 1);
        }
    }

    if (o)
        SAVEFREEOP(o);
    if (proto)
        SAVEFREEOP(proto);
    if (attrs)
        SAVEFREEOP(attrs);

    /* we need this in two places later on, so set it up here */
    if (name && block) {
        const char *s = (char *) my_memrchr(name, ':', namlen);
        s = s ? s+1 : name;
        isBEGIN = strEQ(s,"BEGIN");
    }

    if (isBEGIN) {
        /* Make sure that we do not have any prototypes or
         * attributes associated with this BEGIN block, as the block
         * is already done and dusted, and we will assert or worse
         * if we try to attach the prototype to the now essentially
         * nonexistent sub. */
        if (proto)
            /* diag_listed_as: %s on BEGIN block ignored */
            Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "Prototype on BEGIN block ignored");
        if (attrs)
            /* diag_listed_as: %s on BEGIN block ignored */
            Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "Attribute on BEGIN block ignored");
        proto = NULL;
        attrs = NULL;
    }

    if (proto) {
        assert(proto->op_type == OP_CONST);
        ps = SvPV_const(cSVOPx(proto)->op_sv, ps_len);
        ps_utf8 = SvUTF8(cSVOPx(proto)->op_sv);
    }
    else
        ps = NULL;

    if (ec) {
        op_free(block);

        if (name)
            SvREFCNT_dec(PL_compcv);
        else
            cv = PL_compcv;

        PL_compcv = 0;
        if (isBEGIN) {
            if (PL_in_eval & EVAL_KEEPERR)
                Perl_croak_nocontext("BEGIN not safe after errors--compilation aborted");
            else {
                SV * const errsv = ERRSV;
                /* force display of errors found but not reported */
                sv_catpvs(errsv, "BEGIN not safe after errors--compilation aborted");
                Perl_croak_nocontext("%" SVf, SVfARG(errsv));
            }
        }
        goto done;
    }

    if (!block && SvTYPE(gv) != SVt_PVGV) {
        /* If we are not defining a new sub and the existing one is not a
           full GV + CV... */
        if (attrs || (CvFLAGS(PL_compcv) & CVf_BUILTIN_ATTRS)) {
            /* We are applying attributes to an existing sub, so we need it
               upgraded if it is a constant.  */
            if (SvROK(gv) && SvTYPE(SvRV(gv)) != SVt_PVCV)
                gv_init_pvn(gv, PL_curstash, name, namlen,
                            SVf_UTF8 * name_is_utf8);
        }
        else {			/* Maybe prototype now, and had at maximum
                                   a prototype or const/sub ref before.  */
            if (SvTYPE(gv) > SVt_NULL) {
                cv_ckproto_len_flags((const CV *)gv,
                                    o ? (const GV *)cSVOPo->op_sv : NULL, ps,
                                    ps_len, ps_utf8);
            }

            if (!SvROK(gv)) {
                if (ps) {
                    sv_setpvn(MUTABLE_SV(gv), ps, ps_len);
                    if (ps_utf8)
                        SvUTF8_on(MUTABLE_SV(gv));
                }
                else
                    sv_setiv(MUTABLE_SV(gv), -1);
            }

            SvREFCNT_dec(PL_compcv);
            cv = PL_compcv = NULL;
            goto done;
        }
    }

    cv = (!name || (isGV(gv) && GvCVGEN(gv)))
        ? NULL
        : isGV(gv)
            ? GvCV(gv)
            : SvROK(gv) && SvTYPE(SvRV(gv)) == SVt_PVCV
                ? (CV *)SvRV(gv)
                : NULL;

    if (block) {
        assert(PL_parser);
        if (CvIsMETHOD(PL_compcv))
            block = class_wrap_method_body(block);
        /* This makes sub {}; work as expected.  */
        if (block->op_type == OP_STUB) {
            const line_t l = PL_parser->copline;
            op_free(block);
            block = newSTATEOP(0, NULL, 0);
            PL_parser->copline = l;
        }
        block = CvLVALUE(PL_compcv)
             || (cv && CvLVALUE(cv) && !CvROOT(cv) && !CvXSUB(cv)
                    && (!isGV(gv) || !GvASSUMECV(gv)))
                   ? newUNOP(OP_LEAVESUBLV, 0,
                             op_lvalue(voidnonfinal(block), OP_LEAVESUBLV))
                   : newUNOP(OP_LEAVESUB, 0, voidnonfinal(block));
        start = LINKLIST(block);
        block->op_next = 0;
        if (ps && !*ps && !attrs && !CvLVALUE(PL_compcv))
            const_sv =
                S_op_const_sv(aTHX_ start, PL_compcv,
                                        cBOOL(CvCLONE(PL_compcv)));
        else
            const_sv = NULL;
    }
    else
        const_sv = NULL;

    if (SvPOK(gv) || (SvROK(gv) && SvTYPE(SvRV(gv)) != SVt_PVCV)) {
        cv_ckproto_len_flags((const CV *)gv,
                             o ? (const GV *)cSVOPo->op_sv : NULL, ps,
                             ps_len, ps_utf8|CV_CKPROTO_CURSTASH);
        if (SvROK(gv)) {
            /* All the other code for sub redefinition warnings expects the
               clobbered sub to be a CV.  Instead of making all those code
               paths more complex, just inline the RV version here.  */
            const line_t oldline = CopLINE(PL_curcop);
            assert(IN_PERL_COMPILETIME);
            if (PL_parser && PL_parser->copline != NOLINE)
                /* This ensures that warnings are reported at the first
                   line of a redefinition, not the last.  */
                CopLINE_set(PL_curcop, PL_parser->copline);
            /* protect against fatal warnings leaking compcv */
            SAVEFREESV(PL_compcv);

            if (ckWARN(WARN_REDEFINE)
             || (  ckWARN_d(WARN_REDEFINE)
                && (  !const_sv || SvRV(gv) == const_sv
                      || SvTYPE(const_sv) == SVt_PVAV
                      || SvTYPE(SvRV(gv)) == SVt_PVAV
                      || sv_cmp(SvRV(gv), const_sv)  ))) {
                assert(cSVOPo);
                Perl_warner(aTHX_ packWARN(WARN_REDEFINE),
                          "Constant subroutine %" SVf " redefined",
                          SVfARG(cSVOPo->op_sv));
            }

            SvREFCNT_inc_simple_void_NN(PL_compcv);
            CopLINE_set(PL_curcop, oldline);
            SvREFCNT_dec(SvRV(gv));
        }
    }

    if (cv) {
        const bool exists = CvROOT(cv) || CvXSUB(cv);

        /* if the subroutine doesn't exist and wasn't pre-declared
         * with a prototype, assume it will be AUTOLOADed,
         * skipping the prototype check
         */
        if (exists || SvPOK(cv))
            cv_ckproto_len_flags(cv, gv, ps, ps_len, ps_utf8);
        /* already defined (or promised)? */
        if (exists || (isGV(gv) && GvASSUMECV(gv))) {
            S_already_defined(aTHX_ cv, block, o, NULL, &const_sv);
            if (block)
                cv = NULL;
            else {
                if (attrs)
                    goto attrs;
                /* just a "sub foo;" when &foo is already defined */
                SAVEFREESV(PL_compcv);
                goto done;
            }
        }
    }

    if (const_sv) {
        SvREFCNT_inc_simple_void_NN(const_sv);
        SvFLAGS(const_sv) |= SVs_PADTMP;
        if (cv) {
            assert(!CvROOT(cv) && !CvCONST(cv));
            cv_forget_slab(cv);
            SvPVCLEAR(MUTABLE_SV(cv));  /* prototype is "" */
            CvXSUBANY(cv).any_ptr = const_sv;
            CvXSUB(cv) = const_sv_xsub;
            CvCONST_on(cv);
            CvISXSUB_on(cv);
            PoisonPADLIST(cv);
            CvFLAGS(cv) |= CvNOWARN_AMBIGUOUS(PL_compcv);
        }
        else {
            if (isGV(gv) || CvNOWARN_AMBIGUOUS(PL_compcv)) {
                if (name && isGV(gv))
                    GvCV_set(gv, NULL);
                cv = newCONSTSUB_flags(
                    NULL, name, namlen, name_is_utf8 ? SVf_UTF8 : 0,
                    const_sv
                );
                assert(cv);
                assert(SvREFCNT((SV*)cv) != 0);
                CvFLAGS(cv) |= CvNOWARN_AMBIGUOUS(PL_compcv);
            }
            else {
                if (!SvROK(gv)) {
                    SV_CHECK_THINKFIRST_COW_DROP((SV *)gv);
                    prepare_SV_for_RV((SV *)gv);
                    SvOK_off((SV *)gv);
                    SvROK_on(gv);
                }
                SvRV_set(gv, const_sv);
            }
        }
        op_free(block);
        SvREFCNT_dec(PL_compcv);
        PL_compcv = NULL;
        goto done;
    }

    /* don't copy new BEGIN CV to old BEGIN CV - RT #129099 */
    if (name && cv && *name == 'B' && strEQ(name, "BEGIN"))
        cv = NULL;

    if (cv) {				/* must reuse cv if autoloaded */
        /* transfer PL_compcv to cv */
        if (block) {
            bool free_file = CvFILE(cv) && CvDYNFILE(cv);
            cv_flags_t existing_builtin_attrs = CvFLAGS(cv) & CVf_BUILTIN_ATTRS;
            PADLIST *const temp_av = CvPADLIST(cv);
            CV *const temp_cv = CvOUTSIDE(cv);
            const cv_flags_t other_flags =
                CvFLAGS(cv) & (CVf_SLABBED|CVf_WEAKOUTSIDE);
            OP * const cvstart = CvSTART(cv);

            if (isGV(gv)) {
                CvGV_set(cv,gv);
                assert(!CvCVGV_RC(cv));
                assert(CvGV(cv) == gv);
            }
            else {
                U32 hash;
                PERL_HASH(hash, name, namlen);
                CvNAME_HEK_set(cv,
                               share_hek(name,
                                         name_is_utf8
                                            ? -(SSize_t)namlen
                                            :  (SSize_t)namlen,
                                         hash));
            }

            SvPOK_off(cv);
            CvFLAGS(cv) = CvFLAGS(PL_compcv) | existing_builtin_attrs
                                             | CvNAMED(cv);
            CvOUTSIDE(cv) = CvOUTSIDE(PL_compcv);
            CvOUTSIDE_SEQ(cv) = CvOUTSIDE_SEQ(PL_compcv);
            CvPADLIST_set(cv,CvPADLIST(PL_compcv));
            CvOUTSIDE(PL_compcv) = temp_cv;
            CvPADLIST_set(PL_compcv, temp_av);
            CvSTART(cv) = CvSTART(PL_compcv);
            CvSTART(PL_compcv) = cvstart;
            CvFLAGS(PL_compcv) &= ~(CVf_SLABBED|CVf_WEAKOUTSIDE);
            CvFLAGS(PL_compcv) |= other_flags;

            if (free_file) {
                Safefree(CvFILE(cv));
            }
            CvFILE_set_from_cop(cv, PL_curcop);
            CvSTASH_set(cv, PL_curstash);

            /* inner references to PL_compcv must be fixed up ... */
            pad_fixup_inner_anons(CvPADLIST(cv), PL_compcv, cv);
            if (PERLDB_INTER)/* Advice debugger on the new sub. */
                ++PL_sub_generation;
        }
        else {
            /* Might have had built-in attributes applied -- propagate them. */
            CvFLAGS(cv) |= (CvFLAGS(PL_compcv) & CVf_BUILTIN_ATTRS);
        }
        /* ... before we throw it away */
        SvREFCNT_dec(PL_compcv);
        PL_compcv = cv;
    }
    else {
        cv = PL_compcv;
        if (name && isGV(gv)) {
            GvCV_set(gv, cv);
            GvCVGEN(gv) = 0;
            if (HvENAME_HEK(GvSTASH(gv)))
                /* sub Foo::bar { (shift)+1 } */
                gv_method_changed(gv);
        }
        else if (name) {
            if (!SvROK(gv)) {
                SV_CHECK_THINKFIRST_COW_DROP((SV *)gv);
                prepare_SV_for_RV((SV *)gv);
                SvOK_off((SV *)gv);
                SvROK_on(gv);
            }
            SvRV_set(gv, (SV *)cv);
            if (HvENAME_HEK(PL_curstash))
                mro_method_changed_in(PL_curstash);
        }
    }
    assert(cv);
    assert(SvREFCNT((SV*)cv) != 0);

    if (!CvHASGV(cv)) {
        if (isGV(gv))
            CvGV_set(cv, gv);
        else {
            U32 hash;
            PERL_HASH(hash, name, namlen);
            CvNAME_HEK_set(cv, share_hek(name,
                                         name_is_utf8
                                            ? -(SSize_t)namlen
                                            :  (SSize_t)namlen,
                                         hash));
        }
        CvFILE_set_from_cop(cv, PL_curcop);
        CvSTASH_set(cv, PL_curstash);
    }

    if (ps) {
        sv_setpvn(MUTABLE_SV(cv), ps, ps_len);
        if ( ps_utf8 )
            SvUTF8_on(MUTABLE_SV(cv));
    }

    if (block) {
        /* If we assign an optree to a PVCV, then we've defined a
         * subroutine that the debugger could be able to set a breakpoint
         * in, so signal to pp_entereval that it should not throw away any
         * saved lines at scope exit.  */

        PL_breakable_sub_gen++;
        CvROOT(cv) = block;
        /* The cv no longer needs to hold a refcount on the slab, as CvROOT
           itself has a refcount. */
        CvSLABBED_off(cv);
        OpslabREFCNT_dec_padok((OPSLAB *)CvSTART(cv));
#ifdef PERL_DEBUG_READONLY_OPS
        slab = (OPSLAB *)CvSTART(cv);
#endif
        S_process_optree(aTHX_ cv, block, start);
    }

  attrs:
    if (attrs) {
        /* Need to do a C<use attributes $stash_of_cv,\&cv,@attrs>. */
        HV *stash = name && !CvNAMED(cv) && GvSTASH(CvGV(cv))
                        ? GvSTASH(CvGV(cv))
                        : PL_curstash;
        if (!name)
            SAVEFREESV(cv);
        apply_attrs(stash, MUTABLE_SV(cv), attrs);
        if (!name)
            SvREFCNT_inc_simple_void_NN(cv);
    }

    if (block && has_name) {
        if (PERLDB_SUBLINE && PL_curstash != PL_debstash) {
            SV * const tmpstr = cv_name(cv,NULL,0);
            GV * const db_postponed = gv_fetchpvs("DB::postponed",
                                                  GV_ADDMULTI, SVt_PVHV);
            HV *hv;
            SV * const sv = Perl_newSVpvf(aTHX_ "%s:%" LINE_Tf "-%" LINE_Tf,
                                          CopFILE(PL_curcop),
                                          (line_t)PL_subline,
                                          CopLINE(PL_curcop));
            (void)hv_store_ent(GvHV(PL_DBsub), tmpstr, sv, 0);
            hv = GvHVn(db_postponed);
            if (HvTOTALKEYS(hv) > 0 && hv_exists_ent(hv, tmpstr, 0)) {
                CV * const pcv = GvCV(db_postponed);
                if (pcv) {
                    dSP;
                    PUSHMARK(SP);
                    XPUSHs(tmpstr);
                    PUTBACK;
                    call_sv(MUTABLE_SV(pcv), G_DISCARD);
                }
            }
        }

        if (name) {
            if (PL_parser && PL_parser->error_count)
                clear_special_blocks(name, gv, cv);
            else
                evanescent =
                    process_special_blocks(floor, name, gv, cv);
        }
    }
    assert(cv);

  done:
    assert(!cv || evanescent || SvREFCNT((SV*)cv) != 0);
    if (PL_parser)
        PL_parser->copline = NOLINE;
    LEAVE_SCOPE(floor);

    assert(!cv || evanescent || SvREFCNT((SV*)cv) != 0);
    if (!evanescent) {
#ifdef PERL_DEBUG_READONLY_OPS
    if (slab)
        Slab_to_ro(slab);
#endif
    if (cv && name && block && CvOUTSIDE(cv) && !CvEVAL(CvOUTSIDE(cv)))
        pad_add_weakref(cv);
    }
    return cv;
}

STATIC void
S_clear_special_blocks(pTHX_ const char *const fullname,
                       GV *const gv, CV *const cv) {
    const char *colon;
    const char *name;

    PERL_ARGS_ASSERT_CLEAR_SPECIAL_BLOCKS;

    colon = strrchr(fullname,':');
    name = colon ? colon + 1 : fullname;

    if ((*name == 'B' && strEQ(name, "BEGIN"))
        || (*name == 'E' && strEQ(name, "END"))
        || (*name == 'U' && strEQ(name, "UNITCHECK"))
        || (*name == 'C' && strEQ(name, "CHECK"))
        || (*name == 'I' && strEQ(name, "INIT"))) {
        if (!isGV(gv)) {
            (void)CvGV(cv);
            assert(isGV(gv));
        }
        GvCV_set(gv, NULL);
        SvREFCNT_dec_NN(MUTABLE_SV(cv));
    }
}

/* Returns true if the sub has been freed.  */
STATIC bool
S_process_special_blocks(pTHX_ I32 floor, const char *const fullname,
                         GV *const gv,
                         CV *const cv)
{
    const char *const colon = strrchr(fullname,':');
    const char *const name = colon ? colon + 1 : fullname;

    PERL_ARGS_ASSERT_PROCESS_SPECIAL_BLOCKS;

    if (*name == 'B') {
        if (strEQ(name, "BEGIN")) {
            /* can't goto a declaration, but a null statement is fine */
            module_install_hack: ;
            const I32 oldscope = PL_scopestack_ix;
            SV *max_nest_sv = NULL;
            IV max_nest_iv;
            dSP;
            (void)CvGV(cv);
            if (floor) LEAVE_SCOPE(floor);
            ENTER;

            /* Make sure we don't recurse too deeply into BEGIN blocks,
             * but let the user control it via the new control variable
             *
             *   ${^MAX_NESTED_EVAL_BEGIN_BLOCKS}
             *
             * Note that this code (when max_nest_iv is 1) *looks* like
             * it would block the following code:
             *
             * BEGIN { $n |= 1; BEGIN { $n |= 2; BEGIN { $n |= 4 } } }
             *
             * but it does *not*; this code will happily execute when
             * the nest limit is 1. The reason is revealed in the
             * execution order. If we could watch $n in this code, we
             * would see the following order of modifications:
             *
             * $n |= 4;
             * $n |= 2;
             * $n |= 1;
             *
             * This is because nested BEGIN blocks execute in FILO
             * order; this is because BEGIN blocks are defined to
             * execute immediately once they are closed. So the
             * innermost block is closed first, and it executes, which
             * increments the eval_begin_nest_depth by 1, and then it
             * finishes, which drops eval_begin_nest_depth back to its
             * previous value. This happens in turn as each BEGIN is
             * completed.
             *
             * The *only* place these counts matter is when BEGIN is
             * inside of some kind of string eval, either a require or a
             * true eval. Only in that case would there be any nesting
             * and would perl try to execute a BEGIN before another had
             * completed.
             *
             * Thus this logic puts an upper limit on module nesting.
             * Hence the reason we let the user control it, although it
             * is hard to imagine a 1000-level-deep module use
             * dependency even in a very large codebase. The real
             * objective is to prevent code like this:
             *
             * perl -e'sub f { eval "BEGIN { f() }" } f()'
             *
             * from segfaulting due to stack exhaustion.
             *
             */
            max_nest_sv = get_sv(PERL_VAR_MAX_NESTED_EVAL_BEGIN_BLOCKS, GV_ADD);
            if (!SvOK(max_nest_sv))
                sv_setiv(max_nest_sv, PERL_MAX_NESTED_EVAL_BEGIN_BLOCKS_DEFAULT);
            max_nest_iv = SvIV(max_nest_sv);
            if (max_nest_iv < 0) {
                max_nest_iv = PERL_MAX_NESTED_EVAL_BEGIN_BLOCKS_DEFAULT;
                sv_setiv(max_nest_sv, max_nest_iv);
            }

            /* (UV) below is just to silence a compiler warning, and should be
             * effectively a no-op, as max_nest_iv will never be negative here.
             */
            if (PL_eval_begin_nest_depth >= (UV)max_nest_iv) {
                Perl_croak(aTHX_ "Too many nested BEGIN blocks, maximum of %" IVdf " allowed",
                             max_nest_iv);
            }
            SAVEINT(PL_eval_begin_nest_depth);
            PL_eval_begin_nest_depth++;

            SAVEVPTR(PL_curcop);
            if (PL_curcop == &PL_compiling) {
                /* Avoid pushing the "global" &PL_compiling onto the
                 * context stack. For example, a stack trace inside
                 * nested use's would show all calls coming from whoever
                 * most recently updated PL_compiling.cop_file and
                 * cop_line.  So instead, temporarily set PL_curcop to a
                 * private copy of &PL_compiling. PL_curcop will soon be
                 * set to point back to &PL_compiling anyway but only
                 * after the temp value has been pushed onto the context
                 * stack as blk_oldcop.
                 * This is slightly hacky, but necessary. Note also
                 * that in the brief window before PL_curcop is set back
                 * to PL_compiling, IN_PERL_COMPILETIME/IN_PERL_RUNTIME
                 * will give the wrong answer.
                 */
                PL_curcop = (COP*)newSTATEOP(PL_compiling.op_flags, NULL, NULL);
                CopLINE_set(PL_curcop, CopLINE(&PL_compiling));
                SAVEFREEOP(PL_curcop);
            }

            PUSHSTACKi(PERLSI_REQUIRE);
            SAVECOPFILE(&PL_compiling);
            SAVECOPLINE(&PL_compiling);

            DEBUG_x( dump_sub(gv) );
            Perl_av_create_and_push(aTHX_ &PL_beginav, MUTABLE_SV(cv));
            GvCV_set(gv,0);		/* cv has been hijacked */
            call_list(oldscope, PL_beginav);

            POPSTACK;
            LEAVE;
            return !PL_savebegin;
        }
        else
            return FALSE;
    } else {
        if (*name == 'E') {
            if (strEQ(name, "END")) {
                DEBUG_x( dump_sub(gv) );
                Perl_av_create_and_unshift_one(aTHX_ &PL_endav, MUTABLE_SV(cv));
            } else
                return FALSE;
        } else if (*name == 'U') {
            if (strEQ(name, "UNITCHECK")) {
                /* It's never too late to run a unitcheck block */
                Perl_av_create_and_unshift_one(aTHX_ &PL_unitcheckav, MUTABLE_SV(cv));
            }
            else
                return FALSE;
        } else if (*name == 'C') {
            if (strEQ(name, "CHECK")) {
                if (PL_main_start)
                    /* diag_listed_as: Too late to run %s block */
                    Perl_ck_warner(aTHX_ packWARN(WARN_VOID),
                                   "Too late to run CHECK block");
                Perl_av_create_and_unshift_one(aTHX_ &PL_checkav, MUTABLE_SV(cv));
            }
            else
                return FALSE;
        } else if (*name == 'I') {
            if (strEQ(name, "INIT")) {
#ifdef MI_INIT_WORKAROUND_PACK
                {
                    HV *hv = CvSTASH(cv);
                    STRLEN len = hv ? HvNAMELEN(hv) : 0;
                    char *pv = (len == sizeof(MI_INIT_WORKAROUND_PACK)-1)
                            ? HvNAME_get(hv) : NULL;
                    if ( pv && strEQ(pv, MI_INIT_WORKAROUND_PACK) ) {
                        /* old versions of Module::Install::DSL contain code
                         * that creates an INIT in eval, which expects to run
                         * after an exit(0) in BEGIN. This unfortunately
                         * breaks a lot of code in the CPAN river. So we magically
                         * convert INIT blocks from Module::Install::DSL to
                         * be BEGIN blocks. Which works out, since the INIT
                         * blocks it creates are eval'ed and so are late.
                         */
                        Perl_warn(aTHX_ "Treating %s::INIT block as BEGIN block as workaround",
                                MI_INIT_WORKAROUND_PACK);
                        goto module_install_hack;
                    }

                }
#endif
                if (PL_main_start)
                    /* diag_listed_as: Too late to run %s block */
                    Perl_ck_warner(aTHX_ packWARN(WARN_VOID),
                                   "Too late to run INIT block");
                Perl_av_create_and_push(aTHX_ &PL_initav, MUTABLE_SV(cv));
            }
            else
                return FALSE;
        } else
            return FALSE;
        DEBUG_x( dump_sub(gv) );
        (void)CvGV(cv);
        GvCV_set(gv,0);		/* cv has been hijacked */
        return FALSE;
    }
}

/*
=for apidoc newCONSTSUB

Behaves like L</newCONSTSUB_flags>, except that C<name> is nul-terminated
rather than of counted length, and no flags are set.  (This means that
C<name> is always interpreted as Latin-1.)

=cut
*/

CV *
Perl_newCONSTSUB(pTHX_ HV *stash, const char *name, SV *sv)
{
    return newCONSTSUB_flags(stash, name, name ? strlen(name) : 0, 0, sv);
}

/*
=for apidoc newCONSTSUB_flags

Construct a constant subroutine, also performing some surrounding
jobs.  A scalar constant-valued subroutine is eligible for inlining
at compile-time, and in Perl code can be created by S<C<sub FOO () {
123 }>>.  Other kinds of constant subroutine have other treatment.

The subroutine will have an empty prototype and will ignore any arguments
when called.  Its constant behaviour is determined by C<sv>.  If C<sv>
is null, the subroutine will yield an empty list.  If C<sv> points to a
scalar, the subroutine will always yield that scalar.  If C<sv> points
to an array, the subroutine will always yield a list of the elements of
that array in list context, or the number of elements in the array in
scalar context.  This function takes ownership of one counted reference
to the scalar or array, and will arrange for the object to live as long
as the subroutine does.  If C<sv> points to a scalar then the inlining
assumes that the value of the scalar will never change, so the caller
must ensure that the scalar is not subsequently written to.  If C<sv>
points to an array then no such assumption is made, so it is ostensibly
safe to mutate the array or its elements, but whether this is really
supported has not been determined.

The subroutine will have C<CvFILE> set according to C<PL_curcop>.
Other aspects of the subroutine will be left in their default state.
The caller is free to mutate the subroutine beyond its initial state
after this function has returned.

If C<name> is null then the subroutine will be anonymous, with its
C<CvGV> referring to an C<__ANON__> glob.  If C<name> is non-null then the
subroutine will be named accordingly, referenced by the appropriate glob.
C<name> is a string of length C<len> bytes giving a sigilless symbol
name, in UTF-8 if C<flags> has the C<SVf_UTF8> bit set and in Latin-1
otherwise.  The name may be either qualified or unqualified.  If the
name is unqualified then it defaults to being in the stash specified by
C<stash> if that is non-null, or to C<PL_curstash> if C<stash> is null.
The symbol is always added to the stash if necessary, with C<GV_ADDMULTI>
semantics.

C<flags> should not have bits set other than C<SVf_UTF8>.

If there is already a subroutine of the specified name, then the new sub
will replace the existing one in the glob.  A warning may be generated
about the redefinition.

If the subroutine has one of a few special names, such as C<BEGIN> or
C<END>, then it will be claimed by the appropriate queue for automatic
running of phase-related subroutines.  In this case the relevant glob will
be left not containing any subroutine, even if it did contain one before.
Execution of the subroutine will likely be a no-op, unless C<sv> was
a tied array or the caller modified the subroutine in some interesting
way before it was executed.  In the case of C<BEGIN>, the treatment is
buggy: the sub will be executed when only half built, and may be deleted
prematurely, possibly causing a crash.

The function returns a pointer to the constructed subroutine.  If the sub
is anonymous then ownership of one counted reference to the subroutine
is transferred to the caller.  If the sub is named then the caller does
not get ownership of a reference.  In most such cases, where the sub
has a non-phase name, the sub will be alive at the point it is returned
by virtue of being contained in the glob that names it.  A phase-named
subroutine will usually be alive by virtue of the reference owned by
the phase's automatic run queue.  A C<BEGIN> subroutine may have been
destroyed already by the time this function returns, but currently bugs
occur in that case before the caller gets control.  It is the caller's
responsibility to ensure that it knows which of these situations applies.

=cut
*/

CV *
Perl_newCONSTSUB_flags(pTHX_ HV *stash, const char *name, STRLEN len,
                             U32 flags, SV *sv)
{
    CV* cv;
    const char *const file = CopFILE(PL_curcop);

    ENTER;

    if (IN_PERL_RUNTIME) {
        /* at runtime, it's not safe to manipulate PL_curcop: it may be
         * an op shared between threads. Use a non-shared COP for our
         * dirty work */
         SAVEVPTR(PL_curcop);
         SAVECOMPILEWARNINGS();
         PL_compiling.cop_warnings = DUP_WARNINGS(PL_curcop->cop_warnings);
         PL_curcop = &PL_compiling;
    }
    SAVECOPLINE(PL_curcop);
    CopLINE_set(PL_curcop, PL_parser ? PL_parser->copline : NOLINE);

    SAVEHINTS();
    PL_hints &= ~HINT_BLOCK_SCOPE;

    if (stash) {
        SAVEGENERICSV(PL_curstash);
        PL_curstash = (HV *)SvREFCNT_inc_simple_NN(stash);
    }

    /* Protect sv against leakage caused by fatal warnings. */
    if (sv) SAVEFREESV(sv);

    /* file becomes the CvFILE. For an XS, it's usually static storage,
       and so doesn't get free()d.  (It's expected to be from the C pre-
       processor __FILE__ directive). But we need a dynamically allocated one,
       and we need it to get freed.  */
    cv = newXS_len_flags(name, len,
                         sv && SvTYPE(sv) == SVt_PVAV
                             ? const_av_xsub
                             : const_sv_xsub,
                         file ? file : "", "",
                         &sv, XS_DYNAMIC_FILENAME | flags);
    assert(cv);
    assert(SvREFCNT((SV*)cv) != 0);
    CvXSUBANY(cv).any_ptr = SvREFCNT_inc_simple(sv);
    CvCONST_on(cv);

    LEAVE;

    return cv;
}

/*
=for apidoc newXS

Used by C<xsubpp> to hook up XSUBs as Perl subs.  C<filename> needs to be
static storage, as it is used directly as CvFILE(), without a copy being made.

=cut
*/

CV *
Perl_newXS(pTHX_ const char *name, XSUBADDR_t subaddr, const char *filename)
{
    PERL_ARGS_ASSERT_NEWXS;
    return newXS_len_flags(
        name, name ? strlen(name) : 0, subaddr, filename, NULL, NULL, 0
    );
}

CV *
Perl_newXS_flags(pTHX_ const char *name, XSUBADDR_t subaddr,
                 const char *const filename, const char *const proto,
                 U32 flags)
{
    PERL_ARGS_ASSERT_NEWXS_FLAGS;
    return newXS_len_flags(
       name, name ? strlen(name) : 0, subaddr, filename, proto, NULL, flags
    );
}

CV *
Perl_newXS_deffile(pTHX_ const char *name, XSUBADDR_t subaddr)
{
    PERL_ARGS_ASSERT_NEWXS_DEFFILE;
    return newXS_len_flags(
        name, strlen(name), subaddr, NULL, NULL, NULL, 0
    );
}

/*
=for apidoc newXS_len_flags

Construct an XS subroutine, also performing some surrounding jobs.

The subroutine will have the entry point C<subaddr>.  It will have
the prototype specified by the nul-terminated string C<proto>, or
no prototype if C<proto> is null.  The prototype string is copied;
the caller can mutate the supplied string afterwards.  If C<filename>
is non-null, it must be a nul-terminated filename, and the subroutine
will have its C<CvFILE> set accordingly.  By default C<CvFILE> is set to
point directly to the supplied string, which must be static.  If C<flags>
has the C<XS_DYNAMIC_FILENAME> bit set, then a copy of the string will
be taken instead.

Other aspects of the subroutine will be left in their default state.
If anything else needs to be done to the subroutine for it to function
correctly, it is the caller's responsibility to do that after this
function has constructed it.  However, beware of the subroutine
potentially being destroyed before this function returns, as described
below.

If C<name> is null then the subroutine will be anonymous, with its
C<CvGV> referring to an C<__ANON__> glob.  If C<name> is non-null then the
subroutine will be named accordingly, referenced by the appropriate glob.
C<name> is a string of length C<len> bytes giving a sigilless symbol name,
in UTF-8 if C<flags> has the C<SVf_UTF8> bit set and in Latin-1 otherwise.
The name may be either qualified or unqualified, with the stash defaulting
in the same manner as for C<gv_fetchpvn_flags>.  C<flags> may contain
flag bits understood by C<gv_fetchpvn_flags> with the same meaning as
they have there, such as C<GV_ADDWARN>.  The symbol is always added to
the stash if necessary, with C<GV_ADDMULTI> semantics.

If there is already a subroutine of the specified name, then the new sub
will replace the existing one in the glob.  A warning may be generated
about the redefinition.  If the old subroutine was C<CvCONST> then the
decision about whether to warn is influenced by an expectation about
whether the new subroutine will become a constant of similar value.
That expectation is determined by C<const_svp>.  (Note that the call to
this function doesn't make the new subroutine C<CvCONST> in any case;
that is left to the caller.)  If C<const_svp> is null then it indicates
that the new subroutine will not become a constant.  If C<const_svp>
is non-null then it indicates that the new subroutine will become a
constant, and it points to an C<SV*> that provides the constant value
that the subroutine will have.

If the subroutine has one of a few special names, such as C<BEGIN> or
C<END>, then it will be claimed by the appropriate queue for automatic
running of phase-related subroutines.  In this case the relevant glob will
be left not containing any subroutine, even if it did contain one before.
In the case of C<BEGIN>, the subroutine will be executed and the reference
to it disposed of before this function returns, and also before its
prototype is set.  If a C<BEGIN> subroutine would not be sufficiently
constructed by this function to be ready for execution then the caller
must prevent this happening by giving the subroutine a different name.

The function returns a pointer to the constructed subroutine.  If the sub
is anonymous then ownership of one counted reference to the subroutine
is transferred to the caller.  If the sub is named then the caller does
not get ownership of a reference.  In most such cases, where the sub
has a non-phase name, the sub will be alive at the point it is returned
by virtue of being contained in the glob that names it.  A phase-named
subroutine will usually be alive by virtue of the reference owned by the
phase's automatic run queue.  But a C<BEGIN> subroutine, having already
been executed, will quite likely have been destroyed already by the
time this function returns, making it erroneous for the caller to make
any use of the returned pointer.  It is the caller's responsibility to
ensure that it knows which of these situations applies.

=cut
*/

CV *
Perl_newXS_len_flags(pTHX_ const char *name, STRLEN len,
                           XSUBADDR_t subaddr, const char *const filename,
                           const char *const proto, SV **const_svp,
                           U32 flags)
{
    CV *cv;
    bool interleave = FALSE;
    bool evanescent = FALSE;

    PERL_ARGS_ASSERT_NEWXS_LEN_FLAGS;

    {
        GV * const gv = gv_fetchpvn(
                            name ? name : PL_curstash ? "__ANON__" : "__ANON__::__ANON__",
                            name ? len : PL_curstash ? sizeof("__ANON__") - 1:
                                sizeof("__ANON__::__ANON__") - 1,
                            GV_ADDMULTI | flags, SVt_PVCV);

        if ((cv = (name ? GvCV(gv) : NULL))) {
            if (GvCVGEN(gv)) {
                /* just a cached method */
                SvREFCNT_dec(cv);
                cv = NULL;
            }
            else if (CvROOT(cv) || CvXSUB(cv) || GvASSUMECV(gv)) {
                /* already defined (or promised) */
                /* Redundant check that allows us to avoid creating an SV
                   most of the time: */
                if (CvCONST(cv) || ckWARN(WARN_REDEFINE)) {
                    report_redefined_cv(newSVpvn_flags(
                                         name,len,(flags&SVf_UTF8)|SVs_TEMP
                                        ),
                                        cv, const_svp);
                }
                interleave = TRUE;
                ENTER;
                SAVEFREESV(cv);
                cv = NULL;
            }
        }

        if (cv)				/* must reuse cv if autoloaded */
            cv_undef(cv);
        else {
            cv = MUTABLE_CV(newSV_type(SVt_PVCV));
            if (name) {
                GvCV_set(gv,cv);
                GvCVGEN(gv) = 0;
                if (HvENAME_HEK(GvSTASH(gv)))
                    gv_method_changed(gv); /* newXS */
            }
        }
        assert(cv);
        assert(SvREFCNT((SV*)cv) != 0);

        CvGV_set(cv, gv);
        if(filename) {
            /* XSUBs can't be perl lang/perl5db.pl debugged
            if (PERLDB_LINE_OR_SAVESRC)
                (void)gv_fetchfile(filename); */
            assert(!CvDYNFILE(cv)); /* cv_undef should have turned it off */
            if (flags & XS_DYNAMIC_FILENAME) {
                CvDYNFILE_on(cv);
                CvFILE(cv) = savepv(filename);
            } else {
            /* NOTE: not copied, as it is expected to be an external constant string */
                CvFILE(cv) = (char *)filename;
            }
        } else {
            assert((flags & XS_DYNAMIC_FILENAME) == 0 && PL_xsubfilename);
            CvFILE(cv) = (char*)PL_xsubfilename;
        }
        CvISXSUB_on(cv);
        CvXSUB(cv) = subaddr;
#ifndef MULTIPLICITY
        CvHSCXT(cv) = &PL_stack_sp;
#else
        PoisonPADLIST(cv);
#endif

        if (name)
            evanescent = process_special_blocks(0, name, gv, cv);
        else
            CvANON_on(cv);
    } /* <- not a conditional branch */

    assert(cv);
    assert(evanescent || SvREFCNT((SV*)cv) != 0);

    if (!evanescent) sv_setpv(MUTABLE_SV(cv), proto);
    if (interleave) LEAVE;
    assert(evanescent || SvREFCNT((SV*)cv) != 0);
    return cv;
}

/* Add a stub CV to a typeglob.
 * This is the implementation of a forward declaration, 'sub foo';'
 */

CV *
Perl_newSTUB(pTHX_ GV *gv, bool fake)
{
    CV *cv = MUTABLE_CV(newSV_type(SVt_PVCV));
    GV *cvgv;
    PERL_ARGS_ASSERT_NEWSTUB;
    assert(!GvCVu(gv));
    GvCV_set(gv, cv);
    GvCVGEN(gv) = 0;
    if (!fake && GvSTASH(gv) && HvENAME_HEK(GvSTASH(gv)))
        gv_method_changed(gv);
    if (SvFAKE(gv)) {
        cvgv = gv_fetchsv((SV *)gv, GV_ADDMULTI, SVt_PVCV);
        SvFAKE_off(cvgv);
    }
    else cvgv = gv;
    CvGV_set(cv, cvgv);
    CvFILE_set_from_cop(cv, PL_curcop);
    CvSTASH_set(cv, PL_curstash);
    GvMULTI_on(gv);
    return cv;
}

void
Perl_newFORM(pTHX_ I32 floor, OP *o, OP *block)
{
    CV *cv;
    GV *gv;
    OP *root;
    OP *start;

    if (PL_parser && PL_parser->error_count) {
        op_free(block);
        goto finish;
    }

    gv = o
        ? gv_fetchsv(cSVOPo->op_sv, GV_ADD, SVt_PVFM)
        : gv_fetchpvs("STDOUT", GV_ADD|GV_NOTQUAL, SVt_PVFM);

    GvMULTI_on(gv);
    if ((cv = GvFORM(gv))) {
        if (ckWARN(WARN_REDEFINE)) {
            const line_t oldline = CopLINE(PL_curcop);
            if (PL_parser && PL_parser->copline != NOLINE)
                CopLINE_set(PL_curcop, PL_parser->copline);
            if (o) {
                Perl_warner(aTHX_ packWARN(WARN_REDEFINE),
                            "Format %" SVf " redefined", SVfARG(cSVOPo->op_sv));
            } else {
                /* diag_listed_as: Format %s redefined */
                Perl_warner(aTHX_ packWARN(WARN_REDEFINE),
                            "Format STDOUT redefined");
            }
            CopLINE_set(PL_curcop, oldline);
        }
        SvREFCNT_dec(cv);
    }
    cv = PL_compcv;
    GvFORM(gv) = (CV *)SvREFCNT_inc_simple_NN(cv);
    CvGV_set(cv, gv);
    CvFILE_set_from_cop(cv, PL_curcop);


    root = newUNOP(OP_LEAVEWRITE, 0, voidnonfinal(block));
    CvROOT(cv) = root;
    start = LINKLIST(root);
    root->op_next = 0;
    S_process_optree(aTHX_ cv, root, start);
    cv_forget_slab(cv);

  finish:
    op_free(o);
    if (PL_parser)
        PL_parser->copline = NOLINE;
    LEAVE_SCOPE(floor);
    PL_compiling.cop_seq = 0;
}

OP *
Perl_newANONLIST(pTHX_ OP *o)
{
    return op_convert_list(OP_ANONLIST, OPf_SPECIAL, o);
}

OP *
Perl_newANONHASH(pTHX_ OP *o)
{
    return op_convert_list(OP_ANONHASH, OPf_SPECIAL, o);
}

OP *
Perl_newANONSUB(pTHX_ I32 floor, OP *proto, OP *block)
{
    return newANONATTRSUB(floor, proto, NULL, block);
}

OP *
Perl_newANONATTRSUB(pTHX_ I32 floor, OP *proto, OP *attrs, OP *block)
{
    SV * const cv = MUTABLE_SV(newATTRSUB(floor, 0, proto, attrs, block));

    bool is_const = CvANONCONST(cv);

    OP * anoncode =
        newSVOP(OP_ANONCODE, is_const ? 0 : OPf_REF,
                cv);

    if (is_const) {
        anoncode = newUNOP(OP_ANONCONST, OPf_REF,
                           op_convert_list(OP_ENTERSUB,
                                           OPf_STACKED|OPf_WANT_SCALAR,
                                           anoncode));
    }

    return anoncode;
}

OP *
Perl_oopsAV(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_OOPSAV;

    switch (o->op_type) {
    case OP_PADSV:
    case OP_PADHV:
        OpTYPE_set(o, OP_PADAV);
        return ref(o, OP_RV2AV);

    case OP_RV2SV:
    case OP_RV2HV:
        OpTYPE_set(o, OP_RV2AV);
        ref(o, OP_RV2AV);
        break;

    default:
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL), "oops: oopsAV");
        break;
    }
    return o;
}

OP *
Perl_oopsHV(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_OOPSHV;

    switch (o->op_type) {
    case OP_PADSV:
    case OP_PADAV:
        OpTYPE_set(o, OP_PADHV);
        return ref(o, OP_RV2HV);

    case OP_RV2SV:
    case OP_RV2AV:
        OpTYPE_set(o, OP_RV2HV);
        /* rv2hv steals the bottom bit for its own uses */
        o->op_private &= ~OPpARG1_MASK;
        ref(o, OP_RV2HV);
        break;

    default:
        Perl_ck_warner_d(aTHX_ packWARN(WARN_INTERNAL), "oops: oopsHV");
        break;
    }
    return o;
}

OP *
Perl_newAVREF(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_NEWAVREF;

    if (o->op_type == OP_PADANY) {
        OpTYPE_set(o, OP_PADAV);
        return o;
    }
    else if ((o->op_type == OP_RV2AV || o->op_type == OP_PADAV)) {
        Perl_croak(aTHX_ "Can't use an array as a reference");
    }
    return newUNOP(OP_RV2AV, 0, scalar(o));
}

OP *
Perl_newGVREF(pTHX_ I32 type, OP *o)
{
    if (type == OP_MAPSTART || type == OP_GREPSTART || type == OP_SORT)
        return newUNOP(OP_NULL, 0, o);

    if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED &&
        ((PL_opargs[type] >> OASHIFT) & 7) == OA_FILEREF &&
        o->op_type == OP_CONST && (o->op_private & OPpCONST_BARE)) {
        no_bareword_filehandle(SvPVX(cSVOPo_sv));
    }

    return ref(newUNOP(OP_RV2GV, OPf_REF, o), type);
}

OP *
Perl_newHVREF(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_NEWHVREF;

    if (o->op_type == OP_PADANY) {
        OpTYPE_set(o, OP_PADHV);
        return o;
    }
    else if ((o->op_type == OP_RV2HV || o->op_type == OP_PADHV)) {
        Perl_croak(aTHX_ "Can't use a hash as a reference");
    }
    return newUNOP(OP_RV2HV, 0, scalar(o));
}

OP *
Perl_newCVREF(pTHX_ I32 flags, OP *o)
{
    if (o->op_type == OP_PADANY) {
        OpTYPE_set(o, OP_PADCV);
    }
    return newUNOP(OP_RV2CV, flags, scalar(o));
}

OP *
Perl_newSVREF(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_NEWSVREF;

    if (o->op_type == OP_PADANY) {
        OpTYPE_set(o, OP_PADSV);
        scalar(o);
        return o;
    }
    return newUNOP(OP_RV2SV, 0, scalar(o));
}

/* Check routines. See the comments at the top of this file for details
 * on when these are called */

OP *
Perl_ck_anoncode(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_ANONCODE;

    cSVOPo->op_targ = pad_add_anon((CV*)cSVOPo->op_sv, o->op_type);
    cSVOPo->op_sv = NULL;
    return o;
}

static void
S_io_hints(pTHX_ OP *o)
{
#if O_BINARY != 0 || O_TEXT != 0
    HV * const table =
        PL_hints & HINT_LOCALIZE_HH ? GvHV(PL_hintgv) : NULL;;
    if (table) {
        SV **svp = hv_fetchs(table, "open_IN", FALSE);
        if (svp && *svp) {
            STRLEN len = 0;
            const char *d = SvPV_const(*svp, len);
            const I32 mode = mode_from_discipline(d, len);
            /* bit-and:ing with zero O_BINARY or O_TEXT would be useless. */
#  if O_BINARY != 0
            if (mode & O_BINARY)
                o->op_private |= OPpOPEN_IN_RAW;
#  endif
#  if O_TEXT != 0
            if (mode & O_TEXT)
                o->op_private |= OPpOPEN_IN_CRLF;
#  endif
        }

        svp = hv_fetchs(table, "open_OUT", FALSE);
        if (svp && *svp) {
            STRLEN len = 0;
            const char *d = SvPV_const(*svp, len);
            const I32 mode = mode_from_discipline(d, len);
            /* bit-and:ing with zero O_BINARY or O_TEXT would be useless. */
#  if O_BINARY != 0
            if (mode & O_BINARY)
                o->op_private |= OPpOPEN_OUT_RAW;
#  endif
#  if O_TEXT != 0
            if (mode & O_TEXT)
                o->op_private |= OPpOPEN_OUT_CRLF;
#  endif
        }
    }
#else
    PERL_UNUSED_CONTEXT;
    PERL_UNUSED_ARG(o);
#endif
}

OP *
Perl_ck_backtick(pTHX_ OP *o)
{
    GV *gv;
    OP *newop = NULL;
    OP *sibl;
    PERL_ARGS_ASSERT_CK_BACKTICK;
    o = ck_fun(o);
    /* qx and `` have a null pushmark; CORE::readpipe has only one kid. */
    if (o->op_flags & OPf_KIDS && (sibl = OpSIBLING(cUNOPo->op_first))
     && (gv = gv_override("readpipe",8)))
    {
        /* detach rest of siblings from o and its first child */
        op_sibling_splice(o, cUNOPo->op_first, -1, NULL);
        newop = S_new_entersubop(aTHX_ gv, sibl);
    }
    else if (!(o->op_flags & OPf_KIDS))
        newop = newUNOP(OP_BACKTICK, 0,	newDEFSVOP());
    if (newop) {
        op_free(o);
        return newop;
    }
    S_io_hints(aTHX_ o);
    return o;
}

OP *
Perl_ck_bitop(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_BITOP;

    /* get rid of arg count and indicate if in the scope of 'use integer' */
    o->op_private = (PL_hints & HINT_INTEGER) ? OPpUSEINT : 0;

    if (!(o->op_flags & OPf_STACKED) /* Not an assignment */
            && OP_IS_INFIX_BIT(o->op_type))
    {
        const OP * const left = cBINOPo->op_first;
        const OP * const right = OpSIBLING(left);
        if ((OP_IS_NUMCOMPARE(left->op_type) &&
                (left->op_flags & OPf_PARENS) == 0) ||
            (OP_IS_NUMCOMPARE(right->op_type) &&
                (right->op_flags & OPf_PARENS) == 0))
            Perl_ck_warner(aTHX_ packWARN(WARN_PRECEDENCE),
                          "Possible precedence problem on bitwise %s operator",
                           o->op_type ==  OP_BIT_OR
                         ||o->op_type == OP_NBIT_OR  ? "|"
                        :  o->op_type ==  OP_BIT_AND
                         ||o->op_type == OP_NBIT_AND ? "&"
                        :  o->op_type ==  OP_BIT_XOR
                         ||o->op_type == OP_NBIT_XOR ? "^"
                        :  o->op_type == OP_SBIT_OR  ? "|."
                        :  o->op_type == OP_SBIT_AND ? "&." : "^."
                           );
    }
    return o;
}

PERL_STATIC_INLINE bool
is_dollar_bracket(pTHX_ const OP * const o)
{
    const OP *kid;
    PERL_UNUSED_CONTEXT;
    return o->op_type == OP_RV2SV && o->op_flags & OPf_KIDS
        && (kid = cUNOPx(o)->op_first)
        && kid->op_type == OP_GV
        && strEQ(GvNAME(cGVOPx_gv(kid)), "[");
}

/* for lt, gt, le, ge, eq, ne and their i_ variants */

OP *
Perl_ck_cmp(pTHX_ OP *o)
{
    bool is_eq;
    bool neg;
    bool reverse;
    bool iv0;
    OP *indexop, *constop, *start;
    SV *sv;
    IV iv;

    PERL_ARGS_ASSERT_CK_CMP;

    is_eq = (   o->op_type == OP_EQ
             || o->op_type == OP_NE
             || o->op_type == OP_I_EQ
             || o->op_type == OP_I_NE);

    if (!is_eq && ckWARN(WARN_SYNTAX)) {
        const OP *kid = cUNOPo->op_first;
        if (kid &&
            (
                (   is_dollar_bracket(aTHX_ kid)
                 && OpSIBLING(kid) && OpSIBLING(kid)->op_type == OP_CONST
                )
             || (   kid->op_type == OP_CONST
                 && (kid = OpSIBLING(kid)) && is_dollar_bracket(aTHX_ kid)
                )
           )
        )
            Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                        "$[ used in %s (did you mean $] ?)", OP_DESC(o));
    }

    /* convert (index(...) == -1) and variations into
     *   (r)index/BOOL(,NEG)
     */

    reverse = FALSE;

    indexop = cUNOPo->op_first;
    constop = OpSIBLING(indexop);
    start = NULL;
    if (indexop->op_type == OP_CONST) {
        constop = indexop;
        indexop = OpSIBLING(constop);
        start = constop;
        reverse = TRUE;
    }

    if (indexop->op_type != OP_INDEX && indexop->op_type != OP_RINDEX)
        return o;

    /* ($lex = index(....)) == -1 */
    if (indexop->op_private & OPpTARGET_MY)
        return o;

    if (constop->op_type != OP_CONST)
        return o;

    sv = cSVOPx_sv(constop);
    if (!(sv && SvIOK_notUV(sv)))
        return o;

    iv = SvIVX(sv);
    if (iv != -1 && iv != 0)
        return o;
    iv0 = (iv == 0);

    if (o->op_type == OP_LT || o->op_type == OP_I_LT) {
        if (!(iv0 ^ reverse))
            return o;
        neg = iv0;
    }
    else if (o->op_type == OP_LE || o->op_type == OP_I_LE) {
        if (iv0 ^ reverse)
            return o;
        neg = !iv0;
    }
    else if (o->op_type == OP_GE || o->op_type == OP_I_GE) {
        if (!(iv0 ^ reverse))
            return o;
        neg = !iv0;
    }
    else if (o->op_type == OP_GT || o->op_type == OP_I_GT) {
        if (iv0 ^ reverse)
            return o;
        neg = iv0;
    }
    else if (o->op_type == OP_EQ || o->op_type == OP_I_EQ) {
        if (iv0)
            return o;
        neg = TRUE;
    }
    else {
        assert(o->op_type == OP_NE || o->op_type == OP_I_NE);
        if (iv0)
            return o;
        neg = FALSE;
    }

    indexop->op_flags &= ~OPf_PARENS;
    indexop->op_flags |= (o->op_flags & OPf_PARENS);
    indexop->op_private |= OPpTRUEBOOL;
    if (neg)
        indexop->op_private |= OPpINDEX_BOOLNEG;
    /* cut out the index op and free the eq,const ops */
    (void)op_sibling_splice(o, start, 1, NULL);
    op_free(o);

    return indexop;
}


OP *
Perl_ck_concat(pTHX_ OP *o)
{
    const OP * const kid = cUNOPo->op_first;

    PERL_ARGS_ASSERT_CK_CONCAT;
    PERL_UNUSED_CONTEXT;

    /* reuse the padtmp returned by the concat child */
    if (kid->op_type == OP_CONCAT && !(kid->op_private & OPpTARGET_MY) &&
            !(kUNOP->op_first->op_flags & OPf_MOD))
    {
        o->op_flags |= OPf_STACKED;
        o->op_private |= OPpCONCAT_NESTED;
    }
    return o;
}

OP *
Perl_ck_spair(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_CK_SPAIR;

    if (o->op_flags & OPf_KIDS) {
        OP* newop;
        OP* kid;
        OP* kidkid;
        const OPCODE type = o->op_type;
        o = modkids(ck_fun(o), type);
        kid    = cUNOPo->op_first;
        kidkid = kUNOP->op_first;
        newop = OpSIBLING(kidkid);
        if (newop) {
            const OPCODE type = newop->op_type;
            if (OpHAS_SIBLING(newop))
                return o;
            if (o->op_type == OP_REFGEN
             && (  type == OP_RV2CV
                || (  !(newop->op_flags & OPf_PARENS)
                   && (  type == OP_RV2AV || type == OP_PADAV
                      || type == OP_RV2HV || type == OP_PADHV))))
                NOOP; /* OK (allow srefgen for \@a and \%h) */
            else if (OP_GIMME(newop,0) != G_SCALAR)
                return o;
        }
        /* excise first sibling */
        op_sibling_splice(kid, NULL, 1, NULL);
        op_free(kidkid);
    }
    /* transforms OP_REFGEN into OP_SREFGEN, OP_CHOP into OP_SCHOP,
     * and OP_CHOMP into OP_SCHOMP */
    o->op_ppaddr = PL_ppaddr[++o->op_type];
    return ck_fun(o);
}

OP *
Perl_ck_delete(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_DELETE;

    o = ck_fun(o);
    o->op_private = 0;
    if (o->op_flags & OPf_KIDS) {
        OP * const kid = cUNOPo->op_first;
        switch (kid->op_type) {
        case OP_ASLICE:
            o->op_flags |= OPf_SPECIAL;
            /* FALLTHROUGH */
        case OP_HSLICE:
            o->op_private |= OPpSLICE;
            break;
        case OP_AELEM:
            o->op_flags |= OPf_SPECIAL;
            /* FALLTHROUGH */
        case OP_HELEM:
            break;
        case OP_KVASLICE:
            o->op_flags |= OPf_SPECIAL;
            /* FALLTHROUGH */
        case OP_KVHSLICE:
            o->op_private |= OPpKVSLICE;
            break;
        default:
            Perl_croak(aTHX_ "delete argument is not a HASH or ARRAY "
                             "element or slice");
        }
        if (kid->op_private & OPpLVAL_INTRO)
            o->op_private |= OPpLVAL_INTRO;
        op_null(kid);
    }
    return o;
}

OP *
Perl_ck_eof(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_EOF;

    if (o->op_flags & OPf_KIDS) {
        OP *kid;
        if (cLISTOPo->op_first->op_type == OP_STUB) {
            OP * const newop
                = newUNOP(o->op_type, OPf_SPECIAL, newGVOP(OP_GV, 0, PL_argvgv));
            op_free(o);
            o = newop;
        }
        o = ck_fun(o);
        kid = cLISTOPo->op_first;
        if (kid->op_type == OP_RV2GV)
            kid->op_private |= OPpALLOW_FAKE;
    }
    return o;
}


OP *
Perl_ck_eval(pTHX_ OP *o)
{

    PERL_ARGS_ASSERT_CK_EVAL;

    PL_hints |= HINT_BLOCK_SCOPE;
    if (o->op_flags & OPf_KIDS) {
        SVOP * const kid = cSVOPx(cUNOPo->op_first);
        assert(kid);

        if (o->op_type == OP_ENTERTRY) {
            LOGOP *enter;

            /* cut whole sibling chain free from o */
            op_sibling_splice(o, NULL, -1, NULL);
            op_free(o);

            enter = alloc_LOGOP(OP_ENTERTRY, NULL, NULL);

            /* establish postfix order */
            enter->op_next = (OP*)enter;

            o = op_prepend_elem(OP_LINESEQ, (OP*)enter, (OP*)kid);
            OpTYPE_set(o, OP_LEAVETRY);
            enter->op_other = o;
            return o;
        }
        else {
            scalar((OP*)kid);
            S_set_haseval(aTHX);
        }
    }
    else {
        const U8 priv = o->op_private;
        op_free(o);
        /* the newUNOP will recursively call ck_eval(), which will handle
         * all the stuff at the end of this function, like adding
         * OP_HINTSEVAL
         */
        return newUNOP(OP_ENTEREVAL, priv <<8, newDEFSVOP());
    }
    o->op_targ = (PADOFFSET)PL_hints;
    if (o->op_private & OPpEVAL_BYTES) o->op_targ &= ~HINT_UTF8;
    if ((PL_hints & HINT_LOCALIZE_HH) != 0
     && !(o->op_private & OPpEVAL_COPHH) && GvHV(PL_hintgv)) {
        /* Store a copy of %^H that pp_entereval can pick up. */
        HV *hh = hv_copy_hints_hv(GvHV(PL_hintgv));
        OP *hhop;
        STOREFEATUREBITSHH(hh);
        hhop = newSVOP(OP_HINTSEVAL, 0, MUTABLE_SV(hh));
        /* append hhop to only child  */
        op_sibling_splice(o, cUNOPo->op_first, 0, hhop);

        o->op_private |= OPpEVAL_HAS_HH;
    }
    if (!(o->op_private & OPpEVAL_BYTES)
         && FEATURE_UNIEVAL_IS_ENABLED)
            o->op_private |= OPpEVAL_UNICODE;
    return o;
}

OP *
Perl_ck_trycatch(pTHX_ OP *o)
{
    LOGOP *enter;
    OP *to_free = NULL;
    OP *trykid, *catchkid;
    OP *catchroot, *catchstart;

    PERL_ARGS_ASSERT_CK_TRYCATCH;

    trykid = cUNOPo->op_first;
    if(trykid->op_type == OP_NULL || trykid->op_type == OP_PUSHMARK) {
        to_free = trykid;
        trykid = OpSIBLING(trykid);
    }
    catchkid = OpSIBLING(trykid);

    assert(trykid->op_type == OP_POPTRY);
    assert(catchkid->op_type == OP_CATCH);

    /* cut whole sibling chain free from o */
    op_sibling_splice(o, NULL, -1, NULL);
    if(to_free)
        op_free(to_free);
    op_free(o);

    enter = alloc_LOGOP(OP_ENTERTRYCATCH, NULL, NULL);

    /* establish postfix order */
    enter->op_next = (OP*)enter;

    o = op_prepend_elem(OP_LINESEQ, (OP*)enter, trykid);
    op_append_elem(OP_LINESEQ, (OP*)o, catchkid);

    OpTYPE_set(o, OP_LEAVETRYCATCH);

    /* The returned optree is actually threaded up slightly nonobviously in
     * terms of its ->op_next pointers.
     *
     * This way, if the tryblock dies, its retop points at the OP_CATCH, but
     * if it does not then its leavetry skips over that and continues
     * execution past it.
     */

    /* First, link up the actual body of the catch block */
    catchroot = OpSIBLING(cUNOPx(catchkid)->op_first);
    catchstart = LINKLIST(catchroot);
    cLOGOPx(catchkid)->op_other = catchstart;

    o->op_next = LINKLIST(o);

    /* die within try block should jump to the catch */
    enter->op_other = catchkid;

    /* after try block that doesn't die, just skip straight to leavetrycatch */
    trykid->op_next = o;

    /* after catch block, skip back up to the leavetrycatch */
    catchroot->op_next = o;

    return o;
}

OP *
Perl_ck_exec(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_EXEC;

    if (o->op_flags & OPf_STACKED) {
        OP *kid;
        o = ck_fun(o);
        kid = OpSIBLING(cUNOPo->op_first);
        if (kid->op_type == OP_RV2GV)
            op_null(kid);
    }
    else
        o = listkids(o);
    return o;
}

OP *
Perl_ck_exists(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_EXISTS;

    o = ck_fun(o);
    if (o->op_flags & OPf_KIDS) {
        OP * const kid = cUNOPo->op_first;
        if (kid->op_type == OP_ENTERSUB) {
            (void) ref(kid, o->op_type);
            if (kid->op_type != OP_RV2CV
                        && !(PL_parser && PL_parser->error_count))
                Perl_croak(aTHX_
                          "exists argument is not a subroutine name");
            o->op_private |= OPpEXISTS_SUB;
        }
        else if (kid->op_type == OP_AELEM)
            o->op_flags |= OPf_SPECIAL;
        else if (kid->op_type != OP_HELEM)
            Perl_croak(aTHX_ "exists argument is not a HASH or ARRAY "
                             "element or a subroutine");
        op_null(kid);
    }
    return o;
}

OP *
Perl_ck_helemexistsor(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_HELEMEXISTSOR;

    o = ck_fun(o);

    OP *first;
    if(!(o->op_flags & OPf_KIDS) ||
        !(first = cLOGOPo->op_first) ||
        first->op_type != OP_HELEM)
        /* As this opcode isn't currently exposed to pure-perl, only core or XS
         * authors are ever going to see this message. We don't need to list it
         * in perldiag as to do so would require documenting OP_HELEMEXISTSOR
         * itself
         */
        /* diag_listed_as: SKIPME */
        croak("OP_HELEMEXISTSOR argument is not a HASH element");

    OP *hvop  = cBINOPx(first)->op_first;
    OP *keyop = OpSIBLING(hvop);
    assert(!OpSIBLING(keyop));

    op_null(first); // null out the OP_HELEM

    keyop->op_next = o;

    return o;
}

OP *
Perl_ck_rvconst(pTHX_ OP *o)
{
    SVOP * const kid = cSVOPx(cUNOPo->op_first);

    PERL_ARGS_ASSERT_CK_RVCONST;

    if (o->op_type == OP_RV2HV)
        /* rv2hv steals the bottom bit for its own uses */
        o->op_private &= ~OPpARG1_MASK;

    o->op_private |= (PL_hints & HINT_STRICT_REFS);

    if (kid->op_type == OP_CONST) {
        int iscv;
        GV *gv;
        SV * const kidsv = kid->op_sv;

        /* Is it a constant from cv_const_sv()? */
        if ((SvROK(kidsv) || isGV_with_GP(kidsv)) && SvREADONLY(kidsv)) {
            return o;
        }
        if (SvTYPE(kidsv) == SVt_PVAV) return o;
        if ((o->op_private & HINT_STRICT_REFS) && (kid->op_private & OPpCONST_BARE)) {
            const char *badthing;
            switch (o->op_type) {
            case OP_RV2SV:
                badthing = "a SCALAR";
                break;
            case OP_RV2AV:
                badthing = "an ARRAY";
                break;
            case OP_RV2HV:
                badthing = "a HASH";
                break;
            default:
                badthing = NULL;
                break;
            }
            if (badthing)
                Perl_croak(aTHX_
                           "Can't use bareword (\"%" SVf "\") as %s ref while \"strict refs\" in use",
                           SVfARG(kidsv), badthing);
        }
        /*
         * This is a little tricky.  We only want to add the symbol if we
         * didn't add it in the lexer.  Otherwise we get duplicate strict
         * warnings.  But if we didn't add it in the lexer, we must at
         * least pretend like we wanted to add it even if it existed before,
         * or we get possible typo warnings.  OPpCONST_ENTERED says
         * whether the lexer already added THIS instance of this symbol.
         */
        iscv = o->op_type == OP_RV2CV ? GV_NOEXPAND|GV_ADDMULTI : 0;
        gv = gv_fetchsv(kidsv,
                o->op_type == OP_RV2CV
                        && o->op_private & OPpMAY_RETURN_CONSTANT
                    ? GV_NOEXPAND
                    : iscv | !(kid->op_private & OPpCONST_ENTERED),
                iscv
                    ? SVt_PVCV
                    : o->op_type == OP_RV2SV
                        ? SVt_PV
                        : o->op_type == OP_RV2AV
                            ? SVt_PVAV
                            : o->op_type == OP_RV2HV
                                ? SVt_PVHV
                                : SVt_PVGV);
        if (gv) {
            if (!isGV(gv)) {
                assert(iscv);
                assert(SvROK(gv));
                if (!(o->op_private & OPpMAY_RETURN_CONSTANT)
                  && SvTYPE(SvRV(gv)) != SVt_PVCV)
                    gv_fetchsv(kidsv, GV_ADDMULTI, SVt_PVCV);
            }
            OpTYPE_set(kid, OP_GV);
            SvREFCNT_dec(kid->op_sv);
#ifdef USE_ITHREADS
            /* XXX hack: dependence on sizeof(PADOP) <= sizeof(SVOP) */
            STATIC_ASSERT_STMT(sizeof(PADOP) <= sizeof(SVOP));
            kPADOP->op_padix = pad_alloc(OP_GV, SVf_READONLY);
            SvREFCNT_dec(PAD_SVl(kPADOP->op_padix));
            PAD_SETSV(kPADOP->op_padix, MUTABLE_SV(SvREFCNT_inc_simple_NN(gv)));
#else
            kid->op_sv = SvREFCNT_inc_simple_NN(gv);
#endif
            kid->op_private = 0;
            /* FAKE globs in the symbol table cause weird bugs (#77810) */
            SvFAKE_off(gv);
        }
    }
    return o;
}

OP *
Perl_ck_ftst(pTHX_ OP *o)
{
    const I32 type = o->op_type;

    PERL_ARGS_ASSERT_CK_FTST;

    if (o->op_flags & OPf_REF) {
        NOOP;
    }
    else if (o->op_flags & OPf_KIDS && cUNOPo->op_first->op_type != OP_STUB) {
        SVOP * const kid = cSVOPx(cUNOPo->op_first);
        const OPCODE kidtype = kid->op_type;

        if (kidtype == OP_CONST && (kid->op_private & OPpCONST_BARE)
         && !kid->op_folded) {
            if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED) {
                no_bareword_filehandle(SvPVX(kSVOP_sv));
            }
            OP * const newop = newGVOP(type, OPf_REF,
                gv_fetchsv(kid->op_sv, GV_ADD, SVt_PVIO));
            op_free(o);
            return newop;
        }

        if ((kidtype == OP_RV2AV || kidtype == OP_PADAV) && ckWARN(WARN_SYNTAX)) {
            SV *name = S_op_varname_subscript(aTHX_ (OP*)kid, 2);
            if (name) {
                /* diag_listed_as: Array passed to stat will be coerced to a scalar%s */
                Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "%s (did you want stat %" SVf "?)",
                            array_passed_to_stat, name);
            }
            else {
                /* diag_listed_as: Array passed to stat will be coerced to a scalar%s */
                Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "%s", array_passed_to_stat);
            }
       }
        scalar((OP *) kid);
        if ((PL_hints & HINT_FILETEST_ACCESS) && OP_IS_FILETEST_ACCESS(o->op_type))
            o->op_private |= OPpFT_ACCESS;
        if (OP_IS_FILETEST(type)
            && OP_IS_FILETEST(kidtype)
        ) {
            o->op_private |= OPpFT_STACKED;
            kid->op_private |= OPpFT_STACKING;
            if (kidtype == OP_FTTTY && (
                   !(kid->op_private & OPpFT_STACKED)
                || kid->op_private & OPpFT_AFTER_t
               ))
                o->op_private |= OPpFT_AFTER_t;
        }
    }
    else {
        op_free(o);
        if (type == OP_FTTTY)
            o = newGVOP(type, OPf_REF, PL_stdingv);
        else
            o = newUNOP(type, 0, newDEFSVOP());
    }
    return o;
}

OP *
Perl_ck_fun(pTHX_ OP *o)
{
    const int type = o->op_type;
    I32 oa = PL_opargs[type] >> OASHIFT;

    PERL_ARGS_ASSERT_CK_FUN;

    if (o->op_flags & OPf_STACKED) {
        if ((oa & OA_OPTIONAL) && (oa >> 4) && !((oa >> 4) & OA_OPTIONAL))
            oa &= ~OA_OPTIONAL;
        else
            return no_fh_allowed(o);
    }

    if (o->op_flags & OPf_KIDS) {
        OP *prev_kid = NULL;
        OP *kid = cLISTOPo->op_first;
        I32 numargs = 0;
        bool seen_optional = FALSE;

        if (kid->op_type == OP_PUSHMARK ||
            (kid->op_type == OP_NULL && kid->op_targ == OP_PUSHMARK))
        {
            prev_kid = kid;
            kid = OpSIBLING(kid);
        }
        if (kid && kid->op_type == OP_COREARGS) {
            bool optional = FALSE;
            while (oa) {
                numargs++;
                if (oa & OA_OPTIONAL) optional = TRUE;
                oa = oa >> 4;
            }
            if (optional) o->op_private |= numargs;
            return o;
        }

        while (oa) {
            if (oa & OA_OPTIONAL || (oa & 7) == OA_LIST) {
                if (!kid && !seen_optional && PL_opargs[type] & OA_DEFGV) {
                    kid = newDEFSVOP();
                    /* append kid to chain */
                    op_sibling_splice(o, prev_kid, 0, kid);
                }
                seen_optional = TRUE;
            }
            if (!kid) break;

            numargs++;
            switch (oa & 7) {
            case OA_SCALAR:
                /* list seen where single (scalar) arg expected? */
                if (numargs == 1 && !(oa >> 4)
                    && kid->op_type == OP_LIST && type != OP_SCALAR)
                {
                    return too_many_arguments_pv(o,PL_op_desc[type], 0);
                }
                if (type != OP_DELETE) scalar(kid);
                break;
            case OA_LIST:
                if (oa < 16) {
                    kid = 0;
                    continue;
                }
                else
                    list(kid);
                break;
            case OA_AVREF:
                if ((type == OP_PUSH || type == OP_UNSHIFT)
                    && !OpHAS_SIBLING(kid))
                    Perl_ck_warner(aTHX_ packWARN(WARN_SYNTAX),
                                   "Useless use of %s with no values",
                                   PL_op_desc[type]);

                if (kid->op_type == OP_CONST
                      && (  !SvROK(cSVOPx_sv(kid))
                         || SvTYPE(SvRV(cSVOPx_sv(kid))) != SVt_PVAV  )
                        )
                    bad_type_pv(numargs, "array", o, kid);
                else if (kid->op_type == OP_RV2HV || kid->op_type == OP_PADHV
                         || kid->op_type == OP_RV2GV) {
                    bad_type_pv(1, "array", o, kid);
                }
                else if (kid->op_type != OP_RV2AV && kid->op_type != OP_PADAV) {
                    yyerror_pv(Perl_form(aTHX_ "Experimental %s on scalar is now forbidden",
                                         PL_op_desc[type]), 0);
                }
                else {
                    op_lvalue(kid, type);
                }
                break;
            case OA_HVREF:
                if (kid->op_type != OP_RV2HV && kid->op_type != OP_PADHV)
                    bad_type_pv(numargs, "hash", o, kid);
                op_lvalue(kid, type);
                break;
            case OA_CVREF:
                {
                    /* replace kid with newop in chain */
                    OP * const newop =
                        S_op_sibling_newUNOP(aTHX_ o, prev_kid, OP_NULL, 0);
                    newop->op_next = newop;
                    kid = newop;
                }
                break;
            case OA_FILEREF:
                if (kid->op_type != OP_GV && kid->op_type != OP_RV2GV) {
                    if (kid->op_type == OP_CONST &&
                        (kid->op_private & OPpCONST_BARE))
                    {
                        OP * const newop = newGVOP(OP_GV, 0,
                            gv_fetchsv(kSVOP->op_sv, GV_ADD, SVt_PVIO));
                        /* a first argument is handled by toke.c, ideally we'd
                         just check here but several ops don't use ck_fun() */
                        if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED) {
                            no_bareword_filehandle(SvPVX(kSVOP_sv));
                        }
                        /* replace kid with newop in chain */
                        op_sibling_splice(o, prev_kid, 1, newop);
                        op_free(kid);
                        kid = newop;
                    }
                    else if (kid->op_type == OP_READLINE) {
                        /* neophyte patrol: open(<FH>), close(<FH>) etc. */
                        bad_type_pv(numargs, "HANDLE", o, kid);
                    }
                    else {
                        I32 flags = OPf_SPECIAL;
                        I32 priv = 0;
                        PADOFFSET targ = 0;

                        /* is this op a FH constructor? */
                        if (is_handle_constructor(o,numargs)) {
                            const char *name = NULL;
                            STRLEN len = 0;
                            U32 name_utf8 = 0;
                            bool want_dollar = TRUE;

                            flags = 0;
                            /* Set a flag to tell rv2gv to vivify
                             * need to "prove" flag does not mean something
                             * else already - NI-S 1999/05/07
                             */
                            priv = OPpDEREF;
                            if (kid->op_type == OP_PADSV) {
                                PADNAME * const pn
                                    = PAD_COMPNAME_SV(kid->op_targ);
                                name = PadnamePV (pn);
                                len  = PadnameLEN(pn);
                                name_utf8 = PadnameUTF8(pn);
                            }
                            else if (kid->op_type == OP_RV2SV
                                     && kUNOP->op_first->op_type == OP_GV)
                            {
                                GV * const gv = cGVOPx_gv(kUNOP->op_first);
                                name = GvNAME(gv);
                                len = GvNAMELEN(gv);
                                name_utf8 = GvNAMEUTF8(gv) ? SVf_UTF8 : 0;
                            }
                            else if (kid->op_type == OP_AELEM
                                     || kid->op_type == OP_HELEM)
                            {
                                 OP *firstop;
                                 OP *op = kBINOP->op_first;
                                 name = NULL;
                                 if (op) {
                                      SV *tmpstr = NULL;
                                      const char * const a =
                                           kid->op_type == OP_AELEM ?
                                           "[]" : "{}";
                                      if (((op->op_type == OP_RV2AV) ||
                                           (op->op_type == OP_RV2HV)) &&
                                          (firstop = cUNOPx(op)->op_first) &&
                                          (firstop->op_type == OP_GV)) {
                                           /* packagevar $a[] or $h{} */
                                           GV * const gv = cGVOPx_gv(firstop);
                                           if (gv)
                                                tmpstr =
                                                     Perl_newSVpvf(aTHX_
                                                                   "%s%c...%c",
                                                                   GvNAME(gv),
                                                                   a[0], a[1]);
                                      }
                                      else if (op->op_type == OP_PADAV
                                               || op->op_type == OP_PADHV) {
                                           /* lexicalvar $a[] or $h{} */
                                           const char * const padname =
                                                PAD_COMPNAME_PV(op->op_targ);
                                           if (padname)
                                                tmpstr =
                                                     Perl_newSVpvf(aTHX_
                                                                   "%s%c...%c",
                                                                   padname + 1,
                                                                   a[0], a[1]);
                                      }
                                      if (tmpstr) {
                                           name = SvPV_const(tmpstr, len);
                                           name_utf8 = SvUTF8(tmpstr);
                                           sv_2mortal(tmpstr);
                                      }
                                 }
                                 if (!name) {
                                      name = "__ANONIO__";
                                      len = 10;
                                      want_dollar = FALSE;
                                 }
                                 op_lvalue(kid, type);
                            }
                            if (name) {
                                SV *namesv;
                                targ = pad_alloc(OP_RV2GV, SVf_READONLY);
                                namesv = PAD_SVl(targ);
                                if (want_dollar && *name != '$')
                                    sv_setpvs(namesv, "$");
                                else
                                    SvPVCLEAR(namesv);
                                sv_catpvn(namesv, name, len);
                                if ( name_utf8 ) SvUTF8_on(namesv);
                            }
                        }
                        scalar(kid);
                        kid = S_op_sibling_newUNOP(aTHX_ o, prev_kid,
                                    OP_RV2GV, flags);
                        kid->op_targ = targ;
                        kid->op_private |= priv;
                    }
                }
                scalar(kid);
                break;
            case OA_SCALARREF:
                if ((type == OP_UNDEF || type == OP_POS)
                    && numargs == 1 && !(oa >> 4)
                    && kid->op_type == OP_LIST)
                    return too_many_arguments_pv(o,PL_op_desc[type], 0);
                op_lvalue(scalar(kid), type);
                break;
            }
            oa >>= 4;
            prev_kid = kid;
            kid = OpSIBLING(kid);
        }
        /* FIXME - should the numargs or-ing move after the too many
         * arguments check? */
        o->op_private |= numargs;
        if (kid)
            return too_many_arguments_pv(o,OP_DESC(o), 0);
        listkids(o);
    }
    else if (PL_opargs[type] & OA_DEFGV) {
        /* Ordering of these two is important to keep f_map.t passing.  */
        op_free(o);
        return newUNOP(type, 0, newDEFSVOP());
    }

    if (oa) {
        while (oa & OA_OPTIONAL)
            oa >>= 4;
        if (oa && oa != OA_LIST)
            return too_few_arguments_pv(o,OP_DESC(o), 0);
    }
    return o;
}

OP *
Perl_ck_glob(pTHX_ OP *o)
{
    GV *gv;

    PERL_ARGS_ASSERT_CK_GLOB;

    o = ck_fun(o);
    if ((o->op_flags & OPf_KIDS) && !OpHAS_SIBLING(cLISTOPo->op_first))
        op_append_elem(OP_GLOB, o, newDEFSVOP()); /* glob() => glob($_) */

    if (!(o->op_flags & OPf_SPECIAL) && (gv = gv_override("glob", 4)))
    {
        /* convert
         *     glob
         *       \ null - const(wildcard)
         * into
         *     null
         *       \ enter
         *            \ list
         *                 \ mark - glob - rv2cv
         *                             |        \ gv(CORE::GLOBAL::glob)
         *                             |
         *                              \ null - const(wildcard)
         */
        o->op_flags |= OPf_SPECIAL;
        o->op_targ = pad_alloc(OP_GLOB, SVs_PADTMP);
        o = S_new_entersubop(aTHX_ gv, o);
        o = newUNOP(OP_NULL, 0, o);
        o->op_targ = OP_GLOB; /* hint at what it used to be: eg in newWHILEOP */
        return o;
    }
    else o->op_flags &= ~OPf_SPECIAL;
#if !defined(PERL_EXTERNAL_GLOB)
    if (!PL_globhook) {
        ENTER;
        Perl_load_module(aTHX_ PERL_LOADMOD_NOIMPORT,
                               newSVpvs("File::Glob"), NULL, NULL, NULL);
        LEAVE;
    }
#endif /* !PERL_EXTERNAL_GLOB */
    gv = (GV *)newSV_type(SVt_NULL);
    gv_init(gv, 0, "", 0, 0);
    gv_IOadd(gv);
    op_append_elem(OP_GLOB, o, newGVOP(OP_GV, 0, gv));
    SvREFCNT_dec_NN(gv); /* newGVOP increased it */
    scalarkids(o);
    return o;
}

OP *
Perl_ck_grep(pTHX_ OP *o)
{
    LOGOP *gwop;
    OP *kid;
    const OPCODE type = o->op_type == OP_GREPSTART ? OP_GREPWHILE : OP_MAPWHILE;

    PERL_ARGS_ASSERT_CK_GREP;

    /* don't allocate gwop here, as we may leak it if PL_parser->error_count > 0 */

    if (o->op_flags & OPf_STACKED) {
        kid = cUNOPx(OpSIBLING(cLISTOPo->op_first))->op_first;
        if (kid->op_type != OP_SCOPE && kid->op_type != OP_LEAVE)
            return no_fh_allowed(o);
        o->op_flags &= ~OPf_STACKED;
    }
    kid = OpSIBLING(cLISTOPo->op_first);
    if (type == OP_MAPWHILE)
        list(kid);
    else
        scalar(kid);
    o = ck_fun(o);
    if (PL_parser && PL_parser->error_count)
        return o;
    kid = OpSIBLING(cLISTOPo->op_first);
    if (kid->op_type != OP_NULL)
        Perl_croak(aTHX_ "panic: ck_grep, type=%u", (unsigned) kid->op_type);
    kid = kUNOP->op_first;

    gwop = alloc_LOGOP(type, o, LINKLIST(kid));
    kid->op_next = (OP*)gwop;
    o->op_private = gwop->op_private = 0;
    gwop->op_targ = pad_alloc(type, SVs_PADTMP);

    kid = OpSIBLING(cLISTOPo->op_first);
    for (kid = OpSIBLING(kid); kid; kid = OpSIBLING(kid))
        op_lvalue(kid, OP_GREPSTART);

    return (OP*)gwop;
}

OP *
Perl_ck_index(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_INDEX;

    if (o->op_flags & OPf_KIDS) {
        OP *kid = OpSIBLING(cLISTOPo->op_first);	/* get past pushmark */
        if (kid)
            kid = OpSIBLING(kid);			/* get past "big" */
        if (kid && kid->op_type == OP_CONST) {
            const bool save_taint = TAINT_get;
            SV *sv = kSVOP->op_sv;
            if (   (!SvPOK(sv) || SvNIOKp(sv) || isREGEXP(sv))
                && SvOK(sv) && !SvROK(sv))
            {
                sv = newSV_type(SVt_NULL);
                sv_copypv(sv, kSVOP->op_sv);
                SvREFCNT_dec_NN(kSVOP->op_sv);
                kSVOP->op_sv = sv;
            }
            if (SvOK(sv)) fbm_compile(sv, 0);
            TAINT_set(save_taint);
#ifdef NO_TAINT_SUPPORT
            PERL_UNUSED_VAR(save_taint);
#endif
        }
    }
    return ck_fun(o);
}

OP *
Perl_ck_lfun(pTHX_ OP *o)
{
    const OPCODE type = o->op_type;

    PERL_ARGS_ASSERT_CK_LFUN;

    return modkids(ck_fun(o), type);
}

OP *
Perl_ck_defined(pTHX_ OP *o)		/* 19990527 MJD */
{
    PERL_ARGS_ASSERT_CK_DEFINED;

    if ((o->op_flags & OPf_KIDS)) {
        switch (cUNOPo->op_first->op_type) {
        case OP_RV2AV:
        case OP_PADAV:
            Perl_croak(aTHX_ "Can't use 'defined(@array)'"
                             " (Maybe you should just omit the defined()?)");
            NOT_REACHED; /* NOTREACHED */
            break;
        case OP_RV2HV:
        case OP_PADHV:
            Perl_croak(aTHX_ "Can't use 'defined(%%hash)'"
                             " (Maybe you should just omit the defined()?)");
            NOT_REACHED; /* NOTREACHED */
            break;
        default:
            /* no warning */
            break;
        }
    }
    return ck_rfun(o);
}

OP *
Perl_ck_readline(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_READLINE;

    if (o->op_flags & OPf_KIDS) {
         OP *kid = cLISTOPo->op_first;
         if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED
             && kid->op_type == OP_CONST && (kid->op_private & OPpCONST_BARE)) {
             no_bareword_filehandle(SvPVX(kSVOP_sv));
         }
         if (kid->op_type == OP_RV2GV) kid->op_private |= OPpALLOW_FAKE;
         scalar(kid);
    }
    else {
        OP * const newop
            = newUNOP(OP_READLINE, 0, newGVOP(OP_GV, 0, PL_argvgv));
        op_free(o);
        return newop;
    }
    return o;
}

OP *
Perl_ck_rfun(pTHX_ OP *o)
{
    const OPCODE type = o->op_type;

    PERL_ARGS_ASSERT_CK_RFUN;

    return refkids(ck_fun(o), type);
}

OP *
Perl_ck_listiob(pTHX_ OP *o)
{
    OP *kid;

    PERL_ARGS_ASSERT_CK_LISTIOB;

    kid = cLISTOPo->op_first;
    if (!kid) {
        o = op_force_list(o);
        kid = cLISTOPo->op_first;
    }
    if (kid->op_type == OP_PUSHMARK)
        kid = OpSIBLING(kid);
    if (kid && o->op_flags & OPf_STACKED)
        kid = OpSIBLING(kid);
    else if (kid && !OpHAS_SIBLING(kid)) {		/* print HANDLE; */
        if (kid->op_type == OP_CONST && kid->op_private & OPpCONST_BARE
         && !kid->op_folded) {
            if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED) {
                no_bareword_filehandle(SvPVX(kSVOP_sv));
            }
            o->op_flags |= OPf_STACKED;	/* make it a filehandle */
            scalar(kid);
            /* replace old const op with new OP_RV2GV parent */
            kid = S_op_sibling_newUNOP(aTHX_ o, cLISTOPo->op_first,
                                        OP_RV2GV, OPf_REF);
            kid = OpSIBLING(kid);
        }
    }

    if (!kid)
        op_append_elem(o->op_type, o, newDEFSVOP());

    if (o->op_type == OP_PRTF) return modkids(listkids(o), OP_PRTF);
    return listkids(o);
}

OP *
Perl_ck_smartmatch(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_SMARTMATCH;
    if (0 == (o->op_flags & OPf_SPECIAL)) {
        OP *first  = cBINOPo->op_first;
        OP *second = OpSIBLING(first);

        /* Implicitly take a reference to an array or hash */

        /* remove the original two siblings, then add back the
         * (possibly different) first and second sibs.
         */
        op_sibling_splice(o, NULL, 1, NULL);
        op_sibling_splice(o, NULL, 1, NULL);
        first  = ref_array_or_hash(first);
        second = ref_array_or_hash(second);
        op_sibling_splice(o, NULL, 0, second);
        op_sibling_splice(o, NULL, 0, first);

        /* Implicitly take a reference to a regular expression */
        if (first->op_type == OP_MATCH && !(first->op_flags & OPf_STACKED)) {
            OpTYPE_set(first, OP_QR);
        }
        if (second->op_type == OP_MATCH && !(second->op_flags & OPf_STACKED)) {
            OpTYPE_set(second, OP_QR);
        }
    }

    return o;
}


static OP *
S_maybe_targlex(pTHX_ OP *o)
{
    OP * const kid = cLISTOPo->op_first;
    /* has a disposable target? */
    if ((PL_opargs[kid->op_type] & OA_TARGLEX)
        && !(kid->op_flags & OPf_STACKED)
        /* Cannot steal the second time! */
        && !(kid->op_private & OPpTARGET_MY)
        )
    {
        OP * const kkid = OpSIBLING(kid);

        /* Can just relocate the target. */
        if (kkid && kkid->op_type == OP_PADSV
            && (!(kkid->op_private & OPpLVAL_INTRO)
               || kkid->op_private & OPpPAD_STATE))
        {
            kid->op_targ = kkid->op_targ;
            kkid->op_targ = 0;
            /* Now we do not need PADSV and SASSIGN.
             * Detach kid and free the rest. */
            op_sibling_splice(o, NULL, 1, NULL);
            op_free(o);
            kid->op_private |= OPpTARGET_MY;	/* Used for context settings */
            return kid;
        }
    }
    return o;
}

OP *
Perl_ck_sassign(pTHX_ OP *o)
{
    OP * const kid = cBINOPo->op_first;

    PERL_ARGS_ASSERT_CK_SASSIGN;

    if (OpHAS_SIBLING(kid)) {
        OP *kkid = OpSIBLING(kid);
        /* For state variable assignment with attributes, kkid is a list op
           whose op_last is a padsv. */
        if ((kkid->op_type == OP_PADSV ||
             (OP_TYPE_IS_OR_WAS(kkid, OP_LIST) &&
              (kkid = cLISTOPx(kkid)->op_last)->op_type == OP_PADSV
             )
            )
                && (kkid->op_private & (OPpLVAL_INTRO|OPpPAD_STATE))
                    == (OPpLVAL_INTRO|OPpPAD_STATE)) {
            return S_newONCEOP(aTHX_ o, kkid);
        }
    }
    return S_maybe_targlex(aTHX_ o);
}


OP *
Perl_ck_match(pTHX_ OP *o)
{
    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_CK_MATCH;

    return o;
}

OP *
Perl_ck_method(pTHX_ OP *o)
{
    SV *sv, *methsv, *rclass;
    const char* method;
    char* compatptr;
    int utf8;
    STRLEN len, nsplit = 0, i;
    OP* new_op;
    OP * const kid = cUNOPo->op_first;

    PERL_ARGS_ASSERT_CK_METHOD;
    if (kid->op_type != OP_CONST) return o;

    sv = kSVOP->op_sv;

    /* replace ' with :: */
    while ((compatptr = (char *) memchr(SvPVX(sv), '\'',
                                        SvEND(sv) - SvPVX(sv) )))
    {
        *compatptr = ':';
        sv_insert(sv, compatptr - SvPVX_const(sv), 0, ":", 1);
    }

    method = SvPVX_const(sv);
    len = SvCUR(sv);
    utf8 = SvUTF8(sv) ? -1 : 1;

    for (i = len - 1; i > 0; --i) if (method[i] == ':') {
        nsplit = i+1;
        break;
    }

    methsv = newSVpvn_share(method+nsplit, utf8*(len - nsplit), 0);

    if (!nsplit) { /* $proto->method() */
        op_free(o);
        return newMETHOP_named(OP_METHOD_NAMED, 0, methsv);
    }

    if (memEQs(method, nsplit, "SUPER::")) { /* $proto->SUPER::method() */
        op_free(o);
        return newMETHOP_named(OP_METHOD_SUPER, 0, methsv);
    }

    /* $proto->MyClass::method() and $proto->MyClass::SUPER::method() */
    if (nsplit >= 9 && strBEGINs(method+nsplit-9, "::SUPER::")) {
        rclass = newSVpvn_share(method, utf8*(nsplit-9), 0);
        new_op = newMETHOP_named(OP_METHOD_REDIR_SUPER, 0, methsv);
    } else {
        rclass = newSVpvn_share(method, utf8*(nsplit-2), 0);
        new_op = newMETHOP_named(OP_METHOD_REDIR, 0, methsv);
    }
#ifdef USE_ITHREADS
    op_relocate_sv(&rclass, &cMETHOPx(new_op)->op_rclass_targ);
#else
    cMETHOPx(new_op)->op_rclass_sv = rclass;
#endif
    op_free(o);
    return new_op;
}

OP *
Perl_ck_null(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_NULL;
    PERL_UNUSED_CONTEXT;
    return o;
}

OP *
Perl_ck_open(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_OPEN;

    S_io_hints(aTHX_ o);
    {
         /* In case of three-arg dup open remove strictness
          * from the last arg if it is a bareword. */
         OP * const first = cLISTOPx(o)->op_first; /* The pushmark. */
         OP * const last  = cLISTOPx(o)->op_last;  /* The bareword. */
         OP *oa;
         const char *mode;

         if ((last->op_type == OP_CONST) &&		/* The bareword. */
             (last->op_private & OPpCONST_BARE) &&
             (last->op_private & OPpCONST_STRICT) &&
             (oa = OpSIBLING(first)) &&		/* The fh. */
             (oa = OpSIBLING(oa)) &&			/* The mode. */
             (oa->op_type == OP_CONST) &&
             SvPOK(cSVOPx(oa)->op_sv) &&
             (mode = SvPVX_const(cSVOPx(oa)->op_sv)) &&
             mode[0] == '>' && mode[1] == '&' &&	/* A dup open. */
             (last == OpSIBLING(oa)))			/* The bareword. */
              last->op_private &= ~OPpCONST_STRICT;
    }
    return ck_fun(o);
}

OP *
Perl_ck_prototype(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_PROTOTYPE;
    if (!(o->op_flags & OPf_KIDS)) {
        op_free(o);
        return newUNOP(OP_PROTOTYPE, 0, newDEFSVOP());
    }
    return o;
}

OP *
Perl_ck_refassign(pTHX_ OP *o)
{
    OP * const right = cLISTOPo->op_first;
    OP * const left = OpSIBLING(right);
    OP *varop = cUNOPx(cUNOPx(left)->op_first)->op_first;
    bool stacked = 0;

    PERL_ARGS_ASSERT_CK_REFASSIGN;
    assert (left);
    assert (left->op_type == OP_SREFGEN);

    o->op_private = 0;
    /* we use OPpPAD_STATE in refassign to mean either of those things,
     * and the code assumes the two flags occupy the same bit position
     * in the various ops below */
    assert(OPpPAD_STATE == OPpOUR_INTRO);

    switch (varop->op_type) {
    case OP_PADAV:
        o->op_private |= OPpLVREF_AV;
        goto settarg;
    case OP_PADHV:
        o->op_private |= OPpLVREF_HV;
        /* FALLTHROUGH */
    case OP_PADSV:
      settarg:
        o->op_private |= (varop->op_private & (OPpLVAL_INTRO|OPpPAD_STATE));
        o->op_targ = varop->op_targ;
        varop->op_targ = 0;
        PAD_COMPNAME_GEN_set(o->op_targ, PERL_INT_MAX);
        break;

    case OP_RV2AV:
        o->op_private |= OPpLVREF_AV;
        goto checkgv;
        NOT_REACHED; /* NOTREACHED */
    case OP_RV2HV:
        o->op_private |= OPpLVREF_HV;
        /* FALLTHROUGH */
    case OP_RV2SV:
      checkgv:
        o->op_private |= (varop->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO));
        if (cUNOPx(varop)->op_first->op_type != OP_GV) goto bad;
      detach_and_stack:
        /* Point varop to its GV kid, detached.  */
        varop = op_sibling_splice(varop, NULL, -1, NULL);
        stacked = TRUE;
        break;
    case OP_RV2CV: {
        OP * const kidparent =
            OpSIBLING(cUNOPx(cUNOPx(varop)->op_first)->op_first);
        OP * const kid = cUNOPx(kidparent)->op_first;
        o->op_private |= OPpLVREF_CV;
        if (kid->op_type == OP_GV) {
            SV *sv = (SV*)cGVOPx_gv(kid);
            varop = kidparent;
            if (SvROK(sv) && SvTYPE(SvRV(sv)) == SVt_PVCV) {
                /* a CVREF here confuses pp_refassign, so make sure
                   it gets a GV */
                CV *const cv = (CV*)SvRV(sv);
                SV *name_sv = newSVhek_mortal(CvNAME_HEK(cv));
                (void)gv_init_sv((GV*)sv, CvSTASH(cv), name_sv, 0);
                assert(SvTYPE(sv) == SVt_PVGV);
            }
            goto detach_and_stack;
        }
        if (kid->op_type != OP_PADCV)	goto bad;
        o->op_targ = kid->op_targ;
        kid->op_targ = 0;
        break;
    }
    case OP_AELEM:
    case OP_HELEM:
        o->op_private |= (varop->op_private & OPpLVAL_INTRO);
        o->op_private |= OPpLVREF_ELEM;
        op_null(varop);
        stacked = TRUE;
        /* Detach varop.  */
        op_sibling_splice(cUNOPx(left)->op_first, NULL, -1, NULL);
        break;
    default:
      bad:
        /* diag_listed_as: Can't modify reference to %s in %s assignment */
        yyerror(Perl_form(aTHX_ "Can't modify reference to %s in scalar "
                                "assignment",
                                 OP_DESC(varop)));
        return o;
    }
    if (!FEATURE_REFALIASING_IS_ENABLED)
        Perl_croak(aTHX_
                  "Experimental aliasing via reference not enabled");
    Perl_ck_warner_d(aTHX_
                     packWARN(WARN_EXPERIMENTAL__REFALIASING),
                    "Aliasing via reference is experimental");
    if (stacked) {
        o->op_flags |= OPf_STACKED;
        op_sibling_splice(o, right, 1, varop);
    }
    else {
        o->op_flags &=~ OPf_STACKED;
        op_sibling_splice(o, right, 1, NULL);
    }
    op_free(left);
    return o;
}

OP *
Perl_ck_repeat(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_REPEAT;

    if (cBINOPo->op_first->op_flags & OPf_PARENS) {
        OP* kids;
        o->op_private |= OPpREPEAT_DOLIST;
        kids = op_sibling_splice(o, NULL, 1, NULL); /* detach first kid */
        kids = op_force_list(kids); /* promote it to a list */
        op_sibling_splice(o, NULL, 0, kids); /* and add back */
    }
    else
        scalar(o);
    return o;
}

OP *
Perl_ck_require(pTHX_ OP *o)
{
    GV* gv;

    PERL_ARGS_ASSERT_CK_REQUIRE;

    if (o->op_flags & OPf_KIDS) {	/* Shall we supply missing .pm? */
        SVOP * const kid = cSVOPx(cUNOPo->op_first);
        U32 hash;
        char *s;
        STRLEN len;
        if (kid->op_type == OP_CONST) {
          SV * const sv = kid->op_sv;
          U32 const was_readonly = SvREADONLY(sv);
          if (kid->op_private & OPpCONST_BARE) {
            const char *end;
            HEK *hek;

            if (was_readonly) {
                SvREADONLY_off(sv);
            }

            if (SvIsCOW(sv)) sv_force_normal_flags(sv, 0);

            s = SvPVX(sv);
            len = SvCUR(sv);
            end = s + len;
            /* treat ::foo::bar as foo::bar */
            if (len >= 2 && s[0] == ':' && s[1] == ':')
                DIE(aTHX_ "Bareword in require must not start with a double-colon: \"%s\"\n", s);
            if (s == end)
                DIE(aTHX_ "Bareword in require maps to empty filename");

            for (; s < end; s++) {
                if (*s == ':' && s[1] == ':') {
                    *s = '/';
                    Move(s+2, s+1, end - s - 1, char);
                    --end;
                }
            }
            SvEND_set(sv, end);
            sv_catpvs(sv, ".pm");
            PERL_HASH(hash, SvPVX(sv), SvCUR(sv));
            hek = share_hek(SvPVX(sv),
                            (SSize_t)SvCUR(sv) * (SvUTF8(sv) ? -1 : 1),
                            hash);
            sv_sethek(sv, hek);
            unshare_hek(hek);
            SvFLAGS(sv) |= was_readonly;
          }
          else if (SvPOK(sv) && !SvNIOK(sv) && !SvGMAGICAL(sv)
                && !SvVOK(sv)) {
            s = SvPV(sv, len);
            if (SvREFCNT(sv) > 1) {
                kid->op_sv = newSVpvn_share(
                    s, SvUTF8(sv) ? -(SSize_t)len : (SSize_t)len, 0);
                SvREFCNT_dec_NN(sv);
            }
            else {
                HEK *hek;
                if (was_readonly) SvREADONLY_off(sv);
                PERL_HASH(hash, s, len);
                hek = share_hek(s,
                                SvUTF8(sv) ? -(SSize_t)len : (SSize_t)len,
                                hash);
                sv_sethek(sv, hek);
                unshare_hek(hek);
                SvFLAGS(sv) |= was_readonly;
            }
          }
        }
    }

    if (!(o->op_flags & OPf_SPECIAL) /* Wasn't written as CORE::require */
        /* handle override, if any */
     && (gv = gv_override("require", 7))) {
        OP *kid, *newop;
        if (o->op_flags & OPf_KIDS) {
            kid = cUNOPo->op_first;
            op_sibling_splice(o, NULL, -1, NULL);
        }
        else {
            kid = newDEFSVOP();
        }
        op_free(o);
        newop = S_new_entersubop(aTHX_ gv, kid);
        return newop;
    }

    return ck_fun(o);
}

OP *
Perl_ck_return(pTHX_ OP *o)
{
    OP *kid;

    PERL_ARGS_ASSERT_CK_RETURN;

    kid = OpSIBLING(cLISTOPo->op_first);
    if (PL_compcv && CvLVALUE(PL_compcv)) {
        for (; kid; kid = OpSIBLING(kid))
            op_lvalue(kid, OP_LEAVESUBLV);
    }

    return o;
}

OP *
Perl_ck_select(pTHX_ OP *o)
{
    OP* kid;

    PERL_ARGS_ASSERT_CK_SELECT;

    if (o->op_flags & OPf_KIDS) {
        kid = OpSIBLING(cLISTOPo->op_first);     /* get past pushmark */
        if (kid && OpHAS_SIBLING(kid)) {
            OpTYPE_set(o, OP_SSELECT);
            o = ck_fun(o);
            return fold_constants(op_integerize(op_std_init(o)));
        }
    }
    o = ck_fun(o);
    kid = OpSIBLING(cLISTOPo->op_first);    /* get past pushmark */
    if (kid && kid->op_type == OP_RV2GV)
        kid->op_private &= ~HINT_STRICT_REFS;
    return o;
}

OP *
Perl_ck_shift(pTHX_ OP *o)
{
    const I32 type = o->op_type;

    PERL_ARGS_ASSERT_CK_SHIFT;

    if (!(o->op_flags & OPf_KIDS)) {
        OP *argop;

        if (!CvUNIQUE(PL_compcv)) {
            o->op_flags |= OPf_SPECIAL;
            return o;
        }

        argop = newUNOP(OP_RV2AV, 0, scalar(newGVOP(OP_GV, 0, PL_argvgv)));
        op_free(o);
        return newUNOP(type, 0, scalar(argop));
    }
    return scalar(ck_fun(o));
}

OP *
Perl_ck_sort(pTHX_ OP *o)
{
    OP *firstkid;
    OP *kid;
    U8 stacked;

    PERL_ARGS_ASSERT_CK_SORT;

    if (o->op_flags & OPf_STACKED)
        simplify_sort(o);
    firstkid = OpSIBLING(cLISTOPo->op_first);		/* get past pushmark */

    if (!firstkid)
        return too_few_arguments_pv(o,OP_DESC(o), 0);

    if ((stacked = o->op_flags & OPf_STACKED)) {	/* may have been cleared */
        OP *kid = cUNOPx(firstkid)->op_first;		/* get past null */

        /* if the first arg is a code block, process it and mark sort as
         * OPf_SPECIAL */
        if (kid->op_type == OP_SCOPE || kid->op_type == OP_LEAVE) {
            LINKLIST(kid);
            if (kid->op_type == OP_LEAVE)
                    op_null(kid);			/* wipe out leave */
            /* Prevent execution from escaping out of the sort block. */
            kid->op_next = 0;

            /* provide scalar context for comparison function/block */
            kid = scalar(firstkid);
            kid->op_next = kid;
            o->op_flags |= OPf_SPECIAL;
        }
        else if (kid->op_type == OP_CONST
              && kid->op_private & OPpCONST_BARE) {
            char tmpbuf[256];
            STRLEN len;
            PADOFFSET off;
            const char * const name = SvPV(kSVOP_sv, len);
            *tmpbuf = '&';
            assert (len < 256);
            Copy(name, tmpbuf+1, len, char);
            off = pad_findmy_pvn(tmpbuf, len+1, 0);
            if (off != NOT_IN_PAD) {
                if (PAD_COMPNAME_FLAGS_isOUR(off)) {
                    SV * const fq =
                        newSVhek(HvNAME_HEK(PAD_COMPNAME_OURSTASH(off)));
                    sv_catpvs(fq, "::");
                    sv_catsv(fq, kSVOP_sv);
                    SvREFCNT_dec_NN(kSVOP_sv);
                    kSVOP->op_sv = fq;
                }
                else {
                    /* replace the const op with the pad op */
                    op_sibling_splice(firstkid, NULL, 1,
                        newPADxVOP(OP_PADCV, 0, off));
                    op_free(kid);
                }
            }
        }

        firstkid = OpSIBLING(firstkid);
    }

    for (kid = firstkid; kid; kid = OpSIBLING(kid)) {
        /* provide list context for arguments */
        list(kid);
        if (stacked)
            op_lvalue(kid, OP_GREPSTART);
    }

    return o;
}

/* for sort { X } ..., where X is one of
 *   $a <=> $b, $b <=> $a, $a cmp $b, $b cmp $a
 * elide the second child of the sort (the one containing X),
 * and set these flags as appropriate
        OPpSORT_NUMERIC;
        OPpSORT_INTEGER;
        OPpSORT_DESCEND;
 * Also, check and warn on lexical $a, $b.
 */

STATIC void
S_simplify_sort(pTHX_ OP *o)
{
    OP *kid = OpSIBLING(cLISTOPo->op_first);	/* get past pushmark */
    OP *k;
    int descending;
    GV *gv;
    const char *gvname;
    bool have_scopeop;

    PERL_ARGS_ASSERT_SIMPLIFY_SORT;

    kid = kUNOP->op_first;				/* get past null */
    if (!(have_scopeop = kid->op_type == OP_SCOPE)
     && kid->op_type != OP_LEAVE)
        return;
    kid = kLISTOP->op_last;				/* get past scope */
    switch(kid->op_type) {
        case OP_NCMP:
        case OP_I_NCMP:
        case OP_SCMP:
            if (!have_scopeop) goto padkids;
            break;
        default:
            return;
    }
    k = kid;						/* remember this node*/
    if (kBINOP->op_first->op_type != OP_RV2SV
     || kBINOP->op_last ->op_type != OP_RV2SV)
    {
        /*
           Warn about my($a) or my($b) in a sort block, *if* $a or $b is
           then used in a comparison.  This catches most, but not
           all cases.  For instance, it catches
               sort { my($a); $a <=> $b }
           but not
               sort { my($a); $a < $b ? -1 : $a == $b ? 0 : 1; }
           (although why you'd do that is anyone's guess).
        */

       padkids:
        if (!ckWARN(WARN_SYNTAX)) return;
        kid = kBINOP->op_first;
        do {
            if (kid->op_type == OP_PADSV) {
                PADNAME * const name = PAD_COMPNAME(kid->op_targ);
                if (PadnameLEN(name) == 2 && *PadnamePV(name) == '$'
                 && (  PadnamePV(name)[1] == 'a'
                    || PadnamePV(name)[1] == 'b'  ))
                    /* diag_listed_as: "my %s" used in sort comparison */
                    Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                                     "\"%s %s\" used in sort comparison",
                                      PadnameIsSTATE(name)
                                        ? "state"
                                        : "my",
                                      PadnamePV(name));
            }
        } while ((kid = OpSIBLING(kid)));
        return;
    }
    kid = kBINOP->op_first;				/* get past cmp */
    if (kUNOP->op_first->op_type != OP_GV)
        return;
    kid = kUNOP->op_first;				/* get past rv2sv */
    gv = kGVOP_gv;
    if (GvSTASH(gv) != PL_curstash)
        return;
    gvname = GvNAME(gv);
    if (*gvname == 'a' && gvname[1] == '\0')
        descending = 0;
    else if (*gvname == 'b' && gvname[1] == '\0')
        descending = 1;
    else
        return;

    kid = k;						/* back to cmp */
    /* already checked above that it is rv2sv */
    kid = kBINOP->op_last;				/* down to 2nd arg */
    if (kUNOP->op_first->op_type != OP_GV)
        return;
    kid = kUNOP->op_first;				/* get past rv2sv */
    gv = kGVOP_gv;
    if (GvSTASH(gv) != PL_curstash)
        return;
    gvname = GvNAME(gv);
    if ( descending
         ? !(*gvname == 'a' && gvname[1] == '\0')
         : !(*gvname == 'b' && gvname[1] == '\0'))
        return;
    o->op_flags &= ~(OPf_STACKED | OPf_SPECIAL);
    if (descending)
        o->op_private |= OPpSORT_DESCEND;
    if (k->op_type == OP_NCMP)
        o->op_private |= OPpSORT_NUMERIC;
    if (k->op_type == OP_I_NCMP)
        o->op_private |= OPpSORT_NUMERIC | OPpSORT_INTEGER;
    kid = OpSIBLING(cLISTOPo->op_first);
    /* cut out and delete old block (second sibling) */
    op_sibling_splice(o, cLISTOPo->op_first, 1, NULL);
    op_free(kid);
}

OP *
Perl_ck_split(pTHX_ OP *o)
{
    OP *kid;
    OP *sibs;

    PERL_ARGS_ASSERT_CK_SPLIT;

    assert(o->op_type == OP_LIST);

    if (o->op_flags & OPf_STACKED)
        return no_fh_allowed(o);

    kid = cLISTOPo->op_first;
    /* delete leading NULL node, then add a CONST if no other nodes */
    assert(kid->op_type == OP_NULL);
    op_sibling_splice(o, NULL, 1,
        OpHAS_SIBLING(kid) ? NULL : newSVOP(OP_CONST, 0, newSVpvs(" ")));
    op_free(kid);
    kid = cLISTOPo->op_first;

    if (kid->op_type != OP_MATCH || kid->op_flags & OPf_STACKED) {
        /* remove match expression, and replace with new optree with
         * a match op at its head */
        op_sibling_splice(o, NULL, 1, NULL);
        /* pmruntime will handle split " " behavior with flag==2 */
        kid = pmruntime(newPMOP(OP_MATCH, 0), kid, NULL, 2, 0);
        op_sibling_splice(o, NULL, 0, kid);
    }

    assert(kid->op_type == OP_MATCH || kid->op_type == OP_SPLIT);

    if (kPMOP->op_pmflags & PMf_GLOBAL) {
      Perl_ck_warner(aTHX_ packWARN(WARN_REGEXP),
                     "Use of /g modifier is meaningless in split");
    }

    /* eliminate the split op, and move the match op (plus any children)
     * into its place, then convert the match op into a split op. i.e.
     *
     *  SPLIT                    MATCH                 SPLIT(ex-MATCH)
     *    |                        |                     |
     *  MATCH - A - B - C   =>     R - A - B - C   =>    R - A - B - C
     *    |                        |                     |
     *    R                        X - Y                 X - Y
     *    |
     *    X - Y
     *
     * (R, if it exists, will be a regcomp op)
     */

    op_sibling_splice(o, NULL, 1, NULL); /* detach match op from o */
    sibs = op_sibling_splice(o, NULL, -1, NULL); /* detach any other sibs */
    op_sibling_splice(kid, cLISTOPx(kid)->op_last, 0, sibs); /* and reattach */
    OpTYPE_set(kid, OP_SPLIT);
    kid->op_flags   = (o->op_flags | (kid->op_flags & OPf_KIDS));
    kid->op_private = o->op_private;
    op_free(o);
    o = kid;
    kid = sibs; /* kid is now the string arg of the split */

    if (!kid) {
        kid = newDEFSVOP();
        op_append_elem(OP_SPLIT, o, kid);
    }
    scalar(kid);

    kid = OpSIBLING(kid);
    if (!kid) {
        kid = newSVOP(OP_CONST, 0, newSViv(0));
        op_append_elem(OP_SPLIT, o, kid);
        o->op_private |= OPpSPLIT_IMPLIM;
    }
    scalar(kid);

    if (OpHAS_SIBLING(kid))
        return too_many_arguments_pv(o,OP_DESC(o), 0);

    return o;
}

OP *
Perl_ck_stringify(pTHX_ OP *o)
{
    OP * const kid = OpSIBLING(cUNOPo->op_first);
    PERL_ARGS_ASSERT_CK_STRINGIFY;
    if ((   kid->op_type == OP_JOIN || kid->op_type == OP_QUOTEMETA
         || kid->op_type == OP_LC   || kid->op_type == OP_LCFIRST
         || kid->op_type == OP_UC   || kid->op_type == OP_UCFIRST)
        && !OpHAS_SIBLING(kid)) /* syntax errs can leave extra children */
    {
        op_sibling_splice(o, cUNOPo->op_first, -1, NULL);
        op_free(o);
        return kid;
    }
    return ck_fun(o);
}

OP *
Perl_ck_join(pTHX_ OP *o)
{
    OP * const kid = OpSIBLING(cLISTOPo->op_first);

    PERL_ARGS_ASSERT_CK_JOIN;

    if (kid && kid->op_type == OP_MATCH) {
        if (ckWARN(WARN_SYNTAX)) {
            const REGEXP *re = PM_GETRE(kPMOP);
            const SV *msg = re
                    ? newSVpvn_flags( RX_PRECOMP_const(re), RX_PRELEN(re),
                                            SVs_TEMP | ( RX_UTF8(re) ? SVf_UTF8 : 0 ) )
                    : newSVpvs_flags( "STRING", SVs_TEMP );
            Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                        "/%" SVf "/ should probably be written as \"%" SVf "\"",
                        SVfARG(msg), SVfARG(msg));
        }
    }
    if (kid
     && (kid->op_type == OP_CONST /* an innocent, unsuspicious separator */
        || (kid->op_type == OP_PADSV && !(kid->op_private & OPpLVAL_INTRO))
        || (  kid->op_type==OP_RV2SV && kUNOP->op_first->op_type == OP_GV
           && !(kid->op_private & (OPpLVAL_INTRO|OPpOUR_INTRO)))))
    {
        const OP * const bairn = OpSIBLING(kid); /* the list */
        if (bairn && !OpHAS_SIBLING(bairn) /* single-item list */
         && OP_GIMME(bairn,0) == G_SCALAR)
        {
            OP * const ret = op_convert_list(OP_STRINGIFY, OPf_FOLDED,
                                     op_sibling_splice(o, kid, 1, NULL));
            op_free(o);
            return ret;
        }
    }

    return ck_fun(o);
}

/*
=for apidoc rv2cv_op_cv

Examines an op, which is expected to identify a subroutine at runtime,
and attempts to determine at compile time which subroutine it identifies.
This is normally used during Perl compilation to determine whether
a prototype can be applied to a function call.  C<cvop> is the op
being considered, normally an C<rv2cv> op.  A pointer to the identified
subroutine is returned, if it could be determined statically, and a null
pointer is returned if it was not possible to determine statically.

Currently, the subroutine can be identified statically if the RV that the
C<rv2cv> is to operate on is provided by a suitable C<gv> or C<const> op.
A C<gv> op is suitable if the GV's CV slot is populated.  A C<const> op is
suitable if the constant value must be an RV pointing to a CV.  Details of
this process may change in future versions of Perl.  If the C<rv2cv> op
has the C<OPpENTERSUB_AMPER> flag set then no attempt is made to identify
the subroutine statically: this flag is used to suppress compile-time
magic on a subroutine call, forcing it to use default runtime behaviour.

If C<flags> has the bit C<RV2CVOPCV_MARK_EARLY> set, then the handling
of a GV reference is modified.  If a GV was examined and its CV slot was
found to be empty, then the C<gv> op has the C<OPpEARLY_CV> flag set.
If the op is not optimised away, and the CV slot is later populated with
a subroutine having a prototype, that flag eventually triggers the warning
"called too early to check prototype".

If C<flags> has the bit C<RV2CVOPCV_RETURN_NAME_GV> set, then instead
of returning a pointer to the subroutine it returns a pointer to the
GV giving the most appropriate name for the subroutine in this context.
Normally this is just the C<CvGV> of the subroutine, but for an anonymous
(C<CvANON>) subroutine that is referenced through a GV it will be the
referencing GV.  The resulting C<GV*> is cast to C<CV*> to be returned.
A null pointer is returned as usual if there is no statically-determinable
subroutine.

=for apidoc Amnh||OPpEARLY_CV
=for apidoc Amnh||OPpENTERSUB_AMPER
=for apidoc Amnh||RV2CVOPCV_MARK_EARLY
=for apidoc Amnh||RV2CVOPCV_RETURN_NAME_GV

=cut
*/

/* shared by toke.c:yylex */
CV *
Perl_find_lexical_cv(pTHX_ PADOFFSET off)
{
    const PADNAME *name = PAD_COMPNAME(off);
    CV *compcv = PL_compcv;
    while (PadnameOUTER(name)) {
        compcv = CvOUTSIDE(compcv);
        if (LIKELY(PARENT_PAD_INDEX(name))) {
            name = PadlistNAMESARRAY(CvPADLIST(compcv))
                [off = PARENT_PAD_INDEX(name)];
        }
        else {
            /* In an eval() in an inner scope like a function, the
               intermediate pad in the sub might not be populated with the
               sub.  So search harder.

               It is possible we won't find the name in this
               particular scope, but that's fine, if we don't we'll
               find it in some outer scope.  Finding it here will let us
               go back to following the PARENT_PAD_INDEX() chain.
            */
            const PADNAMELIST * const names = PadlistNAMES(CvPADLIST(compcv));
            PADNAME * const * const name_p = PadnamelistARRAY(names);
            int offset;
            for (offset = PadnamelistMAXNAMED(names); offset > 0; offset--) {
                const PADNAME * const thisname = name_p[offset];
                /* The pv is copied from the outer PADNAME to the
                   inner PADNAMEs so we don't need to compare the
                   string contents
                */
                if (thisname && PadnameLEN(thisname) == PadnameLEN(name)
                    && PadnamePV(thisname) == PadnamePV(name)) {
                    name = thisname;
                    break;
                }
            }
        }
    }
    assert(!PadnameIsOUR(name));
    if (!PadnameIsSTATE(name) && PadnamePROTOCV(name)) {
        return PadnamePROTOCV(name);
    }
    return (CV *)AvARRAY(PadlistARRAY(CvPADLIST(compcv))[1])[off];
}

CV *
Perl_rv2cv_op_cv(pTHX_ OP *cvop, U32 flags)
{
    OP *rvop;
    CV *cv;
    GV *gv;
    PERL_ARGS_ASSERT_RV2CV_OP_CV;
    if (flags & ~RV2CVOPCV_FLAG_MASK)
        Perl_croak(aTHX_ "panic: rv2cv_op_cv bad flags %x", (unsigned)flags);
    if (cvop->op_type != OP_RV2CV)
        return NULL;
    if (cvop->op_private & OPpENTERSUB_AMPER)
        return NULL;
    if (!(cvop->op_flags & OPf_KIDS))
        return NULL;
    rvop = cUNOPx(cvop)->op_first;
    switch (rvop->op_type) {
        case OP_GV: {
            gv = cGVOPx_gv(rvop);
            if (!isGV(gv)) {
                if (SvROK(gv) && SvTYPE(SvRV(gv)) == SVt_PVCV) {
                    cv = MUTABLE_CV(SvRV(gv));
                    gv = NULL;
                    break;
                }
                if (flags & RV2CVOPCV_RETURN_STUB)
                    return (CV *)gv;
                else return NULL;
            }
            cv = GvCVu(gv);
            if (!cv) {
                if (flags & RV2CVOPCV_MARK_EARLY)
                    rvop->op_private |= OPpEARLY_CV;
                return NULL;
            }
        } break;
        case OP_CONST: {
            SV *rv = cSVOPx_sv(rvop);
            if (!SvROK(rv))
                return NULL;
            cv = (CV*)SvRV(rv);
            gv = NULL;
        } break;
        case OP_PADCV: {
            cv = find_lexical_cv(rvop->op_targ);
            gv = NULL;
        } break;
        default: {
            return NULL;
        } NOT_REACHED; /* NOTREACHED */
    }
    if (SvTYPE((SV*)cv) != SVt_PVCV)
        return NULL;
    if (flags & RV2CVOPCV_RETURN_NAME_GV) {
        if ((!CvANON(cv) && !CvLEXICAL(cv)) || !gv)
            gv = CvGV(cv);
        return (CV*)gv;
    }
    else if (flags & RV2CVOPCV_MAYBE_NAME_GV) {
        if (CvLEXICAL(cv) || CvNAMED(cv))
            return NULL;
        if (!CvANON(cv) || !gv)
            gv = CvGV(cv);
        return (CV*)gv;

    } else {
        return cv;
    }
}

/*
=for apidoc ck_entersub_args_list

Performs the default fixup of the arguments part of an C<entersub>
op tree.  This consists of applying list context to each of the
argument ops.  This is the standard treatment used on a call marked
with C<&>, or a method call, or a call through a subroutine reference,
or any other call where the callee can't be identified at compile time,
or a call where the callee has no prototype.

=cut
*/

OP *
Perl_ck_entersub_args_list(pTHX_ OP *entersubop)
{
    OP *aop;

    PERL_ARGS_ASSERT_CK_ENTERSUB_ARGS_LIST;

    aop = cUNOPx(entersubop)->op_first;
    if (!OpHAS_SIBLING(aop))
        aop = cUNOPx(aop)->op_first;
    for (aop = OpSIBLING(aop); OpHAS_SIBLING(aop); aop = OpSIBLING(aop)) {
        /* skip the extra attributes->import() call implicitly added in
         * something like foo(my $x : bar)
         */
        if (   aop->op_type == OP_ENTERSUB
            && (aop->op_flags & OPf_WANT) == OPf_WANT_VOID
        )
            continue;
        list(aop);
        op_lvalue(aop, OP_ENTERSUB);
    }
    return entersubop;
}

/*
=for apidoc ck_entersub_args_proto

Performs the fixup of the arguments part of an C<entersub> op tree
based on a subroutine prototype.  This makes various modifications to
the argument ops, from applying context up to inserting C<refgen> ops,
and checking the number and syntactic types of arguments, as directed by
the prototype.  This is the standard treatment used on a subroutine call,
not marked with C<&>, where the callee can be identified at compile time
and has a prototype.

C<protosv> supplies the subroutine prototype to be applied to the call.
It may be a normal defined scalar, of which the string value will be used.
Alternatively, for convenience, it may be a subroutine object (a C<CV*>
that has been cast to C<SV*>) which has a prototype.  The prototype
supplied, in whichever form, does not need to match the actual callee
referenced by the op tree.

If the argument ops disagree with the prototype, for example by having
an unacceptable number of arguments, a valid op tree is returned anyway.
The error is reflected in the parser state, normally resulting in a single
exception at the top level of parsing which covers all the compilation
errors that occurred.  In the error message, the callee is referred to
by the name defined by the C<namegv> parameter.

=cut
*/

OP *
Perl_ck_entersub_args_proto(pTHX_ OP *entersubop, GV *namegv, SV *protosv)
{
    STRLEN proto_len;
    const char *proto, *proto_end;
    OP *aop, *prev, *cvop, *parent;
    int optional = 0;
    I32 arg = 0;
    I32 contextclass = 0;
    const char *e = NULL;
    PERL_ARGS_ASSERT_CK_ENTERSUB_ARGS_PROTO;
    if (SvTYPE(protosv) == SVt_PVCV ? !SvPOK(protosv) : !SvOK(protosv))
        Perl_croak(aTHX_ "panic: ck_entersub_args_proto CV with no proto, "
                   "flags=%lx", (unsigned long) SvFLAGS(protosv));
    if (SvTYPE(protosv) == SVt_PVCV)
         proto = CvPROTO(protosv), proto_len = CvPROTOLEN(protosv);
    else proto = SvPV(protosv, proto_len);
    proto = S_strip_spaces(aTHX_ proto, &proto_len);
    proto_end = proto + proto_len;
    parent = entersubop;
    aop = cUNOPx(entersubop)->op_first;
    if (!OpHAS_SIBLING(aop)) {
        parent = aop;
        aop = cUNOPx(aop)->op_first;
    }
    prev = aop;
    aop = OpSIBLING(aop);
    for (cvop = aop; OpHAS_SIBLING(cvop); cvop = OpSIBLING(cvop)) ;
    while (aop != cvop) {
        OP* o3 = aop;

        if (proto >= proto_end)
        {
            SV * const namesv = cv_name((CV *)namegv, NULL, 0);
            yyerror_pv(Perl_form(aTHX_ "Too many arguments for %" SVf,
                                        SVfARG(namesv)), SvUTF8(namesv));
            return entersubop;
        }

        switch (*proto) {
            case ';':
                optional = 1;
                proto++;
                continue;
            case '_':
                /* _ must be at the end */
                if (proto[1] && !memCHRs(";@%", proto[1]))
                    goto oops;
                /* FALLTHROUGH */
            case '$':
                proto++;
                arg++;
                scalar(aop);
                break;
            case '%':
            case '@':
                list(aop);
                arg++;
                break;
            case '&':
                proto++;
                arg++;
                if (    o3->op_type != OP_UNDEF
                    && o3->op_type != OP_ANONCODE
                    && (o3->op_type != OP_SREFGEN
                        || (  cUNOPx(cUNOPx(o3)->op_first)->op_first->op_type
                                != OP_ANONCODE
                            && cUNOPx(cUNOPx(o3)->op_first)->op_first->op_type
                                != OP_RV2CV)))
                    bad_type_gv(arg, namegv, o3,
                            arg == 1 ? "block or sub {}" : "sub {}");
                break;
            case '*':
                /* '*' allows any scalar type, including bareword */
                proto++;
                arg++;
                if (o3->op_type == OP_RV2GV)
                    goto wrapref;	/* autoconvert GLOB -> GLOBref */
                else if (o3->op_type == OP_CONST)
                    o3->op_private &= ~OPpCONST_STRICT;
                scalar(aop);
                break;
            case '+':
                proto++;
                arg++;
                if (o3->op_type == OP_RV2AV ||
                    o3->op_type == OP_PADAV ||
                    o3->op_type == OP_RV2HV ||
                    o3->op_type == OP_PADHV
                ) {
                    goto wrapref;
                }
                scalar(aop);
                break;
            case '[': case ']':
                goto oops;

            case '\\':
                proto++;
                arg++;
            again:
                switch (*proto++) {
                    case '[':
                        if (contextclass++ == 0) {
                            e = (char *) memchr(proto, ']', proto_end - proto);
                            if (!e || e == proto)
                                goto oops;
                        }
                        else
                            goto oops;
                        goto again;

                    case ']':
                        if (contextclass) {
                            const char *p = proto;
                            const char *const end = proto;
                            contextclass = 0;
                            while (*--p != '[')
                                /* \[$] accepts any scalar lvalue */
                                if (*p == '$'
                                 && Perl_op_lvalue_flags(aTHX_
                                     scalar(o3),
                                     OP_READ, /* not entersub */
                                     OP_LVALUE_NO_CROAK
                                    )) goto wrapref;
                            bad_type_gv(arg, namegv, o3,
                                    Perl_form(aTHX_ "one of %.*s",(int)(end - p), p));
                        } else
                            goto oops;
                        break;
                    case '*':
                        if (o3->op_type == OP_RV2GV)
                            goto wrapref;
                        if (!contextclass)
                            bad_type_gv(arg, namegv, o3, "symbol");
                        break;
                    case '&':
                        if (o3->op_type == OP_ENTERSUB
                         && !(o3->op_flags & OPf_STACKED))
                            goto wrapref;
                        if (!contextclass)
                            bad_type_gv(arg, namegv, o3, "subroutine");
                        break;
                    case '$':
                        if (o3->op_type == OP_RV2SV ||
                                o3->op_type == OP_PADSV ||
                                o3->op_type == OP_HELEM ||
                                o3->op_type == OP_AELEM)
                            goto wrapref;
                        if (!contextclass) {
                            /* \$ accepts any scalar lvalue */
                            if (Perl_op_lvalue_flags(aTHX_
                                    scalar(o3),
                                    OP_READ,  /* not entersub */
                                    OP_LVALUE_NO_CROAK
                               )) goto wrapref;
                            bad_type_gv(arg, namegv, o3, "scalar");
                        }
                        break;
                    case '@':
                        if (o3->op_type == OP_RV2AV ||
                                o3->op_type == OP_PADAV)
                        {
                            o3->op_flags &=~ OPf_PARENS;
                            goto wrapref;
                        }
                        if (!contextclass)
                            bad_type_gv(arg, namegv, o3, "array");
                        break;
                    case '%':
                        if (o3->op_type == OP_RV2HV ||
                                o3->op_type == OP_PADHV)
                        {
                            o3->op_flags &=~ OPf_PARENS;
                            goto wrapref;
                        }
                        if (!contextclass)
                            bad_type_gv(arg, namegv, o3, "hash");
                        break;
                    wrapref:
                            aop = S_op_sibling_newUNOP(aTHX_ parent, prev,
                                                OP_REFGEN, 0);
                        if (contextclass && e) {
                            proto = e + 1;
                            contextclass = 0;
                        }
                        break;
                    default: goto oops;
                }
                if (contextclass)
                    goto again;
                break;
            case ' ':
                proto++;
                continue;
            default:
            oops: {
                Perl_croak(aTHX_ "Malformed prototype for %" SVf ": %" SVf,
                                  SVfARG(cv_name((CV *)namegv, NULL, 0)),
                                  SVfARG(protosv));
            }
        }

        op_lvalue(aop, OP_ENTERSUB);
        prev = aop;
        aop = OpSIBLING(aop);
    }
    if (aop == cvop && *proto == '_') {
        /* generate an access to $_ */
        op_sibling_splice(parent, prev, 0, newDEFSVOP());
    }
    if (!optional && proto_end > proto &&
        (*proto != '@' && *proto != '%' && *proto != ';' && *proto != '_'))
    {
        SV * const namesv = cv_name((CV *)namegv, NULL, 0);
        yyerror_pv(Perl_form(aTHX_ "Not enough arguments for %" SVf,
                                    SVfARG(namesv)), SvUTF8(namesv));
    }
    return entersubop;
}

/*
=for apidoc ck_entersub_args_proto_or_list

Performs the fixup of the arguments part of an C<entersub> op tree either
based on a subroutine prototype or using default list-context processing.
This is the standard treatment used on a subroutine call, not marked
with C<&>, where the callee can be identified at compile time.

C<protosv> supplies the subroutine prototype to be applied to the call,
or indicates that there is no prototype.  It may be a normal scalar,
in which case if it is defined then the string value will be used
as a prototype, and if it is undefined then there is no prototype.
Alternatively, for convenience, it may be a subroutine object (a C<CV*>
that has been cast to C<SV*>), of which the prototype will be used if it
has one.  The prototype (or lack thereof) supplied, in whichever form,
does not need to match the actual callee referenced by the op tree.

If the argument ops disagree with the prototype, for example by having
an unacceptable number of arguments, a valid op tree is returned anyway.
The error is reflected in the parser state, normally resulting in a single
exception at the top level of parsing which covers all the compilation
errors that occurred.  In the error message, the callee is referred to
by the name defined by the C<namegv> parameter.

=cut
*/

OP *
Perl_ck_entersub_args_proto_or_list(pTHX_ OP *entersubop,
        GV *namegv, SV *protosv)
{
    PERL_ARGS_ASSERT_CK_ENTERSUB_ARGS_PROTO_OR_LIST;
    if (SvTYPE(protosv) == SVt_PVCV ? SvPOK(protosv) : SvOK(protosv))
        return ck_entersub_args_proto(entersubop, namegv, protosv);
    else
        return ck_entersub_args_list(entersubop);
}

OP *
Perl_ck_entersub_args_core(pTHX_ OP *entersubop, GV *namegv, SV *protosv)
{
    IV cvflags = SvIVX(protosv);
    int opnum = cvflags & 0xffff;
    OP *aop = cUNOPx(entersubop)->op_first;

    PERL_ARGS_ASSERT_CK_ENTERSUB_ARGS_CORE;

    if (!opnum) {
        OP *cvop;
        if (!OpHAS_SIBLING(aop))
            aop = cUNOPx(aop)->op_first;
        aop = OpSIBLING(aop);
        for (cvop = aop; OpSIBLING(cvop); cvop = OpSIBLING(cvop)) ;
        if (aop != cvop) {
            SV *namesv = cv_name((CV *)namegv, NULL, CV_NAME_NOTQUAL);
            yyerror_pv(Perl_form(aTHX_ "Too many arguments for %" SVf,
                SVfARG(namesv)), SvUTF8(namesv));
        }

        op_free(entersubop);
        switch(cvflags >> 16) {
        case 'F': return newSVOP(OP_CONST, 0,
                                        newSVpv(CopFILE(PL_curcop),0));
        case 'L': return newSVOP(
                           OP_CONST, 0,
                           Perl_newSVpvf(aTHX_
                             "%" LINE_Tf, CopLINE(PL_curcop)
                           )
                         );
        case 'P': return newSVOP(OP_CONST, 0,
                                   (PL_curstash
                                     ? newSVhek(HvNAME_HEK(PL_curstash))
                                     : &PL_sv_undef
                                   )
                                );
        }
        NOT_REACHED; /* NOTREACHED */
    }
    else {
        OP *prev, *cvop, *first, *parent;
        U32 flags = 0;

        parent = entersubop;
        if (!OpHAS_SIBLING(aop)) {
            parent = aop;
            aop = cUNOPx(aop)->op_first;
        }

        first = prev = aop;
        aop = OpSIBLING(aop);
        /* find last sibling */
        for (cvop = aop;
             OpHAS_SIBLING(cvop);
             prev = cvop, cvop = OpSIBLING(cvop))
            ;
        if (!(cvop->op_private & OPpENTERSUB_NOPAREN)
            /* Usually, OPf_SPECIAL on an op with no args means that it had
             * parens, but these have their own meaning for that flag: */
            && opnum != OP_VALUES && opnum != OP_KEYS && opnum != OP_EACH
            && opnum != OP_DELETE && opnum != OP_EXISTS)
                flags |= OPf_SPECIAL;
        /* excise cvop from end of sibling chain */
        op_sibling_splice(parent, prev, 1, NULL);
        op_free(cvop);
        if (aop == cvop) aop = NULL;

        /* detach remaining siblings from the first sibling, then
         * dispose of original optree */

        if (aop)
            op_sibling_splice(parent, first, -1, NULL);
        op_free(entersubop);

        if (cvflags == (OP_ENTEREVAL | (1<<16)))
            flags |= OPpEVAL_BYTES <<8;

        switch (PL_opargs[opnum] & OA_CLASS_MASK) {
        case OA_UNOP:
        case OA_BASEOP_OR_UNOP:
        case OA_FILESTATOP:
            if (!aop)
                return newOP(opnum,flags);       /* zero args */
            if (aop == prev)
                return newUNOP(opnum,flags,aop); /* one arg */
            /* too many args */
            /* FALLTHROUGH */
        case OA_BASEOP:
            if (aop) {
                SV *namesv;
                OP *nextop;

                namesv = cv_name((CV *)namegv, NULL, CV_NAME_NOTQUAL);
                yyerror_pv(Perl_form(aTHX_ "Too many arguments for %" SVf,
                    SVfARG(namesv)), SvUTF8(namesv));
                while (aop) {
                    nextop = OpSIBLING(aop);
                    op_free(aop);
                    aop = nextop;
                }

            }
            return opnum == OP_RUNCV
                ? newSVOP(OP_RUNCV, 0, &PL_sv_undef)
                : newOP(opnum,0);
        default:
            return op_convert_list(opnum,0,aop);
        }
    }
    NOT_REACHED; /* NOTREACHED */
    return entersubop;
}

/*
=for apidoc cv_get_call_checker_flags

Retrieves the function that will be used to fix up a call to C<cv>.
Specifically, the function is applied to an C<entersub> op tree for a
subroutine call, not marked with C<&>, where the callee can be identified
at compile time as C<cv>.

The C-level function pointer is returned in C<*ckfun_p>, an SV argument
for it is returned in C<*ckobj_p>, and control flags are returned in
C<*ckflags_p>.  The function is intended to be called in this manner:

 entersubop = (*ckfun_p)(aTHX_ entersubop, namegv, (*ckobj_p));

In this call, C<entersubop> is a pointer to the C<entersub> op,
which may be replaced by the check function, and C<namegv> supplies
the name that should be used by the check function to refer
to the callee of the C<entersub> op if it needs to emit any diagnostics.
It is permitted to apply the check function in non-standard situations,
such as to a call to a different subroutine or to a method call.

C<namegv> may not actually be a GV.  If the C<CALL_CHECKER_REQUIRE_GV>
bit is clear in C<*ckflags_p>, it is permitted to pass a CV or other SV
instead, anything that can be used as the first argument to L</cv_name>.
If the C<CALL_CHECKER_REQUIRE_GV> bit is set in C<*ckflags_p> then the
check function requires C<namegv> to be a genuine GV.

By default, the check function is
L<Perl_ck_entersub_args_proto_or_list|/ck_entersub_args_proto_or_list>,
the SV parameter is C<cv> itself, and the C<CALL_CHECKER_REQUIRE_GV>
flag is clear.  This implements standard prototype processing.  It can
be changed, for a particular subroutine, by L</cv_set_call_checker_flags>.

If the C<CALL_CHECKER_REQUIRE_GV> bit is set in C<gflags> then it
indicates that the caller only knows about the genuine GV version of
C<namegv>, and accordingly the corresponding bit will always be set in
C<*ckflags_p>, regardless of the check function's recorded requirements.
If the C<CALL_CHECKER_REQUIRE_GV> bit is clear in C<gflags> then it
indicates the caller knows about the possibility of passing something
other than a GV as C<namegv>, and accordingly the corresponding bit may
be either set or clear in C<*ckflags_p>, indicating the check function's
recorded requirements.

C<gflags> is a bitset passed into C<cv_get_call_checker_flags>, in which
only the C<CALL_CHECKER_REQUIRE_GV> bit currently has a defined meaning
(for which see above).  All other bits should be clear.

=for apidoc Amnh||CALL_CHECKER_REQUIRE_GV

=for apidoc cv_get_call_checker

The original form of L</cv_get_call_checker_flags>, which does not return
checker flags.  When using a checker function returned by this function,
it is only safe to call it with a genuine GV as its C<namegv> argument.

=cut
*/

void
Perl_cv_get_call_checker_flags(pTHX_ CV *cv, U32 gflags,
        Perl_call_checker *ckfun_p, SV **ckobj_p, U32 *ckflags_p)
{
    MAGIC *callmg;
    PERL_ARGS_ASSERT_CV_GET_CALL_CHECKER_FLAGS;
    PERL_UNUSED_CONTEXT;
    callmg = SvMAGICAL((SV*)cv) ? mg_find((SV*)cv, PERL_MAGIC_checkcall) : NULL;
    if (callmg) {
        *ckfun_p = DPTR2FPTR(Perl_call_checker, callmg->mg_ptr);
        *ckobj_p = callmg->mg_obj;
        *ckflags_p = (callmg->mg_flags | gflags) & MGf_REQUIRE_GV;
    } else {
        *ckfun_p = Perl_ck_entersub_args_proto_or_list;
        *ckobj_p = (SV*)cv;
        *ckflags_p = gflags & MGf_REQUIRE_GV;
    }
}

void
Perl_cv_get_call_checker(pTHX_ CV *cv, Perl_call_checker *ckfun_p, SV **ckobj_p)
{
    U32 ckflags;
    PERL_ARGS_ASSERT_CV_GET_CALL_CHECKER;
    PERL_UNUSED_CONTEXT;
    cv_get_call_checker_flags(cv, CALL_CHECKER_REQUIRE_GV, ckfun_p, ckobj_p,
        &ckflags);
}

/*
=for apidoc cv_set_call_checker_flags

Sets the function that will be used to fix up a call to C<cv>.
Specifically, the function is applied to an C<entersub> op tree for a
subroutine call, not marked with C<&>, where the callee can be identified
at compile time as C<cv>.

The C-level function pointer is supplied in C<ckfun>, an SV argument for
it is supplied in C<ckobj>, and control flags are supplied in C<ckflags>.
The function should be defined like this:

    STATIC OP * ckfun(pTHX_ OP *op, GV *namegv, SV *ckobj)

It is intended to be called in this manner:

    entersubop = ckfun(aTHX_ entersubop, namegv, ckobj);

In this call, C<entersubop> is a pointer to the C<entersub> op,
which may be replaced by the check function, and C<namegv> supplies
the name that should be used by the check function to refer
to the callee of the C<entersub> op if it needs to emit any diagnostics.
It is permitted to apply the check function in non-standard situations,
such as to a call to a different subroutine or to a method call.

C<namegv> may not actually be a GV.  For efficiency, perl may pass a
CV or other SV instead.  Whatever is passed can be used as the first
argument to L</cv_name>.  You can force perl to pass a GV by including
C<CALL_CHECKER_REQUIRE_GV> in the C<ckflags>.

C<ckflags> is a bitset, in which only the C<CALL_CHECKER_REQUIRE_GV>
bit currently has a defined meaning (for which see above).  All other
bits should be clear.

The current setting for a particular CV can be retrieved by
L</cv_get_call_checker_flags>.

=for apidoc cv_set_call_checker

The original form of L</cv_set_call_checker_flags>, which passes it the
C<CALL_CHECKER_REQUIRE_GV> flag for backward-compatibility.  The effect
of that flag setting is that the check function is guaranteed to get a
genuine GV as its C<namegv> argument.

=cut
*/

void
Perl_cv_set_call_checker(pTHX_ CV *cv, Perl_call_checker ckfun, SV *ckobj)
{
    PERL_ARGS_ASSERT_CV_SET_CALL_CHECKER;
    cv_set_call_checker_flags(cv, ckfun, ckobj, CALL_CHECKER_REQUIRE_GV);
}

void
Perl_cv_set_call_checker_flags(pTHX_ CV *cv, Perl_call_checker ckfun,
                                     SV *ckobj, U32 ckflags)
{
    PERL_ARGS_ASSERT_CV_SET_CALL_CHECKER_FLAGS;
    if (ckfun == Perl_ck_entersub_args_proto_or_list && ckobj == (SV*)cv) {
        if (SvMAGICAL((SV*)cv))
            mg_free_type((SV*)cv, PERL_MAGIC_checkcall);
    } else {
        MAGIC *callmg;
        sv_magic((SV*)cv, &PL_sv_undef, PERL_MAGIC_checkcall, NULL, 0);
        callmg = mg_find((SV*)cv, PERL_MAGIC_checkcall);
        assert(callmg);
        if (callmg->mg_flags & MGf_REFCOUNTED) {
            SvREFCNT_dec(callmg->mg_obj);
            callmg->mg_flags &= ~MGf_REFCOUNTED;
        }
        callmg->mg_ptr = FPTR2DPTR(char *, ckfun);
        callmg->mg_obj = ckobj;
        if (ckobj != (SV*)cv) {
            SvREFCNT_inc_simple_void_NN(ckobj);
            callmg->mg_flags |= MGf_REFCOUNTED;
        }
        callmg->mg_flags = (callmg->mg_flags &~ MGf_REQUIRE_GV)
                         | (U8)(ckflags & MGf_REQUIRE_GV) | MGf_COPY;
    }
}

static void
S_entersub_alloc_targ(pTHX_ OP * const o)
{
    o->op_targ = pad_alloc(OP_ENTERSUB, SVs_PADTMP);
    o->op_private |= OPpENTERSUB_HASTARG;
}

OP *
Perl_ck_subr(pTHX_ OP *o)
{
    OP *aop, *cvop;
    CV *cv;
    GV *namegv;
    SV **const_class = NULL;

    PERL_ARGS_ASSERT_CK_SUBR;

    aop = cUNOPx(o)->op_first;
    if (!OpHAS_SIBLING(aop))
        aop = cUNOPx(aop)->op_first;
    aop = OpSIBLING(aop);
    for (cvop = aop; OpHAS_SIBLING(cvop); cvop = OpSIBLING(cvop)) ;
    cv = rv2cv_op_cv(cvop, RV2CVOPCV_MARK_EARLY);
    namegv = cv ? (GV*)rv2cv_op_cv(cvop, RV2CVOPCV_MAYBE_NAME_GV) : NULL;

    o->op_private &= ~1;
    o->op_private |= (PL_hints & HINT_STRICT_REFS);
    if (PERLDB_SUB && PL_curstash != PL_debstash)
        o->op_private |= OPpENTERSUB_DB;
    switch (cvop->op_type) {
        case OP_RV2CV:
            o->op_private |= (cvop->op_private & OPpENTERSUB_AMPER);
            op_null(cvop);
            break;
        case OP_METHOD:
        case OP_METHOD_NAMED:
        case OP_METHOD_SUPER:
        case OP_METHOD_REDIR:
        case OP_METHOD_REDIR_SUPER:
            o->op_flags |= OPf_REF;
            if (aop->op_type == OP_CONST) {
                aop->op_private &= ~OPpCONST_STRICT;
                const_class = &cSVOPx(aop)->op_sv;
            }
            else if (aop->op_type == OP_LIST) {
                OP * const sib = OpSIBLING(cUNOPx(aop)->op_first);
                if (sib && sib->op_type == OP_CONST) {
                    sib->op_private &= ~OPpCONST_STRICT;
                    const_class = &cSVOPx(sib)->op_sv;
                }
            }
            /* make class name a shared cow string to speedup method calls */
            /* constant string might be replaced with object, f.e. bigint */
            if (const_class && SvPOK(*const_class)) {
                STRLEN len;
                const char* str = SvPV(*const_class, len);
                if (len) {
                    SV* const shared = newSVpvn_share(
                        str, SvUTF8(*const_class)
                                    ? -(SSize_t)len : (SSize_t)len,
                        0
                    );
                    if (SvREADONLY(*const_class))
                        SvREADONLY_on(shared);
                    SvREFCNT_dec(*const_class);
                    *const_class = shared;
                }
            }
            break;
    }

    if (!cv) {
        S_entersub_alloc_targ(aTHX_ o);
        return ck_entersub_args_list(o);
    } else {
        Perl_call_checker ckfun;
        SV *ckobj;
        U32 ckflags;
        cv_get_call_checker_flags(cv, 0, &ckfun, &ckobj, &ckflags);
        if (CvISXSUB(cv) || !CvROOT(cv))
            S_entersub_alloc_targ(aTHX_ o);
        if (!namegv) {
            /* The original call checker API guarantees that a GV will
               be provided with the right name.  So, if the old API was
               used (or the REQUIRE_GV flag was passed), we have to reify
               the CV’s GV, unless this is an anonymous sub.  This is not
               ideal for lexical subs, as its stringification will include
               the package.  But it is the best we can do.  */
            if (ckflags & CALL_CHECKER_REQUIRE_GV) {
                if (!CvANON(cv) && (!CvNAMED(cv) || CvNAME_HEK(cv)))
                    namegv = CvGV(cv);
            }
            else namegv = MUTABLE_GV(cv);
            /* After a syntax error in a lexical sub, the cv that
               rv2cv_op_cv returns may be a nameless stub. */
            if (!namegv) return ck_entersub_args_list(o);

        }
        return ckfun(aTHX_ o, namegv, ckobj);
    }
}

OP *
Perl_ck_svconst(pTHX_ OP *o)
{
    SV * const sv = cSVOPo->op_sv;
    PERL_ARGS_ASSERT_CK_SVCONST;
    PERL_UNUSED_CONTEXT;
#ifdef PERL_COPY_ON_WRITE
    /* Since the read-only flag may be used to protect a string buffer, we
       cannot do copy-on-write with existing read-only scalars that are not
       already copy-on-write scalars.  To allow $_ = "hello" to do COW with
       that constant, mark the constant as COWable here, if it is not
       already read-only. */
    if (!SvREADONLY(sv) && !SvIsCOW(sv) && SvCANCOW(sv)) {
        SvIsCOW_on(sv);
        CowREFCNT(sv) = 0;
# ifdef PERL_DEBUG_READONLY_COW
        sv_buf_to_ro(sv);
# endif
    }
#endif
    SvREADONLY_on(sv);
    return o;
}

OP *
Perl_ck_trunc(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_TRUNC;

    if (o->op_flags & OPf_KIDS) {
        SVOP *kid = cSVOPx(cUNOPo->op_first);

        if (kid->op_type == OP_NULL)
            kid = cSVOPx(OpSIBLING(kid));
        if (kid && kid->op_type == OP_CONST &&
            (kid->op_private & OPpCONST_BARE) &&
            !kid->op_folded)
        {
            o->op_flags |= OPf_SPECIAL;
            kid->op_private &= ~OPpCONST_STRICT;
            if (!FEATURE_BAREWORD_FILEHANDLES_IS_ENABLED) {
                no_bareword_filehandle(SvPVX(cSVOPx_sv(kid)));
            }
        }
    }
    return ck_fun(o);
}

OP *
Perl_ck_substr(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_SUBSTR;

    o = ck_fun(o);
    if ((o->op_flags & OPf_KIDS) && (o->op_private == 4)) {
        OP *kid = cLISTOPo->op_first;

        if (kid->op_type == OP_NULL)
            kid = OpSIBLING(kid);
        if (kid)
            /* Historically, substr(delete $foo{bar},...) has been allowed
               with 4-arg substr.  Keep it working by applying entersub
               lvalue context.  */
            op_lvalue(kid, OP_ENTERSUB);

    }
    return o;
}

OP *
Perl_ck_tell(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_TELL;
    o = ck_fun(o);
    if (o->op_flags & OPf_KIDS) {
     OP *kid = cLISTOPo->op_first;
     if (kid->op_type == OP_NULL && OpHAS_SIBLING(kid)) kid = OpSIBLING(kid);
     if (kid->op_type == OP_RV2GV) kid->op_private |= OPpALLOW_FAKE;
    }
    return o;
}

PERL_STATIC_INLINE OP *
S_last_non_null_kid(OP *o) {
    OP *last = NULL;
    if (cUNOPo->op_flags & OPf_KIDS) {
        OP *k = cLISTOPo->op_first;
        while (k) {
            if (k->op_type != OP_NULL) {
                last = k;
            }
            k = OpSIBLING(k);
        }
    }

    return last;
}

OP *
Perl_ck_each(pTHX_ OP *o)
{
    OP *kid = o->op_flags & OPf_KIDS ? cUNOPo->op_first : NULL;
    const unsigned orig_type  = o->op_type;

    PERL_ARGS_ASSERT_CK_EACH;

    if (kid) {
        switch (kid->op_type) {
            case OP_PADHV:
                break;

            case OP_RV2HV:
                /* Catch out an anonhash here, since the behaviour might be
                 * confusing.
                 *
                 * The typical tree is:
                 *
                 *     rv2hv
                 *         scope
                 *             null
                 *             anonhash
                 *
                 * If the contents of the block is more complex you might get:
                 *
                 *     rv2hv
                 *         leave
                 *             enter
                 *             ...
                 *             anonhash
                 *
                 * Similarly for the anonlist version below.
                 */
                if (orig_type == OP_EACH &&
                    ckWARN(WARN_SYNTAX) &&
                    (cUNOPx(kid)->op_flags & OPf_KIDS) &&
                    ( cUNOPx(kid)->op_first->op_type == OP_SCOPE ||
                      cUNOPx(kid)->op_first->op_type == OP_LEAVE) &&
                    (cUNOPx(kid)->op_first->op_flags & OPf_KIDS)) {
                    /* look for last non-null kid, since we might have:
                       each %{ some code ; +{ anon hash } }
                    */
                    OP *k = S_last_non_null_kid(cUNOPx(kid)->op_first);
                    if (k && k->op_type == OP_ANONHASH) {
                        /* diag_listed_as: each on anonymous %s will always start from the beginning */
                        Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "each on anonymous hash will always start from the beginning");
                    }
                }
                break;
            case OP_RV2AV:
                if (orig_type == OP_EACH &&
                    ckWARN(WARN_SYNTAX) &&
                    (cUNOPx(kid)->op_flags & OPf_KIDS) &&
                    (cUNOPx(kid)->op_first->op_type == OP_SCOPE ||
                     cUNOPx(kid)->op_first->op_type == OP_LEAVE) &&
                    (cUNOPx(kid)->op_first->op_flags & OPf_KIDS)) {
                    OP *k = S_last_non_null_kid(cUNOPx(kid)->op_first);
                    if (k && k->op_type == OP_ANONLIST) {
                        /* diag_listed_as: each on anonymous %s will always start from the beginning */
                        Perl_warner(aTHX_ packWARN(WARN_SYNTAX), "each on anonymous array will always start from the beginning");
                    }
                }
                /* FALLTHROUGH */
            case OP_PADAV:
                OpTYPE_set(o, orig_type == OP_EACH ? OP_AEACH
                            : orig_type == OP_KEYS ? OP_AKEYS
                            :                        OP_AVALUES);
                break;
            case OP_CONST:
                if (kid->op_private == OPpCONST_BARE
                 || !SvROK(cSVOPx_sv(kid))
                 || (  SvTYPE(SvRV(cSVOPx_sv(kid))) != SVt_PVAV
                    && SvTYPE(SvRV(cSVOPx_sv(kid))) != SVt_PVHV  )
                   )
                    goto bad;
                /* FALLTHROUGH */
            default:
                qerror(Perl_mess(aTHX_
                    "Experimental %s on scalar is now forbidden",
                     PL_op_desc[orig_type]));
               bad:
                bad_type_pv(1, "hash or array", o, kid);
                return o;
        }
    }
    return ck_fun(o);
}

OP *
Perl_ck_length(pTHX_ OP *o)
{
    PERL_ARGS_ASSERT_CK_LENGTH;

    o = ck_fun(o);

    if (ckWARN(WARN_SYNTAX)) {
        const OP *kid = o->op_flags & OPf_KIDS ? cLISTOPo->op_first : NULL;

        if (kid) {
            SV *name = NULL;
            const bool hash = kid->op_type == OP_PADHV
                           || kid->op_type == OP_RV2HV;
            switch (kid->op_type) {
                case OP_PADHV:
                case OP_PADAV:
                case OP_RV2HV:
                case OP_RV2AV:
                    name = op_varname(kid);
                    break;
                default:
                    return o;
            }
            if (name)
                Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                    "length() used on %" SVf " (did you mean \"scalar(%s%" SVf
                    ")\"?)",
                    SVfARG(name), hash ? "keys " : "", SVfARG(name)
                );
            else if (hash)
     /* diag_listed_as: length() used on %s (did you mean "scalar(%s)"?) */
                Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                    "length() used on %%hash (did you mean \"scalar(keys %%hash)\"?)");
            else
     /* diag_listed_as: length() used on %s (did you mean "scalar(%s)"?) */
                Perl_warner(aTHX_ packWARN(WARN_SYNTAX),
                    "length() used on @array (did you mean \"scalar(@array)\"?)");
        }
    }

    return o;
}


OP *
Perl_ck_isa(pTHX_ OP *o)
{
    OP *classop = cBINOPo->op_last;

    PERL_ARGS_ASSERT_CK_ISA;

    /* Convert barename into PV */
    if(classop->op_type == OP_CONST && classop->op_private & OPpCONST_BARE) {
        /* TODO: Optionally convert package to raw HV here */
        classop->op_private &= ~(OPpCONST_BARE|OPpCONST_STRICT);
    }

    return o;
}


/* Check for in place reverse and sort assignments like "@a = reverse @a"
   and modify the optree to make them work inplace */

STATIC void
S_inplace_aassign(pTHX_ OP *o) {

    OP *modop, *modop_pushmark;
    OP *oright;
    OP *oleft, *oleft_pushmark;

    PERL_ARGS_ASSERT_INPLACE_AASSIGN;

    assert((o->op_flags & OPf_WANT) == OPf_WANT_VOID);

    assert(cUNOPo->op_first->op_type == OP_NULL);
    modop_pushmark = cUNOPx(cUNOPo->op_first)->op_first;
    assert(modop_pushmark->op_type == OP_PUSHMARK);
    modop = OpSIBLING(modop_pushmark);

    if (modop->op_type != OP_SORT && modop->op_type != OP_REVERSE)
        return;

    /* no other operation except sort/reverse */
    if (OpHAS_SIBLING(modop))
        return;

    assert(cUNOPx(modop)->op_first->op_type == OP_PUSHMARK);
    if (!(oright = OpSIBLING(cUNOPx(modop)->op_first))) return;

    if (modop->op_flags & OPf_STACKED) {
        /* skip sort subroutine/block */
        assert(oright->op_type == OP_NULL);
        oright = OpSIBLING(oright);
    }

    assert(OpSIBLING(cUNOPo->op_first)->op_type == OP_NULL);
    oleft_pushmark = cUNOPx(OpSIBLING(cUNOPo->op_first))->op_first;
    assert(oleft_pushmark->op_type == OP_PUSHMARK);
    oleft = OpSIBLING(oleft_pushmark);

    /* Check the lhs is an array */
    if (!oleft ||
        (oleft->op_type != OP_RV2AV && oleft->op_type != OP_PADAV)
        || OpHAS_SIBLING(oleft)
        || (oleft->op_private & OPpLVAL_INTRO)
    )
        return;

    /* Only one thing on the rhs */
    if (OpHAS_SIBLING(oright))
        return;

    /* check the array is the same on both sides */
    if (oleft->op_type == OP_RV2AV) {
        if (oright->op_type != OP_RV2AV
            || !cUNOPx(oright)->op_first
            || cUNOPx(oright)->op_first->op_type != OP_GV
            || cUNOPx(oleft )->op_first->op_type != OP_GV
            || cGVOPx_gv(cUNOPx(oleft)->op_first) !=
               cGVOPx_gv(cUNOPx(oright)->op_first)
        )
            return;
    }
    else if (oright->op_type != OP_PADAV
        || oright->op_targ != oleft->op_targ
    )
        return;

    /* This actually is an inplace assignment */

    modop->op_private |= OPpSORT_INPLACE;

    /* transfer MODishness etc from LHS arg to RHS arg */
    oright->op_flags = oleft->op_flags;

    /* remove the aassign op and the lhs */
    op_null(o);
    op_null(oleft_pushmark);
    if (oleft->op_type == OP_RV2AV && cUNOPx(oleft)->op_first)
        op_null(cUNOPx(oleft)->op_first);
    op_null(oleft);
}


/*
=for apidoc_section $custom

=for apidoc Perl_custom_op_xop
Return the XOP structure for a given custom op.  This macro should be
considered internal to C<OP_NAME> and the other access macros: use them instead.
This macro does call a function.  Prior
to 5.19.6, this was implemented as a
function.

=cut
*/


/* use PERL_MAGIC_ext to call a function to free the xop structure when
 * freeing PL_custom_ops */

static int
custom_op_register_free(pTHX_ SV *sv, MAGIC *mg)
{
    XOP *xop;

    PERL_UNUSED_ARG(mg);
    xop = INT2PTR(XOP *, SvIV(sv));
    Safefree(xop->xop_name);
    Safefree(xop->xop_desc);
    Safefree(xop);
    return 0;
}


static const MGVTBL custom_op_register_vtbl = {
    0,                          /* get */
    0,                          /* set */
    0,                          /* len */
    0,                          /* clear */
    custom_op_register_free,     /* free */
    0,                          /* copy */
    0,                          /* dup */
#ifdef MGf_LOCAL
    0,                          /* local */
#endif
};


XOPRETANY
Perl_custom_op_get_field(pTHX_ const OP *o, const xop_flags_enum field)
{
    SV *keysv;
    HE *he = NULL;
    XOP *xop;

    static const XOP xop_null = { 0, 0, 0, 0, 0 };

    PERL_ARGS_ASSERT_CUSTOM_OP_GET_FIELD;
    assert(o->op_type == OP_CUSTOM);

    /* This is wrong. It assumes a function pointer can be cast to IV,
     * which isn't guaranteed, but this is what the old custom OP code
     * did. In principle it should be safer to Copy the bytes of the
     * pointer into a PV: since the new interface is hidden behind
     * functions, this can be changed later if necessary.  */
    /* Change custom_op_xop if this ever happens */
    keysv = sv_2mortal(newSViv(PTR2IV(o->op_ppaddr)));

    if (PL_custom_ops)
        he = hv_fetch_ent(PL_custom_ops, keysv, 0, 0);

    /* See if the op isn't registered, but its name *is* registered.
     * That implies someone is using the pre-5.14 API,where only name and
     * description could be registered. If so, fake up a real
     * registration.
     * We only check for an existing name, and assume no one will have
     * just registered a desc */
    if (!he && PL_custom_op_names &&
        (he = hv_fetch_ent(PL_custom_op_names, keysv, 0, 0))
    ) {
        const char *pv;
        STRLEN l;

        /* XXX does all this need to be shared mem? */
        Newxz(xop, 1, XOP);
        pv = SvPV(HeVAL(he), l);
        XopENTRY_set(xop, xop_name, savepvn(pv, l));
        if (PL_custom_op_descs &&
            (he = hv_fetch_ent(PL_custom_op_descs, keysv, 0, 0))
        ) {
            pv = SvPV(HeVAL(he), l);
            XopENTRY_set(xop, xop_desc, savepvn(pv, l));
        }
        Perl_custom_op_register(aTHX_ o->op_ppaddr, xop);
        he = hv_fetch_ent(PL_custom_ops, keysv, 0, 0);
        /* add magic to the SV so that the xop struct (pointed to by
         * SvIV(sv)) is freed. Normally a static xop is registered, but
         * for this backcompat hack, we've alloced one */
        (void)sv_magicext(HeVAL(he), NULL, PERL_MAGIC_ext,
                &custom_op_register_vtbl, NULL, 0);

    }
    else {
        if (!he)
            xop = (XOP *)&xop_null;
        else
            xop = INT2PTR(XOP *, SvIV(HeVAL(he)));
    }

    {
        XOPRETANY any;
        if(field == XOPe_xop_ptr) {
            any.xop_ptr = xop;
        } else {
            const U32 flags = XopFLAGS(xop);
            if(flags & field) {
                switch(field) {
                case XOPe_xop_name:
                    any.xop_name = xop->xop_name;
                    break;
                case XOPe_xop_desc:
                    any.xop_desc = xop->xop_desc;
                    break;
                case XOPe_xop_class:
                    any.xop_class = xop->xop_class;
                    break;
                case XOPe_xop_peep:
                    any.xop_peep = xop->xop_peep;
                    break;
                default:
                  field_panic:
                    Perl_croak(aTHX_
                        "panic: custom_op_get_field(): invalid field %d\n",
                        (int)field);
                    break;
                }
            } else {
                switch(field) {
                case XOPe_xop_name:
                    any.xop_name = XOPd_xop_name;
                    break;
                case XOPe_xop_desc:
                    any.xop_desc = XOPd_xop_desc;
                    break;
                case XOPe_xop_class:
                    any.xop_class = XOPd_xop_class;
                    break;
                case XOPe_xop_peep:
                    any.xop_peep = XOPd_xop_peep;
                    break;
                default:
                    goto field_panic;
                    break;
                }
            }
        }
        return any;
    }
}

/*
=for apidoc custom_op_register
Register a custom op.  See L<perlguts/"Custom Operators">.

=cut
*/

void
Perl_custom_op_register(pTHX_ Perl_ppaddr_t ppaddr, const XOP *xop)
{
    SV *keysv;

    PERL_ARGS_ASSERT_CUSTOM_OP_REGISTER;

    /* see the comment in custom_op_xop */
    keysv = sv_2mortal(newSViv(PTR2IV(ppaddr)));

    if (!PL_custom_ops)
        PL_custom_ops = newHV();

    if (!hv_store_ent(PL_custom_ops, keysv, newSViv(PTR2IV(xop)), 0))
        Perl_croak(aTHX_ "panic: can't register custom OP %s", xop->xop_name);
}

/*

=for apidoc core_prototype

This function assigns the prototype of the named core function to C<sv>, or
to a new mortal SV if C<sv> is C<NULL>.  It returns the modified C<sv>, or
C<NULL> if the core function has no prototype.  C<code> is a code as returned
by C<keyword()>.  It must not be equal to 0.

=cut
*/

SV *
Perl_core_prototype(pTHX_ SV *sv, const char *name, const int code,
                          int * const opnum)
{
    int i = 0, n = 0, seen_question = 0, defgv = 0;
    I32 oa;
#define MAX_ARGS_OP ((sizeof(I32) - 1) * 2)
    char str[ MAX_ARGS_OP * 2 + 2 ]; /* One ';', one '\0' */
    bool nullret = FALSE;

    PERL_ARGS_ASSERT_CORE_PROTOTYPE;

    assert (code);

    if (!sv) sv = sv_newmortal();

#define retsetpvs(x,y) sv_setpvs(sv, x); if(opnum) *opnum=(y); return sv

    switch (code < 0 ? -code : code) {
    case KEY_and   : case KEY_chop: case KEY_chomp:
    case KEY_cmp   : case KEY_defined: case KEY_delete: case KEY_exec  :
    case KEY_exists: case KEY_eq     : case KEY_ge    : case KEY_goto  :
    case KEY_grep  : case KEY_gt     : case KEY_last  : case KEY_le    :
    case KEY_lt    : case KEY_map    : case KEY_ne    : case KEY_next  :
    case KEY_or    : case KEY_print  : case KEY_printf: case KEY_qr    :
    case KEY_redo  : case KEY_require: case KEY_return: case KEY_say   :
    case KEY_select: case KEY_sort   : case KEY_split : case KEY_system:
    case KEY_x     : case KEY_xor    :
        if (!opnum) return NULL; nullret = TRUE; goto findopnum;
    case KEY_glob:    retsetpvs("_;", OP_GLOB);
    case KEY_keys:    retsetpvs("\\[%@]", OP_KEYS);
    case KEY_values:  retsetpvs("\\[%@]", OP_VALUES);
    case KEY_each:    retsetpvs("\\[%@]", OP_EACH);
    case KEY_pos:     retsetpvs(";\\[$*]", OP_POS);
    case KEY___FILE__: case KEY___LINE__: case KEY___PACKAGE__:
        retsetpvs("", 0);
    case KEY_evalbytes:
        name = "entereval"; break;
    case KEY_readpipe:
        name = "backtick";
    }

#undef retsetpvs

  findopnum:
    while (i < MAXO) {	/* The slow way. */
        if (strEQ(name, PL_op_name[i])
            || strEQ(name, PL_op_desc[i]))
        {
            if (nullret) { assert(opnum); *opnum = i; return NULL; }
            goto found;
        }
        i++;
    }
    return NULL;
  found:
    defgv = PL_opargs[i] & OA_DEFGV;
    oa = PL_opargs[i] >> OASHIFT;
    while (oa) {
        if (oa & OA_OPTIONAL && !seen_question && (
              !defgv || (oa & (OA_OPTIONAL - 1)) == OA_FILEREF
        )) {
            seen_question = 1;
            str[n++] = ';';
        }
        if ((oa & (OA_OPTIONAL - 1)) >= OA_AVREF
            && (oa & (OA_OPTIONAL - 1)) <= OA_SCALARREF
            /* But globs are already references (kinda) */
            && (oa & (OA_OPTIONAL - 1)) != OA_FILEREF
        ) {
            str[n++] = '\\';
        }
        if ((oa & (OA_OPTIONAL - 1)) == OA_SCALARREF
         && !scalar_mod_type(NULL, i)) {
            str[n++] = '[';
            str[n++] = '$';
            str[n++] = '@';
            str[n++] = '%';
            if (i == OP_LOCK || i == OP_UNDEF) str[n++] = '&';
            str[n++] = '*';
            str[n++] = ']';
        }
        else str[n++] = ("?$@@%&*$")[oa & (OA_OPTIONAL - 1)];
        if (oa & OA_OPTIONAL && defgv && str[n-1] == '$') {
            str[n-1] = '_'; defgv = 0;
        }
        oa = oa >> 4;
    }
    if (code == -KEY_not || code == -KEY_getprotobynumber) str[n++] = ';';
    str[n++] = '\0';
    sv_setpvn(sv, str, n - 1);
    if (opnum) *opnum = i;
    return sv;
}

OP *
Perl_coresub_op(pTHX_ SV * const coreargssv, const int code,
                      const int opnum)
{
    OP * const argop = (opnum == OP_SELECT && code) ? NULL :
                                        newSVOP(OP_COREARGS,0,coreargssv);
    OP *o;

    PERL_ARGS_ASSERT_CORESUB_OP;

    switch(opnum) {
    case 0:
        return op_append_elem(OP_LINESEQ,
                       argop,
                       newSLICEOP(0,
                                  newSVOP(OP_CONST, 0, newSViv(-code % 3)),
                                  newOP(OP_CALLER,0)
                       )
               );
    case OP_EACH:
    case OP_KEYS:
    case OP_VALUES:
        o = newUNOP(OP_AVHVSWITCH,0,argop);
        o->op_private = opnum-OP_EACH;
        return o;
    case OP_SELECT: /* which represents OP_SSELECT as well */
        if (code)
            return newCONDOP(
                         0,
                         newBINOP(OP_GT, 0,
                                  newAVREF(newGVOP(OP_GV, 0, PL_defgv)),
                                  newSVOP(OP_CONST, 0, newSVuv(1))
                                 ),
                         coresub_op(newSVuv((UV)OP_SSELECT), 0,
                                    OP_SSELECT),
                         coresub_op(coreargssv, 0, OP_SELECT)
                   );
        /* FALLTHROUGH */
    default:
        switch (PL_opargs[opnum] & OA_CLASS_MASK) {
        case OA_BASEOP:
            return op_append_elem(
                        OP_LINESEQ, argop,
                        newOP(opnum,
                              opnum == OP_WANTARRAY || opnum == OP_RUNCV
                                ? OPpOFFBYONE << 8 : 0)
                   );
        case OA_BASEOP_OR_UNOP:
            if (opnum == OP_ENTEREVAL) {
                o = newUNOP(OP_ENTEREVAL,OPpEVAL_COPHH<<8,argop);
                if (code == -KEY_evalbytes) o->op_private |= OPpEVAL_BYTES;
            }
            else o = newUNOP(opnum,0,argop);
            if (opnum == OP_CALLER) o->op_private |= OPpOFFBYONE;
            else {
          onearg:
              if (is_handle_constructor(o, 1))
                argop->op_private |= OPpCOREARGS_DEREF1;
              if (scalar_mod_type(NULL, opnum))
                argop->op_private |= OPpCOREARGS_SCALARMOD;
            }
            return o;
        default:
            o = op_convert_list(opnum,OPf_SPECIAL*(opnum == OP_GLOB),argop);
            if (is_handle_constructor(o, 2))
                argop->op_private |= OPpCOREARGS_DEREF2;
            if (opnum == OP_SUBSTR) {
                o->op_private |= OPpMAYBE_LVSUB;
                return o;
            }
            else goto onearg;
        }
    }
}

void
Perl_report_redefined_cv(pTHX_ const SV *name, const CV *old_cv,
                               SV * const *new_const_svp)
{
    const char *hvname;
    bool is_const = cBOOL(CvCONST(old_cv));
    SV *old_const_sv = is_const ? cv_const_sv_or_av(old_cv) : NULL;

    PERL_ARGS_ASSERT_REPORT_REDEFINED_CV;

    if (is_const && new_const_svp && old_const_sv == *new_const_svp)
        return;
        /* They are 2 constant subroutines generated from
           the same constant. This probably means that
           they are really the "same" proxy subroutine
           instantiated in 2 places. Most likely this is
           when a constant is exported twice.  Don't warn.
        */
    if (
        (ckWARN(WARN_REDEFINE)
         && !(
                CvGV(old_cv) && GvSTASH(CvGV(old_cv))
             && HvNAMELEN(GvSTASH(CvGV(old_cv))) == 7
             && (hvname = HvNAME(GvSTASH(CvGV(old_cv))),
                 strEQ(hvname, "autouse"))
             )
        )
     || (is_const
         && ckWARN_d(WARN_REDEFINE)
         && (!new_const_svp ||
             !*new_const_svp ||
             !old_const_sv ||
             SvTYPE(old_const_sv) == SVt_PVAV ||
             SvTYPE(*new_const_svp) == SVt_PVAV ||
             sv_cmp(old_const_sv, *new_const_svp))
        )
        ) {
        Perl_warner(aTHX_ packWARN(WARN_REDEFINE),
                          is_const
                            ? "Constant subroutine %" SVf " redefined"
                            : CvIsMETHOD(old_cv)
                              ? "Method %" SVf " redefined"
                              : "Subroutine %" SVf " redefined",
                          SVfARG(name));
    }
}

/*
=for apidoc_section $hook

These functions provide convenient and thread-safe means of manipulating
hook variables.

=cut
*/

/*
=for apidoc wrap_op_checker

Puts a C function into the chain of check functions for a specified op
type.  This is the preferred way to manipulate the L</PL_check> array.
C<opcode> specifies which type of op is to be affected.  C<new_checker>
is a pointer to the C function that is to be added to that opcode's
check chain, and C<old_checker_p> points to the storage location where a
pointer to the next function in the chain will be stored.  The value of
C<new_checker> is written into the L</PL_check> array, while the value
previously stored there is written to C<*old_checker_p>.

L</PL_check> is global to an entire process, and a module wishing to
hook op checking may find itself invoked more than once per process,
typically in different threads.  To handle that situation, this function
is idempotent.  The location C<*old_checker_p> must initially (once
per process) contain a null pointer.  A C variable of static duration
(declared at file scope, typically also marked C<static> to give
it internal linkage) will be implicitly initialised appropriately,
if it does not have an explicit initialiser.  This function will only
actually modify the check chain if it finds C<*old_checker_p> to be null.
This function is also thread safe on the small scale.  It uses appropriate
locking to avoid race conditions in accessing L</PL_check>.

When this function is called, the function referenced by C<new_checker>
must be ready to be called, except for C<*old_checker_p> being unfilled.
In a threading situation, C<new_checker> may be called immediately,
even before this function has returned.  C<*old_checker_p> will always
be appropriately set before C<new_checker> is called.  If C<new_checker>
decides not to do anything special with an op that it is given (which
is the usual case for most uses of op check hooking), it must chain the
check function referenced by C<*old_checker_p>.

Taken all together, XS code to hook an op checker should typically look
something like this:

    static Perl_check_t nxck_frob;
    static OP *myck_frob(pTHX_ OP *op) {
        ...
        op = nxck_frob(aTHX_ op);
        ...
        return op;
    }
    BOOT:
        wrap_op_checker(OP_FROB, myck_frob, &nxck_frob);

If you want to influence compilation of calls to a specific subroutine,
then use L</cv_set_call_checker_flags> rather than hooking checking of
all C<entersub> ops.

=cut
*/

void
Perl_wrap_op_checker(pTHX_ Optype opcode,
    Perl_check_t new_checker, Perl_check_t *old_checker_p)
{

    PERL_UNUSED_CONTEXT;
    PERL_ARGS_ASSERT_WRAP_OP_CHECKER;
    if (*old_checker_p) return;
    OP_CHECK_MUTEX_LOCK;
    if (!*old_checker_p) {
        *old_checker_p = PL_check[opcode];
        PL_check[opcode] = new_checker;
    }
    OP_CHECK_MUTEX_UNLOCK;
}

#include "XSUB.h"

/* Efficient sub that returns a constant scalar value. */
static void
const_sv_xsub(pTHX_ CV* cv)
{
    dXSARGS;
    SV *const sv = MUTABLE_SV(XSANY.any_ptr);
    PERL_UNUSED_ARG(items);
    if (!sv) {
        XSRETURN(0);
    }
    EXTEND(sp, 1);
    ST(0) = sv;
    XSRETURN(1);
}

static void
const_av_xsub(pTHX_ CV* cv)
{
    dXSARGS;
    AV * const av = MUTABLE_AV(XSANY.any_ptr);
    SP -= items;
    assert(av);
#ifndef DEBUGGING
    if (!av) {
        XSRETURN(0);
    }
#endif
    if (SvRMAGICAL(av))
        Perl_croak(aTHX_ "Magical list constants are not supported");
    if (GIMME_V != G_LIST) {
        EXTEND(SP, 1);
        ST(0) = sv_2mortal(newSViv((IV)AvFILLp(av)+1));
        XSRETURN(1);
    }
    EXTEND(SP, AvFILLp(av)+1);
    Copy(AvARRAY(av), &ST(0), AvFILLp(av)+1, SV *);
    XSRETURN(AvFILLp(av)+1);
}

/* Copy an existing cop->cop_warnings field.
 * If it's one of the standard addresses, just re-use the address.
 * This is the e implementation for the DUP_WARNINGS() macro
 */

char *
Perl_dup_warnings(pTHX_ char* warnings)
{
    if (warnings == NULL || specialWARN(warnings))
        return warnings;

    return rcpv_copy(warnings);
}

/*
=for apidoc rcpv_new

Create a new shared memory refcounted string with the requested size, and
with the requested initialization and a refcount of 1. The actual space
allocated will be 1 byte more than requested and rcpv_new() will ensure that
the extra byte is a null regardless of any flags settings.

If the RCPVf_NO_COPY flag is set then the pv argument will be
ignored, otherwise the contents of the pv pointer will be copied into
the new buffer or if it is NULL the function will do nothing and return NULL.

If the RCPVf_USE_STRLEN flag is set then the len argument is ignored and
recomputed using C<strlen(pv)>. It is an error to combine RCPVf_USE_STRLEN
and RCPVf_NO_COPY at the same time.

Under DEBUGGING rcpv_new() will assert() if it is asked to create a 0 length
shared string unless the RCPVf_ALLOW_EMPTY flag is set.

The return value from the function is suitable for passing into rcpv_copy() and
rcpv_free(). To access the RCPV * from the returned value use the RCPVx() macro.
The 'len' member of the RCPV struct stores the allocated length (including the
extra byte), but the RCPV_LEN() macro returns the requested length (not
including the extra byte).

Note that rcpv_new() does NOT use a hash table or anything like that to
dedupe inputs given the same text content. Each call with a non-null pv
parameter will produce a distinct pointer with its own refcount regardless of
the input content.

=cut
*/

char *
Perl_rcpv_new(pTHX_ const char *pv, STRLEN len, U32 flags) {
    RCPV *rcpv;

    PERL_ARGS_ASSERT_RCPV_NEW;

    PERL_UNUSED_CONTEXT;

    /* Musn't use both at the same time */
    assert((flags & (RCPVf_NO_COPY|RCPVf_USE_STRLEN))!=
                    (RCPVf_NO_COPY|RCPVf_USE_STRLEN));

    if (!pv && (flags & RCPVf_NO_COPY) == 0)
        return NULL;

    if (flags & RCPVf_USE_STRLEN)
        len = strlen(pv);

    assert(len || (flags & RCPVf_ALLOW_EMPTY));

    len++; /* add one for the null we will add to the end */

    rcpv = (RCPV *)PerlMemShared_malloc(sizeof(struct rcpv) + len);
    if (!rcpv)
        croak_no_mem();

    rcpv->len = len;    /* store length including null,
                           RCPV_LEN() subtracts 1 to account for this */
    rcpv->refcount = 1;

    if ((flags & RCPVf_NO_COPY) == 0) {
        (void)memcpy(rcpv->pv, pv, len-1);
    }
    rcpv->pv[len-1]= '\0'; /* the last byte should always be null */
    return rcpv->pv;
}

/*
=for apidoc rcpv_free

refcount decrement a shared memory refcounted string, and when
the refcount goes to 0 free it using perlmemshared_free().

it is the callers responsibility to ensure that the pv is the
result of a rcpv_new() call.

Always returns NULL so it can be used like this:

    thing = rcpv_free(thing);

=cut
*/

char *
Perl_rcpv_free(pTHX_ char *pv) {

    PERL_ARGS_ASSERT_RCPV_FREE;

    PERL_UNUSED_CONTEXT;

    if (!pv)
        return NULL;
    RCPV *rcpv = RCPVx(pv);

    assert(rcpv->refcount);
    assert(rcpv->len);

    OP_REFCNT_LOCK;
    if (--rcpv->refcount == 0) {
        rcpv->len = 0;
        PerlMemShared_free(rcpv);
    }
    OP_REFCNT_UNLOCK;
    return NULL;
}

/*
=for apidoc rcpv_copy

refcount increment a shared memory refcounted string, and when
the refcount goes to 0 free it using PerlMemShared_free().

It is the callers responsibility to ensure that the pv is the
result of a rcpv_new() call.

Returns the same pointer that was passed in.

    new = rcpv_copy(pv);

=cut
*/


char *
Perl_rcpv_copy(pTHX_ char *pv) {

    PERL_ARGS_ASSERT_RCPV_COPY;

    PERL_UNUSED_CONTEXT;

    if (!pv)
        return NULL;
    RCPV *rcpv = RCPVx(pv);
    OP_REFCNT_LOCK;
    rcpv->refcount++;
    OP_REFCNT_UNLOCK;
    return pv;
}

/*
 * ex: set ts=8 sts=4 sw=4 et:
 */
