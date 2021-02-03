#include <stdbool.h>
#include <stdint.h>

struct test_struct {
	uintptr_t key;
};

struct test_struct null_placeholder = {0};

static int test_compare(const void *key, const struct test_struct test) {
	return (uintptr_t)key == test.key;
}

// force hash collisions
static uint32_t test_key_hash(const void *ptr) {
	return ((uint32_t)(uintptr_t)ptr)>>2;
}

static uint32_t test_value_hash(const struct test_struct test) {
	return test.key>>2;
}

static int test_is_null(const struct test_struct test) {
	return test.key == 0;
}

#define MAP_TABLE_NAME test
#define MAP_TABLE_COMPARE_FUNCTION test_compare
#define MAP_TABLE_VALUE_TYPE struct test_struct
#define MAP_TABLE_VALUE_NULL test_is_null
#define MAP_TABLE_HASH_KEY test_key_hash
#define MAP_TABLE_HASH_VALUE test_value_hash
#define MAP_TABLE_VALUE_PLACEHOLDER null_placeholder
#define MAP_TABLE_ACCESS_BY_REFERENCE 1
#define MAP_TABLE_SINGLE_THREAD 1
#define MAP_TABLE_NO_LOCK 1

#include "../hash_table.h"

int main(int argc, char *argv[])
{
	test_table *testTable;
	test_initialize(&testTable, 128);

	struct test_struct one, two, three;
	one.key = 1;
	two.key = 2;
	three.key = 3;

	test_insert(testTable, one);
	test_insert(testTable, two);
	test_insert(testTable, three);

	test_remove(testTable, (void*)2);
	test_remove(testTable, (void*)1);

	struct test_struct *pthree = test_table_get(testTable, (void*)3);
	if (!pthree) {
		fprintf(stderr, "failed to find value (key=3) inserted into hash table\n");
		return 1;
	}

	return 0;
}
