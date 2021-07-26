#include <stdio.h>
#include "objc/runtime.h"
#include "visibility.h"
#include "loader.h"
#include "dtable.h"
#include "properties.h"

#define BUFFER_TYPE struct objc_category *
#include "buffer.h"

void objc_send_load_message(Class class);

static void register_methods(struct objc_class *cls, struct objc_method_list *l)
{
	if (NULL == l) { return; }

	// Add the method list at the head of the list of lists.
	l->next = cls->methods;
	cls->methods = l;
	// Update the dtable to catch the new methods, if the dtable has been
	// created (don't bother creating dtables for classes when categories are
	// loaded if the class hasn't received any messages yet.
	if (classHasDtable(cls))
	{
		add_method_list_to_class(cls, l);
	}
}

static void load_category(struct objc_category *cat, struct objc_class *class)
{
	register_methods(class, cat->instance_methods);
	register_methods(class->isa, cat->class_methods);
	//fprintf(stderr, "Loading %s (%s)\n", cat->class_name, cat->name);

	if (cat->protocols)
	{
		objc_init_protocols(cat->protocols);
		cat->protocols->next = class->protocols;
		class->protocols = cat->protocols;
	}
	if (cat->properties)
	{
		cat->properties->next = class->properties;
		class->properties = cat->properties;
	}
	if (cat->class_properties)
	{
		cat->class_properties->next = class->isa->properties;
		class->isa->properties = cat->class_properties;
	}
}

static BOOL try_load_category(struct objc_category *cat)
{
	Class class = (Class)objc_getClass(cat->class_name);
	//fprintf(stderr, "Trying to load %s (%s)\n", cat->class_name, cat->name);
	if (Nil != class)
	{
		load_category(cat, class);
		return YES;
	}
	//fprintf(stderr, "waiting to load %s (%s)\n", cat->class_name, cat->name);
	return NO;
}

/**
 * Attaches a category to its class, if the class is already loaded.  Buffers
 * it for future resolution if not.
 */
PRIVATE void objc_try_load_category(struct objc_category *cat)
{
	if (!try_load_category(cat))
	{
		set_buffered_object_at_index(cat, buffered_objects++);
	}
}

PRIVATE void objc_load_buffered_categories(void)
{
	BOOL shouldReshuffle = NO;

	for (unsigned i=0 ; i<buffered_objects ; i++)
	{
		struct objc_category *c = buffered_object_at_index(i);
		if (NULL != c)
		{
			if (try_load_category(c))
			{
				set_buffered_object_at_index(NULL, i);
				shouldReshuffle = YES;
			}
		}
	}

	if (shouldReshuffle)
	{
		compact_buffer();
	}
}

