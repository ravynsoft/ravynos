#ifndef __OBJECTIVEC2_RUNTIME_H_INCLUDED__
#define __OBJECTIVEC2_RUNTIME_H_INCLUDED__

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <stddef.h>
#include <sys/types.h>

#include "ObjectiveC2/objc/Availability.h"

#if   defined(ERROR_UNSUPPORTED_RUNTIME_FUNCTIONS)
#  define OBJC_GNU_RUNTIME_UNSUPPORTED(x) \
__attribute__((error(x " not supported by the GNU runtime")))
#else
#  define OBJC_GNU_RUNTIME_UNSUPPORTED(x)
#endif

#if !defined(OBJC_DEPRECATED)
#  if !defined(__DEPRECATE_DIRECT_ACCESS) || defined(__OBJC_LEGACY_GNU_MODE__) || defined(__OBJC_RUNTIME_INTERNAL__)
#    define OBJC_DEPRECATED
#  else
#    if ((__GNUC__ > 3) || (__GNUC__ == 3 && __GNUC_MINOR >= 1))
#      define OBJC_DEPRECATED __attribute__((deprecated))
#    else
#      define OBJC_DEPRECATED
#    endif
#  endif
#endif

// Undo GNUstep substitutions
#ifdef class_setVersion
#undef class_setVersion
#endif
#ifdef class_getClassMethod
#undef class_getClassMethod
#endif
#ifdef objc_getClass
#undef objc_getClass
#endif
#ifdef objc_lookUpClass
#undef objc_lookUpClass
#endif

typedef struct objc_ivar* Ivar;

#ifndef __objc_INCLUDE_GNU
typedef const struct objc_selector *SEL;

typedef struct objc_class *Class;

typedef struct objc_object
{
  Class isa;
} *id;

struct objc_super {
  id receiver;
#  if !defined(__cplusplus)  &&  !__OBJC2__
  Class class;
#  else
  Class super_class;
#  endif
};

typedef id (*IMP)(id, SEL, ...);
typedef struct objc_method *Method;

#  ifdef STRICT_APPLE_COMPATIBILITY
typedef signed char BOOL;
#  else
#    ifdef __vxwords
typedef  int BOOL
#    else
typedef unsigned char BOOL;
#    endif
#  endif

#else
// Method in the GNU runtime is a struct, Method_t is the pointer
#	define Method Method_t
#endif // __objc_INCLUDE_GNU



typedef void *objc_property_t;
#ifdef __OBJC__
@class Protocol;
#else
typedef struct objc_protocol Protocol;
#endif

#ifndef YES
#define YES ((BOOL)1)
#endif
#ifndef NO
#define NO ((BOOL)0)
#endif

#ifdef __GNUC
#define _OBJC_NULL_PTR __null
#elif defined(__cplusplus)
#define _OBJC_NULL_PTR 0
#else
#define _OBJC_NULL_PTR ((void*)0)
#endif

#ifndef nil
#define nil ((id)_OBJC_NULL_PTR)
#endif

#ifndef Nil
#define Nil ((Class)_OBJC_NULL_PTR)
#endif

BOOL class_addIvar(Class cls,
                   const char *name,
                   size_t size,
                   uint8_t alignment,
                   const char *types);

BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);

BOOL class_addProtocol(Class cls, Protocol *protocol);

BOOL class_conformsToProtocol(Class cls, Protocol *protocol);

Ivar * class_copyIvarList(Class cls, unsigned int *outCount);

Method * class_copyMethodList(Class cls, unsigned int *outCount);

OBJC_GNU_RUNTIME_UNSUPPORTED("Property introspection")
objc_property_t * class_copyPropertyList(Class cls, unsigned int *outCount);

Protocol ** class_copyProtocolList(Class cls, unsigned int *outCount);

id class_createInstance(Class cls, size_t extraBytes);

Method class_getClassMethod(Class aClass, SEL aSelector);

Ivar class_getClassVariable(Class cls, const char* name);

Method class_getInstanceMethod(Class aClass, SEL aSelector);

size_t class_getInstanceSize(Class cls);

Ivar class_getInstanceVariable(Class cls, const char* name);

const char *class_getIvarLayout(Class cls);

IMP class_getMethodImplementation(Class cls, SEL name);

IMP class_getMethodImplementation_stret(Class cls, SEL name);

const char * class_getName(Class cls);

OBJC_GNU_RUNTIME_UNSUPPORTED("Property introspection")
objc_property_t class_getProperty(Class cls, const char *name);

Class class_getSuperclass(Class cls);

int class_getVersion(Class theClass);

OBJC_GNU_RUNTIME_UNSUPPORTED("Weak instance variables")
const char *class_getWeakIvarLayout(Class cls);

BOOL class_isMetaClass(Class cls);

IMP class_replaceMethod(Class cls, SEL name, IMP imp, const char *types);

BOOL class_respondsToSelector(Class cls, SEL sel);

void class_setIvarLayout(Class cls, const char *layout);

OBJC_DEPRECATED
Class class_setSuperclass(Class cls, Class newSuper);

void class_setVersion(Class theClass, int version);

OBJC_GNU_RUNTIME_UNSUPPORTED("Weak instance variables")
void class_setWeakIvarLayout(Class cls, const char *layout);

const char * ivar_getName(Ivar ivar);

ptrdiff_t ivar_getOffset(Ivar ivar);

const char * ivar_getTypeEncoding(Ivar ivar);

char * method_copyArgumentType(Method method, unsigned int index);

char * method_copyReturnType(Method method);

void method_exchangeImplementations(Method m1, Method m2);

void method_getArgumentType(Method method, unsigned int index, char *dst, size_t dst_len);

IMP method_getImplementation(Method method);

SEL method_getName(Method method);

unsigned method_getNumberOfArguments(Method method);

void method_getReturnType(Method method, char *dst, size_t dst_len);

const char * method_getTypeEncoding(Method method);

IMP method_setImplementation(Method method, IMP imp);

Class
objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);

OBJC_NONPORTABLE
Class objc_allocateMetaClass(Class superclass, size_t extraBytes);

void objc_disposeClassPair(Class cls);

id objc_getClass(const char *name);

int objc_getClassList(Class *buffer, int bufferLen);

id objc_getMetaClass(const char *name);

id objc_getRequiredClass(const char *name);

id objc_lookUpClass(const char *name);

Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);

Protocol *objc_getProtocol(const char *name);

void objc_registerClassPair(Class cls);

void *object_getIndexedIvars(id obj);

// FIXME: The GNU runtime has a version of this which omits the size parameter
//id object_copy(id obj, size_t size);

id object_dispose(id obj);

Class object_getClass(id obj);
Class object_setClass(id obj, Class cls);

const char *object_getClassName(id obj);

IMP objc_msg_lookup(id, SEL);
IMP objc_msg_lookup_super(struct objc_super*, SEL);

OBJC_GNU_RUNTIME_UNSUPPORTED("Protocol introspection")
Protocol **objc_copyProtocolList(unsigned int *count);

BOOL protocol_conformsToProtocol(Protocol *p, Protocol *other);

struct objc_method_description *protocol_copyMethodDescriptionList(Protocol *p,
  BOOL isRequiredMethod, BOOL isInstanceMethod, unsigned int *count);

OBJC_GNU_RUNTIME_UNSUPPORTED("Protocol introspection")
objc_property_t *protocol_copyPropertyList(Protocol *p, unsigned int *count);

Protocol **protocol_copyProtocolList(Protocol *p, unsigned int *count);

struct objc_method_description protocol_getMethodDescription(Protocol *p,
  SEL aSel, BOOL isRequiredMethod, BOOL isInstanceMethod);

const char *protocol_getName(Protocol *p);

OBJC_GNU_RUNTIME_UNSUPPORTED("Protocol introspection")
objc_property_t protocol_getProperty(Protocol *p, const char *name,
  BOOL isRequiredProperty, BOOL isInstanceProperty);

BOOL protocol_isEqual(Protocol *p, Protocol *other);

// Only enable support for object planes when
// -fobjc-sender-dependent-dispatch is specified
#ifdef __OBJC_SENDER_AWARE_DISPATCH__

// Global self so that self is a valid symbol everywhere.  Will be replaced by
// a real self in an inner scope if there is one.
static const id self = nil;
#define objc_msgSend(theReceiver, theSelector, ...) objc_msg_lookup_sender(theReceiver, theSelector, self)(theReceiver, theSelector, ## __VA_ARGS__)

#endif

#if (GCC_VERSION >= 3000)

#define objc_msgSend(theReceiver, theSelector, ...) objc_msg_lookup(theReceiver, theSelector)(theReceiver, theSelector, ## __VA_ARGS__)
#define objc_msgSendSuper(super, op, ...) objc_msg_lookup_super(super, op)(super->receiver, op, ## __VA_ARGS__)

#else /* GCC_VERSION >= 3000 */

#define objc_msgSend(theReceiver, theSelector, args...) objc_msg_lookup(theReceiver, theSelector)(theReceiver, theSelector, args)
#define objc_msgSendSuper(super, op, args...) objc_msg_lookup_super(super, op)(super->receiver, op, args)

#endif /* GCC_VERSION >= 3000 */


const char *sel_getName(SEL sel);

SEL sel_getUid(const char *selName);

BOOL sel_isEqual(SEL sel1, SEL sel2);

SEL sel_registerName(const char *selName);

#endif // __OBJECTIVEC2_RUNTIME_H_INCLUDED
