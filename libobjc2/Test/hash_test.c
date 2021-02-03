#include <assert.h>
#include <stdint.h>

static int compare(const void *i1, uint32_t i2)
{ 
	return ((uint32_t)(uintptr_t)i1) == i2;
}

static uint32_t hash_int(uint32_t i)
{
	return i;
}
static uint32_t hash_key(const void *i)
{
	return hash_int((uint32_t)(uintptr_t)i);
}

static int is_null(uint32_t i)
{
	return i == 0;
}

#define MAP_TABLE_NAME test
#define MAP_TABLE_COMPARE_FUNCTION compare
#define MAP_TABLE_VALUE_TYPE uint32_t
#define MAP_TABLE_VALUE_PLACEHOLDER 0
#define MAP_TABLE_VALUE_NULL is_null
#define MAP_TABLE_HASH_KEY hash_key
#define MAP_TABLE_HASH_VALUE hash_int
#define MAP_TABLE_SINGLE_THREAD 1
#define MAP_TABLE_NO_LOCK 1

#include "../hash_table.h"

static test_table *table;


void check_table()
{
	int count = 0;
	for (int i=0 ; i<table->table_size ; i++)
	{
		struct test_table_cell_struct *s = &table->table[i];
		uint32_t v = ((uint32_t)s->value);
		if (v != 0)
		{
			count++;
			assert(v == test_table_get(table, (void*)(uintptr_t)v));
		}
		else
		{
			assert(s->secondMaps == 0);
		}
	}
	assert(count == table->table_used);
}

int main(void)
{
	test_initialize(&table, 128);
	const int step = 2;
	// 10 iterations was enough to hit the failing case previously.  For
	// extra paranoia, we can run this test a lot.
	const int max =
#ifdef SLOW_TESTS
		8096;
#else
		10;
#endif
	for (int seed = 0 ; seed < max ; seed++)
	{
		fprintf(stderr, "Seed: %d\n", seed);
		srand(seed);
		for (uint32_t i=1 ; i<5000 ; i+=step)
		{
			int x = rand();
			if (x == 0)
			{
				continue;
			}
			test_insert(table, x);
			check_table();
		}
		srand(seed);
		for (uint32_t i=1 ; i<5000 ; i+=step)
		{
			int x = rand();
			if (x == 0)
			{
				continue;
			}
			test_remove(table, (void*)(uintptr_t)x);
			check_table();
		}
		assert(table->table_used == 0);
	}
}
