/*	CFDictionary.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

/*!
	@header CFDictionary
	CFDictionary implements a container which pairs pointer-sized keys
	with pointer-sized values. Values are accessed via arbitrary
	user-defined keys. A CFDictionary differs from a CFArray in that
	the key used to access a particular value in the dictionary remains
	the same as values are added to or removed from the dictionary,
	unless a value associated with its particular key is replaced or
	removed. In a CFArray, the key (or index) used to retrieve a
	particular value can change over time as values are added to or
	deleted from the array. Also unlike an array, there is no ordering
	among values in a dictionary. To enable later retrieval of a value,
	the key of the key-value pair should be constant (or treated as
	constant); if the key changes after being used to put a value in
	the dictionary, the value may not be retrievable. The keys of a
	dictionary form a set; that is, no two keys which are equal to
	one another are present in the dictionary at any time.

	Dictionaries come in two flavors, immutable, which cannot have
	values added to them or removed from them after the dictionary is
	created, and mutable, to which you can add values or from which
	remove values. Mutable dictionaries can have an unlimited number
	of values (or rather, limited only by constraints external to
	CFDictionary, like the amount of available memory).

	As with all CoreFoundation collection types, dictionaries maintain
	hard references on the values you put in them, but the retaining and
	releasing functions are user-defined callbacks that can actually do
	whatever the user wants (for example, nothing).

	Although a particular implementation of CFDictionary may not use
	hashing and a hash table for storage of the values, the keys have
	a hash-code generating function defined for them, and a function
	to test for equality of two keys. These two functions together
	must maintain the invariant that if equal(X, Y), then hash(X) ==
	hash(Y). Note that the converse will not generally be true (but
	the contrapositive, if hash(X) != hash(Y), then !equal(X, Y),
	will be as required by Boolean logic). If the hash() and equal()
	key callbacks are NULL, the key is used as a pointer-sized integer,
	and pointer equality is used. Care should be taken to provide a
	hash() callback which will compute sufficiently dispersed hash
	codes for the key set for best performance.

	Computational Complexity
	The access time for a value in the dictionary is guaranteed to be at
	worst O(N) for any implementation, current and future, but will
	often be O(1) (constant time). Insertion or deletion operations
	will typically be constant time as well, but are O(N*N) in the
	worst case in some implementations. Access of values through a key
	is faster than accessing values directly (if there are any such
	operations). Dictionaries will tend to use significantly more memory
	than a array with the same number of values.
*/

#if !defined(__COREFOUNDATION_CFDICTIONARY__)
#define __COREFOUNDATION_CFDICTIONARY__ 1

#include <CoreFoundation/CFBase.h>

CF_IMPLICIT_BRIDGING_ENABLED
CF_EXTERN_C_BEGIN

/*!
	@typedef CFDictionaryKeyCallBacks
	Structure containing the callbacks for keys of a CFDictionary.
	@field version The version number of the structure type being passed
		in as a parameter to the CFDictionary creation functions.
		This structure is version 0.
	@field retain The callback used to add a retain for the dictionary
		on keys as they are used to put values into the dictionary.
		This callback returns the value to use as the key in the
		dictionary, which is usually the value parameter passed to
		this callback, but may be a different value if a different
		value should be used as the key. The dictionary's allocator
		is passed as the first argument.
	@field release The callback used to remove a retain previously added
		for the dictionary from keys as their values are removed from
		the dictionary. The dictionary's allocator is passed as the
		first argument.
	@field copyDescription The callback used to create a descriptive
		string representation of each key in the dictionary. This
		is used by the CFCopyDescription() function.
	@field equal The callback used to compare keys in the dictionary for
		equality.
	@field hash The callback used to compute a hash code for keys as they
		are used to access, add, or remove values in the dictionary.
*/
typedef const void *	(*CFDictionaryRetainCallBack)(CFAllocatorRef allocator, const void *value);
typedef void		(*CFDictionaryReleaseCallBack)(CFAllocatorRef allocator, const void *value);
typedef CFStringRef	(*CFDictionaryCopyDescriptionCallBack)(const void *value);
typedef Boolean		(*CFDictionaryEqualCallBack)(const void *value1, const void *value2);
typedef CFHashCode	(*CFDictionaryHashCallBack)(const void *value);
typedef struct {
    CFIndex				version;
    CFDictionaryRetainCallBack		retain;
    CFDictionaryReleaseCallBack		release;
    CFDictionaryCopyDescriptionCallBack	copyDescription;
    CFDictionaryEqualCallBack		equal;
    CFDictionaryHashCallBack		hash;
} CFDictionaryKeyCallBacks;

/*!
	@constant kCFTypeDictionaryKeyCallBacks
	Predefined CFDictionaryKeyCallBacks structure containing a
	set of callbacks appropriate for use when the keys of a
	CFDictionary are all CFTypes.
*/
CF_EXPORT
const CFDictionaryKeyCallBacks kCFTypeDictionaryKeyCallBacks;

/*!
	@constant kCFCopyStringDictionaryKeyCallBacks
	Predefined CFDictionaryKeyCallBacks structure containing a
	set of callbacks appropriate for use when the keys of a
	CFDictionary are all CFStrings, which may be mutable and
	need to be copied in order to serve as constant keys for
	the values in the dictionary.
*/
CF_EXPORT
const CFDictionaryKeyCallBacks kCFCopyStringDictionaryKeyCallBacks;

/*!
	@typedef CFDictionaryValueCallBacks
	Structure containing the callbacks for values of a CFDictionary.
	@field version The version number of the structure type being passed
		in as a parameter to the CFDictionary creation functions.
		This structure is version 0.
	@field retain The callback used to add a retain for the dictionary
		on values as they are put into the dictionary.
		This callback returns the value to use as the value in the
		dictionary, which is usually the value parameter passed to
		this callback, but may be a different value if a different
		value should be added to the dictionary. The dictionary's
		allocator is passed as the first argument.
	@field release The callback used to remove a retain previously added
		for the dictionary from values as they are removed from
		the dictionary. The dictionary's allocator is passed as the
		first argument.
	@field copyDescription The callback used to create a descriptive
		string representation of each value in the dictionary. This
		is used by the CFCopyDescription() function.
	@field equal The callback used to compare values in the dictionary for
		equality in some operations.
*/
typedef struct {
    CFIndex				version;
    CFDictionaryRetainCallBack		retain;
    CFDictionaryReleaseCallBack		release;
    CFDictionaryCopyDescriptionCallBack	copyDescription;
    CFDictionaryEqualCallBack		equal;
} CFDictionaryValueCallBacks;

/*!
	@constant kCFTypeDictionaryValueCallBacks
	Predefined CFDictionaryValueCallBacks structure containing a set
	of callbacks appropriate for use when the values in a CFDictionary
	are all CFTypes.
*/
CF_EXPORT
const CFDictionaryValueCallBacks kCFTypeDictionaryValueCallBacks;

/*!
	@typedef CFDictionaryApplierFunction
	Type of the callback function used by the apply functions of
		CFDictionarys.
	@param key The current key for the value.
	@param value The current value from the dictionary.
	@param context The user-defined context parameter given to the apply
		function.
*/
typedef void (*CFDictionaryApplierFunction)(const void *key, const void *value, void *context);

/*!
	@typedef CFDictionaryRef
	This is the type of a reference to immutable CFDictionarys.
*/
typedef const struct CF_BRIDGED_TYPE(NSDictionary) __CFDictionary * CFDictionaryRef;

/*!
	@typedef CFMutableDictionaryRef
	This is the type of a reference to mutable CFDictionarys.
*/
typedef struct CF_BRIDGED_MUTABLE_TYPE(NSMutableDictionary) __CFDictionary * CFMutableDictionaryRef;

/*!
	@function CFDictionaryGetTypeID
	Returns the type identifier of all CFDictionary instances.
*/
CF_EXPORT
CFTypeID CFDictionaryGetTypeID(void);

/*!
	@function CFDictionaryCreate
	Creates a new immutable dictionary with the given values.
	@param allocator The CFAllocator which should be used to allocate
		memory for the dictionary and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
	@param keys A C array of the pointer-sized keys to be used for
		the parallel C array of values to be put into the dictionary.
		This parameter may be NULL if the numValues parameter is 0.
		This C array is not changed or freed by this function. If
		this parameter is not a valid pointer to a C array of at
		least numValues pointers, the behavior is undefined.
	@param values A C array of the pointer-sized values to be in the
		dictionary. This parameter may be NULL if the numValues
		parameter is 0. This C array is not changed or freed by
		this function. If this parameter is not a valid pointer to
		a C array of at least numValues pointers, the behavior is
		undefined.
	@param numValues The number of values to copy from the keys and
		values C arrays into the CFDictionary. This number will be
		the count of the dictionary. If this parameter is
		negative, or greater than the number of values actually
		in the keys or values C arrays, the behavior is undefined.
	@param keyCallBacks A pointer to a CFDictionaryKeyCallBacks structure
		initialized with the callbacks for the dictionary to use on
		each key in the dictionary. The retain callback will be used
		within this function, for example, to retain all of the new
		keys from the keys C array. A copy of the contents of the
		callbacks structure is made, so that a pointer to a structure
		on the stack can be passed in, or can be reused for multiple
		dictionary creations. If the version field of this
		callbacks structure is not one of the defined ones for
		CFDictionary, the behavior is undefined. The retain field may
		be NULL, in which case the CFDictionary will do nothing to add
		a retain to the keys of the contained values. The release field
		may be NULL, in which case the CFDictionary will do nothing
		to remove the dictionary's retain (if any) on the keys when the
		dictionary is destroyed or a key-value pair is removed. If the
		copyDescription field is NULL, the dictionary will create a
		simple description for a key. If the equal field is NULL, the
		dictionary will use pointer equality to test for equality of
		keys. If the hash field is NULL, a key will be converted from
		a pointer to an integer to compute the hash code. This callbacks
		parameter itself may be NULL, which is treated as if a valid
		structure of version 0 with all fields NULL had been passed in.
		Otherwise, if any of the fields are not valid pointers to
		functions of the correct type, or this parameter is not a
		valid pointer to a CFDictionaryKeyCallBacks callbacks structure,
		the behavior is undefined. If any of the keys put into the
		dictionary is not one understood by one of the callback functions
		the behavior when that callback function is used is undefined.
	@param valueCallBacks A pointer to a CFDictionaryValueCallBacks structure
		initialized with the callbacks for the dictionary to use on
		each value in the dictionary. The retain callback will be used
		within this function, for example, to retain all of the new
		values from the values C array. A copy of the contents of the
		callbacks structure is made, so that a pointer to a structure
		on the stack can be passed in, or can be reused for multiple
		dictionary creations. If the version field of this callbacks
		structure is not one of the defined ones for CFDictionary, the
		behavior is undefined. The retain field may be NULL, in which
		case the CFDictionary will do nothing to add a retain to values
		as they are put into the dictionary. The release field may be
		NULL, in which case the CFDictionary will do nothing to remove
		the dictionary's retain (if any) on the values when the
		dictionary is destroyed or a key-value pair is removed. If the
		copyDescription field is NULL, the dictionary will create a
		simple description for a value. If the equal field is NULL, the
		dictionary will use pointer equality to test for equality of
		values. This callbacks parameter itself may be NULL, which is
		treated as if a valid structure of version 0 with all fields
		NULL had been passed in. Otherwise,
		if any of the fields are not valid pointers to functions
		of the correct type, or this parameter is not a valid
		pointer to a CFDictionaryValueCallBacks callbacks structure,
		the behavior is undefined. If any of the values put into the
		dictionary is not one understood by one of the callback functions
		the behavior when that callback function is used is undefined.
	@result A reference to the new immutable CFDictionary.
*/
CF_EXPORT
CFDictionaryRef CFDictionaryCreate(CFAllocatorRef allocator, const void **keys, const void **values, CFIndex numValues, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks);

/*!
	@function CFDictionaryCreateCopy
	Creates a new immutable dictionary with the key-value pairs from
		the given dictionary.
	@param allocator The CFAllocator which should be used to allocate
		memory for the dictionary and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
	@param theDict The dictionary which is to be copied. The keys and values
		from the dictionary are copied as pointers into the new
		dictionary (that is, the values themselves are copied, not
		that which the values point to, if anything). However, the
		keys and values are also retained by the new dictionary using
		the retain function of the original dictionary.
		The count of the new dictionary will be the same as the
		given dictionary. The new dictionary uses the same callbacks
		as the dictionary to be copied. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@result A reference to the new immutable CFDictionary.
*/
CF_EXPORT
CFDictionaryRef CFDictionaryCreateCopy(CFAllocatorRef allocator, CFDictionaryRef theDict);

/*!
	@function CFDictionaryCreateMutable
	Creates a new mutable dictionary.
	@param allocator The CFAllocator which should be used to allocate
		memory for the dictionary and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
        @param capacity A hint about the number of values that will be held
                by the CFDictionary. Pass 0 for no hint. The implementation may
                ignore this hint, or may use it to optimize various
                operations. A dictionary's actual capacity is only limited by 
                address space and available memory constraints). If this 
                parameter is negative, the behavior is undefined.
	@param keyCallBacks A pointer to a CFDictionaryKeyCallBacks structure
		initialized with the callbacks for the dictionary to use on
		each key in the dictionary. A copy of the contents of the
		callbacks structure is made, so that a pointer to a structure
		on the stack can be passed in, or can be reused for multiple
		dictionary creations. If the version field of this
		callbacks structure is not one of the defined ones for
		CFDictionary, the behavior is undefined. The retain field may
		be NULL, in which case the CFDictionary will do nothing to add
		a retain to the keys of the contained values. The release field
		may be NULL, in which case the CFDictionary will do nothing
		to remove the dictionary's retain (if any) on the keys when the
		dictionary is destroyed or a key-value pair is removed. If the
		copyDescription field is NULL, the dictionary will create a
		simple description for a key. If the equal field is NULL, the
		dictionary will use pointer equality to test for equality of
		keys. If the hash field is NULL, a key will be converted from
		a pointer to an integer to compute the hash code. This callbacks
		parameter itself may be NULL, which is treated as if a valid
		structure of version 0 with all fields NULL had been passed in.
		Otherwise, if any of the fields are not valid pointers to
		functions of the correct type, or this parameter is not a
		valid pointer to a CFDictionaryKeyCallBacks callbacks structure,
		the behavior is undefined. If any of the keys put into the
		dictionary is not one understood by one of the callback functions
		the behavior when that callback function is used is undefined.
	@param valueCallBacks A pointer to a CFDictionaryValueCallBacks structure
		initialized with the callbacks for the dictionary to use on
		each value in the dictionary. The retain callback will be used
		within this function, for example, to retain all of the new
		values from the values C array. A copy of the contents of the
		callbacks structure is made, so that a pointer to a structure
		on the stack can be passed in, or can be reused for multiple
		dictionary creations. If the version field of this callbacks
		structure is not one of the defined ones for CFDictionary, the
		behavior is undefined. The retain field may be NULL, in which
		case the CFDictionary will do nothing to add a retain to values
		as they are put into the dictionary. The release field may be
		NULL, in which case the CFDictionary will do nothing to remove
		the dictionary's retain (if any) on the values when the
		dictionary is destroyed or a key-value pair is removed. If the
		copyDescription field is NULL, the dictionary will create a
		simple description for a value. If the equal field is NULL, the
		dictionary will use pointer equality to test for equality of
		values. This callbacks parameter itself may be NULL, which is
		treated as if a valid structure of version 0 with all fields
		NULL had been passed in. Otherwise,
		if any of the fields are not valid pointers to functions
		of the correct type, or this parameter is not a valid
		pointer to a CFDictionaryValueCallBacks callbacks structure,
		the behavior is undefined. If any of the values put into the
		dictionary is not one understood by one of the callback functions
		the behavior when that callback function is used is undefined.
	@result A reference to the new mutable CFDictionary.
*/
CF_EXPORT
CFMutableDictionaryRef CFDictionaryCreateMutable(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks);

/*!
	@function CFDictionaryCreateMutableCopy
	Creates a new mutable dictionary with the key-value pairs from
		the given dictionary.
	@param allocator The CFAllocator which should be used to allocate
		memory for the dictionary and its storage for values. This
		parameter may be NULL in which case the current default
		CFAllocator is used. If this reference is not a valid
		CFAllocator, the behavior is undefined.
        @param capacity A hint about the number of values that will be held
                by the CFDictionary. Pass 0 for no hint. The implementation may
                ignore this hint, or may use it to optimize various
                operations. A dictionary's actual capacity is only limited by
                address space and available memory constraints). 
                This parameter must be greater than or equal
                to the count of the dictionary which is to be copied, or the
                behavior is undefined. If this parameter is negative, the
                behavior is undefined.
	@param theDict The dictionary which is to be copied. The keys and values
		from the dictionary are copied as pointers into the new
		dictionary (that is, the values themselves are copied, not
		that which the values point to, if anything). However, the
		keys and values are also retained by the new dictionary using
		the retain function of the original dictionary.
		The count of the new dictionary will be the same as the
		given dictionary. The new dictionary uses the same callbacks
		as the dictionary to be copied. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@result A reference to the new mutable CFDictionary.
*/
CF_EXPORT
CFMutableDictionaryRef CFDictionaryCreateMutableCopy(CFAllocatorRef allocator, CFIndex capacity, CFDictionaryRef theDict);

/*!
	@function CFDictionaryGetCount
	Returns the number of values currently in the dictionary.
	@param theDict The dictionary to be queried. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@result The number of values in the dictionary.
*/
CF_EXPORT
CFIndex CFDictionaryGetCount(CFDictionaryRef theDict);

/*!
	@function CFDictionaryGetCountOfKey
	Counts the number of times the given key occurs in the dictionary.
	@param theDict The dictionary to be searched. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param key The key for which to find matches in the dictionary. The
		hash() and equal() key callbacks provided when the dictionary
		was created are used to compare. If the hash() key callback
		was NULL, the key is treated as a pointer and converted to
		an integer. If the equal() key callback was NULL, pointer
		equality (in C, ==) is used. If key, or any of the keys in
		the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@result Returns 1 if a matching key is used by the dictionary,
		0 otherwise.
*/
CF_EXPORT
CFIndex CFDictionaryGetCountOfKey(CFDictionaryRef theDict, const void *key);

/*!
	@function CFDictionaryGetCountOfValue
	Counts the number of times the given value occurs in the dictionary.
	@param theDict The dictionary to be searched. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param value The value for which to find matches in the dictionary. The
		equal() callback provided when the dictionary was created is
		used to compare. If the equal() value callback was NULL, pointer
		equality (in C, ==) is used. If value, or any of the values in
		the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@result The number of times the given value occurs in the dictionary.
*/
CF_EXPORT
CFIndex CFDictionaryGetCountOfValue(CFDictionaryRef theDict, const void *value);

/*!
	@function CFDictionaryContainsKey
	Reports whether or not the key is in the dictionary.
	@param theDict The dictionary to be searched. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param key The key for which to find matches in the dictionary. The
		hash() and equal() key callbacks provided when the dictionary
		was created are used to compare. If the hash() key callback
		was NULL, the key is treated as a pointer and converted to
		an integer. If the equal() key callback was NULL, pointer
		equality (in C, ==) is used. If key, or any of the keys in
		the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@result true, if the key is in the dictionary, otherwise false.
*/
CF_EXPORT
Boolean CFDictionaryContainsKey(CFDictionaryRef theDict, const void *key);

/*!
	@function CFDictionaryContainsValue
	Reports whether or not the value is in the dictionary.
	@param theDict The dictionary to be searched. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param value The value for which to find matches in the dictionary. The
		equal() callback provided when the dictionary was created is
		used to compare. If the equal() callback was NULL, pointer
		equality (in C, ==) is used. If value, or any of the values
		in the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@result true, if the value is in the dictionary, otherwise false.
*/
CF_EXPORT
Boolean CFDictionaryContainsValue(CFDictionaryRef theDict, const void *value);

/*!
	@function CFDictionaryGetValue
	Retrieves the value associated with the given key.
	@param theDict The dictionary to be queried. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param key The key for which to find a match in the dictionary. The
		hash() and equal() key callbacks provided when the dictionary
		was created are used to compare. If the hash() key callback
		was NULL, the key is treated as a pointer and converted to
		an integer. If the equal() key callback was NULL, pointer
		equality (in C, ==) is used. If key, or any of the keys in
		the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@result The value with the given key in the dictionary, or NULL if
		no key-value pair with a matching key exists. Since NULL
		can be a valid value in some dictionaries, the function
		CFDictionaryGetValueIfPresent() must be used to distinguish
		NULL-no-found from NULL-is-the-value.
*/
CF_EXPORT
const void *CFDictionaryGetValue(CFDictionaryRef theDict, const void *key);

/*!
	@function CFDictionaryGetValueIfPresent
	Retrieves the value associated with the given key.
	@param theDict The dictionary to be queried. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param key The key for which to find a match in the dictionary. The
		hash() and equal() key callbacks provided when the dictionary
		was created are used to compare. If the hash() key callback
		was NULL, the key is treated as a pointer and converted to
		an integer. If the equal() key callback was NULL, pointer
		equality (in C, ==) is used. If key, or any of the keys in
		the dictionary, are not understood by the equal() callback,
		the behavior is undefined.
	@param value A pointer to memory which should be filled with the
		pointer-sized value if a matching key is found. If no key
		match is found, the contents of the storage pointed to by
		this parameter are undefined. This parameter may be NULL,
		in which case the value from the dictionary is not returned
		(but the return value of this function still indicates
		whether or not the key-value pair was present).
	@result true, if a matching key was found, false otherwise.
*/
CF_EXPORT
Boolean CFDictionaryGetValueIfPresent(CFDictionaryRef theDict, const void *key, const void **value);

/*!
	@function CFDictionaryGetKeysAndValues
	Fills the two buffers with the keys and values from the dictionary.
	@param theDict The dictionary to be queried. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param keys A C array of pointer-sized values to be filled with keys
		from the dictionary. The keys and values C arrays are parallel
		to each other (that is, the items at the same indices form a
		key-value pair from the dictionary). This parameter may be NULL
		if the keys are not desired. If this parameter is not a valid
		pointer to a C array of at least CFDictionaryGetCount() pointers,
		or NULL, the behavior is undefined.
	@param values A C array of pointer-sized values to be filled with values
		from the dictionary. The keys and values C arrays are parallel
		to each other (that is, the items at the same indices form a
		key-value pair from the dictionary). This parameter may be NULL
		if the values are not desired. If this parameter is not a valid
		pointer to a C array of at least CFDictionaryGetCount() pointers,
		or NULL, the behavior is undefined.
*/
CF_EXPORT
void CFDictionaryGetKeysAndValues(CFDictionaryRef theDict, const void **keys, const void **values);

/*!
	@function CFDictionaryApplyFunction
	Calls a function once for each value in the dictionary.
	@param theDict The dictionary to be queried. If this parameter is
		not a valid CFDictionary, the behavior is undefined.
	@param applier The callback function to call once for each value in
		the dictionary. If this parameter is not a
		pointer to a function of the correct prototype, the behavior
		is undefined. If there are keys or values which the
		applier function does not expect or cannot properly apply
		to, the behavior is undefined. 
	@param context A pointer-sized user-defined value, which is passed
		as the third parameter to the applier function, but is
		otherwise unused by this function. If the context is not
		what is expected by the applier function, the behavior is
		undefined.
*/
CF_EXPORT
void CFDictionaryApplyFunction(CFDictionaryRef theDict, CFDictionaryApplierFunction CF_NOESCAPE applier, void *context);

/*!
	@function CFDictionaryAddValue
	Adds the key-value pair to the dictionary if no such key already exists.
	@param theDict The dictionary to which the value is to be added. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined.
	@param key The key of the value to add to the dictionary. The key is
		retained by the dictionary using the retain callback provided
		when the dictionary was created. If the key is not of the sort
		expected by the retain callback, the behavior is undefined. If
		a key which matches this key is already present in the dictionary,
		this function does nothing ("add if absent").
	@param value The value to add to the dictionary. The value is retained
		by the dictionary using the retain callback provided when the
		dictionary was created. If the value is not of the sort expected
		by the retain callback, the behavior is undefined.
*/
CF_EXPORT
void CFDictionaryAddValue(CFMutableDictionaryRef theDict, const void *key, const void *value);

/*!
	@function CFDictionarySetValue
	Sets the value of the key in the dictionary.
	@param theDict The dictionary to which the value is to be set. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined.
	@param key The key of the value to set into the dictionary. If a key 
		which matches this key is already present in the dictionary, only
		the value is changed ("add if absent, replace if present"). If
		no key matches the given key, the key-value pair is added to the
		dictionary. If added, the key is retained by the dictionary,
		using the retain callback provided
		when the dictionary was created. If the key is not of the sort
		expected by the key retain callback, the behavior is undefined.
	@param value The value to add to or replace into the dictionary. The value
		is retained by the dictionary using the retain callback provided
		when the dictionary was created, and the previous value if any is
		released. If the value is not of the sort expected by the
		retain or release callbacks, the behavior is undefined.
*/
CF_EXPORT
void CFDictionarySetValue(CFMutableDictionaryRef theDict, const void *key, const void *value);

/*!
	@function CFDictionaryReplaceValue
	Replaces the value of the key in the dictionary.
	@param theDict The dictionary to which the value is to be replaced. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined.
	@param key The key of the value to replace in the dictionary. If a key 
		which matches this key is present in the dictionary, the value
		is changed to the given value, otherwise this function does
		nothing ("replace if present").
	@param value The value to replace into the dictionary. The value
		is retained by the dictionary using the retain callback provided
		when the dictionary was created, and the previous value is
		released. If the value is not of the sort expected by the
		retain or release callbacks, the behavior is undefined.
*/
CF_EXPORT
void CFDictionaryReplaceValue(CFMutableDictionaryRef theDict, const void *key, const void *value);

/*!
	@function CFDictionaryRemoveValue
	Removes the value of the key from the dictionary.
	@param theDict The dictionary from which the value is to be removed. If this
		parameter is not a valid mutable CFDictionary, the behavior is
		undefined.
	@param key The key of the value to remove from the dictionary. If a key 
		which matches this key is present in the dictionary, the key-value
		pair is removed from the dictionary, otherwise this function does
		nothing ("remove if present").
*/
CF_EXPORT
void CFDictionaryRemoveValue(CFMutableDictionaryRef theDict, const void *key);

/*!
	@function CFDictionaryRemoveAllValues
	Removes all the values from the dictionary, making it empty.
	@param theDict The dictionary from which all of the values are to be
		removed. If this parameter is not a valid mutable
		CFDictionary, the behavior is undefined.
*/
CF_EXPORT
void CFDictionaryRemoveAllValues(CFMutableDictionaryRef theDict);

CF_EXTERN_C_END
CF_IMPLICIT_BRIDGING_DISABLED

#endif /* ! __COREFOUNDATION_CFDICTIONARY__ */

