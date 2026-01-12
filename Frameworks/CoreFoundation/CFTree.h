/*
 * Copyright (c) 2015 Apple Inc. All rights reserved.
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

/*	CFTree.h
	Copyright (c) 1998-2014, Apple Inc. All rights reserved.
*/
/*!
        @header CFTree
        CFTree implements a container which stores references to other CFTrees.
        Each tree may have a parent, and a variable number of children.
*/

#if !defined(__COREFOUNDATION_CFTREE__)
#define __COREFOUNDATION_CFTREE__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

/*!
        @typedef CFTreeRetainCallBack
        Type of the callback function used to add a retain to the user-specified
        info parameter.  This callback may returns the value to use whenever the
        info parameter is retained, which is usually the value parameter passed
        to this callback, but may be a different value if a different value
        should be used.
        @param info A user-supplied info parameter provided in a CFTreeContext.
        @result The retained info parameter.
*/
typedef const void *	(*CFTreeRetainCallBack)(const void *info);

/*!
        @typedef CFTreeReleaseCallBack
        Type of the callback function used to remove a retain previously
        added to the user-specified info parameter.
        @param info A user-supplied info parameter provided in a CFTreeContext.
*/
typedef void		(*CFTreeReleaseCallBack)(const void *info);

/*!
        @typedef CFTreeCopyDescriptionCallBack
        Type of the callback function used to provide a description of the
        user-specified info parameter.
        @param info A user-supplied info parameter provided in a CFTreeContext.
        @result A description of the info parameter.
*/
typedef CFStringRef	(*CFTreeCopyDescriptionCallBack)(const void *info);

/*!
        @typedef CFTreeContext
        Structure containing user-specified data and callbacks for a CFTree.
        @field version The version number of the structure type being passed
                in as a parameter to the CFTree creation function.
                This structure is version 0.
        @field info A C pointer to a user-specified block of data.
        @field retain The callback used to add a retain for the info field.
                If this parameter is not a pointer to a function of the correct
                prototype, the behavior is undefined.  The value may be NULL.
        @field release The calllback used to remove a retain previously added
                for the info field.  If this parameter is not a pointer to a 
                function of the correct prototype, the behavior is undefined.
                The value may be NULL.
        @field copyDescription The callback used to provide a description of
                the info field.
*/
typedef struct {
    CFIndex				version;
    void *				info;
    CFTreeRetainCallBack		retain;
    CFTreeReleaseCallBack		release;	
    CFTreeCopyDescriptionCallBack	copyDescription;
} CFTreeContext;

/*!
        @typedef CFTreeApplierFunction
        Type of the callback function used by the apply functions of
                CFTree.
        @param value The current value from the CFTree
        @param context The user-defined context parameter give to the apply
                function.
*/
typedef void (*CFTreeApplierFunction)(const void *value, void *context);

/*!
        @typedef CFTreeRef
        This is the type of a reference to CFTrees.
*/
typedef struct __CFTree * CFTreeRef;

/*!
        @function CFTreeGetTypeID
        Returns the type identifier of all CFTree instances.
*/
CF_EXPORT
CFTypeID CFTreeGetTypeID(void);

/*!
        @function CFTreeCreate
        Creates a new mutable tree.
        @param allocator The CFAllocator which should be used to allocate
                memory for the tree and storage for its children.  This
                parameter may be NULL in which case the current default
                CFAllocator is used.  If this reference is not a valid
                CFAllocator, the behavior is undefined.
        @param context A C pointer to a CFTreeContext structure to be copied 
                and used as the context of the new tree.  The info parameter
                will be retained by the tree if a retain function is provided.
                If this value is not a valid C pointer to a CFTreeContext 
                structure-sized block of storage, the result is undefined. 
                If the version number of the storage is not a valid CFTreeContext
                version number, the result is undefined.
        @result A reference to the new CFTree.
*/
CF_EXPORT
CFTreeRef CFTreeCreate(CFAllocatorRef allocator, const CFTreeContext *context);

/*!
        @function CFTreeGetParent
        Returns the parent of the specified tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @result The parent of the tree.
*/
CF_EXPORT
CFTreeRef CFTreeGetParent(CFTreeRef tree);

/*!
        @function CFTreeGetNextSibling
        Returns the sibling after the specified tree in the parent tree's list.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @result The next sibling of the tree.
*/
CF_EXPORT
CFTreeRef CFTreeGetNextSibling(CFTreeRef tree);

/*!
        @function CFTreeGetFirstChild
        Returns the first child of the tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @result The first child of the tree.
*/
CF_EXPORT
CFTreeRef CFTreeGetFirstChild(CFTreeRef tree);

/*!
        @function CFTreeGetContext
        Returns the context of the specified tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @param context A C pointer to a CFTreeContext structure to be filled in with
                the context of the specified tree.  If this value is not a valid C
                pointer to a CFTreeContext structure-sized block of storage, the
                result is undefined.  If the version number of the storage is not
                a valid CFTreeContext version number, the result is undefined.
*/
CF_EXPORT
void CFTreeGetContext(CFTreeRef tree, CFTreeContext *context);

/*!
        @function CFTreeGetChildCount
        Returns the number of children of the specified tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @result The number of children.
*/
CF_EXPORT
CFIndex CFTreeGetChildCount(CFTreeRef tree);

/*!
        @function CFTreeGetChildAtIndex
        Returns the nth child of the specified tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @param idx The index of the child tree to be returned.  If this parameter
                is less than zero or greater than the number of children of the
                tree, the result is undefined.
        @result A reference to the specified child tree.
*/
CF_EXPORT
CFTreeRef CFTreeGetChildAtIndex(CFTreeRef tree, CFIndex idx);

/*!
        @function CFTreeGetChildren
        Fills the buffer with children from the tree.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
	@param children A C array of pointer-sized values to be filled with
		children from the tree.  If this parameter is not a valid pointer to a 
                C array of at least CFTreeGetChildCount() pointers, the behavior is undefined.
        @result A reference to the specified child tree.
*/
CF_EXPORT
void CFTreeGetChildren(CFTreeRef tree, CFTreeRef *children);

/*!
	@function CFTreeApplyFunctionToChildren
	Calls a function once for each child of the tree.  Note that the applier
        only operates one level deep, and does not operate on descendents further
        removed than the immediate children of the tree.
        @param heap The tree to be operated upon.  If this parameter is not a
		valid CFTree, the behavior is undefined.
	@param applier The callback function to call once for each child of
		the given tree.  If this parameter is not a pointer to a 
                function of the correct prototype, the behavior is undefined.
                If there are values in the tree which the applier function does
                not expect or cannot properly apply to, the behavior is undefined.
	@param context A pointer-sized user-defined value, which is passed
		as the second parameter to the applier function, but is
		otherwise unused by this function.  If the context is not
		what is expected by the applier function, the behavior is
		undefined.
*/
CF_EXPORT
void CFTreeApplyFunctionToChildren(CFTreeRef tree, CFTreeApplierFunction applier, void *context);

/*!
        @function CFTreeFindRoot
        Returns the root tree of which the specified tree is a descendent.
        @param tree The tree to be queried.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @result A reference to the root of the tree.
*/
CF_EXPORT
CFTreeRef CFTreeFindRoot(CFTreeRef tree);

/*!
        @function CFTreeSetContext
        Replaces the context of a tree.  The tree releases its retain on the
        info of the previous context, and retains the info of the new context.
        @param tree The tree to be operated on.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @param context A C pointer to a CFTreeContext structure to be copied 
                and used as the context of the new tree.  The info parameter
                will be retained by the tree if a retain function is provided.
                If this value is not a valid C pointer to a CFTreeContext 
                structure-sized block of storage, the result is undefined. 
                If the version number of the storage is not a valid CFTreeContext
                version number, the result is undefined.
*/
CF_EXPORT
void CFTreeSetContext(CFTreeRef tree, const CFTreeContext *context);

/*!
        @function CFTreePrependChild
        Adds the newChild to the specified tree as the first in its list of children.
        @param tree The tree to be operated on.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @param newChild The child to be added.
                If this parameter is not a valid CFTree, the behavior is undefined.
                If this parameter is a tree which is already a child of any tree,
                the behavior is undefined.
*/
CF_EXPORT
void CFTreePrependChild(CFTreeRef tree, CFTreeRef newChild);

/*!
        @function CFTreeAppendChild
        Adds the newChild to the specified tree as the last in its list of children.
        @param tree The tree to be operated on.  If this parameter is not a valid
                CFTree, the behavior is undefined.
        @param newChild The child to be added.
                If this parameter is not a valid CFTree, the behavior is undefined.
                If this parameter is a tree which is already a child of any tree,
                the behavior is undefined.
*/
CF_EXPORT
void CFTreeAppendChild(CFTreeRef tree, CFTreeRef newChild);

/*!
        @function CFTreeInsertSibling
        Inserts newSibling into the the parent tree's linked list of children after
        tree.  The newSibling will have the same parent as tree.
        @param tree The tree to insert newSibling after.  If this parameter is not a valid
                CFTree, the behavior is undefined.  If the tree does not have a
                parent, the behavior is undefined.
        @param newSibling The sibling to be added.
                If this parameter is not a valid CFTree, the behavior is undefined.
                If this parameter is a tree which is already a child of any tree,
                the behavior is undefined.  
*/
CF_EXPORT
void CFTreeInsertSibling(CFTreeRef tree, CFTreeRef newSibling);

/*!
        @function CFTreeRemove
        Removes the tree from its parent.
        @param tree The tree to be removed.  If this parameter is not a valid
                CFTree, the behavior is undefined.
*/
CF_EXPORT
void CFTreeRemove(CFTreeRef tree);

/*!
        @function CFTreeRemoveAllChildren
        Removes all the children of the tree.
        @param tree The tree to remove all children from.  If this parameter is not a valid
                CFTree, the behavior is undefined.
*/
CF_EXPORT
void CFTreeRemoveAllChildren(CFTreeRef tree);

/*!
        @function CFTreeSortChildren
        Sorts the children of the specified tree using the specified comparator function.
        @param tree The tree to be operated on.  If this parameter is not a valid
                CFTree, the behavior is undefined.
	@param comparator The function with the comparator function type
		signature which is used in the sort operation to compare
		children of the tree with the given value. If this parameter
		is not a pointer to a function of the correct prototype, the
		the behavior is undefined. The children of the tree are sorted 
                from least to greatest according to this function.
	@param context A pointer-sized user-defined value, which is passed
		as the third parameter to the comparator function, but is
		otherwise unused by this function. If the context is not
		what is expected by the comparator function, the behavior is
		undefined.
*/
CF_EXPORT
void CFTreeSortChildren(CFTreeRef tree, CFComparatorFunction comparator, void *context);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFTREE__ */

