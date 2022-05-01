#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include "dwarf_eh.h"
#include "objc/runtime.h"
#include "objc/hooks.h"
#include "class.h"
#include "objcxx_eh.h"

#ifndef DEBUG_EXCEPTIONS
#define DEBUG_LOG(...)
#else
#define DEBUG_LOG(str, ...) fprintf(stderr, str, ## __VA_ARGS__)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable abort
#endif

void test_cxx_eh_implementation();

// Weak references to C++ runtime functions.  We don't bother testing that
// these are 0 before calling them, because if they are not resolved then we
// should not be in a code path that involves a C++ exception.
__attribute__((weak)) void *__cxa_begin_catch(void *e);
__attribute__((weak)) void __cxa_end_catch(void);
__attribute__((weak)) void __cxa_rethrow(void);

#if defined(__RAVYNOS__)
typedef void NSUncaughtExceptionHandler(id exception);

typedef struct NSExceptionFrame {
	jmp_buf state;
	struct NSExceptionFrame *parent;
	id exception;
} NSExceptionFrame;
typedef struct NSExceptionFrame objc_exception_frame;

#endif 

/**
 * Class of exceptions to distinguish between this and other exception types.
 */
static const uint64_t objc_exception_class = EXCEPTION_CLASS('G','N','U','C','O','B','J','C');

/**
 * Structure used as a header on thrown exceptions.  
 */
struct objc_exception
{
	/** The selector value to be returned when installing the catch handler.
	 * Used at the call site to determine which catch() block should execute.
	 * This is found in phase 1 of unwinding then installed in phase 2.*/
	int handlerSwitchValue;
	/** The cached landing pad for the catch handler.*/
	void *landingPad;
	/**
	 * Next pointer for chained exceptions.  
	 */
	struct objc_exception *next;
	/**
	 * The number of nested catches that may hold this exception.  This is
	 * negative while an exception is being rethrown.
	 */
	int catch_count;
	/** The language-agnostic part of the exception header. */
	struct _Unwind_Exception unwindHeader;
	/** Thrown object.  This is after the unwind header so that the C++
	 * exception handler can catch this as a foreign exception. */
	id object;
	/** C++ exception structure.  Used for mixed exceptions.  When we are in
	 * Objective-C++ code, we create this structure for passing to the C++
	 * exception personality function.  It will then handle installing
	 * exceptions for us.  */
	struct _Unwind_Exception *cxx_exception;
};

struct objc_exception *objc_exception_from_header(struct _Unwind_Exception *ex)
{
	return (struct objc_exception*)((char*)ex -
			offsetof(struct objc_exception, unwindHeader));
}

typedef enum
{
	handler_none,
	handler_cleanup,
	handler_catchall_id,
	handler_catchall,
	handler_class
} handler_type;

enum exception_type
{
	NONE,
	CXX,
	OBJC,
	FOREIGN,
	BOXED_FOREIGN
};
struct thread_data
{
	enum exception_type current_exception_type;
	id lastThrownObject;
	BOOL cxxCaughtException;
	struct objc_exception *caughtExceptions;
#if defined(__RAVYNOS__)
	NSExceptionFrame *exception_frame;
	NSUncaughtExceptionHandler *uncaught_exception_handler;
#endif
};

static __thread struct thread_data thread_data;

static struct thread_data *get_thread_data(void)
{
	return &thread_data;
}

static struct thread_data *get_thread_data_fast(void)
{
	return &thread_data;
}


/**
 * Saves the result of the landing pad that we have found.  For ARM, this is
 * stored in the generic unwind structure, while on other platforms it is
 * stored in the Objective-C exception.
 */
static void saveLandingPad(struct _Unwind_Context *context,
                           struct _Unwind_Exception *ucb,
                           struct objc_exception *ex,
                           int selector,
                           dw_eh_ptr_t landingPad)
{
#if defined(__arm__) && !defined(__ARM_DWARF_EH__)
	// On ARM, we store the saved exception in the generic part of the structure
	ucb->barrier_cache.sp = _Unwind_GetGR(context, 13);
	ucb->barrier_cache.bitpattern[1] = (uint32_t)selector;
	ucb->barrier_cache.bitpattern[3] = (uint32_t)landingPad;
#else
	// Cache the results for the phase 2 unwind, if we found a handler
	// and this is not a foreign exception.  We can't cache foreign exceptions
	// because we don't know their structure (although we could cache C++
	// exceptions...)
	if (ex)
	{
		ex->handlerSwitchValue = selector;
		ex->landingPad = landingPad;
	}
#endif
}

/**
 * Loads the saved landing pad.  Returns 1 on success, 0 on failure.
 */
static int loadLandingPad(struct _Unwind_Context *context,
                          struct _Unwind_Exception *ucb,
                          struct objc_exception *ex,
                          unsigned long *selector,
                          dw_eh_ptr_t *landingPad)
{
#if defined(__arm__) && !defined(__ARM_DWARF_EH__)
	*selector = ucb->barrier_cache.bitpattern[1];
	*landingPad = (dw_eh_ptr_t)ucb->barrier_cache.bitpattern[3];
	return 1;
#else
	if (ex)
	{
		*selector = ex->handlerSwitchValue;
		*landingPad = ex->landingPad;
		return 0;
	}
	return 0;
#endif
}

static inline _Unwind_Reason_Code continueUnwinding(struct _Unwind_Exception *ex,
                                                    struct _Unwind_Context *context)
{
#if defined(__arm__) && !defined(__ARM_DWARF_EH__)
	if (__gnu_unwind_frame(ex, context) != _URC_OK) { return _URC_FAILURE; }
#endif
	return _URC_CONTINUE_UNWIND;
}

static void cleanup(_Unwind_Reason_Code reason, struct _Unwind_Exception *e)
{
	/*
  if (header->exceptionDestructor)
		  header->exceptionDestructor (e + 1);

	free((struct objc_exception*) ((char*)e - offsetof(struct objc_exception,
					unwindHeader)));
					*/
}

void objc_exception_rethrow(struct _Unwind_Exception *e);

/**
 * Throws an Objective-C exception.  This function is, unfortunately, used for
 * rethrowing caught exceptions too, even in @finally() blocks.  Unfortunately,
 * this means that we have some problems if the exception is boxed.
 */
void objc_exception_throw(id object)
{
	struct thread_data *td = get_thread_data();
	fprintf(stderr, "Throwing %p, in flight exception: %p\n", object, td->lastThrownObject);
	fprintf(stderr, "Exception caught by C++: %d\n", td->cxxCaughtException);
	// If C++ caught the exception, then we may need to make C++ rethrow it if
	// we want to preserve exception state.  Rethrows should be handled with
	// objc_exception_rethrow, but clang appears to do the wrong thing for some
	// cases.
	if (td->cxxCaughtException)
	{
		// For catchalls, we may result in our being passed the pointer to the
		// object, not the object.
		if ((object == td->lastThrownObject) ||
			((object != nil) &&
			 !isSmallObject(object) &&
			 (*(id*)object == td->lastThrownObject)))
		{
			__cxa_rethrow();
		}
	}

	SEL rethrow_sel = sel_registerName("rethrow");
	if ((nil != object) &&
	    (class_respondsToSelector(classForObject(object), rethrow_sel)))
	{
		DEBUG_LOG("Rethrowing\n");
		IMP rethrow = objc_msg_lookup(object, rethrow_sel);
		rethrow(object, rethrow_sel);
		// Should not be reached!  If it is, then the rethrow method actually
		// didn't, so we throw it normally.
	}

	DEBUG_LOG("Throwing %p\n", object);

	struct objc_exception *ex = calloc(1, sizeof(struct objc_exception));

	ex->unwindHeader.exception_class = objc_exception_class;
	ex->unwindHeader.exception_cleanup = cleanup;

	ex->object = object;

	td->lastThrownObject = object;
	td->cxxCaughtException = NO;

	_Unwind_Reason_Code err = _Unwind_RaiseException(&ex->unwindHeader);
	free(ex);
	if (_URC_END_OF_STACK == err && 0 != _objc_unexpected_exception)
	{
		_objc_unexpected_exception(object);
	}
	DEBUG_LOG("Throw returned %d\n",(int) err);
	abort();
}

static Class get_type_table_entry(struct _Unwind_Context *context,
                                  struct dwarf_eh_lsda *lsda,
                                  int filter)
{
	dw_eh_ptr_t record = lsda->type_table -
		dwarf_size_of_fixed_size_field(lsda->type_table_encoding)*filter;
	dw_eh_ptr_t start = record;
	int64_t offset = read_value(lsda->type_table_encoding, &record);

	if (0 == offset) { return Nil; }

	// ...so we need to resolve it
	char *class_name = (char*)(intptr_t)resolve_indirect_value(context,
			lsda->type_table_encoding, offset, start);

	if (0 == class_name) { return Nil; }

	DEBUG_LOG("Class name: %s\n", class_name);

	if (strcmp("@id", class_name) == 0) { return (Class)1; }

	return (Class)objc_getClass(class_name);
}

static BOOL isKindOfClass(Class thrown, Class type)
{
	do
	{
		if (thrown == type)
		{
			return YES;
		}
		thrown = class_getSuperclass(thrown);
	} while (Nil != thrown);

	return NO;
}


static handler_type check_action_record(struct _Unwind_Context *context,
                                        BOOL foreignException,
                                        struct dwarf_eh_lsda *lsda,
                                        dw_eh_ptr_t action_record,
                                        Class thrown_class,
                                        unsigned long *selector)
{
	if (!action_record) { return handler_cleanup; }
	while (action_record)
	{
		int filter = read_sleb128(&action_record);
		dw_eh_ptr_t action_record_offset_base = action_record;
		int displacement = read_sleb128(&action_record);
		*selector = filter;
		DEBUG_LOG("Filter: %d\n", filter);
		if (filter > 0)
		{
			Class type = get_type_table_entry(context, lsda, filter);
			DEBUG_LOG("%p type: %d\n", type, !foreignException);
			// Catchall
			if (Nil == type)
			{
				return handler_catchall;
			}
			// We treat id catches as catchalls when an object is thrown and as
			// nothing when a foreign exception is thrown
			else if ((Class)1 == type)
			{
				DEBUG_LOG("Found id catch\n");
				if (!foreignException)
				{
					return handler_catchall_id;
				}
			}
			else if (!foreignException && isKindOfClass(thrown_class, type))
			{
				DEBUG_LOG("found handler for %s\n", type->name);
				return handler_class;
			}
			else if (thrown_class == type)
			{
				return handler_class;
			}
		}
		else if (filter == 0)
		{
			DEBUG_LOG("0 filter\n");
			// Cleanup?  I think the GNU ABI doesn't actually use this, but it
			// would be a good way of indicating a non-id catchall...
			return handler_cleanup;
		}
		else
		{
			DEBUG_LOG("Filter value: %d\n"
					"Your compiler and I disagree on the correct layout of EH data.\n", 
					filter);
			abort();
		}
		*selector = 0;
		action_record = displacement ? 
			action_record_offset_base + displacement : 0;
	}
	return handler_none;
}

/**
 * The Objective-C exception personality function implementation.  This is
 * shared by the GCC-compatible and the new implementation.
 *
 * The key difference is that the new implementation always returns the
 * exception object and boxes it.
 */
static inline _Unwind_Reason_Code internal_objc_personality(int version,
                                                            _Unwind_Action actions,
                                                            uint64_t exceptionClass,
                                                            struct _Unwind_Exception *exceptionObject,
                                                            struct _Unwind_Context *context,
                                                            BOOL isNew)
{
	DEBUG_LOG("%s personality function called %p\n", isNew ? "New" : "Old", exceptionObject);
	
	// This personality function is for version 1 of the ABI.  If you use it
	// with a future version of the ABI, it won't know what to do, so it
	// reports a fatal error and give up before it breaks anything.
	if (1 != version)
	{
		return _URC_FATAL_PHASE1_ERROR;
	}
	struct objc_exception *ex = 0;
#ifdef DEBUG_EXCEPTIONS
	char *cls = (char*)&exceptionClass;
#endif
	DEBUG_LOG("Class: %c%c%c%c%c%c%c%c\n", cls[7], cls[6], cls[5], cls[4], cls[3], cls[2], cls[1], cls[0]);

	// Check if this is a foreign exception.  If it is a C++ exception, then we
	// have to box it.  If it's something else, like a LanguageKit exception
	// then we ignore it (for now)
	BOOL foreignException = exceptionClass != objc_exception_class;
	// Is this a C++ exception containing an Objective-C++ object?
	BOOL objcxxException = NO;
	// The object to return
	void *object = NULL;

#ifndef NO_OBJCXX
	if (cxx_exception_class == 0)
	{
		test_cxx_eh_implementation();
	}

	if (exceptionClass == cxx_exception_class)
	{
		int objcxx;
		id obj = objc_object_for_cxx_exception(exceptionObject, &objcxx);
		objcxxException = objcxx;
		if (objcxxException)
		{
			object = obj;
			DEBUG_LOG("ObjC++ object exception %p\n", object);
			// This is a foreign exception, buy for the purposes of exception
			// matching, we pretend that it isn't.
			foreignException = NO;
		}
	}
#endif

	Class thrown_class = Nil;

	if (objcxxException)
	{
		thrown_class = (object == 0) ? Nil : classForObject((id)object);
	}
	// If it's not a foreign exception, then we know the layout of the
	// language-specific exception stuff.
	else if (!foreignException)
	{
		ex = objc_exception_from_header(exceptionObject);
		if (ex->object != nil)
	        {
			thrown_class = classForObject(ex->object);
		}
	}
	else if (_objc_class_for_boxing_foreign_exception)
	{
		thrown_class = _objc_class_for_boxing_foreign_exception(exceptionClass);
		DEBUG_LOG("Foreign class: %p\n", thrown_class);
	}
	unsigned char *lsda_addr = (void*)_Unwind_GetLanguageSpecificData(context);
	DEBUG_LOG("LSDA: %p\n", lsda_addr);

	// No LSDA implies no landing pads - try the next frame
	if (0 == lsda_addr)
	{
		return continueUnwinding(exceptionObject, context);
	}

	// These two variables define how the exception will be handled.
	struct dwarf_eh_action action = {0};
	unsigned long selector = 0;
	
	if (actions & _UA_SEARCH_PHASE)
	{
		DEBUG_LOG("Search phase...\n");
		struct dwarf_eh_lsda lsda = parse_lsda(context, lsda_addr);
		action = dwarf_eh_find_callsite(context, &lsda);
		handler_type handler = check_action_record(context, foreignException,
				&lsda, action.action_record, thrown_class, &selector);
		DEBUG_LOG("handler: %d\n", handler);
		// If there's no action record, we've only found a cleanup, so keep
		// searching for something real
		if (handler == handler_class ||
		   ((handler == handler_catchall_id) && !foreignException) ||
			(handler == handler_catchall))
		{
			saveLandingPad(context, exceptionObject, ex, selector, action.landing_pad);
			DEBUG_LOG("Found handler! %d\n", handler);
			return _URC_HANDLER_FOUND;
		}
		return continueUnwinding(exceptionObject, context);
	}
	DEBUG_LOG("Phase 2: Fight!\n");

	// TODO: If this is a C++ exception, we can cache the lookup and cheat a
	// bit
	if (!(actions & _UA_HANDLER_FRAME))
	{
		DEBUG_LOG("Not the handler frame, looking up the cleanup again\n");
		struct dwarf_eh_lsda lsda = parse_lsda(context, lsda_addr);
		action = dwarf_eh_find_callsite(context, &lsda);
		// If there's no cleanup here, continue unwinding.
		if (0 == action.landing_pad)
		{
			return continueUnwinding(exceptionObject, context);
		}
		handler_type handler = check_action_record(context, foreignException,
				&lsda, action.action_record, thrown_class, &selector);
		DEBUG_LOG("handler! %d %d\n", (int)handler,  (int)selector);
		// On ARM, we occasionally get called to install a handler without
		// phase 1 running (no idea why, I suspect a bug in the generic
		// unwinder), so skip this check.
#if !(defined(__arm__) && !defined(__ARM_DWARF_EH__))
		// If this is not a cleanup, ignore it and keep unwinding.
		if ((handler != handler_cleanup) && !objcxxException)
		{
			DEBUG_LOG("Ignoring handler! %d\n",handler);
			return continueUnwinding(exceptionObject, context);
		}
#endif
		DEBUG_LOG("Installing cleanup...\n");
		// If there is a cleanup, we need to return the exception structure
		// (not the object) to the calling frame.  The exception object
		object = exceptionObject;
	}
	else if (foreignException || objcxxException)
	{
		struct dwarf_eh_lsda lsda = parse_lsda(context, lsda_addr);
		action = dwarf_eh_find_callsite(context, &lsda);
		check_action_record(context, foreignException, &lsda,
				action.action_record, thrown_class, &selector);
		// If it's a foreign exception, then box it.  If it's an Objective-C++
		// exception, then we need to delete the exception object.
		if (foreignException)
		{
			DEBUG_LOG("Doing the foreign exception thing...\n");
			//[thrown_class exceptionWithForeignException: exceptionObject];
			SEL box_sel = sel_registerName("exceptionWithForeignException:");
			IMP boxfunction = objc_msg_lookup((id)thrown_class, box_sel);
			if (!isNew)
			{
				object = boxfunction((id)thrown_class, box_sel, exceptionObject);
				DEBUG_LOG("Boxed as %p\n", object);
			}
		}
		else if (!isNew) // ObjCXX exception
		{
			_Unwind_DeleteException(exceptionObject);
		}
		// In the new EH ABI, we call objc_begin_catch() / and
		// objc_end_catch(), which will wrap their __cxa* versions.
	}
	else
	{
		// Restore the saved info if we saved some last time.
		loadLandingPad(context, exceptionObject, ex, &selector, &action.landing_pad);
		object = ex->object;
		if (!isNew)
		{
			free(ex);
		}
	}

	_Unwind_SetIP(context, (unsigned long)action.landing_pad);
	_Unwind_SetGR(context, __builtin_eh_return_data_regno(0), 
			(unsigned long)(isNew ? exceptionObject : object));
	_Unwind_SetGR(context, __builtin_eh_return_data_regno(1), selector);

	DEBUG_LOG("Installing context, selector %d\n", (int)selector);
	get_thread_data()->cxxCaughtException = NO;
	return _URC_INSTALL_CONTEXT;
}

BEGIN_PERSONALITY_FUNCTION(__gnu_objc_personality_v0)
	return internal_objc_personality(version, actions, exceptionClass,
			exceptionObject, context, NO);
}
BEGIN_PERSONALITY_FUNCTION(__gnustep_objc_personality_v0)
	return internal_objc_personality(version, actions, exceptionClass,
			exceptionObject, context, YES);
}

BEGIN_PERSONALITY_FUNCTION(__gnustep_objcxx_personality_v0)
#ifndef NO_OBJCXX
	if (cxx_exception_class == 0)
	{
		test_cxx_eh_implementation();
	}
	if (exceptionClass == objc_exception_class)
	{
		struct objc_exception *ex = objc_exception_from_header(exceptionObject);
		if (0 == ex->cxx_exception)
		{
			ex->cxx_exception = objc_init_cxx_exception(ex->object);
		}
		// We now have two copies of the _Unwind_Exception object (which stores
		// state for the unwinder) in flight.  Make sure that they're in sync.
		COPY_EXCEPTION(ex->cxx_exception, exceptionObject)
		exceptionObject = ex->cxx_exception;
		exceptionClass = cxx_exception_class;
		int ret = CALL_PERSONALITY_FUNCTION(__gxx_personality_v0);
		COPY_EXCEPTION(exceptionObject, ex->cxx_exception)
		if (ret == _URC_INSTALL_CONTEXT)
		{
			get_thread_data()->cxxCaughtException = YES;
		}
		return ret;
	}
#endif
	return CALL_PERSONALITY_FUNCTION(__gxx_personality_v0);
}

id objc_begin_catch(struct _Unwind_Exception *exceptionObject)
{
	struct thread_data *td = get_thread_data();
	DEBUG_LOG("Beginning catch %p\n", exceptionObject);
	td->cxxCaughtException = NO;
	if (exceptionObject->exception_class == objc_exception_class)
	{
		td->current_exception_type = OBJC;
		struct objc_exception *ex = objc_exception_from_header(exceptionObject);
		if (ex->catch_count == 0)
		{
			// If this is the first catch, add it to the list.
			ex->catch_count = 1;
			ex->next = td->caughtExceptions;
			td->caughtExceptions = ex;
		}
		else if (ex->catch_count < 0)
		{
			// If this is being thrown, mark it as caught again and increment
			// the refcount
			ex->catch_count = -ex->catch_count + 1;
		}
		else
		{
			// Otherwise, just increment the catch count
			ex->catch_count++;
		}
		DEBUG_LOG("objc catch\n");
		return ex->object;
	}
	// If we have a foreign exception while we have stacked exceptions, we have
	// a problem.  We can't chain them, so we follow the example of C++ and
	// just abort.
	if (td->caughtExceptions != 0)
	{
		// FIXME: Actually, we can handle a C++ exception if only ObjC
		// exceptions are in-flight
		abort();
	}
#ifndef NO_OBJCXX
	// If this is a C++ exception, let the C++ runtime handle it.
	if (exceptionObject->exception_class == cxx_exception_class)
	{
		DEBUG_LOG("c++ catch\n");
		td->current_exception_type = CXX;
		return __cxa_begin_catch(exceptionObject);
	}
#endif
	DEBUG_LOG("foreign exception catch\n");
	// Box if we have a boxing function.
	if (_objc_class_for_boxing_foreign_exception)
	{
		Class thrown_class =
			_objc_class_for_boxing_foreign_exception(exceptionObject->exception_class);
		SEL box_sel = sel_registerName("exceptionWithForeignException:");
		id(*boxfunction)(Class,SEL,struct _Unwind_Exception*) = 
			(id(*)(Class,SEL,struct _Unwind_Exception*))objc_msg_lookup((id)thrown_class, box_sel);
		if (boxfunction != 0)
		{
			id boxed = boxfunction(thrown_class, box_sel, exceptionObject);
			td->caughtExceptions = (struct objc_exception*)boxed;
			td->current_exception_type = BOXED_FOREIGN;
			return boxed;
		}
	}
	td->current_exception_type = FOREIGN;
	td->caughtExceptions = (struct objc_exception*)exceptionObject;
	// If this is some other kind of exception, then assume that the value is
	// at the end of the exception header.
	return (id)((char*)exceptionObject + sizeof(struct _Unwind_Exception));
}

void objc_end_catch(void)
{
	struct thread_data *td = get_thread_data_fast();
	// If this is a boxed foreign exception then the boxing class is
	// responsible for cleaning it up
	if (td->current_exception_type == BOXED_FOREIGN)
	{
		td->caughtExceptions = 0;
		td->current_exception_type = NONE;
		return;
	}
	DEBUG_LOG("Ending catch\n");
	// If this is a C++ exception, then just let the C++ runtime handle it.
	if (td->current_exception_type == CXX)
	{
		__cxa_end_catch();
		td->current_exception_type = OBJC;
		return;
	}
	if (td->current_exception_type == FOREIGN)
	{
		struct _Unwind_Exception *e = ((struct _Unwind_Exception*)td->caughtExceptions);
		e->exception_cleanup(_URC_FOREIGN_EXCEPTION_CAUGHT, e);
		td->current_exception_type = NONE;
		td->caughtExceptions = 0;
		return;
	}
	// Otherwise we should do the cleanup thing.  Nested catches are possible,
	// so we only clean up the exception if this is the last reference.
	assert(td->caughtExceptions != 0);
	struct objc_exception *ex = td->caughtExceptions;
	// If this is being rethrown decrement its (negated) catch count, but don't
	// delete it even if its catch count would be 0.
	if (ex->catch_count < 0)
	{
		ex->catch_count++;
		return;
	}
	ex->catch_count--;
	if (ex->catch_count == 0)
	{
		td->caughtExceptions = ex->next;
		free(ex);
	}
}

void objc_exception_rethrow(struct _Unwind_Exception *e)
{
	struct thread_data *td = get_thread_data_fast();
	// If this is an Objective-C exception, then 
	if (td->current_exception_type == OBJC)
	{
		struct objc_exception *ex = objc_exception_from_header(e);
		assert(e->exception_class == objc_exception_class);
		assert(ex == td->caughtExceptions);
		assert(ex->catch_count > 0);
		// Negate the catch count, so that we can detect that this is a
		// rethrown exception in objc_end_catch
		ex->catch_count = -ex->catch_count;
		_Unwind_Reason_Code err = _Unwind_Resume_or_Rethrow(e);
		free(ex);
		if (_URC_END_OF_STACK == err && 0 != _objc_unexpected_exception)
		{
			_objc_unexpected_exception(ex->object);
		}
		abort();
	}
#ifndef NO_OBJCXX
	else if (td->current_exception_type == CXX)
	{
		assert(e->exception_class == cxx_exception_class);
		__cxa_rethrow();
	}
#endif
	if (td->current_exception_type == BOXED_FOREIGN)
	{
		SEL rethrow_sel = sel_registerName("rethrow");
		id object = (id)td->caughtExceptions;
		if ((nil != object) &&
		    (class_respondsToSelector(classForObject(object), rethrow_sel)))
		{
			DEBUG_LOG("Rethrowing boxed exception\n");
			IMP rethrow = objc_msg_lookup(object, rethrow_sel);
			rethrow(object, rethrow_sel);
		}
	}
	assert(e == (struct _Unwind_Exception*)td->caughtExceptions);
	_Unwind_Resume_or_Rethrow(e);
	abort();
}

#if defined(__RAVYNOS__)
// Emulate the cocotron runtime's exception handling

static NSUncaughtExceptionHandler *uncaughtExceptionHandler = NULL;

OBJC_PUBLIC NSUncaughtExceptionHandler *NSGetUncaughtExceptionHandler(void) {
	return uncaughtExceptionHandler;
}

OBJC_PUBLIC void NSSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *proc) {
	uncaughtExceptionHandler = proc;
}

OBJC_PUBLIC NSExceptionFrame *NSThreadCurrentHandler(void) {
	struct thread_data *td = get_thread_data_fast();
	return td->exception_frame;
}

OBJC_PUBLIC void NSThreadSetCurrentHandler(NSExceptionFrame *handler) {
	struct thread_data *td = get_thread_data_fast();
	td->exception_frame = handler;
}

OBJC_PUBLIC NSUncaughtExceptionHandler *NSThreadUncaughtExceptionHandler(void) {
	struct thread_data *td = get_thread_data_fast();
	return td->uncaught_exception_handler;
}

OBJC_PUBLIC void NSThreadSetUncaughtExceptionHandler(NSUncaughtExceptionHandler *function) {
	struct thread_data *td = get_thread_data_fast();
	td->uncaught_exception_handler = function;
}

OBJC_PUBLIC void __NSPushExceptionFrame(NSExceptionFrame *frame) {
	frame->parent = NSThreadCurrentHandler();
	frame->exception = nil;
	NSThreadSetCurrentHandler(frame);
}

OBJC_PUBLIC void __NSPopExceptionFrame(NSExceptionFrame *frame) {
	NSThreadSetCurrentHandler(frame->parent);
}

static void defaultHandler(id exception) {
	__builtin_trap();
	fprintf(stderr, "*** Uncaught exception\n");
}

OBJC_PUBLIC int objc_exception_match(Class exceptionClass, id exception) {
	return isKindOfClass(object_getClass(exception), exceptionClass);
}

OBJC_PUBLIC id _NSRaiseException(id exception) {
	NSExceptionFrame *top = NSThreadCurrentHandler();
	if(top == NULL) {
		NSUncaughtExceptionHandler *proc = NSGetUncaughtExceptionHandler();
		if(proc == NULL) {
			defaultHandler(exception);
		} else {
			proc(exception);
		}
	} else {
		NSThreadSetCurrentHandler(top->parent);
		top->exception = exception;
		longjmp(top->state, 1);
	}
}

void objc_exception_try_enter(void *exceptionFrame) {
	__NSPushExceptionFrame((NSExceptionFrame *)exceptionFrame);
}

void objc_exception_try_exit(void *exceptionFrame) {
	__NSPopExceptionFrame((NSExceptionFrame *)exceptionFrame);
}

id objc_exception_extract(void *exceptionFrame) {
	NSExceptionFrame *frame = (NSExceptionFrame *)exceptionFrame;
	return (id)frame->exception;
}

#endif


