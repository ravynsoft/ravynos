/* Copyright (c) 2008-2009 Christopher J. W. Lloyd

Permission is hereby granted,free of charge,to any person obtaining a copy of this software and associated documentation files (the "Software"),to deal in the Software without restriction,including without limitation the rights to use,copy,modify,merge,publish,distribute,sublicense,and/or sell copies of the Software,and to permit persons to whom the Software is furnished to do so,subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS",WITHOUT WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,DAMAGES OR OTHER LIABILITY,WHETHER IN AN ACTION OF CONTRACT,TORT OR OTHERWISE,ARISING FROM,OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

typedef struct __CFPlugIn *CFPlugInRef;

#import <CoreFoundation/CFBase.h>
#import <CoreFoundation/CFUUID.h>
#import <CoreFoundation/CFBundle.h>

typedef void (*CFPlugInDynamicRegisterFunction)(CFPlugInRef self);
typedef void *(*CFPlugInFactoryFunction)(CFAllocatorRef allocator, CFUUIDRef type);
typedef void (*CFPlugInUnloadFunction)(CFPlugInRef self);

COREFOUNDATION_EXPORT const CFStringRef kCFPlugInDynamicRegistrationKey;
COREFOUNDATION_EXPORT const CFStringRef kCFPlugInDynamicRegisterFunctionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFPlugInUnloadFunctionKey;
COREFOUNDATION_EXPORT const CFStringRef kCFPlugInFactoriesKey;
COREFOUNDATION_EXPORT const CFStringRef kCFPlugInTypesKey;

COREFOUNDATION_EXPORT CFTypeID CFPlugInGetTypeID(void);

COREFOUNDATION_EXPORT Boolean CFPlugInRegisterPlugInType(CFUUIDRef factory, CFUUIDRef type);
COREFOUNDATION_EXPORT Boolean CFPlugInUnregisterFactory(CFUUIDRef factory);
COREFOUNDATION_EXPORT void CFPlugInAddInstanceForFactory(CFUUIDRef factory);
COREFOUNDATION_EXPORT CFArrayRef CFPlugInFindFactoriesForPlugInType(CFUUIDRef type);
COREFOUNDATION_EXPORT CFArrayRef CFPlugInFindFactoriesForPlugInTypeInPlugIn(CFUUIDRef type, CFPlugInRef self);
COREFOUNDATION_EXPORT void *CFPlugInInstanceCreate(CFAllocatorRef allocator, CFUUIDRef factory, CFUUIDRef type);
COREFOUNDATION_EXPORT Boolean CFPlugInRegisterFactoryFunction(CFUUIDRef factory, CFPlugInFactoryFunction function);
COREFOUNDATION_EXPORT Boolean CFPlugInRegisterFactoryFunctionByName(CFUUIDRef factory, CFPlugInRef self, CFStringRef name);
COREFOUNDATION_EXPORT void CFPlugInRemoveInstanceForFactory(CFUUIDRef factory);
COREFOUNDATION_EXPORT Boolean CFPlugInUnregisterPlugInType(CFUUIDRef factory, CFUUIDRef type);

COREFOUNDATION_EXPORT CFPlugInRef CFPlugInCreate(CFAllocatorRef allocator, CFURLRef url);

COREFOUNDATION_EXPORT CFBundleRef CFPlugInGetBundle(CFPlugInRef self);
COREFOUNDATION_EXPORT Boolean CFPlugInIsLoadOnDemand(CFPlugInRef self);
COREFOUNDATION_EXPORT void CFPlugInSetLoadOnDemand(CFPlugInRef self, Boolean flag);
