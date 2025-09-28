/**
 * hash_table.h provides a template for implementing hopscotch hash tables. 
 *
 * Several macros must be defined before including this file:
 *
 * MAP_TABLE_NAME defines the name of the table.  All of the operations and
 * types related to this table will be prefixed with this value.
 *
 * MAP_TABLE_COMPARE_FUNCTION defines the function used for testing a key
 * against a value in the table for equality.  This must take two void*
 * arguments.  The first is the key and the second is the value.  
 *
 * MAP_TABLE_HASH_KEY and MAP_TABLE_HASH_VALUE define a pair of functions that
 * takes a key and a value pointer respectively as their argument and returns
 * an int32_t representing the hash.
 *
 * Optionally, MAP_TABLE_STATIC_SIZE may be defined, to define a table type
 * which has a static size.
 */
#include "lock.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#ifdef ENABLE_GC
#	include <gc/gc.h>
#	include <gc/gc_typed.h>
#	define CALLOC(x,y) GC_MALLOC(x*y)
#	define IF_NO_GC(x)
#	define IF_GC(x) x
#else
#	define CALLOC(x,y) calloc(x,y)
#	define IF_NO_GC(x) x
#	define IF_GC(x)
#endif

#ifndef MAP_TABLE_NAME
#	error You must define MAP_TABLE_NAME.
#endif
#ifndef MAP_TABLE_COMPARE_FUNCTION
#	error You must define MAP_TABLE_COMPARE_FUNCTION.
#endif
#ifndef MAP_TABLE_HASH_KEY 
#	error You must define MAP_TABLE_HASH_KEY
#endif
#ifndef MAP_TABLE_HASH_VALUE
#	error You must define MAP_TABLE_HASH_VALUE
#endif

// Horrible multiple indirection to satisfy the weird precedence rules in cpp
#define REALLY_PREFIX_SUFFIX(x,y) x ## y
#define PREFIX_SUFFIX(x, y) REALLY_PREFIX_SUFFIX(x, y)

/**
 * PREFIX(x) macro adds the table name prefix to the argument.
 */
#define PREFIX(x) PREFIX_SUFFIX(MAP_TABLE_NAME, x)


/**
 * Map tables are protected by a lock by default.  Defining MAP_TABLE_NO_LOCK
 * will prevent this and make you responsible for synchronization.
 */
#ifdef MAP_TABLE_NO_LOCK
#	define MAP_LOCK()
#	define MAP_UNLOCK()
#else
#	define MAP_LOCK() (LOCK(&table->lock))
#	define MAP_UNLOCK() (UNLOCK(&table->lock))
#endif
#ifndef MAP_TABLE_VALUE_TYPE
#	define MAP_TABLE_VALUE_TYPE void*
static BOOL PREFIX(_is_null)(void *value)
{
	return value == NULL;
}
#	define MAP_TABLE_TYPES_BITMAP 1
#	define MAP_TABLE_VALUE_NULL PREFIX(_is_null)
#	define MAP_TABLE_VALUE_PLACEHOLDER NULL
#endif

typedef struct PREFIX(_table_cell_struct)
{
	uint32_t secondMaps;
	MAP_TABLE_VALUE_TYPE value;
} *PREFIX(_table_cell);

#ifdef MAP_TABLE_STATIC_SIZE
typedef struct 
{
	mutex_t lock;
	unsigned int table_used;
	IF_NO_GC(unsigned int enumerator_count;)
	struct PREFIX(_table_cell_struct) table[MAP_TABLE_STATIC_SIZE];
} PREFIX(_table);
static PREFIX(_table) MAP_TABLE_STATIC_NAME;
#	ifndef MAP_TABLE_NO_LOCK
__attribute__((constructor)) void static PREFIX(_table_initializer)(void)
{
	INIT_LOCK(MAP_TABLE_STATIC_NAME.lock);
}
#	endif
#	define TABLE_SIZE(x) MAP_TABLE_STATIC_SIZE
#else
typedef struct PREFIX(_table_struct)
{
	mutex_t lock;
	unsigned int table_size;
	unsigned int table_used;
	IF_NO_GC(unsigned int enumerator_count;)
#	if defined(ENABLE_GC) && defined(MAP_TABLE_TYPES_BITMAP)
	GC_descr descr;
#	endif 
	struct PREFIX(_table_struct) *old;
	struct PREFIX(_table_cell_struct) *table;
} PREFIX(_table);

static struct PREFIX(_table_cell_struct) *PREFIX(alloc_cells)(PREFIX(_table) *table, int count)
{
#	if defined(ENABLE_GC) && defined(MAP_TABLE_TYPES_BITMAP)
	return GC_CALLOC_EXPLICITLY_TYPED(count,
			sizeof(struct PREFIX(_table_cell_struct)), table->descr);
#	else
	return CALLOC(count, sizeof(struct PREFIX(_table_cell_struct)));
#	endif
}

static PREFIX(_table) *PREFIX(_create)(uint32_t capacity)
{
	PREFIX(_table) *table = CALLOC(1, sizeof(PREFIX(_table)));
#	ifndef MAP_TABLE_NO_LOCK
	INIT_LOCK(table->lock);
#	endif
#	if defined(ENABLE_GC) && defined(MAP_TABLE_TYPES_BITMAP)
	// The low word in the bitmap stores the offsets of the next entries
	GC_word bitmap = (MAP_TABLE_TYPES_BITMAP << 1);
	table->descr = GC_make_descriptor(&bitmap,
			sizeof(struct PREFIX(_table_cell_struct)) / sizeof (void*));
#	endif
	table->table = PREFIX(alloc_cells)(table, capacity);
	table->table_size = capacity;
	return table;
}

static void PREFIX(_initialize)(PREFIX(_table) **table, uint32_t capacity)
{
#ifdef ENABLE_GC
	GC_add_roots(table, table+1);
#endif
	*table = PREFIX(_create)(capacity);
}

#	define TABLE_SIZE(x) (x->table_size)
#endif


#ifdef MAP_TABLE_STATIC_SIZE
static int PREFIX(_table_resize)(PREFIX(_table) *table)
{
	return 0;
}
#else

static int PREFIX(_insert)(PREFIX(_table) *table, MAP_TABLE_VALUE_TYPE value);

static int PREFIX(_table_resize)(PREFIX(_table) *table)
{
	struct PREFIX(_table_cell_struct) *newArray =
		PREFIX(alloc_cells)(table, table->table_size * 2);
	if (NULL == newArray) { return 0; }

	// Allocate a new table structure and move the array into that.  Now
	// lookups will try using that one, if possible.
	PREFIX(_table) *copy = CALLOC(1, sizeof(PREFIX(_table)));
	memcpy(copy, table, sizeof(PREFIX(_table)));
	table->old = copy;

	// Now we make the original table structure point to the new (empty) array.
	table->table = newArray;
	table->table_size *= 2;
	// The table currently has no entries; the copy has them all.
	table->table_used = 0;

	// Finally, copy everything into the new table
	// Note: we should really do this in a background thread.  At this stage,
	// we can do the updates safely without worrying about read contention.
	int copied = 0;
	for (uint32_t i=0 ; i<copy->table_size ; i++)
	{
		MAP_TABLE_VALUE_TYPE value = copy->table[i].value;
		if (!MAP_TABLE_VALUE_NULL(value))
		{
			copied++;
			PREFIX(_insert)(table, value);
		}
	}
	__sync_synchronize();
	table->old = NULL;
#	if !defined(ENABLE_GC) && defined(MAP_TABLE_SINGLE_THREAD)
	free(copy->table);
	free(copy);
#	endif
	return 1;
}
#endif

struct PREFIX(_table_enumerator)
{
	PREFIX(_table) *table;
	unsigned int seen;
	unsigned int index;
};

static inline PREFIX(_table_cell) PREFIX(_table_lookup)(PREFIX(_table) *table, 
                                                        uint32_t hash)
{
	hash = hash % TABLE_SIZE(table);
	return &table->table[hash];
}

static int PREFIX(_table_move_gap)(PREFIX(_table) *table, uint32_t fromHash,
		uint32_t toHash, PREFIX(_table_cell) emptyCell)
{
	for (uint32_t hash = fromHash - 32 ; hash < fromHash ; hash++)
	{
		// Get the cell n before the hash.
		PREFIX(_table_cell) cell = PREFIX(_table_lookup)(table, hash);
		// If this node is a primary entry move it down
		if (MAP_TABLE_HASH_VALUE(cell->value) == hash)
		{
			emptyCell->value = cell->value;
			cell->secondMaps |= (1 << ((fromHash - hash) - 1));
			cell->value = MAP_TABLE_VALUE_PLACEHOLDER;
			if (hash - toHash < 32)
			{
				return 1;
			}
			return PREFIX(_table_move_gap)(table, hash, toHash, cell);
		}
		int hop = __builtin_ffs(cell->secondMaps);
		if (hop > 0 && (hash + hop) < fromHash)
		{
			PREFIX(_table_cell) hopCell = PREFIX(_table_lookup)(table, hash+hop);
			emptyCell->value = hopCell->value;
			// Update the hop bit for the new offset
			cell->secondMaps |= (1 << ((fromHash - hash) - 1));
			// Clear the hop bit in the original cell
			cell->secondMaps &= ~(1 << (hop - 1));
			hopCell->value = MAP_TABLE_VALUE_PLACEHOLDER;
			if (hash - toHash < 32)
			{
				return 1;
			}
			return PREFIX(_table_move_gap)(table, hash + hop, toHash, hopCell);
		}
	}
	return 0;
}
static int PREFIX(_table_rebalance)(PREFIX(_table) *table, uint32_t hash)
{
	for (unsigned i=32 ; i<TABLE_SIZE(table) ; i++)
	{
		PREFIX(_table_cell) cell = PREFIX(_table_lookup)(table, hash + i);
		if (MAP_TABLE_VALUE_NULL(cell->value))
		{
			// We've found a free space, try to move it up.
			return PREFIX(_table_move_gap)(table, hash + i, hash, cell);
		}
	}
	return 0;
}

__attribute__((unused))
static int PREFIX(_insert)(PREFIX(_table) *table, 
                                 MAP_TABLE_VALUE_TYPE value)
{
	MAP_LOCK();
	uint32_t hash = MAP_TABLE_HASH_VALUE(value);
	PREFIX(_table_cell) cell = PREFIX(_table_lookup)(table, hash);
	if (MAP_TABLE_VALUE_NULL(cell->value))
	{
		cell->secondMaps = 0;
		cell->value = value;
		table->table_used++;
		MAP_UNLOCK();
		return 1;
	}
	/* If this cell is full, try the next one. */
	for (unsigned int i=1 ; i<33 ; i++)
	{
		PREFIX(_table_cell) second = 
			PREFIX(_table_lookup)(table, hash+i);
		if (MAP_TABLE_VALUE_NULL(second->value))
		{
			cell->secondMaps |= (1 << (i-1));
			second->value = value;
			table->table_used++;
			MAP_UNLOCK();
			return 1;
		}
	}
	/* If the table is full, or nearly full, then resize it.  Note that we
	 * resize when the table is at 80% capacity because it's cheaper to copy
	 * everything than spend the next few updates shuffling everything around
	 * to reduce contention.  A hopscotch hash table starts to degrade in
	 * performance at around 90% capacity, so stay below that.
	 */
	if (table->table_used > (0.8 * TABLE_SIZE(table)))
	{
		PREFIX(_table_resize)(table);
		MAP_UNLOCK();
		return PREFIX(_insert)(table, value);
	}
	/* If this virtual cell is full, rebalance the hash from this point and
	 * try again. */
	if (PREFIX(_table_rebalance)(table, hash))
	{
		MAP_UNLOCK();
		return PREFIX(_insert)(table, value);
	}
	/** If rebalancing failed, resize even if we are <80% full.  This can
	 * happen if your hash function sucks.  If you don't want this to happen,
	 * get a better hash function. */
	if (PREFIX(_table_resize)(table))
	{
		MAP_UNLOCK();
		return PREFIX(_insert)(table, value);
	}
	fprintf(stderr, "Insert failed\n");
	MAP_UNLOCK();
	return 0;
}

static void *PREFIX(_table_get_cell)(PREFIX(_table) *table, const void *key)
{
	uint32_t hash = MAP_TABLE_HASH_KEY(key);
	PREFIX(_table_cell) cell = PREFIX(_table_lookup)(table, hash);
	// Value does not exist.
	if (!MAP_TABLE_VALUE_NULL(cell->value))
	{
		if (MAP_TABLE_COMPARE_FUNCTION(key, cell->value))
		{
			return cell;
		}
		uint32_t jump = cell->secondMaps;
		// Look at each offset defined by the jump table to find the displaced location.
		for (int hop = __builtin_ffs(jump) ; hop > 0 ; hop = __builtin_ffs(jump))
		{
			PREFIX(_table_cell) hopCell = PREFIX(_table_lookup)(table, hash+hop);
			if (MAP_TABLE_COMPARE_FUNCTION(key, hopCell->value))
			{
				return hopCell;
			}
			// Clear the most significant bit and try again.
			jump &= ~(1 << (hop-1));
		}
	}
#ifndef MAP_TABLE_STATIC_SIZE
	if (table->old)
	{
		return PREFIX(_table_get_cell)(table->old, key);
	}
#endif
	return NULL;
}

__attribute__((unused))
static void PREFIX(_table_move_second)(PREFIX(_table) *table, 
		PREFIX(_table_cell) emptyCell)
{
	uint32_t jump = emptyCell->secondMaps;
	// Look at each offset defined by the jump table to find the displaced location.
	int hop = __builtin_ffs(jump);
	PREFIX(_table_cell) hopCell = 
		PREFIX(_table_lookup)(table, (emptyCell - table->table) + hop);
	emptyCell->value = hopCell->value;
	emptyCell->secondMaps &= ~(1 << (hop-1));
	if (0 == hopCell->secondMaps)
	{
		hopCell->value = MAP_TABLE_VALUE_PLACEHOLDER;
	}
	else
	{
		PREFIX(_table_move_second)(table, hopCell);
	}
}
__attribute__((unused))
static void PREFIX(_remove)(PREFIX(_table) *table, void *key)
{
	MAP_LOCK();
	PREFIX(_table_cell) cell = PREFIX(_table_get_cell)(table, key);
	if (NULL == cell) { return; }

	uint32_t hash = MAP_TABLE_HASH_KEY(key);
	PREFIX(_table_cell) baseCell = PREFIX(_table_lookup)(table, hash);
	if (baseCell && baseCell != cell)
	{
		uint32_t displacement = (cell - baseCell + table->table_size) % table->table_size;
		uint32_t jump = 1 << (displacement - 1);
		if ((baseCell->secondMaps & jump))
		{
			// If we are removing a cell stored adjacent to its base due to hash
			// collision, we have to clear the base cell's neighbor bit.
			// Otherwise, a later remove can move the new placeholder value to the head
			// which will cause further chained lookups to fail.
			baseCell->secondMaps &= ~jump;
		}
	}

	// If the cell contains a value, set it to the placeholder and shuffle up
	// everything
	if (0 == cell->secondMaps)
	{
		cell->value = MAP_TABLE_VALUE_PLACEHOLDER;
	}
	else
	{
		PREFIX(_table_move_second)(table, cell);
	}
	table->table_used--;
	MAP_UNLOCK();
}

__attribute__((unused))
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
static MAP_TABLE_VALUE_TYPE* 
#else
static MAP_TABLE_VALUE_TYPE 
#endif
	PREFIX(_table_get)(PREFIX(_table) *table,
		const void *key)
{
	PREFIX(_table_cell) cell = PREFIX(_table_get_cell)(table, key);
	if (NULL == cell)
	{
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
		return NULL;
#else
		return MAP_TABLE_VALUE_PLACEHOLDER;
#endif
	}
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
	return &cell->value;
#else
	return cell->value;
#endif
}
__attribute__((unused))
static void PREFIX(_table_set)(PREFIX(_table) *table, const void *key,
		MAP_TABLE_VALUE_TYPE value)
{
	PREFIX(_table_cell) cell = PREFIX(_table_get_cell)(table, key);
	if (NULL == cell)
	{
		PREFIX(_insert)(table, value);
	}
	cell->value = value;
}

__attribute__((unused))
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
static MAP_TABLE_VALUE_TYPE* 
#else
static MAP_TABLE_VALUE_TYPE 
#endif
PREFIX(_next)(PREFIX(_table) *table,
                    struct PREFIX(_table_enumerator) **state)
{
	if (NULL == *state)
	{
		*state = CALLOC(1, sizeof(struct PREFIX(_table_enumerator)));
		// Make sure that we are not reallocating the table when we start
		// enumerating
		MAP_LOCK();
		(*state)->table = table;
		(*state)->index = -1;
		IF_NO_GC(__sync_fetch_and_add(&table->enumerator_count, 1);)
		MAP_UNLOCK();
	}
	if ((*state)->seen >= (*state)->table->table_used)
	{
#ifndef ENABLE_GC
		MAP_LOCK();
		__sync_fetch_and_sub(&table->enumerator_count, 1);
		MAP_UNLOCK();
		free(*state);
#endif
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
		return NULL;
#else
		return MAP_TABLE_VALUE_PLACEHOLDER;
#endif
	}
	while ((++((*state)->index)) < TABLE_SIZE((*state)->table))
	{
		if (!MAP_TABLE_VALUE_NULL((*state)->table->table[(*state)->index].value))
		{
			(*state)->seen++;
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
			return &(*state)->table->table[(*state)->index].value;
#else
			return (*state)->table->table[(*state)->index].value;
#endif
		}
	}
#ifndef ENABLE_GC
	// Should not be reached, but may be if the table is unsafely modified.
	MAP_LOCK();
	table->enumerator_count--;
	MAP_UNLOCK();
	free(*state);
#endif
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
	return NULL;
#else
	return MAP_TABLE_VALUE_PLACEHOLDER;
#endif
}
/**
 * Returns the current value for an enumerator.  This is used when you remove
 * objects during enumeration.  It may cause others to be shuffled up the
 * table.
 */
__attribute__((unused))
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
static MAP_TABLE_VALUE_TYPE* 
#else
static MAP_TABLE_VALUE_TYPE 
#endif
PREFIX(_current)(PREFIX(_table) *table,
                    struct PREFIX(_table_enumerator) **state)
{
#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
	return &(*state)->table->table[(*state)->index].value;
#else
	return (*state)->table->table[(*state)->index].value;
#endif
}

#undef TABLE_SIZE
#undef REALLY_PREFIX_SUFFIX
#undef PREFIX_SUFFIX
#undef PREFIX

#undef MAP_TABLE_NAME
#undef MAP_TABLE_COMPARE_FUNCTION
#undef MAP_TABLE_HASH_KEY
#undef MAP_TABLE_HASH_VALUE

#ifdef MAP_TABLE_STATIC_SIZE
#	undef MAP_TABLE_STATIC_SIZE
#endif

#undef MAP_TABLE_VALUE_TYPE

#undef MAP_LOCK
#undef MAP_UNLOCK
#ifdef MAP_TABLE_NO_LOCK
#	undef MAP_TABLE_NO_LOCK
#endif

#ifdef MAP_TABLE_SINGLE_THREAD
#	undef MAP_TABLE_SINGLE_THREAD
#endif

#undef MAP_TABLE_VALUE_NULL
#undef MAP_TABLE_VALUE_PLACEHOLDER

#ifdef MAP_TABLE_ACCESS_BY_REFERENCE
#	undef MAP_TABLE_ACCESS_BY_REFERENCE
#endif

#undef CALLOC
#undef IF_NO_GC
#undef IF_GC
#undef MAP_TABLE_TYPES_BITMAP
