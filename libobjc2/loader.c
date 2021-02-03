#include <stdlib.h>
#include <assert.h>
#include "objc/runtime.h"
#include "objc/objc-auto.h"
#include "objc/objc-arc.h"
#include "lock.h"
#include "loader.h"
#include "visibility.h"
#include "legacy.h"
#ifdef ENABLE_GC
#include <gc/gc.h>
#endif
#include <stdio.h>
#include <string.h>

/**
 * Runtime lock.  This is exposed in 
 */
PRIVATE mutex_t runtime_mutex;
LEGACY void *__objc_runtime_mutex = &runtime_mutex;

void init_alias_table(void);
void init_arc(void);
void init_class_tables(void);
void init_dispatch_tables(void);
void init_gc(void);
void init_protocol_table(void);
void init_selector_tables(void);
void init_trampolines(void);
void objc_send_load_message(Class class);

void log_selector_memory_usage(void);

static void log_memory_stats(void)
{
	log_selector_memory_usage();
}

/* Number of threads that are alive.  */
int __objc_runtime_threads_alive = 1;			/* !T:MUTEX */

// libdispatch hooks for registering threads
__attribute__((weak)) void (*dispatch_begin_thread_4GC)(void);
__attribute__((weak)) void (*dispatch_end_thread_4GC)(void);
__attribute__((weak)) void *(*_dispatch_begin_NSAutoReleasePool)(void);
__attribute__((weak)) void (*_dispatch_end_NSAutoReleasePool)(void *);

__attribute__((used))
static void link_protos(void)
{
	link_protocol_classes();
}

static void init_runtime(void)
{
	static BOOL first_run = YES;
	if (first_run)
	{
#if ENABLE_GC
		init_gc();
#endif
		// Create the main runtime lock.  This is not safe in theory, but in
		// practice the first time that this function is called will be in the
		// loader, from the main thread.  Future loaders may run concurrently,
		// but that is likely to break the semantics of a lot of languages, so
		// we don't have to worry about it for a long time.
		//
		// The only case when this can potentially go badly wrong is when a
		// pure-C main() function spawns two threads which then, concurrently,
		// call dlopen() or equivalent, and the platform's implementation of
		// this does not perform any synchronization.
		INIT_LOCK(runtime_mutex);
		// Create the various tables that the runtime needs.
		init_selector_tables();
		init_protocol_table();
		init_class_tables();
		init_dispatch_tables();
		init_alias_table();
		init_arc();
		init_trampolines();
		first_run = NO;
		if (getenv("LIBOBJC_MEMORY_PROFILE"))
		{
			atexit(log_memory_stats);
		}
		if (dispatch_begin_thread_4GC != 0) {
			dispatch_begin_thread_4GC = objc_registerThreadWithCollector;
		}
		if (dispatch_end_thread_4GC != 0) {
			dispatch_end_thread_4GC = objc_unregisterThreadWithCollector;
		}
		if (_dispatch_begin_NSAutoReleasePool != 0) {
			_dispatch_begin_NSAutoReleasePool = objc_autoreleasePoolPush;
		}
		if (_dispatch_end_NSAutoReleasePool != 0) {
			_dispatch_end_NSAutoReleasePool = objc_autoreleasePoolPop;
		}
	}
}

/**
 * Structure for a class alias.
 */
struct objc_alias
{
	/**
	 * The name by which this class is referenced.
	 */
	const char *alias_name;
	/**
	 * A pointer to the indirection variable for this class.
	 */
	Class *alias;
};

/**
 * Type of the NSConstantString structure.
 */
struct nsstr
{
	/** Class pointer. */
	id isa;
	/**
	 * Flags.  Low 2 bits store the encoding:
	 * 0: ASCII
	 * 1: UTF-8
	 * 2: UTF-16
	 * 3: UTF-32
	 *
	 * Low 16 bits are reserved for the compiler, high 32 bits are reserved for
	 * the Foundation framework.
	 */
	uint32_t flags;
	/**
	 * Number of UTF-16 code units in the string.
	 */
	uint32_t length;
	/**
	 * Number of bytes in the string.
	 */
	uint32_t size;
	/**
	 * Hash (Foundation framework defines the hash algorithm).
	 */
	uint32_t hash;
	/**
	 * Character data.
	 */
	const char *data;
};

// begin: objc_init
struct objc_init
{
	uint64_t version;
	SEL sel_begin;
	SEL sel_end;
	Class *cls_begin;
	Class *cls_end;
	Class *cls_ref_begin;
	Class *cls_ref_end;
	struct objc_category *cat_begin;
	struct objc_category *cat_end;
	struct objc_protocol *proto_begin;
	struct objc_protocol *proto_end;
	struct objc_protocol **proto_ref_begin;
	struct objc_protocol **proto_ref_end;
	struct objc_alias *alias_begin;
	struct objc_alias *alias_end;
	struct nsstr *strings_begin;
	struct nsstr *strings_end;
};
// end: objc_init

#ifdef DEBUG_LOADING
#include <dlfcn.h>
#endif

static enum {
	LegacyABI,
	NewABI,
	UnknownABI
} CurrentABI = UnknownABI;

void registerProtocol(Protocol *proto);

OBJC_PUBLIC void __objc_load(struct objc_init *init)
{
	init_runtime();
#ifdef DEBUG_LOADING
	Dl_info info;
	if (dladdr(init, &info))
	{
		fprintf(stderr, "Loading %p from object: %s (%p)\n", init, info.dli_fname, __builtin_return_address(0));
	}
	else
	{
		fprintf(stderr, "Loading %p from unknown object\n", init);
	}
#endif
	LOCK_RUNTIME_FOR_SCOPE();
	BOOL isFirstLoad = NO;
	switch (CurrentABI)
	{
		case LegacyABI:
			fprintf(stderr, "Version 2 Objective-C ABI may not be mixed with earlier versions.\n");
			abort();
		case UnknownABI:
			isFirstLoad = YES;
			CurrentABI = NewABI;
			break;
		case NewABI:
			break;
	}

	// If we've already loaded this module, don't load it again.
	if (init->version == ULONG_MAX)
	{
		return;
	}

	assert(init->version == 0);
	assert((((uintptr_t)init->sel_end-(uintptr_t)init->sel_begin) % sizeof(*init->sel_begin)) == 0);
	assert((((uintptr_t)init->cls_end-(uintptr_t)init->cls_begin) % sizeof(*init->cls_begin)) == 0);
	assert((((uintptr_t)init->cat_end-(uintptr_t)init->cat_begin) % sizeof(*init->cat_begin)) == 0);
	for (SEL sel = init->sel_begin ; sel < init->sel_end ; sel++)
	{
		if (sel->name == 0)
		{
			continue;
		}
		objc_register_selector(sel);
	}
	for (struct objc_protocol *proto = init->proto_begin ; proto < init->proto_end ;
	     proto++)
	{
		if (proto->name == NULL)
		{
			continue;
		}
		registerProtocol((struct objc_protocol*)proto);
	}
	for (struct objc_protocol **proto = init->proto_ref_begin ; proto < init->proto_ref_end ;
	     proto++)
	{
		if (*proto == NULL)
		{
			continue;
		}
		struct objc_protocol *p = objc_getProtocol((*proto)->name);
		assert(p);
		*proto = p;
	}
	for (Class *cls = init->cls_begin ; cls < init->cls_end ; cls++)
	{
		if (*cls == NULL)
		{
			continue;
		}
		// As a special case, allow using legacy ABI code with a new runtime.
		if (isFirstLoad && (strcmp((*cls)->name, "Protocol") == 0))
		{
			CurrentABI = UnknownABI;
		}
#ifdef DEBUG_LOADING
		fprintf(stderr, "Loading class %s\n", (*cls)->name);
#endif
		objc_load_class(*cls);
	}
#if 0
	// We currently don't do anything with these pointers.  They exist to
	// provide a level of indirection that will permit us to completely change
	// the `objc_class` struct without breaking the ABI (again)
	for (Class *cls = init->cls_ref_begin ; cls < init->cls_ref_end ; cls++)
	{
	}
#endif
	for (struct objc_category *cat = init->cat_begin ; cat < init->cat_end ;
	     cat++)
	{
		if ((cat == NULL) || (cat->class_name == NULL))
		{
			continue;
		}
		objc_try_load_category(cat);
#ifdef DEBUG_LOADING
		fprintf(stderr, "Loading category %s (%s)\n", cat->class_name, cat->name);
#endif
	}
	// Load categories and statics that were deferred.
	objc_load_buffered_categories();
	// Fix up the class links for loaded classes.
	objc_resolve_class_links();
	for (struct objc_category *cat = init->cat_begin ; cat < init->cat_end ;
	     cat++)
	{
		Class class = (Class)objc_getClass(cat->class_name);
		if ((Nil != class) && 
		    objc_test_class_flag(class, objc_class_flag_resolved))
		{
			objc_send_load_message(class);
		}
	}
	// Register aliases
	for (struct objc_alias *alias = init->alias_begin ; alias < init->alias_end ;
	     alias++)
	{
		if (alias->alias_name)
		{
			class_registerAlias_np(*alias->alias, alias->alias_name);
		}
	}
#if 0
	// If future versions of the ABI need to do anything with constant strings,
	// they may do so here.
	for (struct nsstr *string = init->strings_begin ; string < init->strings_end ;
	     string++)
	{
		if (string->isa)
		{
		}
	}
#endif
	init->version = ULONG_MAX;
}

#ifdef OLDABI_COMPAT
OBJC_PUBLIC void __objc_exec_class(struct objc_module_abi_8 *module)
{
	init_runtime();

	switch (CurrentABI)
	{
		case UnknownABI:
			CurrentABI = LegacyABI;
			break;
		case LegacyABI:
			break;
		case NewABI:
			fprintf(stderr, "Version 2 Objective-C ABI may not be mixed with earlier versions.\n");
			abort();
	}

	// Check that this module uses an ABI version that we recognise.  
	// In future, we should pass the ABI version to the class / category load
	// functions so that we can change various structures more easily.
	assert(objc_check_abi_version(module));


	// The runtime mutex is held for the entire duration of a load.  It does
	// not need to be acquired or released in any of the called load functions.
	LOCK_RUNTIME_FOR_SCOPE();

	struct objc_symbol_table_abi_8 *symbols = module->symbol_table;
	// Register all of the selectors used in this module.
	if (symbols->selectors)
	{
		objc_register_selector_array(symbols->selectors,
				symbols->selector_count);
	}

	unsigned short defs = 0;
	// Load the classes from this module
	for (unsigned short i=0 ; i<symbols->class_count ; i++)
	{
		objc_load_class(objc_upgrade_class(symbols->definitions[defs++]));
	}
	unsigned int category_start = defs;
	// Load the categories from this module
	for (unsigned short i=0 ; i<symbols->category_count; i++)
	{
		objc_try_load_category(objc_upgrade_category(symbols->definitions[defs++]));
	}
	// Load the static instances
	struct objc_static_instance_list **statics = (void*)symbols->definitions[defs];
	while (NULL != statics && NULL != *statics)
	{
		objc_init_statics(*(statics++));
	}

	// Load categories and statics that were deferred.
	objc_load_buffered_categories();
	objc_init_buffered_statics();
	// Fix up the class links for loaded classes.
	objc_resolve_class_links();
	for (unsigned short i=0 ; i<symbols->category_count; i++)
	{
		struct objc_category *cat = (struct objc_category*)
			symbols->definitions[category_start++];
		Class class = (Class)objc_getClass(cat->class_name);
		if ((Nil != class) && 
		    objc_test_class_flag(class, objc_class_flag_resolved))
		{
			objc_send_load_message(class);
		}
	}
}
#endif
