/**
 * Handle selector uniquing.
 *
 * When building, you may define TYPE_DEPENDENT_DISPATCH to enable message
 * sends to depend on their types.
 */
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include "lock.h"
#include "objc/runtime.h"
#include "method.h"
#include "class.h"
#include "selector.h"
#include "visibility.h"

#ifdef TYPE_DEPENDENT_DISPATCH
#	define TDD(x) x
#else
#	define TDD(x)
#endif



// Define the pool allocator for selectors.  This is a simple bump-the-pointer
// allocator for low-overhead allocation.
#define POOL_NAME selector
#define POOL_TYPE struct objc_selector
#include "pool.h"


/**
 * The number of selectors currently registered.  When a selector is
 * registered, its name field is replaced with its index in the selector_list
 * array.
 */
static uint32_t selector_count = 1;
/**
 * Size of the buffer.
 */
static size_t table_size;
/**
 * Mapping from selector numbers to selector names.
 */
PRIVATE struct sel_type_list **selector_list  = NULL;

#ifdef DEBUG_SELECTOR_TABLE
#define DEBUG_LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif

// Get the functions for string hashing
#include "string_hash.h"


/**
 * Lock protecting the selector table.
 */
mutex_t selector_table_lock;

static inline struct sel_type_list *selLookup_locked(uint32_t idx)
{
	if (idx > selector_count)
	{
		return NULL;
	}
	return selector_list[idx];
}

static inline struct sel_type_list *selLookup(uint32_t idx)
{
	LOCK_FOR_SCOPE(&selector_table_lock);
	return selLookup_locked(idx);
}

PRIVATE BOOL isSelRegistered(SEL sel)
{
	if ((uintptr_t)sel->name < (uintptr_t)selector_count)
	{
		return YES;
	}
	return NO;
}

static const char *sel_getNameNonUnique(SEL sel)
{
	const char *name = sel->name;
	if (isSelRegistered(sel))
	{
		struct sel_type_list * list = selLookup_locked(sel->index);
		name = (list == NULL) ? NULL : list->value;
	}
	if (NULL == name)
	{
		name = "";
	}
	return name;
}

/**
 * Skip anything in a type encoding that is irrelevant to the comparison
 * between selectors, including type qualifiers and argframe info.
 */
static const char *skip_irrelevant_type_info(const char *t)
{
	switch (*t)
	{
		default: return t;
		case 'r': case 'n': case 'N': case 'o': case 'O': case 'R':
		case 'V': case 'A': case '!': case '0'...'9':
			return skip_irrelevant_type_info(t+1);
	}
}

static BOOL selector_types_equal(const char *t1, const char *t2)
{
	if (t1 == NULL || t2 == NULL) { return t1 == t2; }

	while (('\0' != *t1) && ('\0' != *t1))
	{
		t1 = skip_irrelevant_type_info(t1);
		t2 = skip_irrelevant_type_info(t2);
		// This is a really ugly hack.  For some stupid reason, the people
		// designing Objective-C type encodings decided to allow * as a
		// shorthand for char*, because strings are 'special'.  Unfortunately,
		// FSF GCC generates "*" for @encode(BOOL*), while Clang and Apple GCC
		// generate "^c" or "^C" (depending on whether BOOL is declared
		// unsigned).  
		//
		// The correct fix is to remove * completely from type encodings, but
		// unfortunately my time machine is broken so I can't travel to 1986
		// and apply a cluebat to those responsible.
		if ((*t1 == '*') && (*t2 != '*'))
		{
			if (*t2 == '^' && (((*(t2+1) == 'C') || (*(t2+2) == 'c'))))
			{
				t2++;
			}
			else
			{
				return NO;
			}
		}
		else if ((*t2 == '*') && (*t1 != '*'))
		{
			if (*t1 == '^' && (((*(t1+1) == 'C') || (*(t1+1) == 'c'))))
			{
				t1++;
			}
			else
			{
				return NO;
			}
		}
		else if (*t1 != *t2)
		{
			return NO;
		}

		if ('\0' != *t1) { t1++; }
		if ('\0' != *t2) { t2++; }
	}
	return YES;
}

#ifdef TYPE_DEPENDENT_DISPATCH

static BOOL selector_types_equivalent(const char *t1, const char *t2)
{
	// We always treat untyped selectors as having the same type as typed
	// selectors, for dispatch purposes.
	if (t1 == NULL || t2 == NULL) { return YES; }

	return selector_types_equal(t1, t2);
}
#endif

/**
 * Compare whether two selectors are identical.
 */
static int selector_identical(const void *k,
                              const SEL value)
{
	SEL key = (SEL)k;
	DEBUG_LOG("Comparing %s %s, %s %s\n", sel_getNameNonUnique(key), sel_getNameNonUnique(value), sel_getType_np(key), sel_getType_np(value));
	return string_compare(sel_getNameNonUnique(key), sel_getNameNonUnique(value)) &&
		selector_types_equal(sel_getType_np(key), sel_getType_np(value));
}

/**
 * Compare selectors based on whether they are treated as equivalent for the
 * purpose of dispatch.
 */
static int selector_equal(const void *k,
                            const SEL value)
{
#ifdef TYPE_DEPENDENT_DISPATCH
	return selector_identical(k, value);
#else
	SEL key = (SEL)k;
	return string_compare(sel_getNameNonUnique(key), sel_getNameNonUnique(value));
#endif
}

/**
 * Hash a selector.
 */
static inline uint32_t hash_selector(const void *s)
{
	SEL sel = (SEL)s;
	uint32_t hash = 5381;
	const char *str = sel_getNameNonUnique(sel);
	uint32_t c;
	while((c = (uint32_t)*str++))
	{
		hash = hash * 33 + c;
	}
#ifdef TYPE_DEPENDENT_DISPATCH
	// We can't use all of the values in the type encoding for the hash,
	// because our equality test is a bit more complex than simple string
	// encoding (for example, * and ^C have to be considered equivalent, since
	// they are both used as encodings for C strings in different situations)
	if ((str = sel_getType_np(sel)))
	{
		while((c = (uint32_t)*str++))
		{
			switch (c)
			{
				case '@': case 'i': case 'I': case 'l': case 'L':
				case 'q': case 'Q': case 's': case 'S': 
				hash = hash * 33 + c;
			}
		}
	}
#endif
	return hash;
}

#define MAP_TABLE_NAME selector
#define MAP_TABLE_SINGLE_THREAD
#define MAP_TABLE_COMPARE_FUNCTION selector_identical
#define MAP_TABLE_HASH_KEY hash_selector
#define MAP_TABLE_HASH_VALUE hash_selector
#include "hash_table.h"
/**
 * Table of registered selector.  Maps from selector to selector.
 */
static selector_table *sel_table;

static int selector_name_copies;

PRIVATE void log_selector_memory_usage(void)
{
	fprintf(stderr, "%lu bytes in selector name list.\n", (unsigned long)(table_size * sizeof(void*)));
	fprintf(stderr, "%d bytes in selector names.\n", selector_name_copies);
	fprintf(stderr, "%d bytes (%d entries) in selector hash table.\n", (int)(sel_table->table_size *
	        sizeof(struct selector_table_cell_struct)), sel_table->table_size);
	fprintf(stderr, "%d selectors registered.\n", selector_count);
	fprintf(stderr, "%d hash table cells per selector (%.2f%% full)\n", sel_table->table_size / selector_count,  ((float)selector_count) /  sel_table->table_size * 100);
}




/**
 * Resizes the dtables to ensure that they can store as many selectors as
 * exist.
 */
void objc_resize_dtables(uint32_t);

/**
 * Create data structures to store selectors.
 */
PRIVATE void init_selector_tables()
{
	selector_list = calloc(sizeof(void*), 4096);
	table_size = 4096;
	INIT_LOCK(selector_table_lock);
	selector_initialize(&sel_table, 4096);
}

static SEL selector_lookup(const char *name, const char *types)
{
	struct objc_selector sel = {{name}, types};
	LOCK_FOR_SCOPE(&selector_table_lock);
	return selector_table_get(sel_table, &sel);
}
static inline void add_selector_to_table(SEL aSel, int32_t uid, uint32_t idx)
{
	DEBUG_LOG("Sel %s uid: %d, idx: %d, hash: %d\n", sel_getNameNonUnique(aSel), uid, idx, hash_selector(aSel));
	struct sel_type_list *typeList =
		(struct sel_type_list *)selector_pool_alloc();
	typeList->value = aSel->name;
	typeList->next = 0;
	// Store the name.
	if (idx >= table_size)
	{
		table_size *= 2;
		struct sel_type_list **newList = calloc(sizeof(struct sel_type_list*), table_size);
		if (newList == NULL)
		{
			abort();
		}
		memcpy(newList, selector_list, sizeof(void*)*(table_size/2));
		free(selector_list);
		selector_list = newList;
	}
	selector_list[idx] = typeList;
	// Store the selector.
	selector_insert(sel_table, aSel);
	// Set the selector's name to the uid.
	aSel->name = (const char*)(uintptr_t)uid;
}
/**
 * Really registers a selector.  Must be called with the selector table locked.
 */
static inline void register_selector_locked(SEL aSel)
{
	uintptr_t idx = selector_count++;
	if (NULL == aSel->types)
	{
		DEBUG_LOG("Registering selector %d %s\n", (int)idx, sel_getNameNonUnique(aSel));
		add_selector_to_table(aSel, idx, idx);
		objc_resize_dtables(selector_count);
		return;
	}
	SEL untyped = selector_lookup(aSel->name, 0);
	// If this has a type encoding, store the untyped version too.
	if (untyped == NULL)
	{
		untyped = selector_pool_alloc();
		untyped->name = aSel->name;
		untyped->types = 0;
		DEBUG_LOG("Registering selector %d %s\n", (int)idx, sel_getNameNonUnique(aSel));
		add_selector_to_table(untyped, idx, idx);
		// If we are in type dependent dispatch mode, the uid for the typed
		// and untyped versions will be different
		idx++; selector_count++;
	}
	else
	{
		// Make sure we only store one name
		aSel->name = sel_getNameNonUnique(untyped);
	}
	uintptr_t uid = (uintptr_t)untyped->name;
	TDD(uid = idx);
	DEBUG_LOG("Registering typed selector %d %s %s\n", (int)uid, sel_getNameNonUnique(aSel), sel_getType_np(aSel));
	add_selector_to_table(aSel, uid, idx);

	// Add this set of types to the list.
	// This is quite horrible.  Most selectors will only have one type
	// encoding, so we're wasting a lot of memory like this.
	struct sel_type_list *typeListHead = selLookup_locked(untyped->index);
	struct sel_type_list *typeList =
		(struct sel_type_list *)selector_pool_alloc();
	typeList->value = aSel->types;
	typeList->next = typeListHead->next;
	typeListHead->next = typeList;
	objc_resize_dtables(selector_count);
}
/**
 * Registers a selector.  This assumes that the argument is never deallocated.
 */
PRIVATE SEL objc_register_selector(SEL aSel)
{
	if (isSelRegistered(aSel))
	{
		return aSel;
	}
	// Check that this isn't already registered, before we try
	SEL registered = selector_lookup(aSel->name, aSel->types);
	if (NULL != registered && selector_equal(aSel, registered))
	{
		aSel->name = registered->name;
		return registered;
	}
	assert(!(aSel->types && (strstr(aSel->types, "@\"") != NULL)));
	LOCK(&selector_table_lock);
	register_selector_locked(aSel);
	UNLOCK(&selector_table_lock);
	return aSel;
}

/**
 * Registers a selector by copying the argument.
 */
static SEL objc_register_selector_copy(SEL aSel, BOOL copyArgs)
{
	// If an identical selector is already registered, return it.
	SEL copy = selector_lookup(aSel->name, aSel->types);
	DEBUG_LOG("Checking if old selector is registered: %d (%d)\n", NULL != copy ? selector_equal(aSel, copy) : 0, ((NULL != copy) && selector_equal(aSel, copy)));
	if ((NULL != copy) && selector_identical(aSel, copy))
	{
		DEBUG_LOG("Not adding new copy\n");
		return copy;
	}
	LOCK_FOR_SCOPE(&selector_table_lock);
	copy = selector_lookup(aSel->name, aSel->types);
	if (NULL != copy && selector_identical(aSel, copy))
	{
		return copy;
	}
	assert(!(aSel->types && (strstr(aSel->types, "@\"") != NULL)));
	// Create a copy of this selector.
	copy = selector_pool_alloc();
	copy->name = aSel->name;
	copy->types = (NULL == aSel->types) ? NULL : aSel->types;
	if (copyArgs)
	{
		SEL untyped = selector_lookup(aSel->name, 0);
		if (untyped != NULL)
		{
			copy->name = sel_getName(untyped);
		}
		else
		{
			copy->name = strdup(aSel->name);
			selector_name_copies += strlen(copy->name);
		}
		if (copy->types != NULL)
		{
			copy->types = strdup(copy->types);
			selector_name_copies += strlen(copy->types);
		}
	}
	// Try to register the copy as the authoritative version
	register_selector_locked(copy);
	return copy;
}

PRIVATE uint32_t sel_nextTypeIndex(uint32_t untypedIdx, uint32_t idx)
{
	struct sel_type_list *list = selLookup(untypedIdx);

	if (NULL == list) { return 0; }

	const char *selName = list->value;
	list = list->next;
	BOOL found = untypedIdx == idx;
	while (NULL != list)
	{
		SEL sel = selector_lookup(selName, list->value);
		if (sel->index == untypedIdx) { return 0; }
		if (found)
		{
			return sel->index;
		}
		found = (sel->index == idx);
	}
	return 0;
}

/**
 * Public API functions.
 */

const char *sel_getName(SEL sel)
{
	if (NULL == sel) { return "<null selector>"; }
	const char *name = sel->name;
	if (isSelRegistered(sel))
	{
		struct sel_type_list * list = selLookup(sel->index);
		name = (list == NULL) ? NULL : list->value;
	}
	else
	{
		SEL old = selector_lookup(sel->name, sel->types);
		if (NULL != old)
		{
			return sel_getName(old);
		}
	}
	if (NULL == name)
	{
		name = "";
	}
	return name;
}

SEL sel_getUid(const char *selName)
{
	return sel_registerName(selName);
}

BOOL sel_isEqual(SEL sel1, SEL sel2)
{
	if ((0 == sel1) || (0 == sel2))
	{
		return sel1 == sel2;
	}
	if (sel1->name == sel2->name)
	{
		return YES;
	}
	// Otherwise, do a slow compare
	return string_compare(sel_getNameNonUnique(sel1), sel_getNameNonUnique(sel2)) TDD(&&
			(sel1->types == NULL || sel2->types == NULL ||
		selector_types_equivalent(sel_getType_np(sel1), sel_getType_np(sel2))));
}

SEL sel_registerName(const char *selName)
{
	if (NULL == selName) { return NULL; }
	struct objc_selector sel = {{selName}, 0};
	return objc_register_selector_copy(&sel, YES);
}

SEL sel_registerTypedName_np(const char *selName, const char *types)
{
	if (NULL == selName) { return NULL; }
	struct objc_selector sel = {{selName}, types};
	return objc_register_selector_copy(&sel, YES);
}

const char *sel_getType_np(SEL aSel)
{
	if (NULL == aSel) { return NULL; }
	return aSel->types;
}


unsigned sel_copyTypes_np(const char *selName, const char **types, unsigned count)
{
	if (NULL == selName) { return 0; }
	SEL untyped = selector_lookup(selName, 0);
	if (untyped == NULL) { return 0; }

	struct sel_type_list *l = selLookup(untyped->index);
	// Skip the head, which just contains the name, not the types.
	l = l->next;

	if (count == 0)
	{
		while (NULL != l)
		{
			count++;
			l = l->next;
		}
		return count;
	}

	unsigned found = 0;
	while (NULL != l) 
	{
		if (found<count)
		{
			types[found] = l->value;
		}
		found++;
		l = l->next;
	}
	return found;
}

unsigned sel_copyTypedSelectors_np(const char *selName, SEL *const sels, unsigned count)
{
	if (NULL == selName) { return 0; }
	SEL untyped = selector_lookup(selName, 0);
	if (untyped == NULL) { return 0; }

	struct sel_type_list *l = selLookup(untyped->index);
	// Skip the head, which just contains the name, not the types.
	l = l->next;

	if (count == 0)
	{
		while (NULL != l)
		{
			count++;
			l = l->next;
		}
		return count;
	}

	unsigned found = 0;
	while (NULL != l && found<count)
	{
		sels[found++] = selector_lookup(selName, l->value);
		l = l->next;
	}
	return found;
}

PRIVATE void objc_register_selectors_from_list(struct objc_method_list *l)
{
	for (int i=0 ; i<l->count ; i++)
	{
		Method m = method_at_index(l, i);
		struct objc_selector sel = { {(const char*)m->selector}, m->types };
		m->selector = objc_register_selector_copy(&sel, NO);
	}
}
/**
 * Register all of the (unregistered) selectors that are used in a class.
 */
PRIVATE void objc_register_selectors_from_class(Class class)
{
	for (struct objc_method_list *l=class->methods ; NULL!=l ; l=l->next)
	{
		objc_register_selectors_from_list(l);
	}
}
PRIVATE void objc_register_selector_array(SEL selectors, unsigned long count)
{
	// GCC is broken and always sets the count to 0, so we ignore count until
	// we can throw stupid and buggy compilers in the bin.
	for (unsigned long i=0 ;  (NULL != selectors[i].name) ; i++)
	{
		objc_register_selector(&selectors[i]);
	}
}


/**
 * Legacy GNU runtime compatibility.
 *
 * All of the functions in this section are deprecated and should not be used
 * in new code.
 */
#ifndef NO_LEGACY
SEL sel_get_typed_uid (const char *name, const char *types)
{
	if (NULL == name) { return NULL; }
	SEL sel = selector_lookup(name, types);
	if (NULL == sel) { return sel_registerTypedName_np(name, types); }

	struct sel_type_list *l = selLookup(sel->index);
	// Skip the head, which just contains the name, not the types.
	l = l->next;
	if (NULL != l)
	{
		sel = selector_lookup(name, l->value);
	}
	return sel;
}

SEL sel_get_any_typed_uid (const char *name)
{
	if (NULL == name) { return NULL; }
	SEL sel = selector_lookup(name, 0);
	if (NULL == sel) { return sel_registerName(name); }

	struct sel_type_list *l = selLookup(sel->index);
	// Skip the head, which just contains the name, not the types.
	l = l->next;
	if (NULL != l)
	{
		sel = selector_lookup(name, l->value);
	}
	return sel;
}

SEL sel_get_any_uid (const char *name)
{
	return selector_lookup(name, 0);
}

SEL sel_get_uid(const char *name)
{
	return selector_lookup(name, 0);
}

const char *sel_get_name(SEL selector)
{
	return sel_getNameNonUnique(selector);
}

BOOL sel_is_mapped(SEL selector)
{
	return isSelRegistered(selector);
}

const char *sel_get_type(SEL selector)
{
	return sel_getType_np(selector);
}

SEL sel_register_name(const char *name)
{
	return sel_registerName(name);
}

SEL sel_register_typed_name(const char *name, const char *type)
{
	return sel_registerTypedName_np(name, type);
}

BOOL sel_eq(SEL s1, SEL s2)
{
	return sel_isEqual(s1, s2);
}

#endif // NO_LEGACY
