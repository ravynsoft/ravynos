#include <string.h>
#include <stdint.h>

/**
 * Efficient string hash function.
 */
__attribute__((unused))
static uint32_t string_hash(const char *str)
{
	uint32_t hash = 0;
	int32_t c;
	while ((c = *str++))
	{
		hash = c + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}

/**
 * Test two strings for equality.
 */
__attribute__((unused))
static int string_compare(const char *str1, const char *str2)
{
	if (str1 == str2)
	{
		return 1;
	}
	if (str1 == NULL || str2 == NULL) 
	{
		return 0;
	}
	return strcmp(str1, str2) == 0;
}
