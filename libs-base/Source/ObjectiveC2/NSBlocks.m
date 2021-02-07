
#import <objc/objc.h>

#if defined (__GNU_LIBOBJC__)

#warning Unable to build NSBlocks for this runtime.

/* FIXME ... these let us link, but blocks will be broken.
 */
void *_NSConcreteStackBlock;
BOOL objc_create_block_classes_as_subclasses_of(Class super)
{
  return NO;
}

#else

#import <objc/objc-api.h>
#import "ObjectiveC2/objc/runtime.h"

#import "ObjectiveC2/objc/blocks_runtime.h"
#include <assert.h>

struct objc_class _NSConcreteGlobalBlock;
struct objc_class _NSConcreteStackBlock;

static struct objc_class _NSConcreteGlobalBlockMeta;
static struct objc_class _NSConcreteStackBlockMeta;

static struct objc_class _NSBlock;
static struct objc_class _NSBlockMeta;

void __objc_update_dispatch_table_for_class(Class);
extern struct sarray *__objc_uninstalled_dtable;
extern objc_mutex_t __objc_runtime_mutex;

static void
createNSBlockSubclass(Class superclass, Class newClass, 
  Class metaClass, char *name)
{
  // Initialize the metaclass
  metaClass->class_pointer = superclass->class_pointer;
  metaClass->super_class = superclass->class_pointer;
  metaClass->info = _CLS_META;
  metaClass->dtable = __objc_uninstalled_dtable;

  // Set up the new class
  newClass->class_pointer = metaClass;
  newClass->super_class = superclass;
  newClass->name = name;
  newClass->info = _CLS_CLASS;
  newClass->dtable = __objc_uninstalled_dtable;

  // Initialize the dispatch table for the class and metaclass.
  __objc_update_dispatch_table_for_class(metaClass);
  __objc_update_dispatch_table_for_class(newClass);
  CLS_SETINITIALIZED(metaClass);
  CLS_SETINITIALIZED(newClass);
  CLS_SETRESOLV(metaClass);
  CLS_SETRESOLV(newClass);
  // Add pointer from super class
  objc_mutex_lock(__objc_runtime_mutex);
  newClass->sibling_class = newClass->super_class->subclass_list;
  newClass->super_class->subclass_list = newClass;
  metaClass->sibling_class = metaClass->super_class->subclass_list;
  metaClass->super_class->subclass_list = metaClass;
  objc_mutex_unlock(__objc_runtime_mutex);
}

#define NEW_CLASS(super, sub) \
	createNSBlockSubclass(super, &sub, &sub ## Meta, #sub)

BOOL objc_create_block_classes_as_subclasses_of(Class super)
{
  if (_NSBlock.super_class != NULL) { return NO; }

  NEW_CLASS(super, _NSBlock);
  NEW_CLASS(&_NSBlock, _NSConcreteStackBlock);
  NEW_CLASS(&_NSBlock, _NSConcreteGlobalBlock);
  return YES;
}

#endif /* defined (__GNU_LIBOBJC__) */

