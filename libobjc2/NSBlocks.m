#include "objc/runtime.h"
#include "class.h"
#include "loader.h"
#include "lock.h"
#include "objc/blocks_runtime.h"
#include "dtable.h"
#include <assert.h>

OBJC_PUBLIC struct objc_class _NSConcreteGlobalBlock;
OBJC_PUBLIC struct objc_class _NSConcreteStackBlock;
OBJC_PUBLIC struct objc_class _NSConcreteMallocBlock;

static struct objc_class _NSConcreteGlobalBlockMeta;
static struct objc_class _NSConcreteStackBlockMeta;
static struct objc_class _NSConcreteMallocBlockMeta;

static struct objc_class _NSBlock;
static struct objc_class _NSBlockMeta;

static void createNSBlockSubclass(Class superclass, Class newClass, 
		Class metaClass, char *name)
{
	// Initialize the metaclass
	//metaClass->class_pointer = superclass->class_pointer;
	//metaClass->super_class = superclass->class_pointer;
	metaClass->info = objc_class_flag_meta;
	metaClass->dtable = uninstalled_dtable;

	// Set up the new class
	newClass->isa = metaClass;
	newClass->super_class = superclass;
	newClass->name = name;
	newClass->dtable = uninstalled_dtable;

	LOCK_RUNTIME_FOR_SCOPE();
	objc_load_class(newClass);

}

#define NEW_CLASS(super, sub) \
	createNSBlockSubclass(super, &sub, &sub ## Meta, #sub)

OBJC_PUBLIC
BOOL objc_create_block_classes_as_subclasses_of(Class super)
{
	if (_NSBlock.super_class != NULL) { return NO; }

	NEW_CLASS(super, _NSBlock);
	NEW_CLASS(&_NSBlock, _NSConcreteStackBlock);
	NEW_CLASS(&_NSBlock, _NSConcreteGlobalBlock);
	NEW_CLASS(&_NSBlock, _NSConcreteMallocBlock);
	// Global blocks never need refcount manipulation.
	objc_set_class_flag(&_NSConcreteGlobalBlock,
	                    objc_class_flag_permanent_instances);
	return YES;
}
