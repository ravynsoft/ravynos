/* CTF type deduplication.
   Copyright (C) 2019-2023 Free Software Foundation, Inc.

   This file is part of libctf.

   libctf is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
   See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <ctf-impl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "hashtab.h"

/* (In the below, relevant functions are named in square brackets.)  */

/* Type deduplication is a three-phase process:

    [ctf_dedup, ctf_dedup_hash_type, ctf_dedup_rhash_type]
    1) come up with unambiguous hash values for all types: no two types may have
       the same hash value, and any given type should have only one hash value
       (for optimal deduplication).

    [ctf_dedup, ctf_dedup_detect_name_ambiguity,
     ctf_dedup_conflictify_unshared, ctf_dedup_mark_conflicting_hash]
    2) mark those distinct types with names that collide (and thus cannot be
       declared simultaneously in the same translation unit) as conflicting, and
       recursively mark all types that cite one of those types as conflicting as
       well.  Possibly mark all types cited in only one TU as conflicting, if
       the CTF_LINK_SHARE_DUPLICATED link mode is active.

    [ctf_dedup_emit, ctf_dedup_emit_struct_members, ctf_dedup_id_to_target]
    3) emit all the types, one hash value at a time.  Types not marked
       conflicting are emitted once, into the shared dictionary: types marked
       conflicting are emitted once per TU into a dictionary corresponding to
       each TU in which they appear.  Structs marked conflicting get at the very
       least a forward emitted into the shared dict so that other dicts can cite
       it if needed.

   [id_to_packed_id]
   This all works over an array of inputs (usually in the same order as the
   inputs on the link line).  We don't use the ctf_link_inputs hash directly
   because it is convenient to be able to address specific input types as a
   *global type ID* or 'GID', a pair of an array offset and a ctf_id_t.  Since
   both are already 32 bits or less or can easily be constrained to that range,
   we can pack them both into a single 64-bit hash word for easy lookups, which
   would be much more annoying to do with a ctf_dict_t * and a ctf_id_t.  (On
   32-bit platforms, we must do that anyway, since pointers, and thus hash keys
   and values, are only 32 bits wide).  We track which inputs are parents of
   which other inputs so that we can correctly recognize that types we have
   traversed in children may cite types in parents, and so that we can process
   the parents first.)

   Note that thanks to ld -r, the deduplicator can be fed its own output, so the
   inputs may themselves have child dicts.  Since we need to support this usage
   anyway, we can use it in one other place.  If the caller finds translation
   units to be too small a unit ambiguous types, links can be 'cu-mapped', where
   the caller provides a mapping of input TU names to output child dict names.
   This mapping can fuse many child TUs into one potential child dict, so that
   ambiguous types in any of those input TUs go into the same child dict.
   When a many:1 cu-mapping is detected, the ctf_dedup machinery is called
   repeatedly, once for every output name that has more than one input, to fuse
   all the input TUs associated with a given output dict into one, and once again
   as normal to deduplicate all those intermediate outputs (and any 1:1 inputs)
   together.  This has much higher memory usage than otherwise, because in the
   intermediate state, all the output TUs are in memory at once and cannot be
   lazily opened.  It also has implications for the emission code: if types
   appear ambiguously in multiple input TUs that are all mapped to the same
   child dict, we cannot put them in children in the cu-mapping link phase
   because this output is meant to *become* a child in the next link stage and
   parent/child relationships are only one level deep: so instead, we just hide
   all but one of the ambiguous types.

   There are a few other subtleties here that make this more complex than it
   seems.  Let's go over the steps above in more detail.

   1) HASHING.

   [ctf_dedup_hash_type, ctf_dedup_rhash_type]
   Hashing proceeds recursively, mixing in the properties of each input type
   (including its name, if any), and then adding the hash values of every type
   cited by that type.  The result is stashed in the cd_type_hashes so other
   phases can find the hash values of input types given their IDs, and so that
   if we encounter this type again while hashing we can just return its hash
   value: it is also stashed in the *output mapping*, a mapping from hash value
   to the set of GIDs corresponding to that type in all inputs.  We also keep
   track of the GID of the first appearance of the type in any input (in
   cd_output_first_gid), and the GID of structs, unions, and forwards that only
   appear in one TU (in cd_struct_origin).  See below for where these things are
   used.

   Everything in this phase is time-critical, because it is operating over
   non-deduplicated types and so may have hundreds or thousands of times the
   data volume to deal with than later phases.  Trace output is hidden behind
   ENABLE_LIBCTF_HASH_DEBUGGING to prevent the sheer number of calls to
   ctf_dprintf from slowing things down (tenfold slowdowns are observed purely
   from the calls to ctf_dprintf(), even with debugging switched off), and keep
   down the volume of output (hundreds of gigabytes of debug output are not
   uncommon on larger links).

   We have to do *something* about potential cycles in the type graph.  We'd
   like to avoid emitting forwards in the final output if possible, because
   forwards aren't much use: they have no members.  We are mostly saved from
   needing to worry about this at emission time by ctf_add_struct*()
   automatically replacing newly-created forwards when the real struct/union
   comes along.  So we only have to avoid getting stuck in cycles during the
   hashing phase, while also not confusing types that cite members that are
   structs with each other.  It is easiest to solve this problem by noting two
   things:

    - all cycles in C depend on the presence of tagged structs/unions
    - all tagged structs/unions have a unique name they can be disambiguated by

   [ctf_dedup_is_stub]
   This means that we can break all cycles by ceasing to hash in cited types at
   every tagged struct/union and instead hashing in a stub consisting of the
   struct/union's *decorated name*, which is the name preceded by "s " or "u "
   depending on the namespace (cached in cd_decorated_names).  Forwards are
   decorated identically (so a forward to "struct foo" would be represented as
   "s foo"): this means that a citation of a forward to a type and a citation of
   a concrete definition of a type with the same name ends up getting the same
   hash value.

   Of course, it is quite possible to have two TUs with structs with the same
   name and different definitions, but that's OK because when we scan for types
   with ambiguous names we will identify these and mark them conflicting.

   We populate one thing to help conflictedness marking.  No unconflicted type
   may cite a conflicted one, but this means that conflictedness marking must
   walk from types to the types that cite them, which is the opposite of the
   usual order.  We can make this easier to do by constructing a *citers* graph
   in cd_citers, which points from types to the types that cite them: because we
   emit forwards corresponding to every conflicted struct/union, we don't need
   to do this for citations of structs/unions by other types.  This is very
   convenient for us, because that's the only type we don't traverse
   recursively: so we can construct the citers graph at the same time as we
   hash, rather than needing to add an extra pass.  (This graph is a dynhash of
   *type hash values*, so it's small: in effect it is automatically
   deduplicated.)

   2) COLLISIONAL MARKING.

   [ctf_dedup_detect_name_ambiguity, ctf_dedup_mark_conflicting_hash]
   We identify types whose names collide during the hashing process, and count
   the rough number of uses of each name (caching may throw it off a bit: this
   doesn't need to be accurate).  We then mark the less-frequently-cited types
   with each names conflicting: the most-frequently-cited one goes into the
   shared type dictionary, while all others are duplicated into per-TU
   dictionaries, named after the input TU, that have the shared dictionary as a
   parent.  For structures and unions this is not quite good enough: we'd like
   to have citations of forwards to ambiguously named structures and unions
   *stay* as citations of forwards, so that the user can tell that the caller
   didn't actually know which structure definition was meant: but if we put one
   of those structures into the shared dictionary, it would supplant and replace
   the forward, leaving no sign.  So structures and unions do not take part in
   this popularity contest: if their names are ambiguous, they are just
   duplicated, and only a forward appears in the shared dict.

   [ctf_dedup_propagate_conflictedness]
   The process of marking types conflicted is itself recursive: we recursively
   traverse the cd_citers graph populated in the hashing pass above and mark
   everything that we encounter conflicted (without wasting time re-marking
   anything that is already marked).  This naturally terminates just where we
   want it to (at types that are cited by no other types, and at structures and
   unions) and suffices to ensure that types that cite conflicted types are
   always marked conflicted.

   [ctf_dedup_conflictify_unshared, ctf_dedup_multiple_input_dicts]
   When linking in CTF_LINK_SHARE_DUPLICATED mode, we would like all types that
   are used in only one TU to end up in a per-CU dict. The easiest way to do
   that is to mark them conflicted.  ctf_dedup_conflictify_unshared does this,
   traversing the output mapping and using ctf_dedup_multiple_input_dicts to
   check the number of input dicts each distinct type hash value came from:
   types that only came from one get marked conflicted.  One caveat here is that
   we need to consider both structs and forwards to them: a struct that appears
   in one TU and has a dozen citations to an opaque forward in other TUs should
   *not* be considered to be used in only one TU, because users would find it
   useful to be able to traverse into opaque structures of that sort: so we use
   cd_struct_origin to check both structs/unions and the forwards corresponding
   to them.

   3) EMISSION.

   [ctf_dedup_walk_output_mapping, ctf_dedup_rwalk_output_mapping,
    ctf_dedup_rwalk_one_output_mapping]
   Emission involves another walk of the entire output mapping, this time
   traversing everything other than struct members, recursively.  Types are
   emitted from leaves to trunk, emitting all types a type cites before emitting
   the type itself.  We sort the output mapping before traversing it, for
   reproducibility and also correctness: the input dicts may have parent/child
   relationships, so we simply sort all types that first appear in parents
   before all children, then sort types that first appear in dicts appearing
   earlier on the linker command line before those that appear later, then sort
   by input ctf_id_t.  (This is where we use cd_output_first_gid, collected
   above.)

   The walking is done using a recursive traverser which arranges to not revisit
   any type already visited and to call its callback once per input GID for
   input GIDs corresponding to conflicted output types.  The traverser only
   finds input types and calls a callback for them as many times as the output
   needs to appear: it doesn't try to figure out anything about where the output
   might go.  That's done by the callback based on whether the type is
   marked conflicted or not.

   [ctf_dedup_emit_type, ctf_dedup_id_to_target, ctf_dedup_synthesize_forward]
   ctf_dedup_emit_type is the (sole) callback for ctf_dedup_walk_output_mapping.
   Conflicted types have all necessary dictionaries created, and then we emit
   the type into each dictionary in turn, working over each input CTF type
   corresponding to each hash value and using ctf_dedup_id_to_target to map each
   input ctf_id_t into the corresponding type in the output (dealing with input
   ctf_id_t's with parents in the process by simply chasing to the parent dict
   if the type we're looking up is in there).  Emitting structures involves
   simply noting that the members of this structure need emission later on:
   because you cannot cite a single structure member from another type, we avoid
   emitting the members at this stage to keep recursion depths down a bit.

   At this point, if we have by some mischance decided that two different types
   with child types that hash to different values have in fact got the same hash
   value themselves and *not* marked it conflicting, the type walk will walk
   only *one* of them and in all likelihood we'll find that we are trying to
   emit a type into some child dictionary that references a type that was never
   emitted into that dictionary and assertion-fail.  This always indicates a bug
   in the conflictedness marking machinery or the hashing code, or both.

   ctf_dedup_id_to_target calls ctf_dedup_synthesize_forward to do one extra
   thing, alluded to above: if this is a conflicted tagged structure or union,
   and the target is the shared dict (i.e., the type we're being asked to emit
   is not itself conflicted so can't just point straight at the conflicted
   type), we instead synthesise a forward with the same name, emit it into the
   shared dict, record it in cd_output_emission_conflicted_forwards so that we
   don't re-emit it, and return it.  This means that cycles that contain
   conflicts do not cause the entire cycle to be replicated in every child: only
   that piece of the cycle which takes you back as far as the closest tagged
   struct/union needs to be replicated.  This trick means that no part of the
   deduplicator needs a cycle detector: every recursive walk can stop at tagged
   structures.

   [ctf_dedup_emit_struct_members]
   The final stage of emission is to walk over all structures with members
   that need emission and emit all of them. Every type has been emitted at
   this stage, so emission cannot fail.

   [ctf_dedup_populate_type_mappings, ctf_dedup_populate_type_mapping]
   Finally, we update the input -> output type ID mappings used by the ctf-link
   machinery to update all the other sections.  This is surprisingly expensive
   and may be replaced with a scheme which lets the ctf-link machinery extract
   the needed info directly from the deduplicator.  */

/* Possible future optimizations are flagged with 'optimization opportunity'
   below.  */

/* Global optimization opportunity: a GC pass, eliminating types with no direct
   or indirect citations from the other sections in the dictionary.  */

/* Internal flag values for ctf_dedup_hash_type.  */

/* Child call: consider forwardable types equivalent to forwards or stubs below
   this point.  */
#define CTF_DEDUP_HASH_INTERNAL_CHILD         0x01

/* Transform references to single ctf_id_ts in passed-in inputs into a number
   that will fit in a uint64_t.  Needs rethinking if CTF_MAX_TYPE is boosted.

   On 32-bit platforms, we pack things together differently: see the note
   above.  */

#if UINTPTR_MAX < UINT64_MAX
# define IDS_NEED_ALLOCATION 1
# define CTF_DEDUP_GID(fp, input, type) id_to_packed_id (fp, input, type)
# define CTF_DEDUP_GID_TO_INPUT(id) packed_id_to_input (id)
# define CTF_DEDUP_GID_TO_TYPE(id) packed_id_to_type (id)
#else
# define CTF_DEDUP_GID(fp, input, type)	\
  (void *) (((uint64_t) input) << 32 | (type))
# define CTF_DEDUP_GID_TO_INPUT(id) ((int) (((uint64_t) id) >> 32))
# define CTF_DEDUP_GID_TO_TYPE(id) (ctf_id_t) (((uint64_t) id) & ~(0xffffffff00000000ULL))
#endif

#ifdef IDS_NEED_ALLOCATION

 /* This is the 32-bit path, which stores GIDs in a pool and returns a pointer
    into the pool.  It is notably less efficient than the 64-bit direct storage
    approach, but with a smaller key, this is all we can do.  */

static void *
id_to_packed_id (ctf_dict_t *fp, int input_num, ctf_id_t type)
{
  const void *lookup;
  ctf_type_id_key_t *dynkey = NULL;
  ctf_type_id_key_t key = { input_num, type };

  if (!ctf_dynhash_lookup_kv (fp->ctf_dedup.cd_id_to_dict_t,
			      &key, &lookup, NULL))
    {
      if ((dynkey = malloc (sizeof (ctf_type_id_key_t))) == NULL)
	goto oom;
      memcpy (dynkey, &key, sizeof (ctf_type_id_key_t));

      if (ctf_dynhash_insert (fp->ctf_dedup.cd_id_to_dict_t, dynkey, NULL) < 0)
	goto oom;

      ctf_dynhash_lookup_kv (fp->ctf_dedup.cd_id_to_dict_t,
			     dynkey, &lookup, NULL);
    }
  /* We use a raw assert() here because there isn't really a way to get any sort
     of error back from this routine without vastly complicating things for the
     much more common case of !IDS_NEED_ALLOCATION.  */
  assert (lookup);
  return (void *) lookup;

 oom:
  free (dynkey);
  ctf_set_errno (fp, ENOMEM);
  return NULL;
}

static int
packed_id_to_input (const void *id)
{
  const ctf_type_id_key_t *key = (ctf_type_id_key_t *) id;

  return key->ctii_input_num;
}

static ctf_id_t
packed_id_to_type (const void *id)
{
  const ctf_type_id_key_t *key = (ctf_type_id_key_t *) id;

  return key->ctii_type;
}
#endif

/* Make an element in a dynhash-of-dynsets, or return it if already present.  */

static ctf_dynset_t *
make_set_element (ctf_dynhash_t *set, const void *key)
{
  ctf_dynset_t *element;

  if ((element = ctf_dynhash_lookup (set, key)) == NULL)
    {
      if ((element = ctf_dynset_create (htab_hash_string,
					htab_eq_string,
					NULL)) == NULL)
	return NULL;

      if (ctf_dynhash_insert (set, (void *) key, element) < 0)
	{
	  ctf_dynset_destroy (element);
	  return NULL;
	}
    }

  return element;
}

/* Initialize the dedup atoms table.  */
int
ctf_dedup_atoms_init (ctf_dict_t *fp)
{
  if (fp->ctf_dedup_atoms)
    return 0;

  if (!fp->ctf_dedup_atoms_alloc)
    {
      if ((fp->ctf_dedup_atoms_alloc
	   = ctf_dynset_create (htab_hash_string, htab_eq_string,
				free)) == NULL)
	return ctf_set_errno (fp, ENOMEM);
    }
  fp->ctf_dedup_atoms = fp->ctf_dedup_atoms_alloc;
  return 0;
}

/* Intern things in the dedup atoms table.  */

static const char *
intern (ctf_dict_t *fp, char *atom)
{
  const void *foo;

  if (atom == NULL)
    return NULL;

  if (!ctf_dynset_exists (fp->ctf_dedup_atoms, atom, &foo))
    {
      if (ctf_dynset_insert (fp->ctf_dedup_atoms, atom) < 0)
	{
	  ctf_set_errno (fp, ENOMEM);
	  return NULL;
	}
      foo = atom;
    }
  else
    free (atom);

  return (const char *) foo;
}

/* Add an indication of the namespace to a type name in a way that is not valid
   for C identifiers.  Used to maintain hashes of type names to other things
   while allowing for the four C namespaces (normal, struct, union, enum).
   Return a pointer into the cd_decorated_names atoms table.  */
static const char *
ctf_decorate_type_name (ctf_dict_t *fp, const char *name, int kind)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  const char *ret;
  const char *k;
  char *p;
  size_t i;

  switch (kind)
    {
    case CTF_K_STRUCT:
      k = "s ";
      i = 0;
      break;
    case CTF_K_UNION:
      k = "u ";
      i = 1;
      break;
    case CTF_K_ENUM:
      k = "e ";
      i = 2;
      break;
    default:
      k = "";
      i = 3;
    }

  if ((ret = ctf_dynhash_lookup (d->cd_decorated_names[i], name)) == NULL)
    {
      char *str;

      if ((str = malloc (strlen (name) + strlen (k) + 1)) == NULL)
	goto oom;

      p = stpcpy (str, k);
      strcpy (p, name);
      ret = intern (fp, str);
      if (!ret)
	goto oom;

      if (ctf_dynhash_cinsert (d->cd_decorated_names[i], name, ret) < 0)
	goto oom;
    }

  return ret;

 oom:
  ctf_set_errno (fp, ENOMEM);
  return NULL;
}

/* Hash a type, possibly debugging-dumping something about it as well.  */
static inline void
ctf_dedup_sha1_add (ctf_sha1_t *sha1, const void *buf, size_t len,
		    const char *description _libctf_unused_,
		    unsigned long depth _libctf_unused_)
{
  ctf_sha1_add (sha1, buf, len);

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_sha1_t tmp;
  char tmp_hval[CTF_SHA1_SIZE];
  tmp = *sha1;
  ctf_sha1_fini (&tmp, tmp_hval);
  ctf_dprintf ("%lu: after hash addition of %s: %s\n", depth, description,
	       tmp_hval);
#endif
}

static const char *
ctf_dedup_hash_type (ctf_dict_t *fp, ctf_dict_t *input,
		     ctf_dict_t **inputs, uint32_t *parents,
		     int input_num, ctf_id_t type, int flags,
		     unsigned long depth,
		     int (*populate_fun) (ctf_dict_t *fp,
					  ctf_dict_t *input,
					  ctf_dict_t **inputs,
					  int input_num,
					  ctf_id_t type,
					  void *id,
					  const char *decorated_name,
					  const char *hash));

/* Determine whether this type is being hashed as a stub (in which case it is
   unsafe to cache it).  */
static int
ctf_dedup_is_stub (const char *name, int kind, int fwdkind, int flags)
{
  /* We can cache all types unless we are recursing to children and are hashing
     in a tagged struct, union or forward, all of which are replaced with their
     decorated name as a stub and will have different hash values when hashed at
     the top level.  */

  return ((flags & CTF_DEDUP_HASH_INTERNAL_CHILD) && name
	  && (kind == CTF_K_STRUCT || kind == CTF_K_UNION
	      || (kind == CTF_K_FORWARD && (fwdkind == CTF_K_STRUCT
					    || fwdkind == CTF_K_UNION))));
}

/* Populate struct_origin if need be (not already populated, or populated with
   a different origin), in which case it must go to -1, "shared".)

   Only called for forwards or forwardable types with names, when the link mode
   is CTF_LINK_SHARE_DUPLICATED.  */
static int
ctf_dedup_record_origin (ctf_dict_t *fp, int input_num, const char *decorated,
			 void *id)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  void *origin;
  int populate_origin = 0;

  if (ctf_dynhash_lookup_kv (d->cd_struct_origin, decorated, NULL, &origin))
    {
      if (CTF_DEDUP_GID_TO_INPUT (origin) != input_num
	  && CTF_DEDUP_GID_TO_INPUT (origin) != -1)
	{
	  populate_origin = 1;
	  origin = CTF_DEDUP_GID (fp, -1, -1);
	}
    }
  else
    {
      populate_origin = 1;
      origin = id;
    }

  if (populate_origin)
    if (ctf_dynhash_cinsert (d->cd_struct_origin, decorated, origin) < 0)
      return ctf_set_errno (fp, errno);
  return 0;
}

/* Do the underlying hashing and recursion for ctf_dedup_hash_type (which it
   calls, recursively).  */

static const char *
ctf_dedup_rhash_type (ctf_dict_t *fp, ctf_dict_t *input, ctf_dict_t **inputs,
		      uint32_t *parents, int input_num, ctf_id_t type,
		      void *type_id, const ctf_type_t *tp, const char *name,
		      const char *decorated, int kind, int flags,
		      unsigned long depth,
		      int (*populate_fun) (ctf_dict_t *fp,
					   ctf_dict_t *input,
					   ctf_dict_t **inputs,
					   int input_num,
					   ctf_id_t type,
					   void *id,
					   const char *decorated_name,
					   const char *hash))
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  ctf_next_t *i = NULL;
  ctf_sha1_t hash;
  ctf_id_t child_type;
  char hashbuf[CTF_SHA1_SIZE];
  const char *hval = NULL;
  const char *whaterr;
  int err = 0;

  const char *citer = NULL;
  ctf_dynset_t *citers = NULL;

  /* Add a citer to the citers set.  */
#define ADD_CITER(citers, hval)						\
  do									\
    {									\
      whaterr = N_("error updating citers");				\
      if (!citers)							\
	if ((citers = ctf_dynset_create (htab_hash_string,		\
					 htab_eq_string,		\
					 NULL)) == NULL)		\
	  goto oom;							\
      if (ctf_dynset_cinsert (citers, hval) < 0)			\
	goto oom;							\
    }									\
  while (0)

  /* If this is a named struct or union or a forward to one, and this is a child
     traversal, treat this type as if it were a forward -- do not recurse to
     children, ignore all content not already hashed in, and hash in the
     decorated name of the type instead.  */

  if (ctf_dedup_is_stub (name, kind, tp->ctt_type, flags))
    {
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
      ctf_dprintf ("Struct/union/forward citation: substituting forwarding "
		   "stub with decorated name %s\n", decorated);

#endif
      ctf_sha1_init (&hash);
      ctf_dedup_sha1_add (&hash, decorated, strlen (decorated) + 1,
			  "decorated struct/union/forward name", depth);
      ctf_sha1_fini (&hash, hashbuf);

      if ((hval = intern (fp, strdup (hashbuf))) == NULL)
	{
	  ctf_err_warn (fp, 0, 0, _("%s (%i): out of memory during forwarding-"
				    "stub hashing for type with GID %p"),
			ctf_link_input_name (input), input_num, type_id);
	  return NULL;				/* errno is set for us.  */
	}

      /* In share-duplicated link mode, make sure the origin of this type is
	 recorded, even if this is a type in a parent dict which will not be
	 directly traversed.  */
      if (d->cd_link_flags & CTF_LINK_SHARE_DUPLICATED
	  && ctf_dedup_record_origin (fp, input_num, decorated, type_id) < 0)
	return NULL;				/* errno is set for us.  */

      return hval;
    }

  /* Now ensure that subsequent recursive calls (but *not* the top-level call)
     get this treatment.  */
  flags |= CTF_DEDUP_HASH_INTERNAL_CHILD;

  /* If this is a struct, union, or forward with a name, record the unique
     originating input TU, if there is one.  */

  if (decorated && (ctf_forwardable_kind (kind) || kind != CTF_K_FORWARD))
    if (d->cd_link_flags & CTF_LINK_SHARE_DUPLICATED
	&& ctf_dedup_record_origin (fp, input_num, decorated, type_id) < 0)
      return NULL;				/* errno is set for us.  */

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_dprintf ("%lu: hashing thing with ID %i/%lx (kind %i): %s.\n",
	       depth, input_num, type, kind, name ? name : "");
#endif

  /* Some type kinds don't have names: the API provides no way to set the name,
     so the type the deduplicator outputs will be nameless even if the input
     somehow has a name, and the name should not be mixed into the hash.  */

  switch (kind)
    {
    case CTF_K_POINTER:
    case CTF_K_ARRAY:
    case CTF_K_FUNCTION:
    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
    case CTF_K_SLICE:
      name = NULL;
    }

  /* Mix in invariant stuff, transforming the type kind if needed.  Note that
     the vlen is *not* hashed in: the actual variable-length info is hashed in
     instead, piecewise.  The vlen is not part of the type, only the
     variable-length data is: identical types with distinct vlens are quite
     possible.  Equally, we do not want to hash in the isroot flag: both the
     compiler and the deduplicator set the nonroot flag to indicate clashes with
     *other types in the same TU* with the same name: so two types can easily
     have distinct nonroot flags, yet be exactly the same type.*/

  ctf_sha1_init (&hash);
  if (name)
    ctf_dedup_sha1_add (&hash, name, strlen (name) + 1, "name", depth);
  ctf_dedup_sha1_add (&hash, &kind, sizeof (uint32_t), "kind", depth);

  /* Hash content of this type.  */
  switch (kind)
    {
    case CTF_K_UNKNOWN:
      /* No extra state.  */
      break;
    case CTF_K_FORWARD:

      /* Add the forwarded kind, stored in the ctt_type.  */
      ctf_dedup_sha1_add (&hash, &tp->ctt_type, sizeof (tp->ctt_type),
			  "forwarded kind", depth);
      break;
    case CTF_K_INTEGER:
    case CTF_K_FLOAT:
      {
	ctf_encoding_t ep;
	memset (&ep, 0, sizeof (ctf_encoding_t));

	ctf_dedup_sha1_add (&hash, &tp->ctt_size, sizeof (uint32_t), "size",
			    depth);
	if (ctf_type_encoding (input, type, &ep) < 0)
	  {
	    whaterr = N_("error getting encoding");
	    goto input_err;
	  }
	ctf_dedup_sha1_add (&hash, &ep, sizeof (ctf_encoding_t), "encoding",
			    depth);
	break;
      }
      /* Types that reference other types.  */
    case CTF_K_TYPEDEF:
    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
    case CTF_K_POINTER:
      /* Hash the referenced type, if not already hashed, and mix it in.  */
      child_type = ctf_type_reference (input, type);
      if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents, input_num,
				       child_type, flags, depth,
				       populate_fun)) == NULL)
	{
	  whaterr = N_("error doing referenced type hashing");
	  goto err;
	}
      ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "referenced type",
			  depth);
      citer = hval;

      break;

      /* The slices of two types hash identically only if the type they overlay
	 also has the same encoding.  This is not ideal, but in practice will work
	 well enough.  We work directly rather than using the CTF API because
	 we do not want the slice's normal automatically-shine-through
	 semantics to kick in here.  */
    case CTF_K_SLICE:
      {
	const ctf_slice_t *slice;
	const ctf_dtdef_t *dtd;
	ssize_t size;
	ssize_t increment;

	child_type = ctf_type_reference (input, type);
	ctf_get_ctt_size (input, tp, &size, &increment);
	ctf_dedup_sha1_add (&hash, &size, sizeof (ssize_t), "size", depth);

	if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents, input_num,
					 child_type, flags, depth,
					 populate_fun)) == NULL)
	  {
	    whaterr = N_("error doing slice-referenced type hashing");
	    goto err;
	  }
	ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "sliced type",
			    depth);
	citer = hval;

	if ((dtd = ctf_dynamic_type (input, type)) != NULL)
	  slice = (ctf_slice_t *) dtd->dtd_vlen;
	else
	  slice = (ctf_slice_t *) ((uintptr_t) tp + increment);

	ctf_dedup_sha1_add (&hash, &slice->cts_offset,
			    sizeof (slice->cts_offset), "slice offset", depth);
	ctf_dedup_sha1_add (&hash, &slice->cts_bits,
			    sizeof (slice->cts_bits), "slice bits", depth);
	break;
      }

    case CTF_K_ARRAY:
      {
	ctf_arinfo_t ar;

	if (ctf_array_info (input, type, &ar) < 0)
	  {
	    whaterr = N_("error getting array info");
	    goto input_err;
	  }

	if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents, input_num,
					 ar.ctr_contents, flags, depth,
					 populate_fun)) == NULL)
	  {
	    whaterr = N_("error doing array contents type hashing");
	    goto err;
	  }
	ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "array contents",
			    depth);
	ADD_CITER (citers, hval);

	if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents, input_num,
					 ar.ctr_index, flags, depth,
					 populate_fun)) == NULL)
	  {
	    whaterr = N_("error doing array index type hashing");
	    goto err;
	  }
	ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "array index",
			    depth);
	ctf_dedup_sha1_add (&hash, &ar.ctr_nelems, sizeof (ar.ctr_nelems),
			    "element count", depth);
	ADD_CITER (citers, hval);

	break;
      }
    case CTF_K_FUNCTION:
      {
	ctf_funcinfo_t fi;
	ctf_id_t *args;
	uint32_t j;

	if (ctf_func_type_info (input, type, &fi) < 0)
	  {
	    whaterr = N_("error getting func type info");
	    goto input_err;
	  }

	if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents, input_num,
					 fi.ctc_return, flags, depth,
					 populate_fun)) == NULL)
	  {
	    whaterr = N_("error getting func return type");
	    goto err;
	  }
	ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "func return",
			    depth);
	ctf_dedup_sha1_add (&hash, &fi.ctc_argc, sizeof (fi.ctc_argc),
			    "func argc", depth);
	ctf_dedup_sha1_add (&hash, &fi.ctc_flags, sizeof (fi.ctc_flags),
			    "func flags", depth);
	ADD_CITER (citers, hval);

	if ((args = calloc (fi.ctc_argc, sizeof (ctf_id_t))) == NULL)
	  {
	    err = ENOMEM;
	    whaterr = N_("error doing memory allocation");
	    goto err;
	  }

	if (ctf_func_type_args (input, type, fi.ctc_argc, args) < 0)
	  {
	    free (args);
	    whaterr = N_("error getting func arg type");
	    goto input_err;
	  }
	for (j = 0; j < fi.ctc_argc; j++)
	  {
	    if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents,
					     input_num, args[j], flags, depth,
					     populate_fun)) == NULL)
	      {
		free (args);
		whaterr = N_("error doing func arg type hashing");
		goto err;
	      }
	    ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "func arg type",
				depth);
	    ADD_CITER (citers, hval);
	  }
	free (args);
	break;
      }
    case CTF_K_ENUM:
      {
	int val;
	const char *ename;

	ctf_dedup_sha1_add (&hash, &tp->ctt_size, sizeof (uint32_t),
			    "enum size", depth);
	while ((ename = ctf_enum_next (input, type, &i, &val)) != NULL)
	  {
	    ctf_dedup_sha1_add (&hash, ename, strlen (ename) + 1, "enumerator",
				depth);
	    ctf_dedup_sha1_add (&hash, &val, sizeof (val), "enumerand", depth);
	  }
	if (ctf_errno (input) != ECTF_NEXT_END)
	  {
	    whaterr = N_("error doing enum member iteration");
	    goto input_err;
	  }
	break;
      }
    /* Top-level only.  */
    case CTF_K_STRUCT:
    case CTF_K_UNION:
      {
	ssize_t offset;
	const char *mname;
	ctf_id_t membtype;
	ssize_t size;

	ctf_get_ctt_size (input, tp, &size, NULL);
	ctf_dedup_sha1_add (&hash, &size, sizeof (ssize_t), "struct size",
			    depth);

	while ((offset = ctf_member_next (input, type, &i, &mname, &membtype,
					  0)) >= 0)
	  {
	    if (mname == NULL)
	      mname = "";
	    ctf_dedup_sha1_add (&hash, mname, strlen (mname) + 1,
				"member name", depth);

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
	    ctf_dprintf ("%lu: Traversing to member %s\n", depth, mname);
#endif
	    if ((hval = ctf_dedup_hash_type (fp, input, inputs, parents,
					     input_num, membtype, flags, depth,
					     populate_fun)) == NULL)
	      {
		whaterr = N_("error doing struct/union member type hashing");
		goto iterr;
	      }

	    ctf_dedup_sha1_add (&hash, hval, strlen (hval) + 1, "member hash",
				depth);
	    ctf_dedup_sha1_add (&hash, &offset, sizeof (offset), "member offset",
				depth);
	    ADD_CITER (citers, hval);
	  }
	if (ctf_errno (input) != ECTF_NEXT_END)
	  {
	    whaterr = N_("error doing struct/union member iteration");
	    goto input_err;
	  }
	break;
      }
    default:
      whaterr = N_("error: unknown type kind");
      goto err;
    }
  ctf_sha1_fini (&hash, hashbuf);

  if ((hval = intern (fp, strdup (hashbuf))) == NULL)
    {
      whaterr = N_("cannot intern hash");
      goto oom;
    }

  /* Populate the citers for this type's subtypes, now the hash for the type
     itself is known.  */
  whaterr = N_("error tracking citers");

  if (citer)
    {
      ctf_dynset_t *citer_hashes;

      if ((citer_hashes = make_set_element (d->cd_citers, citer)) == NULL)
	goto oom;
      if (ctf_dynset_cinsert (citer_hashes, hval) < 0)
	goto oom;
    }
  else if (citers)
    {
      const void *k;

      while ((err = ctf_dynset_cnext (citers, &i, &k)) == 0)
	{
	  ctf_dynset_t *citer_hashes;
	  citer = (const char *) k;

	  if ((citer_hashes = make_set_element (d->cd_citers, citer)) == NULL)
	    goto oom;

	  if (ctf_dynset_exists (citer_hashes, hval, NULL))
	    continue;
	  if (ctf_dynset_cinsert (citer_hashes, hval) < 0)
	    goto oom;
	}
      if (err != ECTF_NEXT_END)
	goto err;
      ctf_dynset_destroy (citers);
    }

  return hval;

 iterr:
  ctf_next_destroy (i);
 input_err:
  err = ctf_errno (input);
 err:
  ctf_sha1_fini (&hash, NULL);
  ctf_err_warn (fp, 0, err, _("%s (%i): %s: during type hashing for type %lx, "
			      "kind %i"), ctf_link_input_name (input),
		input_num, gettext (whaterr), type, kind);
  return NULL;
 oom:
  ctf_set_errno (fp, errno);
  ctf_err_warn (fp, 0, 0, _("%s (%i): %s: during type hashing for type %lx, "
			    "kind %i"), ctf_link_input_name (input),
		input_num, gettext (whaterr), type, kind);
  return NULL;
}

/* Hash a TYPE in the INPUT: FP is the eventual output, where the ctf_dedup
   state is stored.  INPUT_NUM is the number of this input in the set of inputs.
   Record its hash in FP's cd_type_hashes once it is known.  PARENTS is
   described in the comment above ctf_dedup.

   (The flags argument currently accepts only the flag
   CTF_DEDUP_HASH_INTERNAL_CHILD, an implementation detail used to prevent
   struct/union hashing in recursive traversals below the TYPE.)

   We use the CTF API rather than direct access wherever possible, because types
   that appear identical through the API should be considered identical, with
   one exception: slices should only be considered identical to other slices,
   not to the corresponding unsliced type.

   The POPULATE_FUN is a mandatory hook that populates other mappings with each
   type we see (excepting types that are recursively hashed as stubs).  The
   caller should not rely on the order of calls to this hook, though it will be
   called at least once for every non-stub reference to every type.

   Returns a hash value (an atom), or NULL on error.  */

static const char *
ctf_dedup_hash_type (ctf_dict_t *fp, ctf_dict_t *input,
		     ctf_dict_t **inputs, uint32_t *parents,
		     int input_num, ctf_id_t type, int flags,
		     unsigned long depth,
		     int (*populate_fun) (ctf_dict_t *fp,
					  ctf_dict_t *input,
					  ctf_dict_t **inputs,
					  int input_num,
					  ctf_id_t type,
					  void *id,
					  const char *decorated_name,
					  const char *hash))
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  const ctf_type_t *tp;
  void *type_id;
  const char *hval = NULL;
  const char *name;
  const char *whaterr;
  const char *decorated = NULL;
  uint32_t kind, fwdkind;

  depth++;

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_dprintf ("%lu: ctf_dedup_hash_type (%i, %lx, flags %x)\n", depth, input_num, type, flags);
#endif

  /* The unimplemented type doesn't really exist, but must be noted in parent
     hashes: so it gets a fixed, arbitrary hash.  */
  if (type == 0)
    return "00000000000000000000";

  /* Possible optimization: if the input type is in the parent type space, just
     copy recursively-cited hashes from the parent's types into the output
     mapping rather than rehashing them.  */

  type_id = CTF_DEDUP_GID (fp, input_num, type);

  if ((tp = ctf_lookup_by_id (&input, type)) == NULL)
    {
      ctf_set_errno (fp, ctf_errno (input));
      ctf_err_warn (fp, 0, 0, _("%s (%i): lookup failure for type %lx: "
				"flags %x"), ctf_link_input_name (input),
		    input_num, type, flags);
      return NULL;		/* errno is set for us.  */
    }

  kind = LCTF_INFO_KIND (input, tp->ctt_info);
  name = ctf_strraw (input, tp->ctt_name);

  if (tp->ctt_name == 0 || !name || name[0] == '\0')
    name = NULL;

  /* Decorate the name appropriately for the namespace it appears in: forwards
     appear in the namespace of their referent.  */

  fwdkind = kind;
  if (name)
    {
      if (kind == CTF_K_FORWARD)
	fwdkind = tp->ctt_type;

      if ((decorated = ctf_decorate_type_name (fp, name, fwdkind)) == NULL)
	return NULL;				/* errno is set for us.  */
    }

  /* If not hashing a stub, we can rely on various sorts of caches.

     Optimization opportunity: we may be able to avoid calling the populate_fun
     sometimes here.  */

  if (!ctf_dedup_is_stub (name, kind, fwdkind, flags))
    {
      if ((hval = ctf_dynhash_lookup (d->cd_type_hashes, type_id)) != NULL)
	{
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
	  ctf_dprintf ("%lu: Known hash for ID %i/%lx: %s\n", depth, input_num,
		       type,  hval);
#endif
	  populate_fun (fp, input, inputs, input_num, type, type_id,
			decorated, hval);

	  return hval;
	}
    }

  /* We have never seen this type before, and must figure out its hash and the
     hashes of the types it cites.

     Hash this type, and call ourselves recursively.  (The hashing part is
     optional, and is disabled if overidden_hval is set.)  */

  if ((hval = ctf_dedup_rhash_type (fp, input, inputs, parents, input_num,
				    type, type_id, tp, name, decorated,
				    kind, flags, depth, populate_fun)) == NULL)
    return NULL;				/* errno is set for us.  */

  /* The hash of this type is now known: record it unless caching is unsafe
     because the hash value will change later.  This will be the final storage
     of this type's hash, so we call the population function on it.  */

  if (!ctf_dedup_is_stub (name, kind, fwdkind, flags))
    {
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
      ctf_dprintf ("Caching %lx, ID %p (%s), %s in final location\n", type,
		   type_id, name ? name : "", hval);
#endif

      if (ctf_dynhash_cinsert (d->cd_type_hashes, type_id, hval) < 0)
	{
	  whaterr = N_("error hash caching");
	  goto oom;
	}

      if (populate_fun (fp, input, inputs, input_num, type, type_id,
			decorated, hval) < 0)
	{
	  whaterr = N_("error calling population function");
	  goto err;				/* errno is set for us. */
	}
    }

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_dprintf ("%lu: Returning final hash for ID %i/%lx: %s\n", depth,
	       input_num, type, hval);
#endif
  return hval;

 oom:
  ctf_set_errno (fp, errno);
 err:
  ctf_err_warn (fp, 0, 0, _("%s (%i): %s: during type hashing, "
			    "type %lx, kind %i"),
		ctf_link_input_name (input), input_num,
		gettext (whaterr), type, kind);
  return NULL;
}

/* Populate a number of useful mappings not directly used by the hashing
   machinery: the output mapping, the cd_name_counts mapping from name -> hash
   -> count of hashval deduplication state for a given hashed type, and the
   cd_output_first_tu mapping.  */

static int
ctf_dedup_populate_mappings (ctf_dict_t *fp, ctf_dict_t *input _libctf_unused_,
			     ctf_dict_t **inputs _libctf_unused_,
			     int input_num _libctf_unused_,
			     ctf_id_t type _libctf_unused_, void *id,
			     const char *decorated_name,
			     const char *hval)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  ctf_dynset_t *type_ids;
  ctf_dynhash_t *name_counts;
  long int count;

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_dprintf ("Hash %s, %s, into output mapping for %i/%lx @ %s\n",
	       hval, decorated_name ? decorated_name : "(unnamed)",
	       input_num, type, ctf_link_input_name (input));

  const char *orig_hval;

  /* Make sure we never map a single GID to multiple hash values.  */

  if ((orig_hval = ctf_dynhash_lookup (d->cd_output_mapping_guard, id)) != NULL)
    {
      /* We can rely on pointer identity here, since all hashes are
	 interned.  */
      if (!ctf_assert (fp, orig_hval == hval))
	return -1;
    }
  else
    if (ctf_dynhash_cinsert (d->cd_output_mapping_guard, id, hval) < 0)
      return ctf_set_errno (fp, errno);
#endif

  /* Record the type in the output mapping: if this is the first time this type
     has been seen, also record it in the cd_output_first_gid.  Because we
     traverse types in TU order and we do not merge types after the hashing
     phase, this will be the lowest TU this type ever appears in.  */

  if ((type_ids = ctf_dynhash_lookup (d->cd_output_mapping,
				      hval)) == NULL)
    {
      if (ctf_dynhash_cinsert (d->cd_output_first_gid, hval, id) < 0)
	return ctf_set_errno (fp, errno);

      if ((type_ids = ctf_dynset_create (htab_hash_pointer,
					 htab_eq_pointer,
					 NULL)) == NULL)
	return ctf_set_errno (fp, errno);
      if (ctf_dynhash_insert (d->cd_output_mapping, (void *) hval,
			      type_ids) < 0)
	{
	  ctf_dynset_destroy (type_ids);
	  return ctf_set_errno (fp, errno);
	}
    }
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
    {
      /* Verify that all types with this hash are of the same kind, and that the
	 first TU a type was seen in never falls.  */

      int err;
      const void *one_id;
      ctf_next_t *i = NULL;
      int orig_kind = ctf_type_kind_unsliced (input, type);
      int orig_first_tu;

      orig_first_tu = CTF_DEDUP_GID_TO_INPUT
	(ctf_dynhash_lookup (d->cd_output_first_gid, hval));
      if (!ctf_assert (fp, orig_first_tu <= CTF_DEDUP_GID_TO_INPUT (id)))
	return -1;

      while ((err = ctf_dynset_cnext (type_ids, &i, &one_id)) == 0)
	{
	  ctf_dict_t *foo = inputs[CTF_DEDUP_GID_TO_INPUT (one_id)];
	  ctf_id_t bar = CTF_DEDUP_GID_TO_TYPE (one_id);
	  if (ctf_type_kind_unsliced (foo, bar) != orig_kind)
	    {
	      ctf_err_warn (fp, 1, 0, "added wrong kind to output mapping "
			    "for hash %s named %s: %p/%lx from %s is "
			    "kind %i, but newly-added %p/%lx from %s is "
			    "kind %i", hval,
			    decorated_name ? decorated_name : "(unnamed)",
			    (void *) foo, bar,
			    ctf_link_input_name (foo),
			    ctf_type_kind_unsliced (foo, bar),
			    (void *) input, type,
			    ctf_link_input_name (input), orig_kind);
	      if (!ctf_assert (fp, ctf_type_kind_unsliced (foo, bar)
			       == orig_kind))
		return -1;
	    }
	}
      if (err != ECTF_NEXT_END)
	return ctf_set_errno (fp, err);
    }
#endif

  /* This function will be repeatedly called for the same types many times:
     don't waste time reinserting the same keys in that case.  */
  if (!ctf_dynset_exists (type_ids, id, NULL)
      && ctf_dynset_insert (type_ids, id) < 0)
    return ctf_set_errno (fp, errno);

  /* The rest only needs to happen for types with names.  */
  if (!decorated_name)
    return 0;

  /* Count the number of occurrences of the hash value for this GID.  */

  hval = ctf_dynhash_lookup (d->cd_type_hashes, id);

  /* Mapping from name -> hash(hashval, count) not already present?  */
  if ((name_counts = ctf_dynhash_lookup (d->cd_name_counts,
					 decorated_name)) == NULL)
    {
      if ((name_counts = ctf_dynhash_create (ctf_hash_string,
					     ctf_hash_eq_string,
					     NULL, NULL)) == NULL)
	  return ctf_set_errno (fp, errno);
      if (ctf_dynhash_cinsert (d->cd_name_counts, decorated_name,
			       name_counts) < 0)
	{
	  ctf_dynhash_destroy (name_counts);
	  return ctf_set_errno (fp, errno);
	}
    }

  /* This will, conveniently, return NULL (i.e. 0) for a new entry.  */
  count = (long int) (uintptr_t) ctf_dynhash_lookup (name_counts, hval);

  if (ctf_dynhash_cinsert (name_counts, hval,
			   (const void *) (uintptr_t) (count + 1)) < 0)
    return ctf_set_errno (fp, errno);

  return 0;
}

/* Mark a single hash as corresponding to a conflicting type.  Mark all types
   that cite it as conflicting as well, terminating the recursive walk only when
   types that are already conflicted or types do not cite other types are seen.
   (Tagged structures and unions do not appear in the cd_citers graph, so the
   walk also terminates there, since any reference to a conflicting structure is
   just going to reference an unconflicting forward instead: see
   ctf_dedup_maybe_synthesize_forward.)  */

static int
ctf_dedup_mark_conflicting_hash (ctf_dict_t *fp, const char *hval)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  ctf_next_t *i = NULL;
  int err;
  const void *k;
  ctf_dynset_t *citers;

  /* Mark conflicted if not already so marked.  */
  if (ctf_dynset_exists (d->cd_conflicting_types, hval, NULL))
    return 0;

  ctf_dprintf ("Marking %s as conflicted\n", hval);

  if (ctf_dynset_cinsert (d->cd_conflicting_types, hval) < 0)
    {
      ctf_dprintf ("Out of memory marking %s as conflicted\n", hval);
      ctf_set_errno (fp, errno);
      return -1;
    }

  /* If any types cite this type, mark them conflicted too.  */
  if ((citers = ctf_dynhash_lookup (d->cd_citers, hval)) == NULL)
    return 0;

  while ((err = ctf_dynset_cnext (citers, &i, &k)) == 0)
    {
      const char *hv = (const char *) k;

      if (ctf_dynset_exists (d->cd_conflicting_types, hv, NULL))
	continue;

      if (ctf_dedup_mark_conflicting_hash (fp, hv) < 0)
	{
	  ctf_next_destroy (i);
	  return -1;				/* errno is set for us.  */
	}
    }
  if (err != ECTF_NEXT_END)
    return ctf_set_errno (fp, err);

  return 0;
}

/* Look up a type kind from the output mapping, given a type hash value.  */
static int
ctf_dedup_hash_kind (ctf_dict_t *fp, ctf_dict_t **inputs, const char *hash)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  void *id;
  ctf_dynset_t *type_ids;

  /* Precondition: the output mapping is populated.  */
  if (!ctf_assert (fp, ctf_dynhash_elements (d->cd_output_mapping) > 0))
    return -1;

  /* Look up some GID from the output hash for this type.  (They are all
     identical, so we can pick any).  Don't assert if someone calls this
     function wrongly, but do assert if the output mapping knows about the hash,
     but has nothing associated with it.  */

  type_ids = ctf_dynhash_lookup (d->cd_output_mapping, hash);
  if (!type_ids)
    {
      ctf_dprintf ("Looked up type kind by nonexistent hash %s.\n", hash);
      return ctf_set_errno (fp, ECTF_INTERNAL);
    }
  id = ctf_dynset_lookup_any (type_ids);
  if (!ctf_assert (fp, id))
    return -1;

  return ctf_type_kind_unsliced (inputs[CTF_DEDUP_GID_TO_INPUT (id)],
				 CTF_DEDUP_GID_TO_TYPE (id));
}

/* Used to keep a count of types: i.e. distinct type hash values.  */
typedef struct ctf_dedup_type_counter
{
  ctf_dict_t *fp;
  ctf_dict_t **inputs;
  int num_non_forwards;
} ctf_dedup_type_counter_t;

/* Add to the type counter for one name entry from the cd_name_counts.  */
static int
ctf_dedup_count_types (void *key_, void *value _libctf_unused_, void *arg_)
{
  const char *hval = (const char *) key_;
  int kind;
  ctf_dedup_type_counter_t *arg = (ctf_dedup_type_counter_t *) arg_;

  kind = ctf_dedup_hash_kind (arg->fp, arg->inputs, hval);

  /* We rely on ctf_dedup_hash_kind setting the fp to -ECTF_INTERNAL on error to
     smuggle errors out of here.  */

  if (kind != CTF_K_FORWARD)
    {
      arg->num_non_forwards++;
      ctf_dprintf ("Counting hash %s: kind %i: num_non_forwards is %i\n",
		   hval, kind, arg->num_non_forwards);
    }

  /* We only need to know if there is more than one non-forward (an ambiguous
     type): don't waste time iterating any more than needed to figure that
     out.  */

  if (arg->num_non_forwards > 1)
    return 1;

  return 0;
}

/* Detect name ambiguity and mark ambiguous names as conflicting, other than the
   most common.  */
static int
ctf_dedup_detect_name_ambiguity (ctf_dict_t *fp, ctf_dict_t **inputs)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  ctf_next_t *i = NULL;
  void *k;
  void *v;
  int err;
  const char *whaterr;

  /* Go through cd_name_counts for all CTF namespaces in turn.  */

  while ((err = ctf_dynhash_next (d->cd_name_counts, &i, &k, &v)) == 0)
    {
      const char *decorated = (const char *) k;
      ctf_dynhash_t *name_counts = (ctf_dynhash_t *) v;
      ctf_next_t *j = NULL;

      /* If this is a forwardable kind or a forward (which we can tell without
	 consulting the type because its decorated name has a space as its
	 second character: see ctf_decorate_type_name), we are only interested
	 in whether this name has many hashes associated with it: any such name
	 is necessarily ambiguous, and types with that name are conflicting.
	 Once we know whether this is true, we can skip to the next name: so use
	 ctf_dynhash_iter_find for efficiency.  */

      if (decorated[0] != '\0' && decorated[1] == ' ')
	{
	  ctf_dedup_type_counter_t counters = { fp, inputs, 0 };
	  ctf_dynhash_t *counts = (ctf_dynhash_t *) v;

	  ctf_dynhash_iter_find (counts, ctf_dedup_count_types, &counters);

	  /* Check for assertion failure and pass it up.  */
	  if (ctf_errno (fp) == ECTF_INTERNAL)
	    goto assert_err;

	  if (counters.num_non_forwards > 1)
	    {
	      const void *hval_;

	      while ((err = ctf_dynhash_cnext (counts, &j, &hval_, NULL)) == 0)
		{
		  const char *hval = (const char *) hval_;
		  ctf_dynset_t *type_ids;
		  void *id;
		  int kind;

		  /* Dig through the types in this hash to find the non-forwards
		     and mark them ambiguous.  */

		  type_ids = ctf_dynhash_lookup (d->cd_output_mapping, hval);

		  /* Nonexistent? Must be a forward with no referent.  */
		  if (!type_ids)
		    continue;

		  id = ctf_dynset_lookup_any (type_ids);

		  kind = ctf_type_kind (inputs[CTF_DEDUP_GID_TO_INPUT (id)],
					CTF_DEDUP_GID_TO_TYPE (id));

		  if (kind != CTF_K_FORWARD)
		    {
		      ctf_dprintf ("Marking %p, with hash %s, conflicting: one "
				   "of many non-forward GIDs for %s\n", id,
				   hval, (char *) k);
		      ctf_dedup_mark_conflicting_hash (fp, hval);
		    }
		}
	      if (err != ECTF_NEXT_END)
		{
		  whaterr = N_("error marking conflicting structs/unions");
		  goto iterr;
		}
	    }
	}
      else
	{
	  /* This is an ordinary type.  Find the most common type with this
	     name, and mark it unconflicting: all others are conflicting.  (We
	     cannot do this sort of popularity contest with forwardable types
	     because any forwards to that type would be immediately unified with
	     the most-popular type on insertion, and we want conflicting structs
	     et al to have all forwards left intact, so the user is notified
	     that this type is conflicting.  TODO: improve this in future by
	     setting such forwards non-root-visible.)

	     If multiple distinct types are "most common", pick the one that
	     appears first on the link line, and within that, the one with the
	     lowest type ID.  (See sort_output_mapping.)  */

	  const void *key;
	  const void *count;
	  const char *hval;
	  long max_hcount = -1;
	  void *max_gid = NULL;
	  const char *max_hval = NULL;

	  if (ctf_dynhash_elements (name_counts) <= 1)
	    continue;

	  /* First find the most common.  */
	  while ((err = ctf_dynhash_cnext (name_counts, &j, &key, &count)) == 0)
	    {
	      hval = (const char *) key;

	      if ((long int) (uintptr_t) count > max_hcount)
		{
		  max_hcount = (long int) (uintptr_t) count;
		  max_hval = hval;
		  max_gid = ctf_dynhash_lookup (d->cd_output_first_gid, hval);
		}
	      else if ((long int) (uintptr_t) count == max_hcount)
		{
		  void *gid = ctf_dynhash_lookup (d->cd_output_first_gid, hval);

		  if (CTF_DEDUP_GID_TO_INPUT(gid) < CTF_DEDUP_GID_TO_INPUT(max_gid)
		      || (CTF_DEDUP_GID_TO_INPUT(gid) == CTF_DEDUP_GID_TO_INPUT(max_gid)
			  && CTF_DEDUP_GID_TO_TYPE(gid) < CTF_DEDUP_GID_TO_TYPE(max_gid)))
		    {
		      max_hval = hval;
		      max_gid = ctf_dynhash_lookup (d->cd_output_first_gid, hval);
		    }
		}
	    }
	  if (err != ECTF_NEXT_END)
	    {
	      whaterr = N_("error finding commonest conflicting type");
	      goto iterr;
	    }

	  /* Mark all the others as conflicting.   */
	  while ((err = ctf_dynhash_cnext (name_counts, &j, &key, NULL)) == 0)
	    {
	      hval = (const char *) key;
	      if (strcmp (max_hval, hval) == 0)
		continue;

	      ctf_dprintf ("Marking %s, an uncommon hash for %s, conflicting\n",
			   hval, (const char *) k);
	      if (ctf_dedup_mark_conflicting_hash (fp, hval) < 0)
		{
		  whaterr = N_("error marking hashes as conflicting");
		  goto err;
		}
	    }
	  if (err != ECTF_NEXT_END)
	    {
	      whaterr = N_("marking uncommon conflicting types");
	      goto iterr;
	    }
	}
    }
  if (err != ECTF_NEXT_END)
    {
      whaterr = N_("scanning for ambiguous names");
      goto iterr;
    }

  return 0;

 err:
  ctf_next_destroy (i);
  ctf_err_warn (fp, 0, 0, "%s", gettext (whaterr));
  return -1;					/* errno is set for us.  */

 iterr:
  ctf_err_warn (fp, 0, err, _("iteration failed: %s"), gettext (whaterr));
  return ctf_set_errno (fp, err);

 assert_err:
  ctf_next_destroy (i);
  return -1; 					/* errno is set for us.  */
}

/* Initialize the deduplication machinery.  */

static int
ctf_dedup_init (ctf_dict_t *fp)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  size_t i;

  if (ctf_dedup_atoms_init (fp) < 0)
      goto oom;

#if IDS_NEED_ALLOCATION
  if ((d->cd_id_to_dict_t = ctf_dynhash_create (ctf_hash_type_id_key,
						ctf_hash_eq_type_id_key,
						free, NULL)) == NULL)
    goto oom;
#endif

  for (i = 0; i < 4; i++)
    {
      if ((d->cd_decorated_names[i] = ctf_dynhash_create (ctf_hash_string,
							  ctf_hash_eq_string,
							  NULL, NULL)) == NULL)
	goto oom;
    }

  if ((d->cd_name_counts
       = ctf_dynhash_create (ctf_hash_string,
			     ctf_hash_eq_string, NULL,
			     (ctf_hash_free_fun) ctf_dynhash_destroy)) == NULL)
    goto oom;

  if ((d->cd_type_hashes
       = ctf_dynhash_create (ctf_hash_integer,
			     ctf_hash_eq_integer,
			     NULL, NULL)) == NULL)
    goto oom;

  if ((d->cd_struct_origin
       = ctf_dynhash_create (ctf_hash_string,
			     ctf_hash_eq_string,
			     NULL, NULL)) == NULL)
    goto oom;

  if ((d->cd_citers
       = ctf_dynhash_create (ctf_hash_string,
			     ctf_hash_eq_string, NULL,
			     (ctf_hash_free_fun) ctf_dynset_destroy)) == NULL)
    goto oom;

  if ((d->cd_output_mapping
       = ctf_dynhash_create (ctf_hash_string,
			     ctf_hash_eq_string, NULL,
			     (ctf_hash_free_fun) ctf_dynset_destroy)) == NULL)
    goto oom;

  if ((d->cd_output_first_gid
       = ctf_dynhash_create (ctf_hash_string,
			     ctf_hash_eq_string,
			     NULL, NULL)) == NULL)
    goto oom;

#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  if ((d->cd_output_mapping_guard
       = ctf_dynhash_create (ctf_hash_integer,
			     ctf_hash_eq_integer, NULL, NULL)) == NULL)
    goto oom;
#endif

  if ((d->cd_input_nums
       = ctf_dynhash_create (ctf_hash_integer,
			     ctf_hash_eq_integer,
			     NULL, NULL)) == NULL)
    goto oom;

  if ((d->cd_emission_struct_members
       = ctf_dynhash_create (ctf_hash_integer,
			     ctf_hash_eq_integer,
			     NULL, NULL)) == NULL)
    goto oom;

  if ((d->cd_conflicting_types
       = ctf_dynset_create (htab_hash_string,
			    htab_eq_string, NULL)) == NULL)
    goto oom;

  return 0;

 oom:
  ctf_err_warn (fp, 0, ENOMEM, _("ctf_dedup_init: cannot initialize: "
				 "out of memory"));
  return ctf_set_errno (fp, ENOMEM);
}

/* No ctf_dedup calls are allowed after this call other than starting a new
   deduplication via ctf_dedup (not even ctf_dedup_type_mapping lookups).  */
void
ctf_dedup_fini (ctf_dict_t *fp, ctf_dict_t **outputs, uint32_t noutputs)
{
  ctf_dedup_t *d = &fp->ctf_dedup;
  size_t i;

  /* ctf_dedup_atoms is kept across links.  */
#if IDS_NEED_ALLOCATION
  ctf_dynhash_destroy (d->cd_id_to_dict_t);
#endif
  for (i = 0; i < 4; i++)
    ctf_dynhash_destroy (d->cd_decorated_names[i]);
  ctf_dynhash_destroy (d->cd_name_counts);
  ctf_dynhash_destroy (d->cd_type_hashes);
  ctf_dynhash_destroy (d->cd_struct_origin);
  ctf_dynhash_destroy (d->cd_citers);
  ctf_dynhash_destroy (d->cd_output_mapping);
  ctf_dynhash_destroy (d->cd_output_first_gid);
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
  ctf_dynhash_destroy (d->cd_output_mapping_guard);
#endif
  ctf_dynhash_destroy (d->cd_input_nums);
  ctf_dynhash_destroy (d->cd_emission_struct_members);
  ctf_dynset_destroy (d->cd_conflicting_types);

  /* Free the per-output state.  */
  if (outputs)
    {
      for (i = 0; i < noutputs; i++)
	{
	  ctf_dedup_t *od = &outputs[i]->ctf_dedup;
	  ctf_dynhash_destroy (od->cd_output_emission_hashes);
	  ctf_dynhash_destroy (od->cd_output_emission_conflicted_forwards);
	  ctf_dict_close (od->cd_output);
	}
    }
  memset (d, 0, sizeof (ctf_dedup_t));
}

/* Return 1 if this type is cited by multiple input dictionaries.  */

static int
ctf_dedup_multiple_input_dicts (ctf_dict_t *output, ctf_dict_t **inputs,
				const char *hval)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  ctf_dynset_t *type_ids;
  ctf_next_t *i = NULL;
  void *id;
  ctf_dict_t *found = NULL, *relative_found = NULL;
  const char *type_id;
  ctf_dict_t *input_fp;
  ctf_id_t input_id;
  const char *name;
  const char *decorated;
  int fwdkind;
  int multiple = 0;
  int err;

  type_ids = ctf_dynhash_lookup (d->cd_output_mapping, hval);
  if (!ctf_assert (output, type_ids))
    return -1;

  /* Scan across the IDs until we find proof that two disjoint dictionaries
     are referenced.  Exit as soon as possible.  Optimization opportunity, but
     possibly not worth it, given that this is only executed in
     CTF_LINK_SHARE_DUPLICATED mode.  */

  while ((err = ctf_dynset_next (type_ids, &i, &id)) == 0)
    {
      ctf_dict_t *fp = inputs[CTF_DEDUP_GID_TO_INPUT (id)];

      if (fp == found || fp == relative_found)
	continue;

      if (!found)
	{
	  found = fp;
	  continue;
	}

      if (!relative_found
	  && (fp->ctf_parent == found || found->ctf_parent == fp))
	{
	  relative_found = fp;
	  continue;
	}

      multiple = 1;
      ctf_next_destroy (i);
      break;
    }
  if ((err != ECTF_NEXT_END) && (err != 0))
    {
      ctf_err_warn (output, 0, err, _("iteration error "
				      "propagating conflictedness"));
      return ctf_set_errno (output, err);
    }

  if (multiple)
    return multiple;

  /* This type itself does not appear in multiple input dicts: how about another
     related type with the same name (e.g. a forward if this is a struct,
     etc).  */

  type_id = ctf_dynset_lookup_any (type_ids);
  if (!ctf_assert (output, type_id))
    return -1;

  input_fp = inputs[CTF_DEDUP_GID_TO_INPUT (type_id)];
  input_id = CTF_DEDUP_GID_TO_TYPE (type_id);
  fwdkind = ctf_type_kind_forwarded (input_fp, input_id);
  name = ctf_type_name_raw (input_fp, input_id);

  if ((fwdkind == CTF_K_STRUCT || fwdkind == CTF_K_UNION)
      && name[0] != '\0')
    {
      const void *origin;

      if ((decorated = ctf_decorate_type_name (output, name,
					       fwdkind)) == NULL)
	return -1;				/* errno is set for us.  */

      origin = ctf_dynhash_lookup (d->cd_struct_origin, decorated);
      if ((origin != NULL) && (CTF_DEDUP_GID_TO_INPUT (origin) < 0))
	multiple = 1;
    }

  return multiple;
}

/* Demote unconflicting types which reference only one input, or which reference
   two inputs where one input is the parent of the other, into conflicting
   types.  Only used if the link mode is CTF_LINK_SHARE_DUPLICATED.  */

static int
ctf_dedup_conflictify_unshared (ctf_dict_t *output, ctf_dict_t **inputs)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  ctf_next_t *i = NULL;
  int err;
  const void *k;
  ctf_dynset_t *to_mark = NULL;

  if ((to_mark = ctf_dynset_create (htab_hash_string, htab_eq_string,
				    NULL)) == NULL)
    goto err_no;

  while ((err = ctf_dynhash_cnext (d->cd_output_mapping, &i, &k, NULL)) == 0)
    {
      const char *hval = (const char *) k;
      int conflicting;

      /* Types referenced by only one dict, with no type appearing under that
	 name elsewhere, are marked conflicting.  */

      conflicting = !ctf_dedup_multiple_input_dicts (output, inputs, hval);

      if (conflicting < 0)
	goto err;				/* errno is set for us.  */

      if (conflicting)
	if (ctf_dynset_cinsert (to_mark, hval) < 0)
	  goto err;
    }
  if (err != ECTF_NEXT_END)
    goto iterr;

  while ((err = ctf_dynset_cnext (to_mark, &i, &k)) == 0)
    {
      const char *hval = (const char *) k;

      if (ctf_dedup_mark_conflicting_hash (output, hval) < 0)
	goto err;
    }
  if (err != ECTF_NEXT_END)
    goto iterr;

  ctf_dynset_destroy (to_mark);

  return 0;

 err_no:
  ctf_set_errno (output, errno);
 err:
  err = ctf_errno (output);
  ctf_next_destroy (i);
 iterr:
  ctf_dynset_destroy (to_mark);
  ctf_err_warn (output, 0, err, _("conflictifying unshared types"));
  return ctf_set_errno (output, err);
}

/* The core deduplicator.  Populate cd_output_mapping in the output ctf_dedup
   with a mapping of all types that belong in this dictionary and where they
   come from, and cd_conflicting_types with an indication of whether each type
   is conflicted or not.  OUTPUT is the top-level output: INPUTS is the array of
   input dicts; NINPUTS is the size of that array; PARENTS is an NINPUTS-element
   array with each element corresponding to a input which is a child dict set to
   the number in the INPUTS array of that input's parent.

   If CU_MAPPED is set, this is a first pass for a link with a non-empty CU
   mapping: only one output will result.

   Only deduplicates: does not emit the types into the output.  Call
   ctf_dedup_emit afterwards to do that.  */

int
ctf_dedup (ctf_dict_t *output, ctf_dict_t **inputs, uint32_t ninputs,
	   uint32_t *parents, int cu_mapped)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  size_t i;
  ctf_next_t *it = NULL;

  if (ctf_dedup_init (output) < 0)
    return -1; 					/* errno is set for us.  */

  for (i = 0; i < ninputs; i++)
    {
      ctf_dprintf ("Input %i: %s\n", (int) i, ctf_link_input_name (inputs[i]));
      if (ctf_dynhash_insert (d->cd_input_nums, inputs[i],
			      (void *) (uintptr_t) i) < 0)
	{
	  ctf_set_errno (output, errno);
	  ctf_err_warn (output, 0, errno, _("ctf_dedup: cannot initialize: %s\n"),
			ctf_errmsg (errno));
	  goto err;
	}
    }

  /* Some flags do not apply when CU-mapping: this is not a duplicated link,
     because there is only one output and we really don't want to end up marking
     all nonconflicting but appears-only-once types as conflicting (which in the
     CU-mapped link means we'd mark them all as non-root-visible!).  */
  d->cd_link_flags = output->ctf_link_flags;
  if (cu_mapped)
    d->cd_link_flags &= ~(CTF_LINK_SHARE_DUPLICATED);

  /* Compute hash values for all types, recursively, treating child structures
     and unions equivalent to forwards, and hashing in the name of the referent
     of each such type into structures, unions, and non-opaque forwards.
     Populate a mapping from decorated name (including an indication of
     struct/union/enum namespace) to count of type hash values in
     cd_name_counts, a mapping from and a mapping from hash values to input type
     IDs in cd_output_mapping.  */

  ctf_dprintf ("Computing type hashes\n");
  for (i = 0; i < ninputs; i++)
    {
      ctf_id_t id;

      while ((id = ctf_type_next (inputs[i], &it, NULL, 1)) != CTF_ERR)
	{
	  if (ctf_dedup_hash_type (output, inputs[i], inputs,
				   parents, i, id, 0, 0,
				   ctf_dedup_populate_mappings) == NULL)
	    goto err;				/* errno is set for us.  */
	}
      if (ctf_errno (inputs[i]) != ECTF_NEXT_END)
	{
	  ctf_set_errno (output, ctf_errno (inputs[i]));
	  ctf_err_warn (output, 0, 0, _("iteration failure "
					"computing type hashes"));
	  goto err;
	}
    }

  /* Go through the cd_name_counts name->hash->count mapping for all CTF
     namespaces: any name with many hashes associated with it at this stage is
     necessarily ambiguous.  Mark all the hashes except the most common as
     conflicting in the output.  */

  ctf_dprintf ("Detecting type name ambiguity\n");
  if (ctf_dedup_detect_name_ambiguity (output, inputs) < 0)
      goto err;					/* errno is set for us.  */

  /* If the link mode is CTF_LINK_SHARE_DUPLICATED, we change any unconflicting
     types whose output mapping references only one input dict into a
     conflicting type, so that they end up in the per-CU dictionaries.  */

  if (d->cd_link_flags & CTF_LINK_SHARE_DUPLICATED)
    {
      ctf_dprintf ("Conflictifying unshared types\n");
      if (ctf_dedup_conflictify_unshared (output, inputs) < 0)
	goto err;				/* errno is set for us.  */
    }
  return 0;

 err:
  ctf_dedup_fini (output, NULL, 0);
  return -1;
}

static int
ctf_dedup_rwalk_output_mapping (ctf_dict_t *output, ctf_dict_t **inputs,
				uint32_t ninputs, uint32_t *parents,
				ctf_dynset_t *already_visited,
				const char *hval,
				int (*visit_fun) (const char *hval,
						  ctf_dict_t *output,
						  ctf_dict_t **inputs,
						  uint32_t ninputs,
						  uint32_t *parents,
						  int already_visited,
						  ctf_dict_t *input,
						  ctf_id_t type,
						  void *id,
						  int depth,
						  void *arg),
				void *arg, unsigned long depth);

/* Like ctf_dedup_rwalk_output_mapping (which see), only takes a single target
   type and visits it.  */
static int
ctf_dedup_rwalk_one_output_mapping (ctf_dict_t *output,
				    ctf_dict_t **inputs, uint32_t ninputs,
				    uint32_t *parents,
				    ctf_dynset_t *already_visited,
				    int visited, void *type_id,
				    const char *hval,
				    int (*visit_fun) (const char *hval,
						      ctf_dict_t *output,
						      ctf_dict_t **inputs,
						      uint32_t ninputs,
						      uint32_t *parents,
						      int already_visited,
						      ctf_dict_t *input,
						      ctf_id_t type,
						      void *id,
						      int depth,
						      void *arg),
				    void *arg, unsigned long depth)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  ctf_dict_t *fp;
  int input_num;
  ctf_id_t type;
  int ret;
  const char *whaterr;

  input_num = CTF_DEDUP_GID_TO_INPUT (type_id);
  fp = inputs[input_num];
  type = CTF_DEDUP_GID_TO_TYPE (type_id);

  ctf_dprintf ("%lu: Starting walk over type %s, %i/%lx (%p), from %s, "
	       "kind %i\n", depth, hval, input_num, type, (void *) fp,
	       ctf_link_input_name (fp), ctf_type_kind_unsliced (fp, type));

  /* Get the single call we do if this type has already been visited out of the
     way.  */
  if (visited)
    return visit_fun (hval, output, inputs, ninputs, parents, visited, fp,
		      type, type_id, depth, arg);

  /* This macro is really ugly, but the alternative is repeating this code many
     times, which is worse.  */

#define CTF_TYPE_WALK(type, errlabel, errmsg)				\
  do									\
    {									\
      void *type_id;							\
      const char *hashval;						\
      int cited_type_input_num = input_num;				\
									\
      if ((fp->ctf_flags & LCTF_CHILD) && (LCTF_TYPE_ISPARENT (fp, type))) \
	cited_type_input_num = parents[input_num];			\
									\
      type_id = CTF_DEDUP_GID (output, cited_type_input_num, type);	\
									\
      if (type == 0)							\
	{								\
	  ctf_dprintf ("Walking: unimplemented type\n");		\
	  break;							\
	}								\
									\
      ctf_dprintf ("Looking up ID %i/%lx in type hashes\n",		\
		   cited_type_input_num, type);				\
      hashval = ctf_dynhash_lookup (d->cd_type_hashes, type_id);	\
      if (!ctf_assert (output, hashval))				\
	{								\
	  whaterr = N_("error looking up ID in type hashes");		\
	  goto errlabel;						\
	}								\
      ctf_dprintf ("ID %i/%lx has hash %s\n", cited_type_input_num, type, \
		   hashval);						\
									\
      ret = ctf_dedup_rwalk_output_mapping (output, inputs, ninputs, parents, \
					    already_visited, hashval,	\
					    visit_fun, arg, depth);	\
      if (ret < 0)							\
	{								\
	  whaterr = errmsg;						\
	  goto errlabel;						\
	}								\
    }									\
  while (0)

  switch (ctf_type_kind_unsliced (fp, type))
    {
    case CTF_K_UNKNOWN:
    case CTF_K_FORWARD:
    case CTF_K_INTEGER:
    case CTF_K_FLOAT:
    case CTF_K_ENUM:
      /* No types referenced.  */
      break;

    case CTF_K_TYPEDEF:
    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
    case CTF_K_POINTER:
    case CTF_K_SLICE:
      CTF_TYPE_WALK (ctf_type_reference (fp, type), err,
		     N_("error during referenced type walk"));
      break;

    case CTF_K_ARRAY:
      {
	ctf_arinfo_t ar;

	if (ctf_array_info (fp, type, &ar) < 0)
	  {
	    whaterr = N_("error during array info lookup");
	    goto err_msg;
	  }

	CTF_TYPE_WALK (ar.ctr_contents, err,
		       N_("error during array contents type walk"));
	CTF_TYPE_WALK (ar.ctr_index, err,
		       N_("error during array index type walk"));
	break;
      }

    case CTF_K_FUNCTION:
      {
	ctf_funcinfo_t fi;
	ctf_id_t *args;
	uint32_t j;

	if (ctf_func_type_info (fp, type, &fi) < 0)
	  {
	    whaterr = N_("error during func type info lookup");
	    goto err_msg;
	  }

	CTF_TYPE_WALK (fi.ctc_return, err,
		       N_("error during func return type walk"));

	if ((args = calloc (fi.ctc_argc, sizeof (ctf_id_t))) == NULL)
	  {
	    whaterr = N_("error doing memory allocation");
	    goto err_msg;
	  }

	if (ctf_func_type_args (fp, type, fi.ctc_argc, args) < 0)
	  {
	    whaterr = N_("error doing func arg type lookup");
	    free (args);
	    goto err_msg;
	  }

	for (j = 0; j < fi.ctc_argc; j++)
	  CTF_TYPE_WALK (args[j], err_free_args,
			 N_("error during Func arg type walk"));
	free (args);
	break;

      err_free_args:
	free (args);
	goto err;
      }
    case CTF_K_STRUCT:
    case CTF_K_UNION:
      /* We do not recursively traverse the members of structures: they are
	 emitted later, in a separate pass.  */
	break;
    default:
      whaterr = N_("CTF dict corruption: unknown type kind");
      goto err_msg;
    }

  return visit_fun (hval, output, inputs, ninputs, parents, visited, fp, type,
		    type_id, depth, arg);

 err_msg:
  ctf_set_errno (output, ctf_errno (fp));
  ctf_err_warn (output, 0, 0, _("%s in input file %s at type ID %lx"),
		gettext (whaterr), ctf_link_input_name (fp), type);
 err:
  return -1;
}
/* Recursively traverse the output mapping, and do something with each type
   visited, from leaves to root.  VISIT_FUN, called as recursion unwinds,
   returns a negative error code or zero.  Type hashes may be visited more than
   once, but are not recursed through repeatedly: ALREADY_VISITED tracks whether
   types have already been visited.  */
static int
ctf_dedup_rwalk_output_mapping (ctf_dict_t *output, ctf_dict_t **inputs,
				uint32_t ninputs, uint32_t *parents,
				ctf_dynset_t *already_visited,
				const char *hval,
				int (*visit_fun) (const char *hval,
						  ctf_dict_t *output,
						  ctf_dict_t **inputs,
						  uint32_t ninputs,
						  uint32_t *parents,
						  int already_visited,
						  ctf_dict_t *input,
						  ctf_id_t type,
						  void *id,
						  int depth,
						  void *arg),
				void *arg, unsigned long depth)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  ctf_next_t *i = NULL;
  int err;
  int visited = 1;
  ctf_dynset_t *type_ids;
  void *id;

  depth++;

  type_ids = ctf_dynhash_lookup (d->cd_output_mapping, hval);
  if (!type_ids)
    {
      ctf_err_warn (output, 0, ECTF_INTERNAL,
		    _("looked up type kind by nonexistent hash %s"), hval);
      return ctf_set_errno (output, ECTF_INTERNAL);
    }

  /* Have we seen this type before?  */

  if (!ctf_dynset_exists (already_visited, hval, NULL))
    {
      /* Mark as already-visited immediately, to eliminate the possibility of
	 cycles: but remember we have not actually visited it yet for the
	 upcoming call to the visit_fun.  (All our callers handle cycles
	 properly themselves, so we can just abort them aggressively as soon as
	 we find ourselves in one.)  */

      visited = 0;
      if (ctf_dynset_cinsert (already_visited, hval) < 0)
	{
	  ctf_err_warn (output, 0, ENOMEM,
			_("out of memory tracking already-visited types"));
	  return ctf_set_errno (output, ENOMEM);
	}
    }

  /* If this type is marked conflicted, traverse members and call
     ctf_dedup_rwalk_output_mapping_once on all the unique ones: otherwise, just
     pick a random one and use it.  */

  if (!ctf_dynset_exists (d->cd_conflicting_types, hval, NULL))
    {
      id = ctf_dynset_lookup_any (type_ids);
      if (!ctf_assert (output, id))
	return -1;

      return ctf_dedup_rwalk_one_output_mapping (output, inputs, ninputs,
						 parents, already_visited,
						 visited, id, hval, visit_fun,
						 arg, depth);
    }

  while ((err = ctf_dynset_next (type_ids, &i, &id)) == 0)
    {
      int ret;

      ret = ctf_dedup_rwalk_one_output_mapping (output, inputs, ninputs,
						parents, already_visited,
						visited, id, hval,
						visit_fun, arg, depth);
      if (ret < 0)
	{
	  ctf_next_destroy (i);
	  return ret;				/* errno is set for us.  */
	}
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (output, 0, err, _("cannot walk conflicted type"));
      return ctf_set_errno (output, err);
    }

  return 0;
}

typedef struct ctf_sort_om_cb_arg
{
  ctf_dict_t **inputs;
  uint32_t ninputs;
  ctf_dedup_t *d;
} ctf_sort_om_cb_arg_t;

/* Sort the output mapping into order: types first appearing in earlier inputs
   first, parents preceding children: if types first appear in the same input,
   sort those with earlier ctf_id_t's first.  */
static int
sort_output_mapping (const ctf_next_hkv_t *one, const ctf_next_hkv_t *two,
		     void *arg_)
{
  ctf_sort_om_cb_arg_t *arg = (ctf_sort_om_cb_arg_t *) arg_;
  ctf_dedup_t *d = arg->d;
  const char *one_hval = (const char *) one->hkv_key;
  const char *two_hval = (const char *) two->hkv_key;
  void *one_gid, *two_gid;
  uint32_t one_ninput;
  uint32_t two_ninput;
  ctf_dict_t *one_fp;
  ctf_dict_t *two_fp;
  ctf_id_t one_type;
  ctf_id_t two_type;

  /* Inputs are always equal to themselves.  */
  if (one == two)
    return 0;

  one_gid = ctf_dynhash_lookup (d->cd_output_first_gid, one_hval);
  two_gid = ctf_dynhash_lookup (d->cd_output_first_gid, two_hval);

  one_ninput = CTF_DEDUP_GID_TO_INPUT (one_gid);
  two_ninput = CTF_DEDUP_GID_TO_INPUT (two_gid);

  one_type = CTF_DEDUP_GID_TO_TYPE (one_gid);
  two_type = CTF_DEDUP_GID_TO_TYPE (two_gid);

  /* It's kind of hard to smuggle an assertion failure out of here.  */
  assert (one_ninput < arg->ninputs && two_ninput < arg->ninputs);

  one_fp = arg->inputs[one_ninput];
  two_fp = arg->inputs[two_ninput];

  /* Parents before children.  */

  if (!(one_fp->ctf_flags & LCTF_CHILD)
      && (two_fp->ctf_flags & LCTF_CHILD))
    return -1;
  else if ((one_fp->ctf_flags & LCTF_CHILD)
      && !(two_fp->ctf_flags & LCTF_CHILD))
    return 1;

  /* ninput order, types appearing in earlier TUs first.  */

  if (one_ninput < two_ninput)
    return -1;
  else if (two_ninput < one_ninput)
    return 1;

  /* Same TU.  Earliest ctf_id_t first.  They cannot be the same.  */

  assert (one_type != two_type);
  if (one_type < two_type)
    return -1;
  else
    return 1;
}

/* The public entry point to ctf_dedup_rwalk_output_mapping, above.  */
static int
ctf_dedup_walk_output_mapping (ctf_dict_t *output, ctf_dict_t **inputs,
			       uint32_t ninputs, uint32_t *parents,
			       int (*visit_fun) (const char *hval,
						 ctf_dict_t *output,
						 ctf_dict_t **inputs,
						 uint32_t ninputs,
						 uint32_t *parents,
						 int already_visited,
						 ctf_dict_t *input,
						 ctf_id_t type,
						 void *id,
						 int depth,
						 void *arg),
			       void *arg)
{
  ctf_dynset_t *already_visited;
  ctf_next_t *i = NULL;
  ctf_sort_om_cb_arg_t sort_arg;
  int err;
  void *k;

  if ((already_visited = ctf_dynset_create (htab_hash_string,
					    htab_eq_string,
					    NULL)) == NULL)
    return ctf_set_errno (output, ENOMEM);

  sort_arg.inputs = inputs;
  sort_arg.ninputs = ninputs;
  sort_arg.d = &output->ctf_dedup;

  while ((err = ctf_dynhash_next_sorted (output->ctf_dedup.cd_output_mapping,
					 &i, &k, NULL, sort_output_mapping,
					 &sort_arg)) == 0)
    {
      const char *hval = (const char *) k;

      err = ctf_dedup_rwalk_output_mapping (output, inputs, ninputs, parents,
					    already_visited, hval, visit_fun,
					    arg, 0);
      if (err < 0)
	{
	  ctf_next_destroy (i);
	  goto err;				/* errno is set for us.  */
	}
    }
  if (err != ECTF_NEXT_END)
    {
      ctf_err_warn (output, 0, err, _("cannot recurse over output mapping"));
      ctf_set_errno (output, err);
      goto err;
    }
  ctf_dynset_destroy (already_visited);

  return 0;
 err:
  ctf_dynset_destroy (already_visited);
  return -1;
}

/* Possibly synthesise a synthetic forward in TARGET to subsitute for a
   conflicted per-TU type ID in INPUT with hash HVAL.  Return its CTF ID, or 0
   if none was needed.  */
static ctf_id_t
ctf_dedup_maybe_synthesize_forward (ctf_dict_t *output, ctf_dict_t *target,
				    ctf_dict_t *input, ctf_id_t id,
				    const char *hval)
{
  ctf_dedup_t *od = &output->ctf_dedup;
  ctf_dedup_t *td = &target->ctf_dedup;
  int kind;
  int fwdkind;
  const char *name = ctf_type_name_raw (input, id);
  const char *decorated;
  void *v;
  ctf_id_t emitted_forward;

  if (!ctf_dynset_exists (od->cd_conflicting_types, hval, NULL)
      || target->ctf_flags & LCTF_CHILD
      || name[0] == '\0'
      || (((kind = ctf_type_kind_unsliced (input, id)) != CTF_K_STRUCT
	   && kind != CTF_K_UNION && kind != CTF_K_FORWARD)))
    return 0;

  fwdkind = ctf_type_kind_forwarded (input, id);

  ctf_dprintf ("Using synthetic forward for conflicted struct/union with "
	       "hval %s\n", hval);

  if (!ctf_assert (output, name))
    return CTF_ERR;

  if ((decorated = ctf_decorate_type_name (output, name, fwdkind)) == NULL)
    return CTF_ERR;

  if (!ctf_dynhash_lookup_kv (td->cd_output_emission_conflicted_forwards,
			      decorated, NULL, &v))
    {
      if ((emitted_forward = ctf_add_forward (target, CTF_ADD_ROOT, name,
					      fwdkind)) == CTF_ERR)
	{
	  ctf_set_errno (output, ctf_errno (target));
	  return CTF_ERR;
	}

      if (ctf_dynhash_cinsert (td->cd_output_emission_conflicted_forwards,
			       decorated, (void *) (uintptr_t)
			       emitted_forward) < 0)
	{
	  ctf_set_errno (output, ENOMEM);
	  return CTF_ERR;
	}
    }
  else
    emitted_forward = (ctf_id_t) (uintptr_t) v;

  ctf_dprintf ("Cross-TU conflicted struct: passing back forward, %lx\n",
	       emitted_forward);

  return emitted_forward;
}

/* Map a GID in some INPUT dict, in the form of an input number and a ctf_id_t,
   into a GID in a target output dict.  If it returns 0, this is the
   unimplemented type, and the input type must have been 0.  The OUTPUT dict is
   assumed to be the parent of the TARGET, if it is not the TARGET itself.

   Returns CTF_ERR on failure.  Responds to an incoming CTF_ERR as an 'id' by
   returning CTF_ERR, to simplify callers.  Errors are always propagated to the
   input, even if they relate to the target, for the same reason.  (Target
   errors are expected to be very rare.)

   If the type in question is a citation of a conflicted type in a different TU,
   emit a forward of the right type in its place (if not already emitted), and
   record that forward in cd_output_emission_conflicted_forwards.  This avoids
   the need to replicate the entire type graph below this point in the current
   TU (an appalling waste of space).

   TODO: maybe replace forwards in the same TU with their referents?  Might
   make usability a bit better.  */

static ctf_id_t
ctf_dedup_id_to_target (ctf_dict_t *output, ctf_dict_t *target,
			ctf_dict_t **inputs, uint32_t ninputs,
			uint32_t *parents, ctf_dict_t *input, int input_num,
			ctf_id_t id)
{
  ctf_dedup_t *od = &output->ctf_dedup;
  ctf_dedup_t *td = &target->ctf_dedup;
  ctf_dict_t *err_fp = input;
  const char *hval;
  void *target_id;
  ctf_id_t emitted_forward;

  /* The target type of an error is an error.  */
  if (id == CTF_ERR)
    return CTF_ERR;

  /* The unimplemented type's ID never changes.  */
  if (!id)
    {
      ctf_dprintf ("%i/%lx: unimplemented type\n", input_num, id);
      return 0;
    }

  ctf_dprintf ("Mapping %i/%lx to target %p (%s)\n", input_num,
	       id, (void *) target, ctf_link_input_name (target));

  /* If the input type is in the parent type space, and this is a child, reset
     the input to the parent (which must already have been emitted, since
     emission of parent dicts happens before children).  */
  if ((input->ctf_flags & LCTF_CHILD) && (LCTF_TYPE_ISPARENT (input, id)))
    {
      if (!ctf_assert (output, parents[input_num] <= ninputs))
	return -1;
      input = inputs[parents[input_num]];
      input_num = parents[input_num];
    }

  hval = ctf_dynhash_lookup (od->cd_type_hashes,
			     CTF_DEDUP_GID (output, input_num, id));

  if (!ctf_assert (output, hval && td->cd_output_emission_hashes))
    return -1;

  /* If this type is a conflicted tagged structure, union, or forward,
     substitute a synthetic forward instead, emitting it if need be.  Only do
     this if the target is in the parent dict: if it's in the child dict, we can
     just point straight at the thing itself.  Of course, we might be looking in
     the child dict right now and not find it and have to look in the parent, so
     we have to do this check twice.  */

  emitted_forward = ctf_dedup_maybe_synthesize_forward (output, target,
							input, id, hval);
  switch (emitted_forward)
    {
    case 0: /* No forward needed.  */
      break;
    case -1:
      ctf_set_errno (err_fp, ctf_errno (output));
      ctf_err_warn (err_fp, 0, 0, _("cannot add synthetic forward for type "
				    "%i/%lx"), input_num, id);
      return -1;
    default:
      return emitted_forward;
    }

  ctf_dprintf ("Looking up %i/%lx, hash %s, in target\n", input_num, id, hval);

  target_id = ctf_dynhash_lookup (td->cd_output_emission_hashes, hval);
  if (!target_id)
    {
      /* Must be in the parent, so this must be a child, and they must not be
	 the same dict.  */
      ctf_dprintf ("Checking shared parent for target\n");
      if (!ctf_assert (output, (target != output)
		       && (target->ctf_flags & LCTF_CHILD)))
	return -1;

      target_id = ctf_dynhash_lookup (od->cd_output_emission_hashes, hval);

      emitted_forward = ctf_dedup_maybe_synthesize_forward (output, output,
							    input, id, hval);
      switch (emitted_forward)
	{
	case 0: /* No forward needed.  */
	  break;
	case -1:
	  ctf_err_warn (err_fp, 0, ctf_errno (output),
			_("cannot add synthetic forward for type %i/%lx"),
			input_num, id);
	  return ctf_set_errno (err_fp, ctf_errno (output));
	default:
	  return emitted_forward;
	}
    }
  if (!ctf_assert (output, target_id))
    return -1;
  return (ctf_id_t) (uintptr_t) target_id;
}

/* Emit a single deduplicated TYPE with the given HVAL, located in a given
   INPUT, with the given (G)ID, into the shared OUTPUT or a
   possibly-newly-created per-CU dict.  All the types this type depends upon
   have already been emitted.  (This type itself may also have been emitted.)

   If the ARG is 1, this is a CU-mapped deduplication round mapping many
   ctf_dict_t's into precisely one: conflicting types should be marked
   non-root-visible.  If the ARG is 0, conflicting types go into per-CU
   dictionaries stored in the input's ctf_dedup.cd_output: otherwise, everything
   is emitted directly into the output.  No struct/union members are emitted.

   Optimization opportunity: trace the ancestry of non-root-visible types and
   elide all that neither have a root-visible type somewhere towards their root,
   nor have the type visible via any other route (the function info section,
   data object section, backtrace section etc).  */

static int
ctf_dedup_emit_type (const char *hval, ctf_dict_t *output, ctf_dict_t **inputs,
		     uint32_t ninputs, uint32_t *parents, int already_visited,
		     ctf_dict_t *input, ctf_id_t type, void *id, int depth,
		     void *arg)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  int kind = ctf_type_kind_unsliced (input, type);
  const char *name;
  ctf_dict_t *target = output;
  ctf_dict_t *real_input;
  const ctf_type_t *tp;
  int input_num = CTF_DEDUP_GID_TO_INPUT (id);
  int output_num = (uint32_t) -1;		/* 'shared' */
  int cu_mapped = *(int *)arg;
  int isroot = 1;
  int is_conflicting;

  ctf_next_t *i = NULL;
  ctf_id_t new_type;
  ctf_id_t ref;
  ctf_id_t maybe_dup = 0;
  ctf_encoding_t ep;
  const char *errtype;
  int emission_hashed = 0;

  /* We don't want to re-emit something we've already emitted.  */

  if (already_visited)
    return 0;

  ctf_dprintf ("%i: Emitting type with hash %s from %s: determining target\n",
	       depth, hval, ctf_link_input_name (input));

  /* Conflicting types go into a per-CU output dictionary, unless this is a
     CU-mapped run.  The import is not refcounted, since it goes into the
     ctf_link_outputs dict of the output that is its parent.  */
  is_conflicting = ctf_dynset_exists (d->cd_conflicting_types, hval, NULL);

  if (is_conflicting && !cu_mapped)
    {
      ctf_dprintf ("%i: Type %s in %i/%lx is conflicted: "
		   "inserting into per-CU target.\n",
		   depth, hval, input_num, type);

      if (input->ctf_dedup.cd_output)
	target = input->ctf_dedup.cd_output;
      else
	{
	  int err;

	  if ((target = ctf_create (&err)) == NULL)
	    {
	      ctf_err_warn (output, 0, err,
			    _("cannot create per-CU CTF archive for CU %s"),
			    ctf_link_input_name (input));
	      return ctf_set_errno (output, err);
	    }

	  ctf_import_unref (target, output);
	  if (ctf_cuname (input) != NULL)
	    ctf_cuname_set (target, ctf_cuname (input));
	  else
	    ctf_cuname_set (target, "unnamed-CU");
	  ctf_parent_name_set (target, _CTF_SECTION);

	  input->ctf_dedup.cd_output = target;
	  input->ctf_link_in_out = target;
	  target->ctf_link_in_out = input;
	}
      output_num = input_num;
    }

  real_input = input;
  if ((tp = ctf_lookup_by_id (&real_input, type)) == NULL)
    {
      ctf_err_warn (output, 0, ctf_errno (input),
		    _("%s: lookup failure for type %lx"),
		    ctf_link_input_name (real_input), type);
      return ctf_set_errno (output, ctf_errno (input));
    }

  name = ctf_strraw (real_input, tp->ctt_name);

  /* Hide conflicting types, if we were asked to: also hide if a type with this
     name already exists and is not a forward.  */
  if (cu_mapped && is_conflicting)
    isroot = 0;
  else if (name
	   && (maybe_dup = ctf_lookup_by_rawname (target, kind, name)) != 0)
    {
      if (ctf_type_kind (target, maybe_dup) != CTF_K_FORWARD)
	isroot = 0;
    }

  ctf_dprintf ("%i: Emitting type with hash %s (%s), into target %i/%p\n",
	       depth, hval, name ? name : "", input_num, (void *) target);

  if (!target->ctf_dedup.cd_output_emission_hashes)
    if ((target->ctf_dedup.cd_output_emission_hashes
	 = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			      NULL, NULL)) == NULL)
      goto oom_hash;

  if (!target->ctf_dedup.cd_output_emission_conflicted_forwards)
    if ((target->ctf_dedup.cd_output_emission_conflicted_forwards
	 = ctf_dynhash_create (ctf_hash_string, ctf_hash_eq_string,
			      NULL, NULL)) == NULL)
      goto oom_hash;

  switch (kind)
    {
    case CTF_K_UNKNOWN:
      /* These are types that CTF cannot encode, marked as such by the
	 compiler.  */
      errtype = _("unknown type");
      if ((new_type = ctf_add_unknown (target, isroot, name)) == CTF_ERR)
	goto err_target;
      break;
    case CTF_K_FORWARD:
      /* This will do nothing if the type to which this forwards already exists,
	 and will be replaced with such a type if it appears later.  */

      errtype = _("forward");
      if ((new_type = ctf_add_forward (target, isroot, name,
				       ctf_type_kind_forwarded (input, type)))
	  == CTF_ERR)
	goto err_target;
      break;

    case CTF_K_FLOAT:
    case CTF_K_INTEGER:
      errtype = _("float/int");
      if (ctf_type_encoding (input, type, &ep) < 0)
	goto err_input;				/* errno is set for us.  */
      if ((new_type = ctf_add_encoded (target, isroot, name, &ep, kind))
	  == CTF_ERR)
	goto err_target;
      break;

    case CTF_K_ENUM:
      {
	int val;
	errtype = _("enum");
	if ((new_type = ctf_add_enum (target, isroot, name)) == CTF_ERR)
	  goto err_input;				/* errno is set for us.  */

	while ((name = ctf_enum_next (input, type, &i, &val)) != NULL)
	  {
	    if (ctf_add_enumerator (target, new_type, name, val) < 0)
	      {
		ctf_err_warn (target, 0, ctf_errno (target),
			      _("%s (%i): cannot add enumeration value %s "
				"from input type %lx"),
			      ctf_link_input_name (input), input_num, name,
			      type);
		ctf_next_destroy (i);
		return ctf_set_errno (output, ctf_errno (target));
	      }
	  }
	if (ctf_errno (input) != ECTF_NEXT_END)
	  goto err_input;
	break;
      }

    case CTF_K_TYPEDEF:
      errtype = _("typedef");

      ref = ctf_type_reference (input, type);
      if ((ref = ctf_dedup_id_to_target (output, target, inputs, ninputs,
					 parents, input, input_num,
					 ref)) == CTF_ERR)
	goto err_input;				/* errno is set for us.  */

      if ((new_type = ctf_add_typedef (target, isroot, name, ref)) == CTF_ERR)
	goto err_target;			/* errno is set for us.  */
      break;

    case CTF_K_VOLATILE:
    case CTF_K_CONST:
    case CTF_K_RESTRICT:
    case CTF_K_POINTER:
      errtype = _("pointer or cvr-qual");

      ref = ctf_type_reference (input, type);
      if ((ref = ctf_dedup_id_to_target (output, target, inputs, ninputs,
					 parents, input, input_num,
					 ref)) == CTF_ERR)
	goto err_input;				/* errno is set for us.  */

      if ((new_type = ctf_add_reftype (target, isroot, ref, kind)) == CTF_ERR)
	goto err_target;			/* errno is set for us.  */
      break;

    case CTF_K_SLICE:
      errtype = _("slice");

      if (ctf_type_encoding (input, type, &ep) < 0)
	goto err_input;				/* errno is set for us.  */

      ref = ctf_type_reference (input, type);
      if ((ref = ctf_dedup_id_to_target (output, target, inputs, ninputs,
					 parents, input, input_num,
					 ref)) == CTF_ERR)
	goto err_input;

      if ((new_type = ctf_add_slice (target, isroot, ref, &ep)) == CTF_ERR)
	goto err_target;
      break;

    case CTF_K_ARRAY:
      {
	ctf_arinfo_t ar;

	errtype = _("array info");
	if (ctf_array_info (input, type, &ar) < 0)
	  goto err_input;

	ar.ctr_contents = ctf_dedup_id_to_target (output, target, inputs,
						  ninputs, parents, input,
						  input_num, ar.ctr_contents);
	ar.ctr_index = ctf_dedup_id_to_target (output, target, inputs, ninputs,
					       parents, input, input_num,
					       ar.ctr_index);

	if (ar.ctr_contents == CTF_ERR || ar.ctr_index == CTF_ERR)
	  goto err_input;

	if ((new_type = ctf_add_array (target, isroot, &ar)) == CTF_ERR)
	  goto err_target;

	break;
      }

    case CTF_K_FUNCTION:
      {
	ctf_funcinfo_t fi;
	ctf_id_t *args;
	uint32_t j;

	errtype = _("function");
	if (ctf_func_type_info (input, type, &fi) < 0)
	  goto err_input;

	fi.ctc_return = ctf_dedup_id_to_target (output, target, inputs, ninputs,
						parents, input, input_num,
						fi.ctc_return);
	if (fi.ctc_return == CTF_ERR)
	  goto err_input;

	if ((args = calloc (fi.ctc_argc, sizeof (ctf_id_t))) == NULL)
	  {
	    ctf_set_errno (input, ENOMEM);
	    goto err_input;
	  }

	errtype = _("function args");
	if (ctf_func_type_args (input, type, fi.ctc_argc, args) < 0)
	  {
	    free (args);
	    goto err_input;
	  }

	for (j = 0; j < fi.ctc_argc; j++)
	  {
	    args[j] = ctf_dedup_id_to_target (output, target, inputs, ninputs,
					      parents, input, input_num,
					      args[j]);
	    if (args[j] == CTF_ERR)
	      goto err_input;
	  }

	if ((new_type = ctf_add_function (target, isroot,
					  &fi, args)) == CTF_ERR)
	  {
	    free (args);
	    goto err_target;
	  }
	free (args);
	break;
      }

    case CTF_K_STRUCT:
    case CTF_K_UNION:
      {
	size_t size = ctf_type_size (input, type);
	void *out_id;
	/* Insert the structure itself, so other types can refer to it.  */

	errtype = _("structure/union");
	if (kind == CTF_K_STRUCT)
	  new_type = ctf_add_struct_sized (target, isroot, name, size);
	else
	  new_type = ctf_add_union_sized (target, isroot, name, size);

	if (new_type == CTF_ERR)
	  goto err_target;

	out_id = CTF_DEDUP_GID (output, output_num, new_type);
	ctf_dprintf ("%i: Noting need to emit members of %p -> %p\n", depth,
		     id, out_id);
	/* Record the need to emit the members of this structure later.  */
	if (ctf_dynhash_insert (d->cd_emission_struct_members, id, out_id) < 0)
	  {
	    ctf_set_errno (target, errno);
	    goto err_target;
	  }
	break;
      }
    default:
      ctf_err_warn (output, 0, ECTF_CORRUPT, _("%s: unknown type kind for "
					       "input type %lx"),
		    ctf_link_input_name (input), type);
      return ctf_set_errno (output, ECTF_CORRUPT);
    }

  if (!emission_hashed
      && new_type != 0
      && ctf_dynhash_cinsert (target->ctf_dedup.cd_output_emission_hashes,
			      hval, (void *) (uintptr_t) new_type) < 0)
    {
      ctf_err_warn (output, 0, ENOMEM, _("out of memory tracking deduplicated "
					 "global type IDs"));
	return ctf_set_errno (output, ENOMEM);
    }

  if (!emission_hashed && new_type != 0)
    ctf_dprintf ("%i: Inserted %s, %i/%lx -> %lx into emission hash for "
		 "target %p (%s)\n", depth, hval, input_num, type, new_type,
		 (void *) target, ctf_link_input_name (target));

  return 0;

 oom_hash:
  ctf_err_warn (output, 0, ENOMEM, _("out of memory creating emission-tracking "
				     "hashes"));
  return ctf_set_errno (output, ENOMEM);

 err_input:
  ctf_err_warn (output, 0, ctf_errno (input),
		_("%s (%i): while emitting deduplicated %s, error getting "
		  "input type %lx"), ctf_link_input_name (input),
		input_num, errtype, type);
  return ctf_set_errno (output, ctf_errno (input));
 err_target:
  ctf_err_warn (output, 0, ctf_errno (target),
		_("%s (%i): while emitting deduplicated %s, error emitting "
		  "target type from input type %lx"),
		ctf_link_input_name (input), input_num,
		errtype, type);
  return ctf_set_errno (output, ctf_errno (target));
}

/* Traverse the cd_emission_struct_members and emit the members of all
   structures and unions.  All other types are emitted and complete by this
   point.  */

static int
ctf_dedup_emit_struct_members (ctf_dict_t *output, ctf_dict_t **inputs,
			       uint32_t ninputs, uint32_t *parents)
{
  ctf_dedup_t *d = &output->ctf_dedup;
  ctf_next_t *i = NULL;
  void *input_id, *target_id;
  int err;
  ctf_dict_t *err_fp, *input_fp;
  int input_num;
  ctf_id_t err_type;

  while ((err = ctf_dynhash_next (d->cd_emission_struct_members, &i,
				  &input_id, &target_id)) == 0)
    {
      ctf_next_t *j = NULL;
      ctf_dict_t *target;
      uint32_t target_num;
      ctf_id_t input_type, target_type;
      ssize_t offset;
      ctf_id_t membtype;
      const char *name;

      input_num = CTF_DEDUP_GID_TO_INPUT (input_id);
      input_fp = inputs[input_num];
      input_type = CTF_DEDUP_GID_TO_TYPE (input_id);

      /* The output is either -1 (for the shared, parent output dict) or the
	 number of the corresponding input.  */
      target_num = CTF_DEDUP_GID_TO_INPUT (target_id);
      if (target_num == (uint32_t) -1)
	target = output;
      else
	{
	  target = inputs[target_num]->ctf_dedup.cd_output;
	  if (!ctf_assert (output, target))
	    {
	      err_fp = output;
	      err_type = input_type;
	      goto err_target;
	    }
	}
      target_type = CTF_DEDUP_GID_TO_TYPE (target_id);

      while ((offset = ctf_member_next (input_fp, input_type, &j, &name,
					&membtype, 0)) >= 0)
	{
	  err_fp = target;
	  err_type = target_type;
	  if ((membtype = ctf_dedup_id_to_target (output, target, inputs,
						  ninputs, parents, input_fp,
						  input_num,
						  membtype)) == CTF_ERR)
	    {
	      ctf_next_destroy (j);
	      goto err_target;
	    }

	  if (name == NULL)
	    name = "";
#ifdef ENABLE_LIBCTF_HASH_DEBUGGING
	  ctf_dprintf ("Emitting %s, offset %zi\n", name, offset);
#endif
	  if (ctf_add_member_offset (target, target_type, name,
				     membtype, offset) < 0)
	    {
	      ctf_next_destroy (j);
	      goto err_target;
	    }
	}
      if (ctf_errno (input_fp) != ECTF_NEXT_END)
	{
	  err = ctf_errno (input_fp);
	  ctf_next_destroy (i);
	  goto iterr;
	}
    }
  if (err != ECTF_NEXT_END)
    goto iterr;

  return 0;
 err_target:
  ctf_next_destroy (i);
  ctf_err_warn (output, 0, ctf_errno (err_fp),
		_("%s (%i): error emitting members for structure type %lx"),
		ctf_link_input_name (input_fp), input_num, err_type);
  return ctf_set_errno (output, ctf_errno (err_fp));
 iterr:
  ctf_err_warn (output, 0, err, _("iteration failure emitting "
				  "structure members"));
  return ctf_set_errno (output, err);
}

/* Emit deduplicated types into the outputs.  The shared type repository is
   OUTPUT, on which the ctf_dedup function must have already been called.  The
   PARENTS array contains the INPUTS index of the parent dict for every child
   dict at the corresponding index in the INPUTS (for non-child dicts, the value
   is undefined).

   Return an array of fps with content emitted into them (starting with OUTPUT,
   which is the parent of all others, then all the newly-generated outputs).

   If CU_MAPPED is set, this is a first pass for a link with a non-empty CU
   mapping: only one output will result.  */

ctf_dict_t **
ctf_dedup_emit (ctf_dict_t *output, ctf_dict_t **inputs, uint32_t ninputs,
		uint32_t *parents, uint32_t *noutputs, int cu_mapped)
{
  size_t num_outputs = 1;		/* Always at least one output: us.  */
  ctf_dict_t **outputs;
  ctf_dict_t **walk;
  size_t i;

  ctf_dprintf ("Triggering emission.\n");
  if (ctf_dedup_walk_output_mapping (output, inputs, ninputs, parents,
				     ctf_dedup_emit_type, &cu_mapped) < 0)
    return NULL;				/* errno is set for us.  */

  ctf_dprintf ("Populating struct members.\n");
  if (ctf_dedup_emit_struct_members (output, inputs, ninputs, parents) < 0)
    return NULL;				/* errno is set for us.  */

  for (i = 0; i < ninputs; i++)
    {
      if (inputs[i]->ctf_dedup.cd_output)
	num_outputs++;
    }

  if (!ctf_assert (output, !cu_mapped || (cu_mapped && num_outputs == 1)))
    return NULL;

  if ((outputs = calloc (num_outputs, sizeof (ctf_dict_t *))) == NULL)
    {
      ctf_err_warn (output, 0, ENOMEM,
		    _("out of memory allocating link outputs array"));
      ctf_set_errno (output, ENOMEM);
      return NULL;
    }
  *noutputs = num_outputs;

  walk = outputs;
  *walk = output;
  output->ctf_refcnt++;
  walk++;

  for (i = 0; i < ninputs; i++)
    {
      if (inputs[i]->ctf_dedup.cd_output)
	{
	  *walk = inputs[i]->ctf_dedup.cd_output;
	  inputs[i]->ctf_dedup.cd_output = NULL;
	  walk++;
	}
    }

  return outputs;
}

/* Determine what type SRC_FP / SRC_TYPE was emitted as in the FP, which
   must be the shared dict or have it as a parent: return 0 if none.  The SRC_FP
   must be a past input to ctf_dedup.  */

ctf_id_t
ctf_dedup_type_mapping (ctf_dict_t *fp, ctf_dict_t *src_fp, ctf_id_t src_type)
{
  ctf_dict_t *output = NULL;
  ctf_dedup_t *d;
  int input_num;
  void *num_ptr;
  void *type_ptr;
  int found;
  const char *hval;

  /* It is an error (an internal error in the caller, in ctf-link.c) to call
     this with an FP that is not a per-CU output or shared output dict, or with
     a SRC_FP that was not passed to ctf_dedup as an input; it is an internal
     error in ctf-dedup for the type passed not to have been hashed, though if
     the src_fp is a child dict and the type is not a child type, it will have
     been hashed under the GID corresponding to the parent.  */

  if (fp->ctf_dedup.cd_type_hashes != NULL)
    output = fp;
  else if (fp->ctf_parent && fp->ctf_parent->ctf_dedup.cd_type_hashes != NULL)
    output = fp->ctf_parent;
  else
    {
      ctf_set_errno (fp, ECTF_INTERNAL);
      ctf_err_warn (fp, 0, ECTF_INTERNAL,
		    _("dict %p passed to ctf_dedup_type_mapping is not a "
		      "deduplicated output"), (void *) fp);
      return CTF_ERR;
    }

  if (src_fp->ctf_parent && ctf_type_isparent (src_fp, src_type))
    src_fp = src_fp->ctf_parent;

  d = &output->ctf_dedup;

  found = ctf_dynhash_lookup_kv (d->cd_input_nums, src_fp, NULL, &num_ptr);
  if (!ctf_assert (output, found != 0))
    return CTF_ERR;				/* errno is set for us.  */
  input_num = (uintptr_t) num_ptr;

  hval = ctf_dynhash_lookup (d->cd_type_hashes,
			     CTF_DEDUP_GID (output, input_num, src_type));

  if (!ctf_assert (output, hval != NULL))
    return CTF_ERR;				/* errno is set for us.  */

  /* The emission hashes may be unset if this dict was created after
     deduplication to house variables or other things that would conflict if
     stored in the shared dict.  */
  if (fp->ctf_dedup.cd_output_emission_hashes)
    if (ctf_dynhash_lookup_kv (fp->ctf_dedup.cd_output_emission_hashes, hval,
			       NULL, &type_ptr))
      return (ctf_id_t) (uintptr_t) type_ptr;

  if (fp->ctf_parent)
    {
      ctf_dict_t *pfp = fp->ctf_parent;
      if (pfp->ctf_dedup.cd_output_emission_hashes)
	if (ctf_dynhash_lookup_kv (pfp->ctf_dedup.cd_output_emission_hashes,
				   hval, NULL, &type_ptr))
	  return (ctf_id_t) (uintptr_t) type_ptr;
    }

  return 0;
}
