/*
 * array.c - functions to create, destroy, access, and manipulate arrays
 *	     of strings.
 *
 * Arrays are structs containing an array of elements and bookkeeping information.
 * An element's index is stored with it.
 *
 * Chet Ramey
 * chet@ins.cwru.edu
 */

/* Copyright (C) 1997-2021 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#if defined (ARRAY_VARS)

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <stdio.h>
#include "bashansi.h"

#include "shell.h"
#include "array.h"
#include "builtins/common.h"

#define ARRAY_MAX_DOUBLE	16777216

static ARRAY_ELEMENT **array_copy_elements PARAMS((ARRAY *));
static char *array_to_string_internal PARAMS((ARRAY *, arrayind_t, arrayind_t, char *, int));

static char *spacesep = " ";

void
array_alloc (a, n)
ARRAY	*a;
arrayind_t n;
{
	arrayind_t i;

	if (a == 0)
		return;	/* for now */
	a->elements = (ARRAY_ELEMENT **)xmalloc (n * sizeof (ARRAY_ELEMENT *));
	for (i = 0; i < n; i++)
		a->elements[i] = 0;
	a->alloc_size = n;
}

void
array_resize (a, n)
ARRAY	*a;
arrayind_t n;
{
	ARRAY_ELEMENT **e, *ae;
	arrayind_t i, nsize;

	if (a == 0)
		return;
	if (a->alloc_size > 0 && n >= a->max_index && n <= a->alloc_size)
		return;

	e = (ARRAY_ELEMENT **)xrealloc (a->elements, n * sizeof (ARRAY_ELEMENT *));
	a->elements = e;

	for (i = a->alloc_size; i < n; i++)
		a->elements[i] = (ARRAY_ELEMENT *)NULL;

	a->alloc_size = n;
}

void
array_expand (a, n)
ARRAY	*a;
arrayind_t	n;
{
	arrayind_t nsize;

	if (n >= a->alloc_size) {
		nsize = a->alloc_size ? a->alloc_size : ARRAY_DEFAULT_SIZE;
		while (n >= nsize)
			nsize <<= 1;
		if (nsize > ARRAY_MAX_DOUBLE)
			nsize = n + ARRAY_DEFAULT_SIZE;
		array_resize (a, nsize);
	}
}
	
ARRAY *
array_create()
{
	ARRAY	*r;

	r = (ARRAY *)xmalloc(sizeof(ARRAY));
	r->max_index = r->first_index = -1;
	r->num_elements = 0;
	r->alloc_size = 0;
	r->elements = (ARRAY_ELEMENT **)NULL;
	return(r);
}

void
array_flush (a)
ARRAY	*a;
{
	int r;

	if (a == 0)
		return;
	if (array_empty(a)) {
		a->max_index = a->first_index = -1;	/* paranoia */
		return;
	}
 	for (r = a->first_index; r <= a->max_index; r++)
		if (a->elements[r]) {
			array_dispose_element(a->elements[r]);
			a->elements[r] = 0;
		}
	a->max_index = a->first_index = -1;
	a->num_elements = 0;
}

void
array_dispose_elements(elist)
ARRAY_ELEMENT	**elist;
{
	arrayind_t i;

	if (elist == 0)
		return;
	for (i = 0; elist[i]; i++)
		array_dispose_element(elist[i]);
	free(elist);
}

void
array_dispose(a)
ARRAY	*a;
{
	if (a == 0)
		return;
	array_dispose_elements (a->elements);
	a->alloc_size = 0;
	free(a);
}

static ARRAY_ELEMENT **
array_copy_elements (a)
ARRAY	*a;
{
	ARRAY_ELEMENT **ret;
	arrayind_t i;

	if (a == 0 || a->num_elements == 0)
		return (ARRAY_ELEMENT **)NULL;
	ret = (ARRAY_ELEMENT **)xmalloc (a->alloc_size * sizeof (ARRAY_ELEMENT *));
	for (i = 0; i < a->alloc_size; i++)
		ret[i] = a->elements[i] ? array_copy_element (a->elements[i]) : 0;
	return ret;
}

ARRAY *
array_copy(a)
ARRAY	*a;
{
	ARRAY *a1;

	if (a == 0)
		return((ARRAY *) NULL);
	a1 = array_create();
	a1->max_index = a->max_index;
	a1->first_index = a->first_index;
	a1->num_elements = a->num_elements;

	a1->alloc_size = a->alloc_size;
	a1->elements = array_copy_elements (a);

	return(a1);
}

/*
 * Make and return a new array composed of the elements in array A from
 * S to E, inclusive. The callers do the bounds checking.
 */
ARRAY *
array_slice(array, s, e)
ARRAY		*array;
arrayind_t	s, e;
{
	ARRAY	*a;
	ARRAY_ELEMENT *p, *n;
	arrayind_t i, ni;
	arrayind_t mi, nsize;

	a = array_create ();

	nsize = ARRAY_DEFAULT_SIZE;
	while (nsize < array->alloc_size)
		nsize <<= 1;
	if (nsize > ARRAY_MAX_DOUBLE)
		nsize = array->alloc_size + ARRAY_DEFAULT_SIZE;

	array_resize (a, nsize);

	for (i = s; i < e; i++) {
		p = array->elements[i];
		n = p ? array_create_element (element_index(p), element_value(p)) : (ARRAY_ELEMENT *)NULL;
		a->elements[i] = n;		
	}
	a->num_elements = e - s;
	a->max_index = e;
	a->first_index = s;

	return a;
}

/*
 * Walk the array, calling FUNC once for each element, with the array
 * element as the argument.
 */
void
array_walk(a, func, udata)
ARRAY	*a;
sh_ae_map_func_t *func;
void	*udata;
{
	arrayind_t i;
	register ARRAY_ELEMENT *ae;

	if (a == 0 || array_empty(a))
		return;
	for (i = array_first_index (a); i <= array_max_index(a); i++) {
		if ((ae = a->elements[i]) == 0)
			continue;
		if ((*func)(ae, udata) < 0)
			return;
	}
}

/*
 * Shift the array A N elements to the left.  Delete the first N elements
 * and subtract N from the indices of the remaining elements.  If FLAGS
 * does not include AS_DISPOSE, this returns a null-terminated array of
 * elements so the caller can dispose of the chain.  If FLAGS includes
 * AS_DISPOSE, this function disposes of the shifted-out elements and
 * returns NULL.
 */
ARRAY_ELEMENT **
array_shift(a, n, flags)
ARRAY	*a;
int	n, flags;
{
	ARRAY_ELEMENT **r, *ae;
	register arrayind_t ni, ri;
	int	i, j;

	if (a == 0 || array_empty(a) || n <= 0)
		return ((ARRAY_ELEMENT **)NULL);

	r = (ARRAY_ELEMENT **)xmalloc ((n + 1) * sizeof (ARRAY_ELEMENT *));

	/* Easy case; shifting out all of the elements */
	if (n >= a->num_elements) {
		if (flags & AS_DISPOSE) {
			array_flush (a);
			return ((ARRAY_ELEMENT **)NULL);
		}
		for (ri = 0, i = a->first_index; i <= a->max_index; i++)
			if (a->elements[i]) {
				r[ri++] = a->elements[i];
				a->elements[i] = 0;
			}

		a->first_index = a->max_index = -1;
		a->num_elements = 0;
		r[ri] = (ARRAY_ELEMENT *)NULL;
		return r;
	}

	/* Shift out the first N elements, return them in R. Handle sparse
	   arrays by skipping over NULL array elements. */
	for (i = a->first_index, ri = 0, j = 0; j < n; i++) {
		if ((ae = a->elements[i]) == 0)
			continue;
		if (i > a->max_index)
			break;
		ni = i + n;
		j++;
		if (ae)
			r[ri++] = a->elements[i];
		a->elements[i] = a->elements[ni];
		if (a->elements[i])
			element_index(a->elements[i]) = i;
	}
	r[ri]= (ARRAY_ELEMENT *)NULL;

#ifdef DEBUG
if (j < n)
	itrace("array_shift: short count: j = %d n = %d", j, n);
#endif

	/* Now shift everything else, modifying the index in each element */
	for (; i <= a->max_index; i++) {
		ni = i + n;
		a->elements[i] = (ni <= a->max_index) ? a->elements[ni] : (ARRAY_ELEMENT *)NULL;
		if (a->elements[i])
			element_index(a->elements[i]) = i;
	}

	a->num_elements -= n;		/* modify bookkeeping information */
	if (a->num_elements == 0)
		a->first_index = a->max_index == -1;
	else {
		a->max_index -= n;
		for (i = 0; i <= a->max_index; i++)
			if (a->elements[i])
				break;
		a->first_index = i;
	}

	if (flags & AS_DISPOSE) {
		for (i = 0; i < ri; i++)
			array_dispose_element(r[i]);
		free (r);
		return ((ARRAY_ELEMENT **)NULL);
	}

	return r;
}

/*
 * Shift array A right N indices.  If S is non-null, it becomes the value of
 * the new element 0.  Returns the number of elements in the array after the
 * shift.
 */
int
array_rshift (a, n, s)
ARRAY	*a;
int	n;
char	*s;
{
	register ARRAY_ELEMENT	*ae, *new;
	arrayind_t ni, nsize;

	if (a == 0 || (array_empty(a) && s == 0))
		return 0;
	else if (n <= 0)
		return (a->num_elements);

	if (n >= a->alloc_size)
		array_expand(a, n);

	/* Shift right, adjusting the element indexes as we go */
	for (ni = a->max_index; ni >= 0; ni--) {
		a->elements[ni+n] = a->elements[ni];
		if (a->elements[ni+n])
			element_index(a->elements[ni+n]) = ni + n;
		a->elements[ni] = (ARRAY_ELEMENT *)NULL;
	}
	a->max_index += n;

#if 0
	/* Null out all the old indexes we just copied from */
	for (ni = a->first_index; ni >= 0 && ni < n; ni++)
		a->elements[ni] = (ARRAY_ELEMENT *)NULL;
#endif
	a->first_index += n;
	
	if (s) {
		new = array_create_element(0, s);
		a->elements[0] = new;
		a->num_elements++;
		a->first_index = 0;
		if (array_num_elements(a) == 1)		/* array was empty */
			a->max_index = 0;
	}

	return (a->num_elements);
}

ARRAY_ELEMENT *
array_unshift_element(a)
ARRAY	*a;
{
	ARRAY_ELEMENT **r, *ret;
	
	r = array_shift (a, 1, 0);
	ret = r[0];
	free (r);
	return ret;
}

int
array_shift_element(a, v)
ARRAY	*a;
char	*v;
{
	return (array_rshift (a, 1, v));
}

ARRAY *
array_quote(array)
ARRAY	*array;
{
	register arrayind_t i;
	ARRAY_ELEMENT	*a;
	char	*t;

	if (array == 0 || array_head(array) == 0 || array_empty(array))
		return (ARRAY *)NULL;
	for (i = array_first_index(array); i <= array_max_index(array); i++) {
		if ((a = array->elements[i]) == 0)
			continue;
		t = quote_string (a->value);
		FREE(a->value);
		a->value = t;
	}
	return array;
}

ARRAY *
array_quote_escapes(array)
ARRAY	*array;
{
	register arrayind_t i;
	ARRAY_ELEMENT	*a;
	char	*t;

	if (array == 0 || array_head(array) == 0 || array_empty(array))
		return (ARRAY *)NULL;
	for (i = array_first_index(array); i <= array_max_index(array); i++) {
		if ((a = array->elements[i]) == 0)
			continue;
		t = quote_escapes (a->value);
		FREE(a->value);
		a->value = t;
	}
	return array;
}

ARRAY *
array_dequote(array)
ARRAY	*array;
{
	register arrayind_t i;
	ARRAY_ELEMENT	*a;
	char	*t;

	if (array == 0 || array_head(array) == 0 || array_empty(array))
		return (ARRAY *)NULL;

	for (i = array->first_index; i <= array->max_index; i++) {
		if ((a = array->elements[i]) == 0)
			continue;
		t = dequote_string (a->value);
		FREE(a->value);
		a->value = t;
	}
	return array;
}

ARRAY *
array_dequote_escapes(array)
ARRAY	*array;
{
	register arrayind_t i;
	ARRAY_ELEMENT	*a;
	char	*t;

	if (array == 0 || array_head(array) == 0 || array_empty(array))
		return (ARRAY *)NULL;
	for (i = array->first_index; i <= array->max_index; i++) {
		if ((a = array->elements[i]) == 0)
			continue;
		t = dequote_escapes (a->value);
		FREE(a->value);
		a->value = t;
	}
	return array;
}

ARRAY *
array_remove_quoted_nulls(array)
ARRAY	*array;
{
	register arrayind_t i;
	ARRAY_ELEMENT	*a;

	if (array == 0 || array_head(array) == 0 || array_empty(array))
		return (ARRAY *)NULL;
	for (i = array->first_index; i <= array->max_index; i++) {
		if ((a = array->elements[i]) == 0)
			continue;
		a->value = remove_quoted_nulls (a->value);
	}
	return array;
}

/*
 * Return a string whose elements are the members of array A beginning at
 * index START and spanning NELEM members.  Null elements are counted.
 * Since arrays are sparse, unset array elements are not counted.
 */
char *
array_subrange (a, start, nelem, starsub, quoted, pflags)
ARRAY	*a;
arrayind_t	start, nelem;
int	starsub, quoted, pflags;
{
	ARRAY		*a2;
	arrayind_t	s, e;
	int		i;
	char		*t;
	WORD_LIST	*wl;

	if (array_empty (a) || start > array_max_index(a))
		return ((char *)NULL);

	/*
	 * Find element with index START.  If START corresponds to an unset
	 * element (arrays can be sparse), use the first element whose index
	 * is >= START.  If START is < 0, we count START indices back from
	 * the end of A (not elements, even with sparse arrays -- START is an
	 * index).
	 */
	for (s = start; a->elements[s] == 0 && s <= a->max_index; s++)
		;

	if (s > a->max_index)
		return ((char *)NULL);

	/* Starting at S, take NELEM elements, inclusive. */
	for (i = 0, e = s; e <= a->max_index && i < nelem; e++) {
		if (a->elements[e])		/* arrays are sparse */
			i++;
	}

	a2 = array_slice(a, s, e);

	wl = array_to_word_list(a2);
	array_dispose(a2);
	if (wl == 0)
		return (char *)NULL;
	t = string_list_pos_params(starsub ? '*' : '@', wl, quoted, pflags);	/* XXX */
	dispose_words(wl);

	return t;
}

char *
array_patsub (a, pat, rep, mflags)
ARRAY	*a;
char	*pat, *rep;
int	mflags;
{
	char	*t;
	int	pchar, qflags, pflags;
	WORD_LIST	*wl, *save;

	if (a == 0 || array_head(a) == 0 || array_empty(a))
		return ((char *)NULL);

	wl = array_to_word_list(a);
	if (wl == 0)
		return (char *)NULL;

	for (save = wl; wl; wl = wl->next) {
		t = pat_subst (wl->word->word, pat, rep, mflags);
		FREE (wl->word->word);
		wl->word->word = t;
	}

	pchar = (mflags & MATCH_STARSUB) == MATCH_STARSUB ? '*' : '@';
	qflags = (mflags & MATCH_QUOTED) == MATCH_QUOTED ? Q_DOUBLE_QUOTES : 0;
	pflags = (mflags & MATCH_ASSIGNRHS) ? PF_ASSIGNRHS : 0;

	t = string_list_pos_params (pchar, save, qflags, pflags);
	dispose_words(save);

	return t;
}

char *
array_modcase (a, pat, modop, mflags)
ARRAY	*a;
char	*pat;
int	modop;
int	mflags;
{
	char	*t;
	int	pchar, qflags, pflags;
	WORD_LIST	*wl, *save;

	if (a == 0 || array_head(a) == 0 || array_empty(a))
		return ((char *)NULL);

	wl = array_to_word_list(a);
	if (wl == 0)
		return ((char *)NULL);

	for (save = wl; wl; wl = wl->next) {
		t = sh_modcase(wl->word->word, pat, modop);
		FREE(wl->word->word);
		wl->word->word = t;
	}

	pchar = (mflags & MATCH_STARSUB) == MATCH_STARSUB ? '*' : '@';
	qflags = (mflags & MATCH_QUOTED) == MATCH_QUOTED ? Q_DOUBLE_QUOTES : 0;
	pflags = (mflags & MATCH_ASSIGNRHS) ? PF_ASSIGNRHS : 0;

	t = string_list_pos_params (pchar, save, qflags, pflags);
	dispose_words(save);

	return t;
}

/*
 * Allocate and return a new array element with index INDEX and value
 * VALUE.
 */
ARRAY_ELEMENT *
array_create_element(indx, value)
arrayind_t	indx;
char	*value;
{
	ARRAY_ELEMENT *r;

	r = (ARRAY_ELEMENT *)xmalloc(sizeof(ARRAY_ELEMENT));
	r->ind = indx;
	r->value = value ? savestring(value) : (char *)NULL;
	return(r);
}

ARRAY_ELEMENT *
array_copy_element(ae)
ARRAY_ELEMENT	*ae;
{
	return(ae ? array_create_element(element_index(ae), element_value(ae))
		  : (ARRAY_ELEMENT *) NULL);
}

void
array_dispose_element(ae)
ARRAY_ELEMENT	*ae;
{
	if (ae) {
		FREE(ae->value);
		free(ae);
	}
}

/*
 * Add a new element with index I and value V to array A (a[i] = v).
 */
int
array_insert(a, i, v)
ARRAY	*a;
arrayind_t	i;
char	*v;
{
	register ARRAY_ELEMENT *new, *old;
	arrayind_t nsize;

	if (a == 0)
		return(-1);
	if (i >= a->alloc_size)
		array_expand(a, i);
	old = a->elements[i];
	if (i > array_max_index(a))
		a->max_index = i;
	if (array_first_index(a) < 0 || i < array_first_index(a))
		a->first_index = i;

	if (old) {	/* Replacing an existing element. */
		free(element_value(old));
		old->value = v ? savestring (v) : (char *)NULL;
		old->ind = i;
		return(0);
	} else {
		a->elements[i] = array_create_element(i, v);
		a->num_elements++;
	}

	return (-1);		/* problem */
}

/*
 * Delete the element with index I from array A and return it so the
 * caller can dispose of it.
 */
ARRAY_ELEMENT *
array_remove(a, i)
ARRAY	*a;
arrayind_t	i;
{
	register ARRAY_ELEMENT *ae;
	arrayind_t ind;

	if (a == 0 || array_empty(a))
		return((ARRAY_ELEMENT *) NULL);
	if (i > array_max_index(a) || i < array_first_index(a))
		return((ARRAY_ELEMENT *)NULL);
	ae = a->elements[i];
	a->elements[i] = 0;
	if (ae) {
		a->num_elements--;
		if (a->num_elements == 0)
			a->first_index = a->max_index == -1;
		if (i == array_max_index(a)) {
			for (ind = i; ind >= array_first_index(a); ind--)
				if (a->elements[ind])
					break;
			a->max_index = ind;
		}
		if (i == array_first_index(a)) {
			for (ind = i; ind <= array_max_index(a); ind++)
				if (a->elements[ind])
					break;
			a->first_index = ind;
		}
	}
	return (ae);
}

/*
 * Return the value of a[i].
 */
char *
array_reference(a, i)
ARRAY	*a;
arrayind_t	i;
{
	register ARRAY_ELEMENT *ae;

	if (a == 0 || array_empty(a))
		return((char *) NULL);
	if (i > array_max_index(a) || i < array_first_index(a))
		return((char *)NULL);
	ae = a->elements[i];

	return(ae ? element_value(ae) : (char *)NULL);
}

/* Convenience routines for the shell to translate to and from the form used
   by the rest of the code. */

WORD_LIST *
array_to_word_list(a)
ARRAY	*a;
{
	register arrayind_t i;
	WORD_LIST	*list;
	ARRAY_ELEMENT	*ae;

	if (a == 0 || array_empty(a))
		return((WORD_LIST *)NULL);
	list = (WORD_LIST *)NULL;

	for (i = array_first_index(a); i <= array_max_index(a); i++) {
		if ((ae = a->elements[i]) == 0)
			continue;
		list = make_word_list (make_bare_word(element_value(ae)), list);
	}
	return (REVERSE_LIST(list, WORD_LIST *));
}

ARRAY *
array_from_word_list (list)
WORD_LIST	*list;
{
	ARRAY	*a;

	if (list == 0)
		return((ARRAY *)NULL);
	a = array_create();
	return (array_assign_list (a, list));
}

WORD_LIST *
array_keys_to_word_list(a)
ARRAY	*a;
{
	arrayind_t	ind;
	WORD_LIST	*list;
	ARRAY_ELEMENT	*ae;
	char		*t;

	if (a == 0 || array_empty(a))
		return((WORD_LIST *)NULL);
	list = (WORD_LIST *)NULL;
	for (ind = array_first_index(a); ind <= array_max_index(a); ind++) {
		if ((ae = a->elements[ind]) == 0)
			continue;
		t = itos(element_index(ae));
		list = make_word_list (make_bare_word(t), list);
		free(t);
	}
	return (REVERSE_LIST(list, WORD_LIST *));
}

WORD_LIST *
array_to_kvpair_list(a)
ARRAY	*a;
{
	arrayind_t	ind;
	WORD_LIST	*list;
	ARRAY_ELEMENT	*ae;
	char		*k, *v;

	if (a == 0 || array_empty(a))
		return((WORD_LIST *)NULL);
	list = (WORD_LIST *)NULL;
	for (ind = array_first_index(a); ind <= array_max_index(a); ind++) {
		if ((ae = a->elements[ind]) == 0)
			continue;
		k = itos(element_index(ae));
		v = element_value (ae);
		list = make_word_list (make_bare_word(k), list);
		list = make_word_list (make_bare_word(v), list);
		free(k);
	}
	return (REVERSE_LIST(list, WORD_LIST *));
}

ARRAY *
array_assign_list (array, list)
ARRAY	*array;
WORD_LIST	*list;
{
	register WORD_LIST *l;
	register arrayind_t i;

	for (l = list, i = 0; l; l = l->next, i++)
		array_insert(array, i, l->word->word);
	return array;
}

char **
array_to_argv (a, countp)
ARRAY	*a;
int	*countp;
{
	char		**ret, *t;
	int		i;
	arrayind_t	ind;
	ARRAY_ELEMENT	*ae;

	if (a == 0 || array_empty(a)) {
		if (countp)
			*countp = 0;
		return ((char **)NULL);
	}
	ret = strvec_create (array_num_elements (a) + 1);
	i = 0;
	for (ind = array_first_index(a); ind <= array_max_index(a); ind++) {
		if (a->elements[ind])
			ret[i++] = savestring (element_value(a->elements[ind]));
	}
	ret[i] = (char *)NULL;
	if (countp)
		*countp = i;
	return (ret);
}

ARRAY *
array_from_argv(a, vec, count)
ARRAY	*a;
char	**vec;
int	count;
{
	arrayind_t	i;
	char	*t;

	if (a == 0 || array_num_elements (a) == 0) {
		for (i = 0; i < count; i++)
			array_insert(a, i, vec[i]);
		return a;
	}

	/* Fast case */
	if (array_num_elements (a) == count && count == 1) {
		t = vec[0] ? savestring (vec[0]) : 0;
		ARRAY_VALUE_REPLACE(a, 0, t);
	} else if (array_num_elements (a) <= count) {
		/* modify in array_num_elements members in place, then add */
		for (i = 0; i < array_num_elements (a); i++) {
			t = vec[i] ? savestring (vec[i]) : 0;
			ARRAY_VALUE_REPLACE(a, i, t);
		}

		/* add any more */
		for ( ; i < count; i++)
			array_insert(a, i, vec[i]);
	} else {
		/* deleting elements. replace the first COUNT, free the rest */
		for (i = 0; i < count; i++) {
			t = vec[i] ? savestring (vec[i]) : 0;
			ARRAY_VALUE_REPLACE(a, i, t);
		}

		for ( ; i <= array_max_index (a); i++) {
			array_dispose_element(a->elements[i]);
			a->elements[i] = (ARRAY_ELEMENT *)NULL;
		}

		/* bookkeeping usually taken care of by array_insert */
		set_max_index(a, count - 1);
		set_first_index(a, 0);
		set_num_elements(a, count);
	}
	return a;
}

/*
 * Return the next non-null array element after A[IND]
 */
arrayind_t
element_forw(a, ind)
ARRAY	*a;
arrayind_t ind;
{
	register arrayind_t	i;

	for (i = ind + 1; i <= array_max_index(a); i++)
		if (a->elements[i])
			break;
	if (a->elements[i])
		return i;
	return (array_max_index(a));
}

/*
 * Return the previous non-null array element before A[IND]
 */
arrayind_t
element_back (a, ind)
ARRAY	*a;
arrayind_t ind;
{
	register arrayind_t	i;

	for (i = ind - 1; i >= array_first_index(a); i--)
		if (a->elements[i])
			break;
	if (a->elements[i] && i >= array_first_index(a))
		return i;
	return (array_first_index(a));
}

/*
 * Return a string that is the concatenation of the elements in A from START
 * to END, separated by SEP.
 */
static char *
array_to_string_internal (a, start, end, sep, quoted)
ARRAY	*a;
arrayind_t	start, end;
char	*sep;
int	quoted;
{
	arrayind_t i;
	char	*result, *t;
	ARRAY_ELEMENT *ae;
	int	slen, rsize, rlen, reg;

	slen = strlen(sep);
	result = NULL;
	for (rsize = rlen = 0, i = start; i <= end; i++) {
		if ((ae = a->elements[i]) == 0)
			continue;
		if (rsize == 0)
			result = (char *)xmalloc (rsize = 64);
		if (element_value(ae)) {
			t = quoted ? quote_string(element_value(ae)) : element_value(ae);
			reg = strlen(t);
			RESIZE_MALLOCED_BUFFER (result, rlen, (reg + slen + 2),
						rsize, rsize);
			strcpy(result + rlen, t);
			rlen += reg;
			if (quoted)
				free(t);
			/*
			 * Add a separator only after non-null elements.
			 */
			if (element_forw(a, i) <= end) {
				strcpy(result + rlen, sep);
				rlen += slen;
			}
		}
	}
	if (result)
		result[rlen] = '\0';	/* XXX */
	return(result);
}

char *
array_to_kvpair (a, quoted)
ARRAY	*a;
int	quoted;
{
	arrayind_t	ind;
	char	*result, *valstr, *is;
	char	indstr[INT_STRLEN_BOUND(intmax_t) + 1];
	ARRAY_ELEMENT *ae;
	int	rsize, rlen, elen;

	if (a == 0 || array_empty (a))
		return((char *)NULL);

	result = (char *)xmalloc (rsize = 128);
	result[rlen = 0] = '\0';

	for (ind = array_first_index(a); ind <= array_max_index(a); ind++) {
		if ((ae = a->elements[ind]) == 0)
			continue;
		is = inttostr (element_index(ae), indstr, sizeof(indstr));
		valstr = element_value (ae) ?
				(ansic_shouldquote (element_value (ae)) ?
				   ansic_quote (element_value(ae), 0, (int *)0) :
				   sh_double_quote (element_value (ae)))
					    : (char *)NULL;
		elen = STRLEN (is) + 8 + STRLEN (valstr);
		RESIZE_MALLOCED_BUFFER (result, rlen, (elen + 1), rsize, rsize);

		strcpy (result + rlen, is);
		rlen += STRLEN (is);
		result[rlen++] = ' ';
		if (valstr) {
			strcpy (result + rlen, valstr);
			rlen += STRLEN (valstr);
		} else {
			strcpy (result + rlen, "\"\"");
			rlen += 2;
		}

		if (ind < array_max_index (a))
		  result[rlen++] = ' ';

		FREE (valstr);
	}
	RESIZE_MALLOCED_BUFFER (result, rlen, 1, rsize, 8);
	result[rlen] = '\0';

	if (quoted) {
		/* This is not as efficient as it could be... */
		valstr = sh_single_quote (result);
		free (result);
		result = valstr;
	}
	return(result);
}

char *
array_to_assign (a, quoted)
ARRAY	*a;
int	quoted;
{
	arrayind_t	ind;
	char	*result, *valstr, *is;
	char	indstr[INT_STRLEN_BOUND(intmax_t) + 1];
	ARRAY_ELEMENT *ae;
	int	rsize, rlen, elen;

	if (a == 0 || array_empty (a))
		return((char *)NULL);

	result = (char *)xmalloc (rsize = 128);
	result[0] = '(';
	rlen = 1;

	for (ind = array_first_index(a); ind <= array_max_index(a); ind++) {
		if ((ae = a->elements[ind]) == 0)
			continue;
		is = inttostr (element_index(ae), indstr, sizeof(indstr));
		valstr = element_value (ae) ?
				(ansic_shouldquote (element_value (ae)) ?
				   ansic_quote (element_value(ae), 0, (int *)0) :
				   sh_double_quote (element_value (ae)))
					    : (char *)NULL;
		elen = STRLEN (is) + 8 + STRLEN (valstr);
		RESIZE_MALLOCED_BUFFER (result, rlen, (elen + 1), rsize, rsize);

		result[rlen++] = '[';
		strcpy (result + rlen, is);
		rlen += STRLEN (is);
		result[rlen++] = ']';
		result[rlen++] = '=';
		if (valstr) {
			strcpy (result + rlen, valstr);
			rlen += STRLEN (valstr);
		}

		if (ind < array_max_index(a))
		  result[rlen++] = ' ';

		FREE (valstr);
	}
	RESIZE_MALLOCED_BUFFER (result, rlen, 1, rsize, 8);
	result[rlen++] = ')';
	result[rlen] = '\0';
	if (quoted) {
		/* This is not as efficient as it could be... */
		valstr = sh_single_quote (result);
		free (result);
		result = valstr;
	}
	return(result);
}

char *
array_to_string (a, sep, quoted)
ARRAY	*a;
char	*sep;
int	quoted;
{
	if (a == 0)
		return((char *)NULL);
	if (array_empty(a))
		return(savestring(""));
	return (array_to_string_internal (a, array_first_index(a), array_max_index(a), sep, quoted));
}

#if defined (INCLUDE_UNUSED) || defined (TEST_ARRAY)
/*
 * Return an array consisting of elements in S, separated by SEP
 */
ARRAY *
array_from_string(s, sep)
char	*s, *sep;
{
	ARRAY	*a;
	WORD_LIST *w;

	if (s == 0)
		return((ARRAY *)NULL);
	w = list_string (s, sep, 0);
	if (w == 0)
		return((ARRAY *)NULL);
	a = array_from_word_list (w);
	return (a);
}
#endif

#if defined (TEST_ARRAY)
/*
 * To make a running version, compile -DTEST_ARRAY and link with:
 * 	xmalloc.o syntax.o lib/malloc/libmalloc.a lib/sh/libsh.a
 */
int interrupt_immediately = 0;

int
signal_is_trapped(s)
int	s;
{
	return 0;
}

void
fatal_error(const char *s, ...)
{
	fprintf(stderr, "array_test: fatal memory error\n");
	abort();
}

void
programming_error(const char *s, ...)
{
	fprintf(stderr, "array_test: fatal programming error\n");
	abort();
}

WORD_DESC *
make_bare_word (s)
const char	*s;
{
	WORD_DESC *w;

	w = (WORD_DESC *)xmalloc(sizeof(WORD_DESC));
	w->word = s ? savestring(s) : savestring ("");
	w->flags = 0;
	return w;
}

WORD_LIST *
make_word_list(x, l)
WORD_DESC	*x;
WORD_LIST	*l;
{
	WORD_LIST *w;

	w = (WORD_LIST *)xmalloc(sizeof(WORD_LIST));
	w->word = x;
	w->next = l;
	return w;
}

WORD_LIST *
list_string(s, t, i)
char	*s, *t;
int	i;
{
	char	*r, *a;
	WORD_LIST	*wl;

	if (s == 0)
		return (WORD_LIST *)NULL;
	r = savestring(s);
	wl = (WORD_LIST *)NULL;
	a = strtok(r, t);
	while (a) {
		wl = make_word_list (make_bare_word(a), wl);
		a = strtok((char *)NULL, t);
	}
	return (REVERSE_LIST (wl, WORD_LIST *));
}

GENERIC_LIST *
list_reverse (list)
GENERIC_LIST	*list;
{
	register GENERIC_LIST *next, *prev;

	for (prev = 0; list; ) {
		next = list->next;
		list->next = prev;
		prev = list;
		list = next;
	}
	return prev;
}

char *
pat_subst(s, t, u, i)
char	*s, *t, *u;
int	i;
{
	return ((char *)NULL);
}

char *
quote_string(s)
char	*s;
{
	return savestring(s);
}

print_element(ae)
ARRAY_ELEMENT	*ae;
{
	char	lbuf[INT_STRLEN_BOUND (intmax_t) + 1];

	printf("array[%s] = %s\n",
		inttostr (element_index(ae), lbuf, sizeof (lbuf)),
		element_value(ae));
}

print_array(a)
ARRAY	*a;
{
	printf("\n");
	array_walk(a, print_element, (void *)NULL);
}

main()
{
	ARRAY	*a, *new_a, *copy_of_a;
	ARRAY_ELEMENT	*ae, *aew;
	char	*s;

	a = array_create();
	array_insert(a, 1, "one");
	array_insert(a, 7, "seven");
	array_insert(a, 4, "four");
	array_insert(a, 1029, "one thousand twenty-nine");
	array_insert(a, 12, "twelve");
	array_insert(a, 42, "forty-two");
	print_array(a);
	s = array_to_string (a, " ", 0);
	printf("s = %s\n", s);
	copy_of_a = array_from_string(s, " ");
	printf("copy_of_a:");
	print_array(copy_of_a);
	array_dispose(copy_of_a);
	printf("\n");
	free(s);
	ae = array_remove(a, 4);
	array_dispose_element(ae);
	ae = array_remove(a, 1029);
	array_dispose_element(ae);
	array_insert(a, 16, "sixteen");
	print_array(a);
	s = array_to_string (a, " ", 0);
	printf("s = %s\n", s);
	copy_of_a = array_from_string(s, " ");
	printf("copy_of_a:");
	print_array(copy_of_a);
	array_dispose(copy_of_a);
	printf("\n");
	free(s);
	array_insert(a, 2, "two");
	array_insert(a, 1029, "new one thousand twenty-nine");
	array_insert(a, 0, "zero");
	array_insert(a, 134, "");
	print_array(a);
	s = array_to_string (a, ":", 0);
	printf("s = %s\n", s);
	copy_of_a = array_from_string(s, ":");
	printf("copy_of_a:");
	print_array(copy_of_a);
	array_dispose(copy_of_a);
	printf("\n");
	free(s);
	new_a = array_copy(a);
	print_array(new_a);
	s = array_to_string (new_a, ":", 0);
	printf("s = %s\n", s);
	copy_of_a = array_from_string(s, ":");
	free(s);
	printf("copy_of_a:");
	print_array(copy_of_a);
	array_shift(copy_of_a, 2, AS_DISPOSE);
	printf("copy_of_a shifted by two:");
	print_array(copy_of_a);
	ae = array_shift(copy_of_a, 2, 0);
	printf("copy_of_a shifted by two:");
	print_array(copy_of_a);
	for ( ; ae; ) {
		aew = element_forw(ae);
		array_dispose_element(ae);
		ae = aew;
	}
	array_rshift(copy_of_a, 1, (char *)0);
	printf("copy_of_a rshift by 1:");
	print_array(copy_of_a);
	array_rshift(copy_of_a, 2, "new element zero");
	printf("copy_of_a rshift again by 2 with new element zero:");
	print_array(copy_of_a);
	s = array_to_assign(copy_of_a, 0);
	printf("copy_of_a=%s\n", s);
	free(s);
	ae = array_shift(copy_of_a, array_num_elements(copy_of_a), 0);
	for ( ; ae; ) {
		aew = element_forw(ae);
		array_dispose_element(ae);
		ae = aew;
	}
	array_dispose(copy_of_a);
	printf("\n");
	array_dispose(a);
	array_dispose(new_a);
}

#endif /* TEST_ARRAY */
#endif /* ARRAY_VARS */
