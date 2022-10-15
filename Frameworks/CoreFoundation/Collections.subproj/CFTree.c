/*	CFTree.c
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
	Responsibility: Michael LeHew
*/

#include <CoreFoundation/CFTree.h>
#include "CFInternal.h"
#include "CFRuntime_Internal.h"
#include <CoreFoundation/CFPriv.h>

struct __CFTreeCallBacks {
    CFTreeRetainCallBack		retain;
    CFTreeReleaseCallBack		release;	
    CFTreeCopyDescriptionCallBack	copyDescription;
};

struct __CFTree {
    CFRuntimeBase _base;
    CFTreeRef _parent;	/* Not retained */
    CFTreeRef _sibling;	/* Not retained */
    CFTreeRef _child;	/* All children get a retain from the parent */
    CFTreeRef _rightmostChild;	/* Not retained */
    /* This is the context, exploded.
     * Currently the only valid version is 0, so we do not store that.
     * _callbacks initialized if not a special form. */
    void *_info;
    struct __CFTreeCallBacks *_callbacks;
};

static const struct __CFTreeCallBacks __kCFTypeTreeCallBacks = {CFRetain, CFRelease, CFCopyDescription};
static const struct __CFTreeCallBacks __kCFNullTreeCallBacks = {NULL, NULL, NULL};

enum {          /* Bits 0-1 */
    __kCFTreeHasNullCallBacks = 0,
    __kCFTreeHasCFTypeCallBacks = 1,
    __kCFTreeHasCustomCallBacks = 3    /* callbacks pointed to by _callbacks */
};

CF_INLINE uint32_t __CFTreeGetCallBacksType(CFTreeRef tree) {
    return (__CFRuntimeGetValue(tree, 1, 0));
}

CF_INLINE const struct __CFTreeCallBacks *__CFTreeGetCallBacks(CFTreeRef tree) {
    switch (__CFTreeGetCallBacksType(tree)) {
    case __kCFTreeHasNullCallBacks:
	return &__kCFNullTreeCallBacks;
    case __kCFTreeHasCFTypeCallBacks:
	return &__kCFTypeTreeCallBacks;
    case __kCFTreeHasCustomCallBacks:
	break;
    }
    return tree->_callbacks;
}

CF_INLINE bool __CFTreeCallBacksMatchNull(const CFTreeContext *c) {
    return (NULL == c || (c->retain == NULL && c->release == NULL && c->copyDescription == NULL));
}   

CF_INLINE bool __CFTreeCallBacksMatchCFType(const CFTreeContext *c) {
    return (NULL != c && (c->retain == CFRetain && c->release == CFRelease && c->copyDescription == CFCopyDescription));
}   

static CFStringRef __CFTreeCopyDescription(CFTypeRef cf) {
    CFTreeRef tree = (CFTreeRef)cf;
    CFMutableStringRef result;
    CFStringRef contextDesc = NULL;
    const struct __CFTreeCallBacks *cb;
    CFAllocatorRef allocator;
    allocator = CFGetAllocator(tree);
    result = CFStringCreateMutable(allocator, 0);
    cb = __CFTreeGetCallBacks(tree);
    if (NULL != cb->copyDescription) {
	contextDesc = (CFStringRef)INVOKE_CALLBACK1(cb->copyDescription, tree->_info);
    }
    if (NULL == contextDesc) {
	contextDesc = CFStringCreateWithFormat(allocator, NULL, CFSTR("<CFTree context %p>"), tree->_info);
    }
    CFStringAppendFormat(result, NULL, CFSTR("<CFTree %p [%p]>{children = %lu, context = %@}"), cf, allocator, (unsigned long)CFTreeGetChildCount(tree), contextDesc);
    if (contextDesc) CFRelease(contextDesc);
    return result;
}

static void __CFTreeDeallocate(CFTypeRef cf) {
    CFTreeRef tree = (CFTreeRef)cf;
    const struct __CFTreeCallBacks *cb;
#if TARGET_OS_OSX
    CFTreeRemoveAllChildren(tree);
#endif
    cb = __CFTreeGetCallBacks(tree);
    if (NULL != cb->release) {
        INVOKE_CALLBACK1(cb->release, tree->_info);
    }
    if (__kCFTreeHasCustomCallBacks == __CFTreeGetCallBacksType(tree)) {
        CFAllocatorDeallocate(CFGetAllocator(tree), tree->_callbacks);
    }
}

const CFRuntimeClass __CFTreeClass = {
    _kCFRuntimeScannedObject,
    "CFTree",
    NULL,	// init
    NULL,	// copy
    __CFTreeDeallocate,
    NULL,	// equal
    NULL,	// hash
    NULL,	// 
    __CFTreeCopyDescription
};

CFTypeID CFTreeGetTypeID(void) {
    return _kCFRuntimeIDCFTree;
}

CFTreeRef CFTreeCreate(CFAllocatorRef allocator, const CFTreeContext *context) {
    CFTreeRef memory;
    uint32_t size;

    CFAssert1(NULL != context, __kCFLogAssertion, "%s(): pointer to context may not be NULL", __PRETTY_FUNCTION__);
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    size = sizeof(struct __CFTree) - sizeof(CFRuntimeBase);
    memory = (CFTreeRef)_CFRuntimeCreateInstance(allocator, CFTreeGetTypeID(), size, NULL);
    if (NULL == memory) {
	return NULL;
    }

    /* Start the context off in a recognizable state */
    __CFRuntimeSetValue(memory, 1, 0, __kCFTreeHasNullCallBacks);
    CFTreeSetContext(memory, context);
    return memory;
}

CFIndex CFTreeGetChildCount(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    SInt32 cnt = 0;
    tree = tree->_child;
    while (NULL != tree) {
	cnt++;
	tree = tree->_sibling;
    }
    return cnt;
}

CFTreeRef CFTreeGetParent(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    return tree->_parent;
}

CFTreeRef CFTreeGetNextSibling(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    return tree->_sibling;
}

CFTreeRef CFTreeGetFirstChild(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    return tree->_child;
}

CFTreeRef CFTreeFindRoot(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    while (NULL != tree->_parent) {
	tree = tree->_parent;
    }
    return tree;
}

void CFTreeGetContext(CFTreeRef tree, CFTreeContext *context) {
    const struct __CFTreeCallBacks *cb;
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    CFAssert1(0 == context->version, __kCFLogAssertion, "%s(): context version not initialized to 0", __PRETTY_FUNCTION__);
    cb = __CFTreeGetCallBacks(tree);
    context->version = 0;
    context->info = tree->_info;
    context->retain = cb->retain;
    context->release = cb->release;
    context->copyDescription = cb->copyDescription;
    UNFAULT_CALLBACK(context->retain);
    UNFAULT_CALLBACK(context->release);
    UNFAULT_CALLBACK(context->copyDescription);
}

void CFTreeSetContext(CFTreeRef tree, const CFTreeContext *context) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    uint32_t newtype, oldtype = __CFTreeGetCallBacksType(tree);
    struct __CFTreeCallBacks *oldcb = (struct __CFTreeCallBacks *)__CFTreeGetCallBacks(tree);
    struct __CFTreeCallBacks *newcb;
    void *oldinfo = tree->_info;
    CFAllocatorRef allocator = CFGetAllocator(tree);
    
    if (__CFTreeCallBacksMatchNull(context)) {
        newtype = __kCFTreeHasNullCallBacks;
    } else if (__CFTreeCallBacksMatchCFType(context)) {
        newtype = __kCFTreeHasCFTypeCallBacks;
    } else {
        newtype = __kCFTreeHasCustomCallBacks;
        *((void **)&tree->_callbacks) = CFAllocatorAllocate(allocator, sizeof(struct __CFTreeCallBacks), 0);
        if (__CFOASafe) __CFSetLastAllocationEventName(tree->_callbacks, "CFTree (callbacks)");
        tree->_callbacks->retain = context->retain;
        tree->_callbacks->release = context->release;
        tree->_callbacks->copyDescription = context->copyDescription;
        FAULT_CALLBACK((void **)&(tree->_callbacks->retain));
        FAULT_CALLBACK((void **)&(tree->_callbacks->release));
        FAULT_CALLBACK((void **)&(tree->_callbacks->copyDescription));
    }
    __CFRuntimeSetValue(tree, 1, 0, newtype);
    newcb = (struct __CFTreeCallBacks *)__CFTreeGetCallBacks(tree);
    if (NULL != newcb->retain) {
        tree->_info = (void *)INVOKE_CALLBACK1(newcb->retain, context->info);
    } else {
        *((void **)&tree->_info) = context->info;
    }
    if (NULL != oldcb->release) {
        INVOKE_CALLBACK1(oldcb->release, oldinfo);
    }
    if (oldtype == __kCFTreeHasCustomCallBacks) {
        CFAllocatorDeallocate(allocator, oldcb);
    }
}

#if 0
CFTreeRef CFTreeFindNextSibling(CFTreeRef tree, const void *info) {
    __CFGenericValidateType(tree, CFTreeGetTypeID());
    tree = tree->_sibling;
    while (NULL != tree) {
	if (info == tree->_context.info || (tree->_context.equal && tree->_context.equal(info, tree->_context.info))) {
	    return tree;
	}
	tree = tree->_sibling;
    }
    return NULL;
}

CFTreeRef CFTreeFindFirstChild(CFTreeRef tree, const void *info) {
    __CFGenericValidateType(tree, CFTreeGetTypeID());
    tree = tree->_child;
    while (NULL != tree) {
	if (info == tree->_context.info || (tree->_context.equal && tree->_context.equal(info, tree->_context.info))) {
	    return tree;
	}
	tree = tree->_sibling;
    }
    return NULL;
}

CFTreeRef CFTreeFind(CFTreeRef tree, const void *info) {
    __CFGenericValidateType(tree, CFTreeGetTypeID());
    if (info == tree->_context.info || (tree->_context.equal && tree->_context.equal(info, tree->info))) {
	return tree;
    }
    tree = tree->_child;
    while (NULL != tree) {
	CFTreeRef found = CFTreeFind(tree, info);
	if (NULL != found) {
	    return found;
	}
	tree = tree->_sibling;
    }
    return NULL;
}
#endif

CFTreeRef CFTreeGetChildAtIndex(CFTreeRef tree, CFIndex idx) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    tree = tree->_child;
    while (NULL != tree) {
	if (0 == idx) return tree;
	idx--;
	tree = tree->_sibling;
    }
    return NULL;
}

void CFTreeGetChildren(CFTreeRef tree, CFTreeRef *children) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    tree = tree->_child;
    while (NULL != tree) {
	*children++ = tree;
	tree = tree->_sibling;
    }
}

void CFTreeApplyFunctionToChildren(CFTreeRef tree, CFTreeApplierFunction applier, void *context) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    CFAssert1(NULL != applier, __kCFLogAssertion, "%s(): pointer to applier function may not be NULL", __PRETTY_FUNCTION__);
    tree = tree->_child;
    while (NULL != tree) {
	applier(tree, context);
	tree = tree->_sibling;
    }
}

void CFTreePrependChild(CFTreeRef tree, CFTreeRef newChild) {
     CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    __CFGenericValidateType(newChild, CFTreeGetTypeID());
    CFAssert1(NULL == newChild->_parent, __kCFLogAssertion, "%s(): must remove newChild from previous parent first", __PRETTY_FUNCTION__);
    CFAssert1(NULL == newChild->_sibling, __kCFLogAssertion, "%s(): must remove newChild from previous parent first", __PRETTY_FUNCTION__);
    CFRetain(newChild);
    *((void **)&newChild->_parent) = tree;
    *((void **)&newChild->_sibling) = tree->_child;
    if (!tree->_child) {
        *((void **)&tree->_rightmostChild) = newChild;
    }
    *((void **)&tree->_child) = newChild;
}

void CFTreeAppendChild(CFTreeRef tree, CFTreeRef newChild) {
    CFAllocatorRef allocator;
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, newChild);
    CFAssert1(NULL == newChild->_parent, __kCFLogAssertion, "%s(): must remove newChild from previous parent first", __PRETTY_FUNCTION__);
    CFAssert1(NULL == newChild->_sibling, __kCFLogAssertion, "%s(): must remove newChild from previous parent first", __PRETTY_FUNCTION__);
    if (newChild->_parent) {
        HALT;
    }
    CFRetain(newChild);
    allocator = CFGetAllocator(tree);
    *((void **)&newChild->_parent) = tree;
    newChild->_sibling = NULL;
    if (!tree->_child) {
        *((void **)&tree->_child) = newChild;
    } else {
        *((void **)&tree->_rightmostChild->_sibling) = newChild;
    }
    *((void **)&tree->_rightmostChild) = newChild;
}

void CFTreeInsertSibling(CFTreeRef tree, CFTreeRef newSibling) {
    CFAllocatorRef allocator;
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, newSibling);
    CFAssert1(NULL != tree->_parent, __kCFLogAssertion, "%s(): tree must have a parent", __PRETTY_FUNCTION__);
    CFAssert1(NULL == newSibling->_parent, __kCFLogAssertion, "%s(): must remove newSibling from previous parent first", __PRETTY_FUNCTION__);
    CFAssert1(NULL == newSibling->_sibling, __kCFLogAssertion, "%s(): must remove newSibling from previous parent first", __PRETTY_FUNCTION__);
    CFRetain(newSibling);
    allocator = CFGetAllocator(tree);
    *((void **)&newSibling->_parent) = tree->_parent;
    *((void **)&newSibling->_sibling) = tree->_sibling;
    *((void **)&tree->_sibling) = newSibling;
    if (tree->_parent) {
        if (tree->_parent->_rightmostChild == tree) {
            *((void **)&tree->_parent->_rightmostChild) = newSibling;
        }
    }
}

void CFTreeRemove(CFTreeRef tree) {
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    if (NULL != tree->_parent) {
	if (tree == tree->_parent->_child) {
            *((void **)&tree->_parent->_child) = tree->_sibling;
            if (tree->_sibling == NULL) {
                tree->_parent->_rightmostChild = NULL;
            }
	} else {
	    CFTreeRef prevSibling = NULL;
	    for (prevSibling = tree->_parent->_child; prevSibling; prevSibling = prevSibling->_sibling) {
		if (prevSibling->_sibling == tree) {
                    *((void **)&prevSibling->_sibling) = tree->_sibling;
                    if (tree->_parent->_rightmostChild == tree) {
                        *((void **)&tree->_parent->_rightmostChild) = prevSibling;
                    }
		    break;
		}
	    }
	}
	tree->_parent = NULL;
	tree->_sibling = NULL;
        CFRelease(tree);
    }
}

void CFTreeRemoveAllChildren(CFTreeRef tree) {
    CFTreeRef nextChild;
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    nextChild = tree->_child;
    tree->_child = NULL;
    tree->_rightmostChild = NULL;
    while (NULL != nextChild) {
	CFTreeRef nextSibling = nextChild->_sibling;
	nextChild->_parent = NULL;
	nextChild->_sibling = NULL;
        CFRelease(nextChild);
	nextChild = nextSibling;
    }
}

struct _tcompareContext {
    CFComparatorFunction func;
    void *context;
};

static CFComparisonResult __CFTreeCompareValues(const void *v1, const void *v2, struct _tcompareContext *context) {
    const void **val1 = (const void **)v1;
    const void **val2 = (const void **)v2;
    return (CFComparisonResult)(INVOKE_CALLBACK3(context->func, *val1, *val2, context->context));
}

void CFTreeSortChildren(CFTreeRef tree, CFComparatorFunction comparator, void *context) {
    CFIndex children;
    CF_ASSERT_TYPE(_kCFRuntimeIDCFTree, tree);
    CFAssert1(NULL != comparator, __kCFLogAssertion, "%s(): pointer to comparator function may not be NULL", __PRETTY_FUNCTION__);
    children = CFTreeGetChildCount(tree);
    if (1 < children) {
        CFIndex idx;
        CFTreeRef nextChild;
        struct _tcompareContext ctx;
        CFTreeRef *list, buffer[128];

        list = (children < 128) ? buffer : (CFTreeRef *)CFAllocatorAllocate(kCFAllocatorSystemDefault, children * sizeof(CFTreeRef), 0); // XXX_PCB GC OK
	if (__CFOASafe && list != buffer) __CFSetLastAllocationEventName(tree->_callbacks, "CFTree (temp)");
        nextChild = tree->_child;
        for (idx = 0; NULL != nextChild; idx++) {
            list[idx] = nextChild;
            nextChild = nextChild->_sibling;
        }

        ctx.func = comparator;
        ctx.context = context;
        CFQSortArray(list, children, sizeof(CFTreeRef), (CFComparatorFunction)__CFTreeCompareValues, &ctx);

        *((void **)&tree->_child) = list[0];
        for (idx = 1; idx < children; idx++) {
            *((void **)&list[idx - 1]->_sibling) = list[idx];
        }
        list[idx - 1]->_sibling = NULL;
        tree->_rightmostChild = list[children - 1];
        if (list != buffer) CFAllocatorDeallocate(kCFAllocatorSystemDefault, list); // XXX_PCB GC OK
    }
}

