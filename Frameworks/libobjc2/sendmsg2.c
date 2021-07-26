#include "objc/runtime.h"
#include "lock.h"
#include "dtable.h"
#include "selector.h"
#include "loader.h"
#include "objc/hooks.h"
#include <stdint.h>
#include <stdio.h>

void objc_send_initialize(id object);

static long long nil_method(id self, SEL _cmd) { return 0; }
static long double nil_method_D(id self, SEL _cmd) { return 0; }
static double nil_method_d(id self, SEL _cmd) { return 0; }
static float nil_method_f(id self, SEL _cmd) { return 0; }

static struct objc_slot nil_slot_v1 = { Nil, Nil, 0, 1, (IMP)nil_method };
static struct objc_slot nil_slot_D_v1 = { Nil, Nil, 0, 1, (IMP)nil_method_D };
static struct objc_slot nil_slot_d_v1 = { Nil, Nil, 0, 1, (IMP)nil_method_d };
static struct objc_slot nil_slot_f_v1 = { Nil, Nil, 0, 1, (IMP)nil_method_f };

static struct objc_method nil_slot = { (IMP)nil_method, NULL, NULL  };
static struct objc_method nil_slot_D = { (IMP)nil_method_D, NULL, NULL };
static struct objc_method nil_slot_d = { (IMP)nil_method_d, NULL, NULL };
static struct objc_method nil_slot_f = { (IMP)nil_method_f, NULL, NULL };

static struct objc_slot2* objc_slot_lookup(id *receiver, SEL selector);

// Default implementations of the two new hooks.  Return NULL.
static id objc_proxy_lookup_null(id receiver, SEL op) { return nil; }
static struct objc_slot *objc_msg_forward3_null(id receiver, SEL op) { return &nil_slot_v1; }

id (*objc_proxy_lookup)(id receiver, SEL op) = objc_proxy_lookup_null;
struct objc_slot *(*__objc_msg_forward3)(id receiver, SEL op) = objc_msg_forward3_null;

static IMP forward2(id self, SEL _cmd)
{
	return __objc_msg_forward3(self, _cmd)->method;
}
IMP (*__objc_msg_forward2)(id, SEL) = forward2;

__thread struct objc_method uncacheable_slot = { (IMP)nil_method, NULL, NULL };
__thread struct objc_slot uncacheable_slot_v1 = { Nil, Nil, 0, 0, (IMP)nil_method };

#ifndef NO_SELECTOR_MISMATCH_WARNINGS
static IMP objc_selector_type_mismatch(Class cls, SEL
		selector, struct objc_slot2 *result)
{
	fprintf(stderr, "Calling [%s %c%s] with incorrect signature.  "
			"Method has %s (%s), selector has %s\n",
			cls->name,
			class_isMetaClass(cls) ? '+' : '-',
			sel_getName(selector),
			sel_getType_np(((struct objc_method*)result)->selector),
			((struct objc_method*)result)->types,
			sel_getType_np(selector));
	return result->method;
}
#else
static IMP objc_selector_type_mismatch(Class cls, SEL
		selector, struct objc_slot2 *result)
{
	return result->method;
}
#endif

IMP (*_objc_selector_type_mismatch2)(Class cls, SEL
		selector, struct objc_slot2 *result) = objc_selector_type_mismatch;
struct objc_slot *(*_objc_selector_type_mismatch)(Class cls, SEL
		selector, struct objc_slot *result);

static IMP call_mismatch_hook(Class cls, SEL sel, struct objc_slot2 *slot)
{
	if (_objc_selector_type_mismatch &&
	    (!_objc_selector_type_mismatch2 ||
	     (_objc_selector_type_mismatch2 == objc_selector_type_mismatch)))
	{
		struct objc_slot fwdslot;
		fwdslot.types = ((struct objc_method*)slot)->types;
		fwdslot.selector = sel;
		fwdslot.method = slot->method;
		struct objc_slot *slot_v1 = _objc_selector_type_mismatch(cls, sel, &uncacheable_slot_v1);
		return slot_v1->method;
	}
	return _objc_selector_type_mismatch2(cls, sel, slot);
}

static
// Uncomment for debugging
//__attribute__((noinline))
__attribute__((always_inline))
struct objc_slot2 *objc_msg_lookup_internal(id *receiver, SEL selector, uint64_t *version)
{
	if (version)
	{
		*version = objc_method_cache_version;
	}
	Class class = classForObject((*receiver));
retry:;
	struct objc_slot2 * result = objc_dtable_lookup(class->dtable, selector->index);
	if (UNLIKELY(0 == result))
	{
		dtable_t dtable = dtable_for_class(class);
		/* Install the dtable if it hasn't already been initialized. */
		if (dtable == uninstalled_dtable)
		{
			objc_send_initialize(*receiver);
			dtable = dtable_for_class(class);
			result = objc_dtable_lookup(dtable, selector->index);
		}
		else
		{
			// Check again incase another thread updated the dtable while we
			// weren't looking
			result = objc_dtable_lookup(dtable, selector->index);
		}
		if (0 == result)
		{
			if (!isSelRegistered(selector))
			{
				objc_register_selector(selector);
				// This should be a tail call, but GCC is stupid and won't let
				// us tail call an always_inline function.
				goto retry;
			}
			if ((result = objc_dtable_lookup(dtable, get_untyped_idx(selector))))
			{
				if (version)
				{
					*version = 0;
				}
				uncacheable_slot.imp = call_mismatch_hook(class, selector, result);
				result = (struct objc_slot2*)&uncacheable_slot;
			}
			id newReceiver = objc_proxy_lookup(*receiver, selector);
			// If some other library wants us to play forwarding games, try
			// again with the new object.
			if (nil != newReceiver)
			{
				*receiver = newReceiver;
				return objc_slot_lookup(receiver, selector);
			}
			if (0 == result)
			{
				if (version)
				{
					*version = 0;
				}
				uncacheable_slot.imp = __objc_msg_forward2(*receiver, selector);
				result = (struct objc_slot2*)&uncacheable_slot;
			}
		}
	}
	return result;
}

PRIVATE IMP slowMsgLookup(id *receiver, SEL cmd)
{
	// By the time we've got here, the assembly version of this function has
	// already done the nil checks.
	return objc_msg_lookup_internal(receiver, cmd, NULL)->method;
}

PRIVATE void logInt(void *a)
{
	fprintf(stderr, "Value: %p\n", a);
}

/**
 * New Objective-C lookup function.  This permits the lookup to modify the
 * receiver and also supports multi-dimensional dispatch based on the sender.
 */
struct objc_slot *objc_msg_lookup_sender(id *receiver, SEL selector, id sender)
{
	// Returning a nil slot allows the caller to cache the lookup for nil too,
	// although this is not particularly useful because the nil method can be
	// inlined trivially.
	if (UNLIKELY(*receiver == nil))
	{
		// Return the correct kind of zero, depending on the type encoding.
		if (selector->types)
		{
			const char *t = selector->types;
			// Skip type qualifiers
			while ('r' == *t || 'n' == *t || 'N' == *t || 'o' == *t ||
			       'O' == *t || 'R' == *t || 'V' == *t || 'A' == *t)
			{
				t++;
			}
			switch (selector->types[0])
			{
				case 'D': return &nil_slot_D_v1;
				case 'd': return &nil_slot_d_v1;
				case 'f': return &nil_slot_f_v1;
			}
		}
		return &nil_slot_v1;
	}

	struct objc_slot2 *slot = objc_msg_lookup_internal(receiver, selector, NULL);
	uncacheable_slot_v1.owner = Nil;
	uncacheable_slot_v1.types = sel_getType_np(((struct objc_method*)slot)->selector);
	uncacheable_slot_v1.selector = selector;
	uncacheable_slot_v1.method = slot->method;
	return &uncacheable_slot_v1;
}


static struct objc_slot2* objc_slot_lookup(id *receiver, SEL selector)
{
	// Returning a nil slot allows the caller to cache the lookup for nil too,
	// although this is not particularly useful because the nil method can be
	// inlined trivially.
	if (UNLIKELY(*receiver == nil))
	{
		// Return the correct kind of zero, depending on the type encoding.
		if (selector->types)
		{
			const char *t = selector->types;
			// Skip type qualifiers
			while ('r' == *t || 'n' == *t || 'N' == *t || 'o' == *t ||
			       'O' == *t || 'R' == *t || 'V' == *t || 'A' == *t)
			{
				t++;
			}
			switch (selector->types[0])
			{
				case 'D': return (struct objc_slot2*)&nil_slot_D;
				case 'd': return (struct objc_slot2*)&nil_slot_d;
				case 'f': return (struct objc_slot2*)&nil_slot_f;
			}
		}
		return (struct objc_slot2*)&nil_slot;
	}

	return objc_msg_lookup_internal(receiver, selector, NULL);
}

struct objc_slot2 *objc_slot_lookup_version(id *receiver, SEL selector, uint64_t *version)
{
	// Returning a nil slot allows the caller to cache the lookup for nil too,
	// although this is not particularly useful because the nil method can be
	// inlined trivially.
	if (UNLIKELY(*receiver == nil))
	{
		if (version)
		{
			*version = 0;
		}
		// Return the correct kind of zero, depending on the type encoding.
		if (selector->types)
		{
			const char *t = selector->types;
			// Skip type qualifiers
			while ('r' == *t || 'n' == *t || 'N' == *t || 'o' == *t ||
			       'O' == *t || 'R' == *t || 'V' == *t || 'A' == *t)
			{
				t++;
			}
			switch (selector->types[0])
			{
				case 'D': return (struct objc_slot2*)&nil_slot_D;
				case 'd': return (struct objc_slot2*)&nil_slot_d;
				case 'f': return (struct objc_slot2*)&nil_slot_f;
			}
		}
		return (struct objc_slot2*)&nil_slot;
	}

	return objc_msg_lookup_internal(receiver, selector, version);
}

IMP objc_msg_lookup2(id *receiver, SEL selector)
{
	return objc_slot_lookup(receiver, selector)->method;
}


struct objc_slot2 *objc_slot_lookup_super2(struct objc_super *super, SEL selector)
{
	id receiver = super->receiver;
	if (receiver)
	{
		Class class = super->class;
		struct objc_slot2 * result = objc_dtable_lookup(dtable_for_class(class),
				selector->index);
		if (0 == result)
		{
			Class class = classForObject(receiver);
			// Dtable should always be installed in the superclass in
			// Objective-C, but may not be for other languages (Python).
			if (dtable_for_class(class) == uninstalled_dtable)
			{
				if (class_isMetaClass(class))
				{
					objc_send_initialize(receiver);
				}
				else
				{
					objc_send_initialize((id)class);
				}
				objc_send_initialize((id)class);
				return objc_slot_lookup_super2(super, selector);
			}
			uncacheable_slot.imp = __objc_msg_forward2(receiver, selector);
			return (struct objc_slot2*)&uncacheable_slot;
		}
		return result;
	}
	return (struct objc_slot2*)&nil_slot;
}

struct objc_slot *objc_slot_lookup_super(struct objc_super *super, SEL selector)
{
	id receiver = super->receiver;
	if (receiver)
	{
		Class class = super->class;
		struct objc_slot2 * result = objc_dtable_lookup(dtable_for_class(class),
				selector->index);
		if (0 == result)
		{
			Class class = classForObject(receiver);
			// Dtable should always be installed in the superclass in
			// Objective-C, but may not be for other languages (Python).
			if (dtable_for_class(class) == uninstalled_dtable)
			{
				if (class_isMetaClass(class))
				{
					objc_send_initialize(receiver);
				}
				else
				{
					objc_send_initialize((id)class);
				}
				objc_send_initialize((id)class);
				return objc_slot_lookup_super(super, selector);
			}
			uncacheable_slot_v1.owner = Nil;
			uncacheable_slot_v1.types = sel_getType_np(selector);
			uncacheable_slot_v1.selector = selector;
			uncacheable_slot_v1.method = __objc_msg_forward2(receiver, selector);
			return &uncacheable_slot_v1;
		}
		uncacheable_slot_v1.owner = Nil;
		uncacheable_slot_v1.types = sel_getType_np(((struct objc_method*)result)->selector);
		uncacheable_slot_v1.selector = selector;
		uncacheable_slot_v1.method = result->method;
		return &uncacheable_slot_v1;
	}
	return &nil_slot_v1;
}

/**
 * looks up a slot without invoking any forwarding mechanisms
 */
struct objc_slot2 *objc_get_slot2(Class cls, SEL selector, uint64_t *version)
{
	if (version)
	{
		*version = objc_method_cache_version;
	}
	struct objc_slot2 * result = objc_dtable_lookup(cls->dtable, selector->index);
	if (0 == result)
	{
		void *dtable = dtable_for_class(cls);
		/* Install the dtable if it hasn't already been initialized. */
		if (dtable == uninstalled_dtable)
		{
			dtable = dtable_for_class(cls);
			result = objc_dtable_lookup(dtable, selector->index);
		}
		else
		{
			// Check again incase another thread updated the dtable while we
			// weren't looking
			result = objc_dtable_lookup(dtable, selector->index);
		}
		if (NULL == result)
		{
			if (!isSelRegistered(selector))
			{
				objc_register_selector(selector);
				return objc_get_slot2(cls, selector, version);
			}
			if ((result = objc_dtable_lookup(dtable, get_untyped_idx(selector))))
			{
				if (version)
				{
					*version = 0;
				}
				uncacheable_slot.imp = call_mismatch_hook(cls, selector, result);
				result = (struct objc_slot2*)&uncacheable_slot;
			}
		}
	}
	return result;
}

struct objc_slot *objc_get_slot(Class cls, SEL selector)
{
	struct objc_slot2 *result = objc_get_slot2(cls, selector, NULL);
	if (result == NULL)
	{
		return NULL;
	}
	uncacheable_slot_v1.owner = Nil;
	// Don't leak extended type encodings!
	uncacheable_slot_v1.types = sel_getType_np(((struct objc_method*)result)->selector);
	uncacheable_slot_v1.selector = selector;
	uncacheable_slot_v1.method = result->method;
	return &uncacheable_slot_v1;
}

////////////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////////////

BOOL class_respondsToSelector(Class cls, SEL selector)
{
	if (0 == selector || 0 == cls) { return NO; }

	return NULL != objc_get_slot2(cls, selector, NULL);
}

IMP class_getMethodImplementation(Class cls, SEL name)
{
	if ((Nil == cls) || (NULL == name)) { return (IMP)0; }
	struct objc_slot2 * slot = objc_get_slot2(cls, name, NULL);
	return NULL != slot ? slot->method : __objc_msg_forward2(nil, name);
}

IMP class_getMethodImplementation_stret(Class cls, SEL name)
{
	return class_getMethodImplementation(cls, name);
}


////////////////////////////////////////////////////////////////////////////////
// Legacy compatibility
////////////////////////////////////////////////////////////////////////////////

#ifndef NO_LEGACY
/**
 * Legacy message lookup function.
 */
BOOL __objc_responds_to(id object, SEL sel)
{
	return class_respondsToSelector(classForObject(object), sel);
}

IMP get_imp(Class cls, SEL selector)
{
	return class_getMethodImplementation(cls, selector);
}

/**
 * Message send function that only ever worked on a small subset of compiler /
 * architecture combinations.
 */
void *objc_msg_sendv(void)
{
	fprintf(stderr, "objc_msg_sendv() never worked correctly.  Don't use it.\n");
	abort();
}
#endif
/**
 * Legacy message lookup function.  Does not support fast proxies or safe IMP
 * caching.
 */
IMP objc_msg_lookup(id receiver, SEL selector)
{
	if (nil == receiver) { return (IMP)nil_method; }

	id self = receiver;
	struct objc_slot2 * slot = objc_msg_lookup_internal(&self, selector, NULL);
	// If the receiver is changed by the lookup mechanism then we have to fall
	// back to old-style forwarding.
	if (self != receiver)
	{
		return __objc_msg_forward2(receiver, selector);
	}
	return slot->method;
}

IMP objc_msg_lookup_super(struct objc_super *super, SEL selector)
{
	return objc_slot_lookup_super2(super, selector)->method;
}
