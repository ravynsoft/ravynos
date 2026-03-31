/*
 * Copyright (c) 1999-2007 Apple Inc.  All Rights Reserved.
 * 
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 *	Utilities for registering and looking up selectors.  The sole
 *	purpose of the selector tables is a registry whereby there is
 *	exactly one address (selector) associated with a given string
 *	(method name).
 */

#if !__OBJC2__

#include "objc-private.h"
#include "objc-sel-set.h"

__BEGIN_DECLS

static size_t SelrefCount = 0;

static const char *_objc_empty_selector = "";
static struct __objc_sel_set *_objc_selectors = NULL;


static SEL _objc_search_builtins(const char *key) 
{
#if defined(DUMP_SELECTORS)
    if (NULL != key) printf("\t\"%s\",\n", key);
#endif

    if (!key) return (SEL)0;
    if ('\0' == *key) return (SEL)_objc_empty_selector;

    return (SEL)0;
}


const char *sel_getName(SEL sel) {
    return sel ? (const char *)sel : "<null selector>";
}


BOOL sel_isMapped(SEL name) 
{
    SEL sel;
    
    if (!name) return NO;

    sel = _objc_search_builtins((const char *)name);
    if (sel) return YES;

    mutex_locker_t lock(selLock);
    if (_objc_selectors) {
        sel = __objc_sel_set_get(_objc_selectors, name);
    }
    return bool(sel);
}

static SEL __sel_registerName(const char *name, bool shouldLock, bool copy) 
{
    SEL result = 0;

    if (shouldLock) selLock.assertUnlocked();
    else selLock.assertLocked();

    if (!name) return (SEL)0;
    result = _objc_search_builtins(name);
    if (result) return result;

    conditional_mutex_locker_t lock(selLock, shouldLock);
    if (_objc_selectors) {
        result = __objc_sel_set_get(_objc_selectors, (SEL)name);
    }
    if (result) return result;

    // No match. Insert.

    if (!_objc_selectors) {
        _objc_selectors = __objc_sel_set_create(SelrefCount);
    }
    if (!result) {
        result = (SEL)(copy ? strdup(name) : name);
        __objc_sel_set_add(_objc_selectors, result);
#if defined(DUMP_UNKNOWN_SELECTORS)
        printf("\t\"%s\",\n", name);
#endif
    }

    return result;
}


SEL sel_registerName(const char *name) {
    return __sel_registerName(name, 1, 1);     // YES lock, YES copy
}

SEL sel_registerNameNoLock(const char *name, bool copy) {
    return __sel_registerName(name, 0, copy);  // NO lock, maybe copy
}


// 2001/1/24
// the majority of uses of this function (which used to return NULL if not found)
// did not check for NULL, so, in fact, never return NULL
//
SEL sel_getUid(const char *name) {
    return __sel_registerName(name, 2, 1);  // YES lock, YES copy
}


BOOL sel_isEqual(SEL lhs, SEL rhs)
{
    return bool(lhs == rhs);
}


/***********************************************************************
* sel_init
* Initialize selector tables and register selectors used internally.
**********************************************************************/
void sel_init(size_t selrefCount)
{
    // save this value for later
    SelrefCount = selrefCount;

    // Register selectors used by libobjc

    mutex_locker_t lock(selLock);

    SEL_cxx_construct = sel_registerNameNoLock(".cxx_construct", NO);
    SEL_cxx_destruct = sel_registerNameNoLock(".cxx_destruct", NO);

    extern SEL FwdSel;
    FwdSel = sel_registerNameNoLock("forward::", NO);
}

__END_DECLS

#endif
