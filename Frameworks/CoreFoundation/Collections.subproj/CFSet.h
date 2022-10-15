/*	CFSet.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/
/*!
        @header CFSet
        CFSet implements a container which stores unique values.
*/

#if !defined(__COREFOUNDATION_CFSET__)
#define __COREFOUNDATION_CFSET__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

/*!
	@typedef CFSetRetainCallBack
	Type of the callback function used by CFSets for retaining values.
        @param allocator The allocator of the CFSet.
	@param value The value to retain.
        @result The value to store in the set, which is usually the value
		parameter passed to this callback, but may be a different
		value if a different value should be stored in the set.
*/
typedef const void *	(*CFSetRetainCallBack)(CFAllocatorRef allocator, const void *value);

/*!
	@typedef CFSetReleaseCallBack
	Type of the callback function used by CFSets for releasing a retain on values.
        @param allocator The allocator of the CFSet.
	@param value The value to release.
*/
typedef void		(*CFSetReleaseCallBack)(CFAllocatorRef allocator, const void *value);

/*!
	@typedef CFSetCopyDescriptionCallBack
	Type of the callback function used by CFSets for describing values.
	@param value The value to describe.
        @result A description of the specified value.
*/
typedef CFStringRef	(*CFSetCopyDescriptionCallBack)(const void *value);

/*!
	@typedef CFSetEqualCallBack
	Type of the callback function used by CFSets for comparing values.
	@param value1 The first value to compare.
	@param value2 The second value to compare.
        @result True if the values are equal, otherwise false.
*/
typedef Boolean		(*CFSetEqualCallBack)(const void *value1, const void *value2);

/*!
	@typedef CFSetHashCallBack
	Type of the callback function used by CFSets for hashing values.
	@param value The value to hash.
        @result The hash of the value.
*/
typedef CFHashCode	(*CFSetHashCallBack)(const void *value);

/*!
	@typedef CFSetCallBacks
	Structure containing the callbacks of a CFSet.
	@field version The version number of the structure type being passed
		in as a parameter to the CFSet creation functions. This
		structure is version 0.
	@field retain The callback used to add a retain for the set on
		values as they are put into the set. This callback returns
		the value to store in the set, which is usually the value
		parameter passed to this callback, but may be a different
		value if a different value should be stored in the set.
		The set's allocator is passed as the first argument.
	@field release The callback used to remove a retain previously added
		for the set from values as they are removed from the
		set. The set's allocator is passed as the first
		argument.
	@field copyDescription The callback used to create a descriptive
		string representation of each value in the set. This is
		used by the CFCopyDescription() function.
	@field equal The callback used to compare values in the set for
		equality for some operations.
	@field hash The callback used to compare values in the set for
		uniqueness for some operations.
*/
typedef struct {
    CFIndex				version;
    CFSetRetainCallBack			retain;
    CFSetReleaseCallBack		release;
    CFSetCopyDescriptionCallBack	copyDescription;
    CFSetEqualCallBack			equal;
    CFSetHashCallBack			hash;
} CFSetCallBacks;

/*!
	@constant kCFTypeSetCallBacks
	Predefined CFSetCallBacks structure containing a set of callbacks
	appropriate for use when the values in a CFSet are all CFTypes.
*/
CF_EXPORT
const CFSetCallBacks kCFTypeSetCallBacks;

/*!
	@constant kCFCopyStringSetCallBacks
	Predefined CFSetCallBacks structure containing a set of callbacks
	appropriate for use when the values in a CFSet should be copies
        of a CFString.
*/
CF_EXPORT
const CFSetCallBacks kCFCopyStringSetCallBacks;

/*!
	@typedef CFSetApplierFunction
	Type of the callback function used by the apply functions of
		CFSets.
	@param value The current value from the set.
	@param context The user-defined context parameter given to the apply
		function.
*/
typedef void (*CFSetApplierFunction)(const void *value, void *context);

/*!
        @typedef CFSetRef
	This is the type of a reference to immutable CFSets.
*/
typedef const struct CF_BRIDGED_TYPE(NSSet) __CFSet * CFSetRef;

/*!
        @typedef CFMutableSetRef
	This is the type of a reference to mutable CFSets.
*/
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMutableSet) __CFSet * CFMutableSetRef;

/*!
        @function CFSetGetTypeID
        Returns the type identifier of all CFSet instances.
*/
CF_EXPORT
CFTypeID CFSetGetTypeID(void);

/*!
        @function CFSetCreate
        Creates a new immutable set with the given values.
	@param allocator The CFAllocator which should be used to allocate
		memory for the set and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
	@param values A C array of the pointer-sized values to be in the
		set.  This C array is not changed or freed by this function. 
                If this parameter is not a valid pointer to a C array of at
                least numValues pointers, the behavior is undefined.
	@param numValues The number of values to copy from the values C
		array into the CFSet. This number will be the count of the
		set.  If this parameter is zero, negative, or greater than 
                the number of values actually in the values C array, the 
                behavior is undefined.
	@param callBacks A C pointer to a CFSetCallBacks structure
		initialized with the callbacks for the set to use on each
		value in the set. A copy of the contents of the
		callbacks structure is made, so that a pointer to a
		structure on the stack can be passed in, or can be reused
		for multiple set creations. If the version field of this
		callbacks structure is not one of the defined ones for
		CFSet, the behavior is undefined. The retain field may be
		NULL, in which case the CFSet will do nothing to add a
		retain to the contained values for the set. The release
		field may be NULL, in which case the CFSet will do nothing
		to remove the set's retain (if any) on the values when the
		set is destroyed. If the copyDescription field is NULL,
		the set will create a simple description for the value. If
		the equal field is NULL, the set will use pointer equality
		to test for equality of values. The hash field may be NULL,
                in which case the CFSet will determine uniqueness by pointer
                equality. This callbacks parameter
		itself may be NULL, which is treated as if a valid structure
		of version 0 with all fields NULL had been passed in.
		Otherwise, if any of the fields are not valid pointers to
		functions of the correct type, or this parameter is not a
		valid pointer to a  CFSetCallBacks callbacks structure,
		the behavior is undefined. If any of the values put into the
		set is not one understood by one of the callback functions
		the behavior when that callback function is used is
		undefined.
	@result A reference to the new immutable CFSet.
*/
CF_EXPORT
CFSetRef CFSetCreate(CFAllocatorRef allocator, const void **values, CFIndex numValues, const CFSetCallBacks *callBacks);

/*!
	@function CFSetCreateCopy
	Creates a new immutable set with the values from the given set.
	@param allocator The CFAllocator which should be used to allocate
		memory for the set and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
	@param theSet The set which is to be copied. The values from the
		set are copied as pointers into the new set (that is,
		the values themselves are copied, not that which the values
		point to, if anything). However, the values are also
		retained by the new set. The count of the new set will
		be the same as the copied set. The new set uses the same
		callbacks as the set to be copied. If this parameter is
		not a valid CFSet, the behavior is undefined.
	@result A reference to the new immutable CFSet.
*/
CF_EXPORT
CFSetRef CFSetCreateCopy(CFAllocatorRef allocator, CFSetRef theSet);

/*!
	@function CFSetCreateMutable
	Creates a new empty mutable set.
	@param allocator The CFAllocator which should be used to allocate
		memory for the set and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
        @param capacity A hint about the number of values that will be held
                by the CFSet. Pass 0 for no hint. The implementation may
                ignore this hint, or may use it to optimize various
                operations. A set's actual capacity is only limited by 
                address space and available memory constraints). If this 
                parameter is negative, the behavior is undefined.
	@param callBacks A C pointer to a CFSetCallBacks structure
		initialized with the callbacks for the set to use on each
		value in the set. A copy of the contents of the
		callbacks structure is made, so that a pointer to a
		structure on the stack can be passed in, or can be reused
		for multiple set creations. If the version field of this
		callbacks structure is not one of the defined ones for
		CFSet, the behavior is undefined. The retain field may be
		NULL, in which case the CFSet will do nothing to add a
		retain to the contained values for the set. The release
		field may be NULL, in which case the CFSet will do nothing
		to remove the set's retain (if any) on the values when the
		set is destroyed. If the copyDescription field is NULL,
		the set will create a simple description for the value. If
		the equal field is NULL, the set will use pointer equality
		to test for equality of values. The hash field may be NULL,
                in which case the CFSet will determine uniqueness by pointer
                equality. This callbacks parameter
		itself may be NULL, which is treated as if a valid structure
		of version 0 with all fields NULL had been passed in.
		Otherwise, if any of the fields are not valid pointers to
		functions of the correct type, or this parameter is not a
		valid pointer to a  CFSetCallBacks callbacks structure,
		the behavior is undefined. If any of the values put into the
		set is not one understood by one of the callback functions
		the behavior when that callback function is used is
		undefined.
	@result A reference to the new mutable CFSet.
*/
CF_EXPORT
CFMutableSetRef CFSetCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFSetCallBacks *callBacks);

/*!
	@function CFSetCreateMutableCopy
	Creates a new immutable set with the values from the given set.
	@param allocator The CFAllocator which should be used to allocate
		memory for the set and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
        @param capacity A hint about the number of values that will be held
                by the CFSet. Pass 0 for no hint. The implementation may
                ignore this hint, or may use it to optimize various
                operations. A set's actual capacity is only limited by
                address space and available memory constraints). 
                This parameter must be greater than or equal
                to the count of the set which is to be copied, or the
                behavior is undefined. If this parameter is negative, the
                behavior is undefined.
	@param theSet The set which is to be copied. The values from the
		set are copied as pointers into the new set (that is,
		the values themselves are copied, not that which the values
		point to, if anything). However, the values are also
		retained by the new set. The count of the new set will
		be the same as the copied set. The new set uses the same
		callbacks as the set to be copied. If this parameter is
		not a valid CFSet, the behavior is undefined.
	@result A reference to the new mutable CFSet.
*/
CF_EXPORT
CFMutableSetRef CFSetCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFSetRef theSet);

/*!
	@function CFSetGetCount
	Returns the number of values currently in the set.
	@param theSet The set to be queried. If this parameter is not a valid
		CFSet, the behavior is undefined.
	@result The number of values in the set.
*/
CF_EXPORT
CFIndex CFSetGetCount(CFSetRef theSet);

/*!
	@function CFSetGetCountOfValue
	Counts the number of times the given value occurs in the set. Since 
        sets by definition contain only one instance of a value, this function
        is synonymous to CFSetContainsValue.
	@param theSet The set to be searched. If this parameter is not a
		valid CFSet, the behavior is undefined.
	@param value The value for which to find matches in the set. The
		equal() callback provided when the set was created is
		used to compare. If the equal() callback was NULL, pointer
		equality (in C, ==) is used. If value, or any of the values
		in the set, are not understood by the equal() callback,
		the behavior is undefined.
	@result The number of times the given value occurs in the set.
*/
CF_EXPORT
CFIndex CFSetGetCountOfValue(CFSetRef theSet, const void *value);

/*!
	@function CFSetContainsValue
	Reports whether or not the value is in the set.
	@param theSet The set to be searched. If this parameter is not a
		valid CFSet, the behavior is undefined.
	@param value The value for which to find matches in the set. The
		equal() callback provided when the set was created is
		used to compare. If the equal() callback was NULL, pointer
		equality (in C, ==) is used. If value, or any of the values
		in the set, are not understood by the equal() callback,
		the behavior is undefined.
	@result true, if the value is in the set, otherwise false.
*/
CF_EXPORT
Boolean CFSetContainsValue(CFSetRef theSet, const void *value);

/*!
	@function CFSetGetValue
	Retrieves a value in the set which hashes the same as the specified value.
	@param theSet The set to be queried. If this parameter is not a
		valid CFSet, the behavior is undefined.
	@param value The value to retrieve. The equal() callback provided when
                the set was created is used to compare. If the equal() callback
                was NULL, pointer equality (in C, ==) is used. If a value, or
                any of the values in the set, are not understood by the equal()
                callback, the behavior is undefined.
        @result The value in the set with the given hash.
*/
CF_EXPORT
const void *CFSetGetValue(CFSetRef theSet, const void *value);

/*!
	@function CFSetGetValueIfPresent
	Retrieves a value in the set which hashes the same as the specified value,
        if present.
	@param theSet The set to be queried. If this parameter is not a
		valid CFSet, the behavior is undefined.
	@param candidate This value is hashed and compared with values in the
                set to determine which value to retrieve. The equal() callback provided when
                the set was created is used to compare. If the equal() callback
                was NULL, pointer equality (in C, ==) is used. If a value, or
                any of the values in the set, are not understood by the equal()
                callback, the behavior is undefined.
	@param value A pointer to memory which should be filled with the
		pointer-sized value if a matching value is found. If no
		match is found, the contents of the storage pointed to by
		this parameter are undefined. This parameter may be NULL,
		in which case the value from the dictionary is not returned
		(but the return value of this function still indicates
		whether or not the value was present).
        @result True if the value was present in the set, otherwise false.
*/
CF_EXPORT
Boolean CFSetGetValueIfPresent(CFSetRef theSet, const void *candidate, const void **value);

/*!
	@function CFSetGetValues
	Fills the buffer with values from the set.
	@param theSet The set to be queried. If this parameter is not a
		valid CFSet, the behavior is undefined.
	@param values A C array of pointer-sized values to be filled with
		values from the set. The values in the C array are ordered
		in the same order in which they appear in the set. If this
		parameter is not a valid pointer to a C array of at least
		CFSetGetCount() pointers, the behavior is undefined.
*/
CF_EXPORT
void CFSetGetValues(CFSetRef theSet, const void **values);

/*!
	@function CFSetApplyFunction
	Calls a function once for each value in the set.
	@param theSet The set to be operated upon. If this parameter is not
		a valid CFSet, the behavior is undefined.
	@param applier The callback function to call once for each value in
		the given set. If this parameter is not a
		pointer to a function of the correct prototype, the behavior
		is undefined. If there are values in the set which the
		applier function does not expect or cannot properly apply
		to, the behavior is undefined. 
	@param context A pointer-sized user-defined value, which is passed
		as the second parameter to the applier function, but is
		otherwise unused by this function. If the context is not
		what is expected by the applier function, the behavior is
		undefined.
*/
CF_EXPORT
void CFSetApplyFunction(CFSetRef theSet, CFSetApplierFunction CF_NOESCAPE applier, void *context);

/*!
	@function CFSetAddValue
	Adds the value to the set if it is not already present.
	@param theSet The set to which the value is to be added. If this
		parameter is not a valid mutable CFSet, the behavior is
		undefined.
	@param value The value to add to the set. The value is retained by
		the set using the retain callback provided when the set
		was created. If the value is not of the sort expected by the
		retain callback, the behavior is undefined. The count of the 
                set is increased by one.
*/
CF_EXPORT
void CFSetAddValue(CFMutableSetRef theSet, const void *value);

/*!
	@function CFSetReplaceValue
	Replaces the value in the set if it is present.
	@param theSet The set to which the value is to be replaced. If this
		parameter is not a valid mutable CFSet, the behavior is
		undefined.
        @param value The value to replace in the set. The equal() callback provided when
                the set was created is used to compare. If the equal() callback
                was NULL, pointer equality (in C, ==) is used. If a value, or
                any of the values in the set, are not understood by the equal()
                callback, the behavior is undefined. The value is retained by
		the set using the retain callback provided when the set
		was created. If the value is not of the sort expected by the
		retain callback, the behavior is undefined. The count of the 
                set is increased by one.
*/
CF_EXPORT
void CFSetReplaceValue(CFMutableSetRef theSet, const void *value);

/*!
	@function CFSetSetValue
	Replaces the value in the set if it is present, or adds the value to 
        the set if it is absent.
	@param theSet The set to which the value is to be replaced. If this
		parameter is not a valid mutable CFSet, the behavior is
		undefined.
        @param value The value to set in the CFSet. The equal() callback provided when
                the set was created is used to compare. If the equal() callback
                was NULL, pointer equality (in C, ==) is used. If a value, or
                any of the values in the set, are not understood by the equal()
                callback, the behavior is undefined. The value is retained by
		the set using the retain callback provided when the set
		was created. If the value is not of the sort expected by the
		retain callback, the behavior is undefined. The count of the 
                set is increased by one.
*/
CF_EXPORT
void CFSetSetValue(CFMutableSetRef theSet, const void *value);

/*!
	@function CFSetRemoveValue
	Removes the specified value from the set.
	@param theSet The set from which the value is to be removed.
                If this parameter is not a valid mutable CFSet,
		the behavior is undefined.
        @param value The value to remove. The equal() callback provided when
                the set was created is used to compare. If the equal() callback
                was NULL, pointer equality (in C, ==) is used. If a value, or
                any of the values in the set, are not understood by the equal()
                callback, the behavior is undefined.
*/
CF_EXPORT
void CFSetRemoveValue(CFMutableSetRef theSet, const void *value);

/*!
	@function CFSetRemoveAllValues
	Removes all the values from the set, making it empty.
	@param theSet The set from which all of the values are to be
		removed. If this parameter is not a valid mutable CFSet,
		the behavior is undefined.
*/
CF_EXPORT
void CFSetRemoveAllValues(CFMutableSetRef theSet);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFSET__ */

