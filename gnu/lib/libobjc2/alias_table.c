/** A hash table for mapping compatibility aliases to classes.
   Copyright (c) 2011 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Created: March 2011

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "visibility.h"
#include "objc/runtime.h"
#include "class.h"
#include "lock.h"
#include "string_hash.h"

#include <stdlib.h>

struct objc_alias
{
	const char* name;
	Class class;
};

typedef struct objc_alias Alias;

static int alias_compare(const char *name, const Alias alias)
{
	return string_compare(name, alias.name);
}

static int alias_hash(const Alias alias)
{
	return string_hash(alias.name);
}
static int alias_is_null(const Alias alias)
{
	return alias.name == NULL;
}
static Alias NullAlias;
#define MAP_TABLE_NAME alias_table_internal
#define MAP_TABLE_COMPARE_FUNCTION alias_compare
#define MAP_TABLE_HASH_KEY string_hash
#define MAP_TABLE_HASH_VALUE alias_hash
#define MAP_TABLE_VALUE_TYPE struct objc_alias
#define MAP_TABLE_VALUE_NULL alias_is_null
#define MAP_TABLE_VALUE_PLACEHOLDER NullAlias

#include "hash_table.h"

static alias_table_internal_table *alias_table;

PRIVATE void init_alias_table(void)
{
	alias_table_internal_initialize(&alias_table, 128);
}


static Alias alias_table_get_safe(const char *alias_name)
{
	return alias_table_internal_table_get(alias_table, alias_name);
}


OBJC_PUBLIC Class alias_getClass(const char *alias_name)
{
	if (NULL == alias_name)
	{
		return NULL;
	}

	Alias alias = alias_table_get_safe(alias_name);

	if (NULL == alias.name)
	{
		return NULL;
	}

	return alias.class;
}

PRIVATE void alias_table_insert(Alias alias)
{
	alias_table_internal_insert(alias_table, alias);
}

OBJC_PUBLIC BOOL class_registerAlias_np(Class class, const char *alias)
{
	if ((NULL == alias) || (NULL == class))
	{
		return 0;
	}

	class = (Class)objc_getClass(class->name);

	/*
	 * If there already exists a matching alias, determine whether we the existing
	 * alias is the correct one. Please note that objc_getClass() goes through the
	 * alias lookup and will create the alias table if necessary.
	 */
	Class existingClass = (Class)objc_getClass(alias);
	if (NULL != existingClass)
	{
		/*
		 *  Return YES if the alias has already been registered for this very
		 * class, and NO if the alias is already used for another class.
		 */
		return (class == existingClass);
	}
	Alias newAlias = { strdup(alias), class };
	alias_table_insert(newAlias);
	return 1;
}
