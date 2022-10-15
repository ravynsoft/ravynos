/*	CFCollections_Internal.h
	Copyright (c) 1998-2019, Apple Inc. and the Swift project authors
 
	Portions Copyright (c) 2014-2019, Apple Inc. and the Swift project authors
	Licensed under Apache License v2.0 with Runtime Library Exception
	See http://swift.org/LICENSE.txt for license information
	See http://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
*/

#if !defined(__COREFOUNDATION_CFCOLLECTIONS_INTERNAL__)
#define __COREFOUNDATION_CFCOLLECTIONS_INTERNAL__

#include <CoreFoundation/CFBase.h>
#include <CoreFoundation/CFDictionary.h>
#include "CFInternal.h"

CF_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

CF_PRIVATE void CFDictionaryApply(CFDictionaryRef theDict, void (^block)(const void * _Nullable key, const void * _Nullable value, Boolean *stop));

CF_ASSUME_NONNULL_END
CF_EXTERN_C_END

#endif /* __COREFOUNDATION_CFCOLLECTIONS_INTERNAL__ */
