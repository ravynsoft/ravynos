#include "objc/runtime.h"
#include "objc/hooks.h"
#include "objc/developer.h"
#include "alias.h"
#include "class.h"
#include "method.h"
#include "selector.h"
#include "lock.h"
#include "dtable.h"
#include "legacy.h"
#include "visibility.h"
#include <stdlib.h>
#include <assert.h>

void objc_init_protocols(struct objc_protocol_list *protos);
void objc_compute_ivar_offsets(Class class);

////////////////////////////////////////////////////////////////////////////////
// +load method hash table
////////////////////////////////////////////////////////////////////////////////
static int imp_compare(const void *i1, void *i2)
{
	return i1 == i2;
}
static int32_t imp_hash(const void *imp)
{
	return (int32_t)(((uintptr_t)imp) >> 4);
}
#define MAP_TABLE_NAME load_messages
#define MAP_TABLE_COMPARE_FUNCTION imp_compare
#define MAP_TABLE_HASH_KEY imp_hash
#define MAP_TABLE_HASH_VALUE imp_hash
#include "hash_table.h"

static load_messages_table *load_table;

SEL loadSel;

PRIVATE void objc_init_load_messages_table(void)
{
	load_messages_initialize(&load_table, 4096);
	loadSel = sel_registerName("load");
}

PRIVATE void objc_send_load_message(Class class)
{
	Class meta = class->isa;
	for (struct objc_method_list *l=meta->methods ; NULL!=l ; l=l->next)
	{
		for (int i=0 ; i<l->count ; i++)
		{
			Method m = method_at_index(l, i);
			if (sel_isEqual(m->selector, loadSel))
			{
				if (load_messages_table_get(load_table, m->imp) == 0)
				{
					m->imp((id)class, loadSel);
					load_messages_insert(load_table, m->imp);
				}
			}
		}
	}
}

// Get the functions for string hashing
#include "string_hash.h"

static int class_compare(const char *name, const Class class)
{
	return string_compare(name, class->name);
}
static int class_hash(const Class class)
{
	return string_hash(class->name);
}
#define MAP_TABLE_NAME class_table_internal
#define MAP_TABLE_COMPARE_FUNCTION class_compare
#define MAP_TABLE_HASH_KEY string_hash
#define MAP_TABLE_HASH_VALUE class_hash
// This defines the maximum number of classes that the runtime supports.
/*
#define MAP_TABLE_STATIC_SIZE 2048
#define MAP_TABLE_STATIC_NAME class_table
*/
#include "hash_table.h"

static class_table_internal_table *class_table;


#define unresolved_class_next subclass_list
#define unresolved_class_prev sibling_class
/**
 * Linked list using the subclass_list pointer in unresolved classes.
 */
static Class unresolved_class_list;

static enum objc_developer_mode_np mode;

void objc_setDeveloperMode_np(enum objc_developer_mode_np newMode)
{
	mode = newMode;
}

////////////////////////////////////////////////////////////////////////////////
// Class table manipulation
////////////////////////////////////////////////////////////////////////////////

PRIVATE Class zombie_class;

PRIVATE void class_table_insert(Class class)
{
	if (!objc_test_class_flag(class, objc_class_flag_resolved))
	{
		if (Nil != unresolved_class_list)
		{
			unresolved_class_list->unresolved_class_prev = class;
		}
		class->unresolved_class_next = unresolved_class_list;
		unresolved_class_list = class;
	}
	if ((0 == zombie_class) && (strcmp("NSZombie", class->name) == 0))
	{
		zombie_class = class;
	}
	class_table_internal_insert(class_table, class);
}

PRIVATE Class class_table_get_safe(const char *class_name)
{
	if (NULL == class_name) { return Nil; }
	return class_table_internal_table_get(class_table, class_name);
}

PRIVATE Class class_table_next(void **e)
{
	return class_table_internal_next(class_table,
			(struct class_table_internal_table_enumerator**)e);
}

PRIVATE void init_class_tables(void)
{
	class_table_internal_initialize(&class_table, 4096);
	objc_init_load_messages_table();
}

////////////////////////////////////////////////////////////////////////////////
// Loader functions
////////////////////////////////////////////////////////////////////////////////

PRIVATE BOOL objc_resolve_class(Class cls)
{
	// Skip this if the class is already resolved.
	if (objc_test_class_flag(cls, objc_class_flag_resolved)) { return YES; }

	// We can only resolve the class if its superclass is resolved.
	if (cls->super_class)
	{
		Class super = cls->super_class;

		if (!objc_test_class_flag(super, objc_class_flag_resolved))
		{
			if (!objc_resolve_class(super))
			{
				return NO;
			}
		}
	}
#ifdef OLDABI_COMPAT
	else
	{
		struct objc_class_gsv1 *ocls = objc_legacy_class_for_class(cls);
		if (ocls != NULL)
		{
			const char *super_name = (const char*)ocls->super_class;
			if (super_name)
			{
				Class super = (Class)objc_getClass(super_name);
				if (super == Nil)
				{
					return NO;
				}
				cls->super_class = super;
				return objc_resolve_class(cls);
			}
		}
	}
#endif


	// Remove the class from the unresolved class list
	if (Nil == cls->unresolved_class_prev)
	{
		unresolved_class_list = cls->unresolved_class_next;
	}
	else
	{
		cls->unresolved_class_prev->unresolved_class_next =
			cls->unresolved_class_next;
	}
	if (Nil != cls->unresolved_class_next)
	{
		cls->unresolved_class_next->unresolved_class_prev =
			cls->unresolved_class_prev;
	}
	cls->unresolved_class_prev = Nil;
	cls->unresolved_class_next = Nil;

	// The superclass for the metaclass.  This is the metaclass for the
	// superclass if one exists, otherwise it is the root class itself
	Class superMeta = Nil;
	// The metaclass for the metaclass.  This is always the root class's
	// metaclass.
	Class metaMeta = Nil;

	// Resolve the superclass pointer

	if (NULL == cls->super_class)
	{
		superMeta = cls;
		metaMeta = cls->isa;
	}
	else
	{
		// Resolve the superclass if it isn't already resolved
		Class super = cls->super_class;
		if (!objc_test_class_flag(super, objc_class_flag_resolved))
		{
			objc_resolve_class(super);
		}
		superMeta = super->isa;
		// Set the superclass pointer for the class and the superclass
		do
		{
			metaMeta = super->isa;
			super = super->super_class;
		} while (Nil != super);
	}
	Class meta = cls->isa;

	// Make the root class the superclass of the metaclass (e.g. NSObject is
	// the superclass of all metaclasses in classes that inherit from NSObject)
	meta->super_class = superMeta;
	meta->isa = metaMeta;

	// Don't register root classes as children of anything
	if (Nil != cls->super_class)
	{
		// Set up the class links
		cls->sibling_class = cls->super_class->subclass_list;
		cls->super_class->subclass_list = cls;
	}
	// Set up the metaclass links
	meta->sibling_class = superMeta->subclass_list;
	superMeta->subclass_list = meta;

	// Mark this class (and its metaclass) as resolved
	objc_set_class_flag(cls, objc_class_flag_resolved);
	objc_set_class_flag(cls->isa, objc_class_flag_resolved);


	// Fix up the ivar offsets
	objc_compute_ivar_offsets(cls);
#ifdef OLDABI_COMPAT
	struct objc_class_gsv1 *oldCls = objc_legacy_class_for_class(cls);
	if (oldCls)
	{
		oldCls->super_class = cls->super_class;
		oldCls->isa->super_class = cls->isa->super_class;
	}
#endif
	// Send the +load message, if required
	if (!objc_test_class_flag(cls, objc_class_flag_user_created))
	{
		objc_send_load_message(cls);
	}
	if (_objc_load_callback)
	{
		_objc_load_callback(cls, 0);
	}
	return YES;
}

PRIVATE void objc_resolve_class_links(void)
{
	LOCK_RUNTIME_FOR_SCOPE();
	BOOL resolvedClass;
	do
	{
		Class class = unresolved_class_list;
		resolvedClass = NO;
		while ((Nil != class))
		{
			Class next = class->unresolved_class_next;
			// If the class has been resolved, then this means that the last
			// call to objc_resolve_class resolved it as part of resolving
			// superclasses and removed it from the list.  We now don't have a
			// pointer into the linked list, so abort and try again from the
			// start.
			if (objc_test_class_flag(class, objc_class_flag_resolved))
			{
				assert(resolvedClass);
				break;
			}
			objc_resolve_class(class);
			if (resolvedClass ||
				objc_test_class_flag(class, objc_class_flag_resolved))
			{
				resolvedClass = YES;
			}
			class = next;
		}
	} while (resolvedClass);
}
PRIVATE void __objc_resolve_class_links(void)
{
	static BOOL warned = NO;
	if (!warned)
	{
		fprintf(stderr,
			"Warning: Calling deprecated private ObjC runtime function %s\n", __func__);
		warned = YES;
	}
	objc_resolve_class_links();
}

static void reload_class(struct objc_class *class, struct objc_class *old)
{
	const char *superclassName = (char*)class->super_class;
	class->super_class = class_table_get_safe(superclassName);
	// Checking the instance sizes are equal here is a quick-and-dirty test.
	// It's not actually needed, because we're testing the ivars are at the
	// same locations next, but it lets us skip those tests if the total size
	// is different.
	BOOL equalLayouts = (class->super_class == old->super_class) &&
		(class->instance_size == old->instance_size);
	// If either of the classes has an empty ivar list, then the other one must too.
	if ((NULL == class->ivars) || (NULL == old->ivars))
	{
		equalLayouts &= (class->ivars == old->ivars);
	}
	else
	{
		// If the class sizes are the same, ensure that the ivars have the same
		// types, names, and offsets.  Note: Renaming an ivar is treated as a
		// conflict because name changes are often accompanied by semantic
		// changes.  For example, an object ivar at offset 16 goes from being
		// called 'delegate' to being called 'view' - we almost certainly don't
		// want methods that expect to be working with the delegate ivar to
		// work with the view ivar now!
		for (int i=0 ; equalLayouts && (i<old->ivars->count) ; i++)
		{
			struct objc_ivar *oldIvar = ivar_at_index(old->ivars, i);
			struct objc_ivar *newIvar = ivar_at_index(class->ivars, i);
			equalLayouts &= strcmp(oldIvar->name, newIvar->name) == 0;
			equalLayouts &= strcmp(oldIvar->type, newIvar->type) == 0;
			equalLayouts &= (oldIvar->offset == newIvar->offset);
		}
	}

	// If the layouts are equal, then we can simply tack the class's method
	// list on to the front of the old class and update the dtable.
	if (equalLayouts)
	{
		class->methods->next = old->methods;
		old->methods = class->methods;
		objc_update_dtable_for_class(old);
		return;
	}

	// If we get to here, then we are adding a new class.  This is where things
	// start to get a bit tricky...

	// Ideally, we'd want to capture the subclass list here.  Unfortunately,
	// this is not possible because the subclass will contain methods that
	// refer to ivars in the superclass.
	//
	// We can't use the non-fragile ABI's offset facility easily, because we'd
	// have to have two (or more) offsets for the same ivar.  This gets messy
	// very quickly.  Ideally, we'd want every class to include ivar offsets
	// for every single (public) ivar in its superclasses.  These could then be
	// updated by copies of the class.  Defining a development ABI is something
	// to consider for a future release.
	class->subclass_list = NULL;

	// Replace the old class with this one in the class table.  New lookups for
	// this class will now return this class.
	class_table_internal_table_set(class_table, (void*)class->name, class);

	// Set the uninstalled dtable.  The compiler could do this as well.
	class->dtable = uninstalled_dtable;
	class->isa->dtable = uninstalled_dtable;

	// If this is a root class, make the class into the metaclass's superclass.
	// This means that all instance methods will be available to the class.
	if (NULL == superclassName)
	{
		class->isa->super_class = class;
	}

	if (class->protocols)
	{
		objc_init_protocols(class->protocols);
	}
}

/**
 * Loads a class.  This function assumes that the runtime mutex is locked.
 */
PRIVATE void objc_load_class(struct objc_class *class)
{
	struct objc_class *existingClass = class_table_get_safe(class->name);
	if (Nil != existingClass)
	{
		if (objc_developer_mode_developer != mode)
		{
			fprintf(stderr,
				"Loading two versions of %s.  The class that will be used is undefined\n",
				class->name);
			return;
		}
		reload_class(class, existingClass);
		return;
	}

#ifdef _WIN32
	// On Windows, the super_class pointer may point to the local __imp_
	// symbol, rather than to the external symbol.  The runtime must remove the
	// extra indirection.
	if (class->super_class)
	{
		Class superMeta = class->super_class->isa;
		if (!class_isMetaClass(superMeta))
		{
			class->super_class = superMeta;
		}
	}
#endif

	// Work around a bug in some versions of GCC that don't initialize the
	// class structure correctly.
	class->subclass_list = NULL;

	// Insert the class into the class table
	class_table_insert(class);

	// Set the uninstalled dtable.  The compiler could do this as well.
	class->dtable = uninstalled_dtable;
	class->isa->dtable = uninstalled_dtable;

	// Mark constant string instances as never needing refcount manipulation.
	if (strcmp(class->name, "NSConstantString") == 0)
	{
		objc_set_class_flag(class, objc_class_flag_permanent_instances);
	}

	// If this is a root class, make the class into the metaclass's superclass.
	// This means that all instance methods will be available to the class.
	if (NULL == class->super_class)
	{
		class->isa->super_class = class;
	}

	if (class->protocols)
	{
		objc_init_protocols(class->protocols);
	}
}

PRIVATE Class SmallObjectClasses[7];

BOOL objc_registerSmallObjectClass_np(Class class, uintptr_t mask)
{
	if ((mask & OBJC_SMALL_OBJECT_MASK) != mask)
	{
		return NO;
	}
	if (sizeof(void*) == 4)
	{
		if (Nil == SmallObjectClasses[0])
		{
			SmallObjectClasses[0] = class;
			return YES;
		}
		return NO;
	}
	if (Nil != SmallObjectClasses[mask])
	{
		return NO;
	}
	SmallObjectClasses[mask] = class;
	return YES;
}

PRIVATE void class_table_remove(Class cls)
{
	assert(objc_test_class_flag(cls, objc_class_flag_user_created));
	class_table_internal_remove(class_table, (void*)cls->name);
}


////////////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////////////

int objc_getClassList(Class *buffer, int bufferLen)
{
	if (buffer == NULL || bufferLen == 0)
	{
		return class_table->table_used;
	}
	int count = 0;
	struct class_table_internal_table_enumerator *e = NULL;
	Class next;
	while (count < bufferLen &&
		(next = class_table_internal_next(class_table, &e)))
	{
		buffer[count++] = next;
	}
	return count;
}
Class *objc_copyClassList(unsigned int *outCount)
{
	int count = class_table->table_used;
	Class *buffer = calloc(sizeof(Class), count);
	if (NULL != outCount)
	{
		*outCount = count;
	}
	objc_getClassList(buffer, count);
	return buffer;
}

Class class_getSuperclass(Class cls)
{
	if (Nil == cls) { return Nil; }
	if (!objc_test_class_flag(cls, objc_class_flag_resolved))
	{
		objc_resolve_class(cls);
	}
	return cls->super_class;
}


id objc_getClass(const char *name)
{
	id class = (id)class_table_get_safe(name);

	if (nil != class) { return class; }

	// Second chance lookup via @compatibilty_alias:
	class = (id)alias_getClass(name);
	if (nil != class) { return class; }

	// Third chance lookup via the hook:
	if (0 != _objc_lookup_class)
	{
		class = (id)_objc_lookup_class(name);
	}

	return class;
}

id objc_lookUpClass(const char *name)
{
	return (id)class_table_get_safe(name);
}


id objc_getMetaClass(const char *name)
{
	Class cls = (Class)objc_getClass(name);
	return cls == Nil ? nil : (id)cls->isa;
}

// Legacy interface compatibility

id objc_get_class(const char *name)
{
	return objc_getClass(name);
}

id objc_lookup_class(const char *name)
{
	return objc_getClass(name);
}

id objc_get_meta_class(const char *name)
{
	return objc_getMetaClass(name);
}

Class objc_next_class(void **enum_state)
{
  return class_table_next ( enum_state);
}

Class class_pose_as(Class impostor, Class super_class)
{
	fprintf(stderr, "Class posing is no longer supported.\n");
	fprintf(stderr, "Please use class_replaceMethod() instead.\n");
	abort();
}
