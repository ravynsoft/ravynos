#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif
#include "objc-visibility.h"

#ifndef __LIBOBJC_RUNTIME_H_INCLUDED__
#define __LIBOBJC_RUNTIME_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GNUSTEP_RUNTIME__
#	define __GNUSTEP_RUNTIME__
#endif

#ifndef __has_feature
#	define __has_feature(x) 0
#endif

#ifndef __unsafe_unretained
#	ifndef __has_feature
#		define __unsafe_unretained
#	elif !__has_feature(objc_arc)
#		define __unsafe_unretained
#	endif
#endif

// Make sure we get the limit macros, even in C++ mode
#ifndef __STDC_LIMIT_MACROS                                                     
#	define __STDC_LIMIT_MACROS 1
#endif

#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <sys/types.h>
#include "Availability.h"

// Undo GNUstep substitutions
#ifdef class_setVersion
#	undef class_setVersion
#endif
#ifdef class_getClassMethod
#	undef class_getClassMethod
#endif
#ifdef objc_getClass
#	undef objc_getClass
#endif
#ifdef objc_lookUpClass
#	undef objc_lookUpClass
#endif

/**
 * Opaque type for Objective-C instance variable metadata.
 */
typedef struct objc_ivar* Ivar;

// Don't redefine these types if the old GCC header was included first.
#ifndef __objc_INCLUDE_GNU
// Define the macro so that including the old GCC header does nothing.
#	define __objc_INCLUDE_GNU
#	define __objc_api_INCLUDE_GNU


/**
 * Opaque type used for selectors.
 */
#if !defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
typedef const struct objc_selector *SEL;
#else
typedef struct objc_selector *SEL;
#endif

/**
 * Opaque type for Objective-C classes.
 */
typedef struct objc_class *Class;

/**
 * Type for Objective-C objects.
 */
typedef struct objc_object
{
	/**
	 * Pointer to this object's class.  Accessing this directly is STRONGLY
	 * discouraged.  You are recommended to use object_getClass() instead.
	 */
#ifndef __OBJC_RUNTIME_INTERNAL__
	__attribute__((deprecated))
#endif
	Class isa;
} *id;

/**
 * Structure used for calling superclass methods.
 */
struct objc_super
{
	/** The receiver of the message. */
	__unsafe_unretained id receiver;
	/** The class containing the method to call. */
#	if !defined(__cplusplus)  &&  !__OBJC2__
	Class class;
#	else
	Class super_class;
#	endif
};

/**
 * Instance Method Pointer type.  Note: Since the calling convention for
 * variadic functions sometimes differs from the calling convention for
 * non-variadic functions, you must cast an IMP to the correct type before
 * calling.
 */
typedef id (*IMP)(id, SEL, ...);
/**
 * Opaque type for Objective-C method metadata.
 */
typedef struct objc_method *Method;

/**
 * Objective-C boolean type.
 */
#	ifdef STRICT_APPLE_COMPATIBILITY
typedef signed char BOOL;
#	else
#		if defined(__vxworks) || defined(_WIN32)
typedef  int BOOL;
#		else
typedef unsigned char BOOL;
#		endif
#	endif

#else
// Method in the GCC runtime is a struct, Method_t is the pointer
#	define Method Method_t
#endif // __objc_INCLUDE_GNU


/**
 * Opaque type for Objective-C property metadata.
 */
typedef struct objc_property* objc_property_t;
/**
 * Opaque type for Objective-C protocols.  Note that, although protocols are
 * objects, sending messages to them is deprecated in Objective-C 2 and may not
 * work in the future.
 */
#ifdef __OBJC__
@class Protocol;
#else
typedef struct objc_protocol Protocol;
#endif

/**
 * Objective-C method description.
 */
struct objc_method_description
{
	/**
	 * The name of this method.
	 */
	SEL   name;
	/**
	 * The types of this method.
	 */
	const char *types;
};

/**
 * The objc_property_attribute_t type is used to store attributes for
 * properties.  This is used to store a decomposed version of the property
 * encoding, with each flag stored in the name and each value in the value.
 *
 * All of the strings that these refer to are internal to the runtime and
 * should not be freed.
 */
typedef struct
{
	/**
	 * The flag that this attribute describes.  All current flags are single characters,
	 */
	const char *name;
	/**
	 */
	const char *value;
} objc_property_attribute_t;



#ifndef YES
#	define YES ((BOOL)1)
#endif
#ifndef NO
#	define NO ((BOOL)0)
#endif

#ifdef __GNUC
#	define _OBJC_NULL_PTR __null
#elif defined(__cplusplus)
#	if __has_feature(cxx_nullptr)
#		define _OBJC_NULL_PTR nullptr
#	else
#		define _OBJC_NULL_PTR 0
#	endif
#else
#	define _OBJC_NULL_PTR ((void*)0)
#endif

#ifndef nil
#	define nil ((id)_OBJC_NULL_PTR)
#endif

#ifndef Nil
#	define Nil ((Class)_OBJC_NULL_PTR)
#endif

#include "slot.h"
#include "message.h"


/**
 * Adds an instance variable to the named class.  The class must not have been
 * registered by the runtime.  The alignment must be the base-2 logarithm of
 * the alignment requirement and the types should be an Objective-C type encoding.
 */
OBJC_PUBLIC
BOOL class_addIvar(Class cls,
                   const char *name,
                   size_t size,
                   uint8_t alignment,
                   const char *types);

/**
 * Adds a method to the class.
 */
OBJC_PUBLIC
BOOL class_addMethod(Class cls, SEL name, IMP imp, const char *types);

/**
 * Adds a protocol to the class.
 */
OBJC_PUBLIC
BOOL class_addProtocol(Class cls, Protocol *protocol);

/**
 * Tests for protocol conformance.  Note: Currently, protocols with the same
 * name are regarded as equivalent, even if they have different methods.  This
 * behaviour will change in a future version.
 */
OBJC_PUBLIC
BOOL class_conformsToProtocol(Class cls, Protocol *protocol);

/**
 * Copies the instance variable list for this class.  The integer pointed to by
 * the outCount argument is set to the number of instance variables returned.
 * The caller is responsible for freeing the returned buffer.
 */
OBJC_PUBLIC
Ivar* class_copyIvarList(Class cls, unsigned int *outCount);

/**
 * Copies the method list for this class.  The integer pointed to by the
 * outCount argument is set to the number of methods returned.  The caller is
 * responsible for freeing the returned buffer.
 */
OBJC_PUBLIC
Method * class_copyMethodList(Class cls, unsigned int *outCount);

/**
 * Copies the declared property list for this class.  The integer pointed to by
 * the outCount argument is set to the number of declared properties returned.
 * The caller is responsible for freeing the returned buffer.
 */
OBJC_PUBLIC
objc_property_t* class_copyPropertyList(Class cls, unsigned int *outCount);

/**
 * Copies the protocol list for this class.  The integer pointed to by the
 * outCount argument is set to the number of protocols returned.  The caller is
 * responsible for freeing the returned buffer.
 */
OBJC_PUBLIC
Protocol *__unsafe_unretained* class_copyProtocolList(Class cls, unsigned int *outCount);

/**
 * Creates an instance of this class, allocating memory using malloc.
 */
OBJC_PUBLIC
id class_createInstance(Class cls, size_t extraBytes);

/**
 * Returns a pointer to the method metadata for the specified method in this
 * class.  This is an opaque data type and must be accessed with the method_*()
 * family of functions.
 */
OBJC_PUBLIC
Method class_getClassMethod(Class aClass, SEL aSelector);

/**
 * Returns a pointer to the  metadata for the specified class variable in
 * this class.  This is an opaque data type and must be accessed with the
 * ivar_*() family of functions.
 */
OBJC_PUBLIC
Ivar class_getClassVariable(Class cls, const char* name);

/**
 * Returns a pointer to the method metadata for the specified instance method
 * in this class.  This is an opaque data type and must be accessed with the
 * method_*() family of functions.
 */
OBJC_PUBLIC
Method class_getInstanceMethod(Class aClass, SEL aSelector);

/**
 * Returns the size of an instance of the named class, in bytes.  All of the
 * class's superclasses must be loaded before this call, or the result is
 * undefined with the non-fragile ABI.
 */
OBJC_PUBLIC
size_t class_getInstanceSize(Class cls);

/**
 * Look up the named instance variable in the class (and its superclasses)
 * returning a pointer to the instance variable definition or a null
 * pointer if no instance variable of that name was found.
 */
OBJC_PUBLIC
Ivar class_getInstanceVariable(Class cls, const char* name);

/**
 * Sets the object value of a specified instance variable.
 */
OBJC_PUBLIC
void object_setIvar(id object, Ivar ivar, id value);
/**
 * Sets a named instance variable to the value specified by *value.  Note that
 * the instance variable must be a pointer-sized quantity.
 */
OBJC_PUBLIC
Ivar object_setInstanceVariable(id obj, const char *name, void *value);

/**
 * Returns the value of the named instance variable.  This should not be used
 * with instance variables that are not pointers.
 */
OBJC_PUBLIC
id object_getIvar(id object, Ivar ivar);

/**
 * Returns a named instance variable via the final parameter.  Note that
 * calling object_getIvar() on the value returned from this function is faster.
 *
 * Note that the instance variable must be a pointer-sized quantity.
 */
OBJC_PUBLIC
Ivar object_getInstanceVariable(id obj, const char *name, void **outValue);

/**
 * Returns a pointer to the function used to handle the specified message.  If
 * the receiver does not have a method corresponding to this message then this
 * function may return a runtime function that performs forwarding.
 */
OBJC_PUBLIC
IMP class_getMethodImplementation(Class cls, SEL name);

/**
 * Identical to class_getMethodImplementation().
 */
OBJC_PUBLIC
IMP class_getMethodImplementation_stret(Class cls, SEL name);

/**
 * Returns the name of the class.  This string is owned by the runtime and is
 * valid for (at least) as long as the class remains loaded.
 */
OBJC_PUBLIC
const char * class_getName(Class cls);

/**
 * Retrieves metadata about the property with the specified name.
 */
OBJC_PUBLIC
objc_property_t class_getProperty(Class cls, const char *name);

/**
 * Returns the superclass of the specified class.
 */
OBJC_PUBLIC
Class class_getSuperclass(Class cls);

/**
 * Returns the version of the class.  Currently, the class version is not used
 * inside the runtime at all, however it may be used for the developer-mode ABI.
 */
OBJC_PUBLIC
int class_getVersion(Class theClass);

/**
 * Sets the version for this class.
 */
OBJC_PUBLIC
void class_setVersion(Class theClass, int version);

OBJC_PUBLIC OBJC_GNUSTEP_RUNTIME_UNSUPPORTED("Weak instance variables")
const char *class_getWeakIvarLayout(Class cls);

/**
 * Returns whether the class is a metaclass.  This can be used in conjunction
 * with object_getClass() for differentiating between objects and classes.
 */
OBJC_PUBLIC
BOOL class_isMetaClass(Class cls);

/**
 * Registers an alias for the class. Returns YES if the alias could be
 * registered successfully.
 */
OBJC_PUBLIC OBJC_NONPORTABLE
BOOL class_registerAlias_np(Class cls, const char *alias);

/**
 * Replaces the named method with a new implementation.  Note: the GNUstep
 * Objective-C runtime uses typed selectors, however the types of the selector
 * will be ignored and a new selector registered with the specified types.
 */
OBJC_PUBLIC
IMP class_replaceMethod(Class cls, SEL name, IMP imp, const char *types);

/**
 * Returns YES if instances of this class has a method that implements the
 * specified message, NO otherwise.  If the class handles this message via one
 * or more of the various forwarding mechanisms, then this will still return
 * NO.
 */
OBJC_PUBLIC
BOOL class_respondsToSelector(Class cls, SEL sel);

/**
 * Returns the instance variable layout of this class as an opaque list that
 * can be applied to other classes.
 */
OBJC_PUBLIC
const char *class_getIvarLayout(Class cls);
/**
 * Sets the class's instance variable layout.  The layout argument must be a
 * value returned by class_getIvarLayout().
 */
OBJC_PUBLIC
void class_setIvarLayout(Class cls, const char *layout);

/**
 * Sets the superclass of the specified class.  This function is deprecated,
 * because modifying the superclass of a class at run time is a very complex
 * operation and this function is almost always used incorrectly.
 */
OBJC_PUBLIC __attribute__((deprecated))
Class class_setSuperclass(Class cls, Class newSuper);

OBJC_PUBLIC OBJC_GNUSTEP_RUNTIME_UNSUPPORTED("Weak instance variables")
void class_setWeakIvarLayout(Class cls, const char *layout);

/**
 * Returns the name of an instance variable.
 */
OBJC_PUBLIC
const char* ivar_getName(Ivar ivar);

/**
 * Returns the offset of an instance variable.  This value can be added to the
 * object pointer to get the address of the instance variable.
 */
OBJC_PUBLIC
ptrdiff_t ivar_getOffset(Ivar ivar);

/**
 * Returns the Objective-C type encoding of the instance variable.
 */
OBJC_PUBLIC
const char* ivar_getTypeEncoding(Ivar ivar);

/**
 * Copies the type encoding of an argument of this method.  The caller is
 * responsible for freeing the returned C string.  Arguments 0 and 1 of any
 * Objective-C method will be the self and _cmd parameters, so the returned
 * value will be "@" and ":" respectively.
 */
OBJC_PUBLIC
char* method_copyArgumentType(Method method, unsigned int index);

/**
 * Copies the type encoding of an argument of this method.  The caller is
 * responsible for freeing the returned C string.
 */
OBJC_PUBLIC
char* method_copyReturnType(Method method);

/**
 * Exchanges the implementations of the two methods.  Note: this call is very
 * expensive on the GNUstep runtime and its use is discouraged.  It is
 * recommended that users call class_replaceMethod() instead.
 */
OBJC_PUBLIC
void method_exchangeImplementations(Method m1, Method m2);

/**
 * Copies the Objective-C type encoding of a specified method parameter into a
 * buffer provided by the caller.  This method does not provide any means for
 * the caller to easily detect truncation, and will only NULL-terminate the
 * output string if there is enough space for the argument type and the NULL
 * terminator.  Its use is therefore discouraged.
 */
OBJC_PUBLIC
void method_getArgumentType(Method method, unsigned int index, char *dst, size_t dst_len);

/**
 * Returns a pointer to the function used to implement this method.
 */
OBJC_PUBLIC
IMP method_getImplementation(Method method);

/**
 * Returns the selector used to identify this method.  Note that, unlike the
 * Apple runtimes, the GNUstep runtime uses typed selectors, so the return
 * value for this also identifies the type of the method, not just its name,
 * although calling method_getTypeEncoding() is faster if you just require the
 * types.
 */
OBJC_PUBLIC
SEL method_getName(Method method);

/**
 * Returns the number of arguments (including self and _cmd) that this method
 * expects.
 */
OBJC_PUBLIC
unsigned method_getNumberOfArguments(Method method);

/**
 * Copies the Objective-C type encoding of a method's return value into a
 * buffer provided by the caller.  This method does not provide any means for
 * the caller to easily detect truncation, and will only NULL-terminate the
 * output string if there is enough space for the argument type and the NULL
 * terminator.  Its use is therefore discouraged.
 */
OBJC_PUBLIC
void method_getReturnType(Method method, char *dst, size_t dst_len);

/**
 * Returns the type encoding for the method.  This string is owned by the
 * runtime and will persist for (at least) as long as the class owning the
 * method is loaded.
 */
OBJC_PUBLIC
const char * method_getTypeEncoding(Method method);

/**
 * Sets the function used to implement this method.  This function is very
 * expensive with the GNUstep runtime and its use is discouraged.  It is
 * recommended that you call class_replaceMethod() instead.
 */
OBJC_PUBLIC
IMP method_setImplementation(Method method, IMP imp);

/**
 * Allocates a new class and metaclass inheriting from the specified class,
 * with some space after the class for storing extra data.  This space can be
 * used for class variables by adding instance variables to the returned
 * metaclass.
 */
OBJC_PUBLIC
Class objc_allocateClassPair(Class superclass, const char *name, size_t extraBytes);

/**
 * Frees a class and metaclass allocated with objc_allocateClassPair().  Any
 * attempts to send messages to instances of this class or its subclasses
 * result in undefined behaviour.
 */
OBJC_PUBLIC
void objc_disposeClassPair(Class cls);

/**
 * Returns the class with the specified name, if one has been registered with
 * the runtime, or nil if one does not exist.  If no class of this name is
 * loaded, it calls the _objc_lookup_class() callback to allow an external
 * library to load the module providing this class.
 */
OBJC_PUBLIC
id objc_getClass(const char *name);

/**
 * Copies all of the classes currently registered with the runtime into the
 * buffer specified as the first argument.  If the buffer is NULL or its length
 * is 0, it returns the total number of classes registered with the runtime.
 * Otherwise, it copies classes and returns the number copied.
 */
OBJC_PUBLIC
int objc_getClassList(Class *buffer, int bufferLen);
/**
 * Returns a copy of the list of all classes in the system.  The caller is
 * responsible for freeing this list.  The number of classes is returned in the
 * parameter.
 */
OBJC_PUBLIC
Class *objc_copyClassList(unsigned int *outCount);

/**
 * Returns the metaclass with the specified name.  This is equivalent to
 * calling object_getClass() on the result of objc_getClass().
 */
OBJC_PUBLIC
id objc_getMetaClass(const char *name);

/**
 * Returns the class with the specified name, aborting if none is found.  This
 * function should generally only be called early on in a program, to ensure
 * that all required libraries are loaded.
 */
OBJC_PUBLIC
id objc_getRequiredClass(const char *name);

/**
 * Looks up the class with the specified name, but does not invoke any
 * external lazy loading mechanisms.
 */
OBJC_PUBLIC
id objc_lookUpClass(const char *name);

/**
 * Returns the protocol with the specified name.
 */
OBJC_PUBLIC
Protocol *objc_getProtocol(const char *name);
/**
 * Allocates a new protocol.  This returns NULL if a protocol with the same
 * name already exists in the system.
 *
 * Protocols are immutable after they have been registered, so may only be
 * modified between calling this function and calling objc_registerProtocol().
 */
OBJC_PUBLIC
Protocol *objc_allocateProtocol(const char *name);
/**
 * Registers a protocol with the runtime.  After this point, the protocol may
 * not be modified.
 */
OBJC_PUBLIC
void objc_registerProtocol(Protocol *proto);
/**
 * Adds a method to the protocol.
 */
OBJC_PUBLIC
void protocol_addMethodDescription(Protocol *aProtocol,
                                   SEL name,
                                   const char *types,
                                   BOOL isRequiredMethod,
                                   BOOL isInstanceMethod);
/**
 * Adds a protocol to the protocol.
 */
OBJC_PUBLIC
void protocol_addProtocol(Protocol *aProtocol, Protocol *addition);
/**
 * Adds a property to the protocol.
 */
OBJC_PUBLIC
void protocol_addProperty(Protocol *aProtocol,
                          const char *name,
                          const objc_property_attribute_t *attributes,
                          unsigned int attributeCount,
                          BOOL isRequiredProperty,
                          BOOL isInstanceProperty);


/**
 * Registers a new class and its metaclass with the runtime.  This function
 * should be called after allocating a class with objc_allocateClassPair() and
 * adding instance variables and methods to it.  A class can not have instance
 * variables added to it after objc_registerClassPair() has been called.
 */
OBJC_PUBLIC
void objc_registerClassPair(Class cls);

/**
 * Returns a pointer immediately after the instance variables declared in an
 * object.  This is a pointer to the storage specified with the extraBytes
 * parameter given when allocating an object.
 */
OBJC_PUBLIC
void *object_getIndexedIvars(id obj);

// FIXME: The GNU runtime has a version of this which omits the size parameter
//id object_copy(id obj, size_t size);

/**
 * Free an object created with class_createInstance().
 */
OBJC_PUBLIC
id object_dispose(id obj);

/**
 * Returns the class of the object.  Note: the isa pointer should not be
 * accessed directly with the GNUstep runtime.
 */
OBJC_PUBLIC
Class object_getClass(id obj);

/**
 * Sets the class of the object.  Note: the isa pointer should not be
 * accessed directly with the GNUstep runtime.
 */
OBJC_PUBLIC
Class object_setClass(id obj, Class cls);

/**
 * Returns the name of the class of the object.  This is equivalent to calling
 * class_getName() on the result of object_getClass().
 */
OBJC_PUBLIC
const char *object_getClassName(id obj);


/**
 * Returns the name of a specified property.
 */
OBJC_PUBLIC
const char *property_getName(objc_property_t property);

/**
 * Returns the attributes for the specified property.  This is similar to an
 * Objective-C type encoding, but contains some extra information.  A full
 * description of the format for this string may be found in Apple's
 * Objective-C Runtime Programming Guide.
 */
OBJC_PUBLIC
const char *property_getAttributes(objc_property_t property);

/**
 * Returns an array of attributes for this property.
 */
OBJC_PUBLIC
objc_property_attribute_t *property_copyAttributeList(objc_property_t property,
                                                      unsigned int *outCount);
/**
 * Adds a property to the class, given a specified set of attributes.  Note
 * that this only sets the property metadata.  The property accessor methods
 * must already be created.
 */
OBJC_PUBLIC
BOOL class_addProperty(Class cls,
                       const char *name,
                       const objc_property_attribute_t *attributes, 
                       unsigned int attributeCount);

/**
 * Replaces property metadata.  If the property does not exist, then this is
 * equivalent to calling class_addProperty().
 */
OBJC_PUBLIC
void class_replaceProperty(Class cls,
                           const char *name,
                           const objc_property_attribute_t *attributes,
                           unsigned int attributeCount);

/**
 * Returns a copy of a single attribute.
 */
OBJC_PUBLIC
char *property_copyAttributeValue(objc_property_t property,
                                  const char *attributeName);

/**
 * Testswhether a protocol conforms to another protocol.
 */
OBJC_PUBLIC
BOOL protocol_conformsToProtocol(Protocol *p, Protocol *other);

/**
 * Returns an array of method descriptions.  Stores the number of elements in
 * the array in the variable pointed to by the last parameter.  The caller is
 * responsible for freeing this array.
 */
OBJC_PUBLIC
struct objc_method_description *protocol_copyMethodDescriptionList(Protocol *p,
	BOOL isRequiredMethod, BOOL isInstanceMethod, unsigned int *count);

/**
 * Returns an array of required instance properties, with the number being
 * stored in the variable pointed to by the last argument.  The caller is
 * responsible for freeing the returned array.
 */
OBJC_PUBLIC
objc_property_t *protocol_copyPropertyList(Protocol *p, unsigned int *count);

/**
 * Returns an array of properties specified by this class, with the number
 * being stored in the variable pointed to by the last argument.  The caller is
 * responsible for freeing the returned array.
 */
OBJC_PUBLIC
objc_property_t *protocol_copyPropertyList2(Protocol *p, unsigned int *count,
	BOOL isRequiredProperty, BOOL isInstanceProperty);

/**
 * Returns an array of protocols that this protocol conforms to, with the
 * number of protocols in the array being returned via the last argument.  The
 * caller is responsible for freeing this array.
 */
OBJC_PUBLIC
Protocol *__unsafe_unretained*protocol_copyProtocolList(Protocol *p, unsigned int *count);

/**
 * Returns all of the protocols that the runtime is aware of.  Note that
 * protocols compiled by GCC and not attacked to classes may not have been
 * registered with the runtime.  The number of protocols returned is stored at
 * the address indicated by the pointer argument.
 *
 * The caller is responsible for freeing the returned array.
 */
OBJC_PUBLIC
Protocol *__unsafe_unretained*objc_copyProtocolList(unsigned int *outCount);
/**
 * Returns the method description for the specified method within a given
 * protocol.
 */
OBJC_PUBLIC
struct objc_method_description protocol_getMethodDescription(Protocol *p,
	SEL aSel, BOOL isRequiredMethod, BOOL isInstanceMethod);

/**
 * Returns the extended type encoding of the specified method.
 * 
 * Note: This function is used by JavaScriptCore but is not public in Apple's
 * implementation and so its semantics may change in the future and this
 * runtime may diverge from Apple's.
 */
OBJC_PUBLIC
const char *_protocol_getMethodTypeEncoding(Protocol *p, SEL aSel,
	BOOL isRequiredMethod, BOOL isInstanceMethod);

/**
 * Returns the name of the specified protocol.
 */
OBJC_PUBLIC
const char* protocol_getName(Protocol *p);

/**
 * Returns the property metadata for the property with the specified name.
 */
OBJC_PUBLIC
objc_property_t protocol_getProperty(Protocol *p, const char *name,
	BOOL isRequiredProperty, BOOL isInstanceProperty);

/**
 * Compares two protocols.  Currently, protocols are assumed to be equal if
 * their names match.  This is required for compatibility with the GCC ABI,
 * which made not attempt to unique protocols (or even register them with the
 * runtime).
 */
OBJC_PUBLIC
BOOL protocol_isEqual(Protocol *p, Protocol *other);

/**
 * The message lookup function used by the GCC ABI.  This returns a pointer to
 * the function (either a method or a forwarding hook) that should be called in
 * response to a given message.
 */
OBJC_PUBLIC
IMP objc_msg_lookup(id, SEL) OBJC_NONPORTABLE;
/**
 * The message lookup function used for messages sent to super in the GCC ABI.
 * This specifies both the class and the 
 */
OBJC_PUBLIC
IMP objc_msg_lookup_super(struct objc_super*, SEL) OBJC_NONPORTABLE;

/**
 * Returns the name of the specified selector.
 */
OBJC_PUBLIC
const char *sel_getName(SEL sel);

/**
 * Registers a selector with the runtime.  This is equivalent to sel_registerName().
 */
OBJC_PUBLIC
SEL sel_getUid(const char *selName);

/**
 * Returns whether two selectors are equal.  For the purpose of comparison,
 * selectors with the same name and type are regarded as equal.  Selectors with
 * the same name and different types are regarded as different.  If one
 * selector is typed and the other is untyped, but the names are the same, then
 * they are regarded as equal.  This means that sel_isEqual(a, b) and
 * sel_isEqual(a, c) does not imply sel_isEqual(b, c) - if a is untyped but
 * both b and c are typed selectors with different types, then then the first
 * two will return YES, but the third case will return NO.
 */
OBJC_PUBLIC
BOOL sel_isEqual(SEL sel1, SEL sel2);

/**
 * Registers an untyped selector with the runtime.
 */
OBJC_PUBLIC
SEL sel_registerName(const char *selName);

/**
 * Register a typed selector.
 */
OBJC_PUBLIC
SEL sel_registerTypedName_np(const char *selName, const char *types) OBJC_NONPORTABLE;

/**
 * Returns the type encoding associated with a selector, or the empty string is
 * there is no such type.
 */
OBJC_PUBLIC
const char *sel_getType_np(SEL aSel) OBJC_NONPORTABLE;

/**
 * Enumerates all of the type encodings associated with a given selector name
 * (up to a specified limit).  This function returns the number of types that
 * exist for a specific selector, but only copies up to count of them into the
 * array passed as the types argument.  This allows you to call the function
 * once with a relatively small on-stack buffer and then only call it again
 * with a heap-allocated buffer if there is not enough space.
 */
OBJC_PUBLIC
unsigned sel_copyTypes_np(const char *selName, const char **types, unsigned count) OBJC_NONPORTABLE;

/**
 * Enumerates all of the type encodings associated with a given selector name
 * (up to a specified limit).  This function returns the number of types that
 * exist for a specific selector, but only copies up to count of them into the
 * array passed as the types argument.  This allows you to call the function
 * once with a relatively small on-stack buffer and then only call it again
 * with a heap-allocated buffer if there is not enough space.
 */
OBJC_PUBLIC
unsigned sel_copyTypedSelectors_np(const char *selName, SEL *const sels, unsigned count) OBJC_NONPORTABLE;

/**
 * New ABI lookup function.  Receiver may be modified during lookup or proxy
 * forwarding and the sender may affect how lookup occurs.
 */
OBJC_PUBLIC
extern struct objc_slot *objc_msg_lookup_sender(id *receiver, SEL selector, id sender)
	OBJC_NONPORTABLE OBJC_DEPRECATED;

/**
 * Deprecated function for accessing a slot without going via any forwarding
 * mechanisms.
 */
OBJC_PUBLIC
extern struct objc_slot *objc_get_slot(Class, SEL)
	OBJC_NONPORTABLE OBJC_DEPRECATED;

/**
 * Look up a slot, without invoking any forwarding mechanisms.  The third
 * parameter is used to return a pointer to the current value of the version
 * counter.  If this value is equal to `objc_method_cache_version` then the
 * slot is safe to reuse without performing another lookup.
 */
OBJC_PUBLIC
extern struct objc_slot2 *objc_get_slot2(Class, SEL, uint64_t*)
	OBJC_NONPORTABLE;

/**
 * Look up a slot, invoking any required forwarding mechanisms.  The third
 * parameter is used to return a pointer to the current value of the version
 * counter.  If this value is equal to `objc_method_cache_version` then the
 * slot is safe to reuse without performing another lookup.
 */
OBJC_PUBLIC
extern struct objc_slot2 *objc_slot_lookup_version(id *receiver, SEL selector, uint64_t*)
	OBJC_NONPORTABLE;

/**
 * Look up a slot, invoking any required forwarding mechanisms.
 */
OBJC_PUBLIC
extern IMP objc_msg_lookup2(id *receiver, SEL selector) OBJC_NONPORTABLE;

/**
 * Registers a class for small objects.  Small objects are stored inside a
 * pointer.  If the class can be registered, then this returns YES.  The second
 * argument specifies the bit pattern to use to identify the small object.
 */
OBJC_PUBLIC
BOOL objc_registerSmallObjectClass_np(Class cls, uintptr_t classId);

/**
 * The mask identifying the bits that can be used in an object pointer to
 * identify a small object.  On 32-bit systems, we use the low bit.  On 64-bit
 * systems, we use the low 3 bits.  In both cases, the lowest bit must be 1.
 * This restriction may be relaxed in the future on 64-bit systems.
 */
#ifndef UINTPTR_MAX
#	define OBJC_SMALL_OBJECT_MASK ((sizeof(void*) == 4) ? 1 : 7)
#elif UINTPTR_MAX < UINT64_MAX
#	define OBJC_SMALL_OBJECT_MASK 1
#else
#	define OBJC_SMALL_OBJECT_MASK 7
#endif
/**
 * The number of bits reserved for the class identifier in a small object.
 */
#ifndef UINTPTR_MAX
#	define OBJC_SMALL_OBJECT_SHIFT ((sizeof(void*) == 4) ? 1 : 3)
#elif UINTPTR_MAX < UINT64_MAX
#	define OBJC_SMALL_OBJECT_SHIFT 1
#else
#	define OBJC_SMALL_OBJECT_SHIFT 3
#endif


/**
 * Valid values for objc_AssociationPolicy.  This is really a bitfield, but
 * only specific combinations of flags are permitted.
 */
enum
{
	/**
	 * Perform straight assignment, no message sends.
	 */
	OBJC_ASSOCIATION_ASSIGN = 0,
	/**
	 * Retain the associated object.
	 */
	OBJC_ASSOCIATION_RETAIN_NONATOMIC = 1,
	/**
	 * Copy the associated object, by sending it a -copy message.
	 */
	OBJC_ASSOCIATION_COPY_NONATOMIC = 3,
	/**
	 * Atomic retain.
	 */
	OBJC_ASSOCIATION_RETAIN = 0x301,
	/**
	 * Atomic copy.
	 */
	OBJC_ASSOCIATION_COPY = 0x303
};
/**
 * Association policy, used when setting associated objects.  
 */
typedef uintptr_t objc_AssociationPolicy;

/**
 * Returns an object previously stored by calling objc_setAssociatedObject()
 * with the same arguments, or nil if none exists.
 */
OBJC_PUBLIC
id objc_getAssociatedObject(id object, const void *key);
/**
 * Associates an object with another.  This provides a mechanism for storing
 * extra state with an object, beyond its declared instance variables.  The
 * pointer used as a key is treated as an opaque value.  The best way of
 * ensuring this is to pass the pointer to a static variable as the key.  The
 * value may be any object, but must respond to -copy or -retain, and -release,
 * if an association policy of copy or retain is passed as the final argument.
 */
OBJC_PUBLIC
void objc_setAssociatedObject(id object, const void *key, id value, objc_AssociationPolicy policy);
/**
 * Removes all associations from an object.  
 */
OBJC_PUBLIC
void objc_removeAssociatedObjects(id object);

/**
 * Converts a block into an IMP that can be used as a method.  The block should
 * take an object pointer (self) as its first argument, and then the same
 * arguments as the method.
 */
OBJC_PUBLIC
IMP imp_implementationWithBlock(id block);
/**
 * Returns the type encoding of an IMP that would be returned by passing the
 * block to imp_implementationWithBlock().  Returns NULL if this is not a valid
 * block encoding for transforming to an IMP (it must take id as its first
 * argument).  The caller is responsible for freeing the returned value.
 */
OBJC_PUBLIC
char *block_copyIMPTypeEncoding_np(id block);
/**
 * Returns the block that was used in an IMP created by
 * imp_implementationWithBlock().  The result of calling this function with any
 * other IMP is undefined.
 */
OBJC_PUBLIC
id imp_getBlock(IMP anImp);
/**
 * Removes a block that was converted to an IMP with
 * imp_implementationWithBlock().  The result of calling this function with any
 * other IMP is undefined.  Returns YES on success, NO on failure.
 */
OBJC_PUBLIC
BOOL imp_removeBlock(IMP anImp);

/**
 * Adds a method to a specific object,  This method will not be added to any
 * other instances of the same class.
 */
OBJC_PUBLIC
BOOL object_addMethod_np(id object, SEL name, IMP imp, const char *types);

/**
 * Replaces a method on a specific object,  This method will not be added to
 * any other instances of the same class.
 */
OBJC_PUBLIC
IMP object_replaceMethod_np(id object, SEL name, IMP imp, const char *types);

/**
 * Creates a clone, in the JavaScript sense - an object which inherits both
 * associated references and methods from the original object.
 */
OBJC_PUBLIC
id object_clone_np(id object);

/**
 * Returns the prototype of the object if it was created with
 * object_clone_np(), or nil otherwise.
 */
OBJC_PUBLIC
id object_getPrototype_np(id object);

/**
 * Toggles whether Objective-C objects caught in C++ exception handlers in
 * Objective-C++ mode should follow Objective-C or C++ semantics.  The obvious
 * choice is for them to follow C++ semantics, because people using a C++
 * language construct would intuitively expect them to have C++ semantics,
 * where the catch behaviour depends on the static type of the thrown object,
 * not its run-time type.
 *
 * Apple, therefore, chose the other option.
 *
 * We default to Apple-compatible mode, but can enable the sane behaviour if
 * the user opts in.  Note that doing this when linking against third-party
 * frameworks written in Objective-C++ 2 may cause weird problems if the expect
 * the other behaviour.
 *
 * This currently sets a global value.  In the future, it may be configurable
 * on a per-thread basis.
 */
OBJC_PUBLIC
int objc_set_apple_compatible_objcxx_exceptions(int newValue) OBJC_NONPORTABLE;


#define _C_ID       '@'
#define _C_CLASS    '#'
#define _C_SEL      ':'
#define _C_BOOL     'B'

#define _C_CHR      'c'
#define _C_UCHR     'C'
#define _C_SHT      's'
#define _C_USHT     'S'
#define _C_INT      'i'
#define _C_UINT     'I'
#define _C_LNG      'l'
#define _C_ULNG     'L'
#define _C_LNG_LNG  'q'
#define _C_ULNG_LNG 'Q'

#define _C_FLT      'f'
#define _C_DBL      'd'

#define _C_BFLD     'b'
#define _C_VOID     'v'
#define _C_UNDEF    '?'
#define _C_PTR      '^'

#define _C_CHARPTR  '*'
#define _C_ATOM     '%'

#define _C_ARY_B    '['
#define _C_ARY_E    ']'
#define _C_UNION_B  '('
#define _C_UNION_E  ')'
#define _C_STRUCT_B '{'
#define _C_STRUCT_E '}'
#define _C_VECTOR   '!'

#define _C_COMPLEX  'j'
#define _C_CONST    'r'
#define _C_IN       'n'
#define _C_INOUT    'N'
#define _C_OUT      'o'
#define _C_BYCOPY   'O'
#define _C_ONEWAY   'V'

#include "runtime-deprecated.h"

#ifdef __cplusplus
}
#endif

#endif // __LIBOBJC_RUNTIME_H_INCLUDED__
