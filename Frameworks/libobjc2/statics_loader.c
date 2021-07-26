#include <string.h>
#include <stdio.h>
#include "objc/runtime.h"
#include "module.h"
#include "constant_string.h"
#include "visibility.h"

#define BUFFER_TYPE struct objc_static_instance_list *
#include "buffer.h"

static BOOL try_init_statics(struct objc_static_instance_list *statics)
{
	const char *class_name = statics->class_name;

	// This is a horrible hack.
	//
	// Very bad things happen when you have more than one constant string class
	// used in a program.  Unfortunately, GCC defaults to using
	// NXConstantString, and if you forget to specify
	// -fconstant-string-class=NSConstantString for some compilation units then
	// you will end up with some NSConstantString instances and some
	// NXConstantString instances.  This is a mess.  We hack around this by
	// silently assuming that the user meant NSConstantString when they said
	// NXConstantString if NSConstantString is set as the constant string class
	// in string_class.h or by an external -D flag.
	if (strcmp(class_name, "NXConstantString") == 0)
	{
		class_name = CONSTANT_STRING_CLASS;
	}

	Class class = (Class)objc_getClass(class_name);

	if (Nil == class)
	{
		return NO;
	}
	for (id *instance=statics->instances ; nil!=*instance ; instance++)
	{
		(*instance)->isa = class;
	}
	return YES;
}
PRIVATE void objc_init_statics(struct objc_static_instance_list *statics)
{
	if (!try_init_statics(statics))
	{
		set_buffered_object_at_index(statics, buffered_objects++);
	}
}

PRIVATE void objc_init_buffered_statics(void)
{
	BOOL shouldReshuffle = NO;

	for (unsigned i=0 ; i<buffered_objects ; i++)
	{
		struct objc_static_instance_list *c = buffered_object_at_index(i);
		if (NULL != c)
		{
			if (try_init_statics(c))
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
