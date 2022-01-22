#ifndef __OBJC_CLASS_H_INCLUDED
#define __OBJC_CLASS_H_INCLUDED
#include "visibility.h"
#include "objc/runtime.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Overflow bitfield.  Used for bitfields that are more than 63 bits.
 */
struct objc_bitfield
{
	/**
	 * The number of elements in the values array.
	 */
	int32_t  length;
	/**
	 * An array of values.  Each 32 bits is stored in the native endian for the
	 * platform.
	 */
	int32_t values[0];
};

static inline BOOL objc_bitfield_test(uintptr_t bitfield, uint64_t field)
{
	if (bitfield & 1)
	{
		uint64_t bit = 1<<(field+1);
		return (bitfield & bit) == bit;
	}
	struct objc_bitfield *bf = (struct objc_bitfield*)bitfield;
	uint64_t byte = field / 32;
	if (byte >= bf->length)
	{
		return NO;
	}
	uint64_t bit = 1<<(field%32);
	return (bf->values[byte] & bit) == bit;
}

// begin: objc_class
struct objc_class
{
	/**
	 * Pointer to the metaclass for this class.  The metaclass defines the
	 * methods use when a message is sent to the class, rather than an
	 * instance.
	 */
	Class                      isa;
	/**
	 * Pointer to the superclass.  The compiler will set this to the name of
	 * the superclass, the runtime will initialize it to point to the real
	 * class.
	 */
	Class                      super_class;
	/**
	 * The name of this class.  Set to the same value for both the class and
	 * its associated metaclass.
	 */
	const char                *name;
	/**
	 * The version of this class.  This is not used by the language, but may be
	 * set explicitly at class load time.
	 */
	long                       version;
	/**
	 * A bitfield containing various flags.  See the objc_class_flags
	 * enumerated type for possible values.  
	 */
	unsigned long              info;
	/**
	 * The size of this class.  For classes using the non-fragile ABI, the
	 * compiler will set this to a negative value The absolute value will be
	 * the size of the instance variables defined on just this class.  When
	 * using the fragile ABI, the instance size is the size of instances of
	 * this class, including any instance variables defined on superclasses.
	 *
	 * In both cases, this will be set to the size of an instance of the class
	 * after the class is registered with the runtime.
	 */
	long                       instance_size;
	/**
	 * Metadata describing the instance variables in this class.
	 */
	struct objc_ivar_list     *ivars;
	/**
	 * Metadata for for defining the mappings from selectors to IMPs.  Linked
	 * list of method list structures, one per class and one per category.
	 */
	struct objc_method_list   *methods;
	/**
	 * The dispatch table for this class.  Intialized and maintained by the
	 * runtime.
	 */
	void                      *dtable;
	/**
	 * A pointer to the first subclass for this class.  Filled in by the
	 * runtime.
	 */
	Class                      subclass_list;
	/**
	 * Pointer to the .cxx_construct method if one exists.  This method needs
	 * to be called outside of the normal dispatch mechanism.
	 */
	IMP                        cxx_construct;
	/**
	 * Pointer to the .cxx_destruct method if one exists.  This method needs to
	 * be called outside of the normal dispatch mechanism.
	 */
	IMP                        cxx_destruct;
	/**
	 * A pointer to the next sibling class to this.  You may find all
	 * subclasses of a given class by following the subclass_list pointer and
	 * then subsequently following the sibling_class pointers in the
	 * subclasses.
	 */
	Class                      sibling_class;

	/**
	 * Metadata describing the protocols adopted by this class.  Not used by
	 * the runtime.
	 */
	struct objc_protocol_list *protocols;
	/**
	 * Linked list of extra data attached to this class.
	 */
	struct reference_list     *extra_data;
	/**
	* The version of the ABI used for this class.  Currently always zero for v2
	* ABI classes.
	*/
	long                       abi_version;
	/**
	* List of declared properties on this class (NULL if none).
	*/
	struct objc_property_list *properties;
};
// end: objc_class

struct objc_class_gsv1
{
	/**
	 * Pointer to the metaclass for this class.  The metaclass defines the
	 * methods use when a message is sent to the class, rather than an
	 * instance.
	 */
	Class                      isa;
	/**
	 * Pointer to the superclass.  The compiler will set this to the name of
	 * the superclass, the runtime will initialize it to point to the real
	 * class.
	 */
	Class                      super_class;
	/**
	 * The name of this class.  Set to the same value for both the class and
	 * its associated metaclass.
	 */
	const char                *name;
	/**
	 * The version of this class.  This is not used by the language, but may be
	 * set explicitly at class load time.
	 */
	long                       version;
	/**
	 * A bitfield containing various flags.  See the objc_class_flags
	 * enumerated type for possible values.  
	 */
	unsigned long              info;
	/**
	 * The size of this class.  For classes using the non-fragile ABI, the
	 * compiler will set this to a negative value The absolute value will be
	 * the size of the instance variables defined on just this class.  When
	 * using the fragile ABI, the instance size is the size of instances of
	 * this class, including any instance variables defined on superclasses.
	 *
	 * In both cases, this will be set to the size of an instance of the class
	 * after the class is registered with the runtime.
	 */
	long                       instance_size;
	/**
	 * Metadata describing the instance variables in this class.
	 */
	struct objc_ivar_list_gcc *ivars;
	/**
	 * Metadata for for defining the mappings from selectors to IMPs.  Linked
	 * list of method list structures, one per class and one per category.
	 */
	struct objc_method_list_gcc   *methods;
	/**
	 * The dispatch table for this class.  Intialized and maintained by the
	 * runtime.
	 */
	void                      *dtable;
	/**
	 * A pointer to the first subclass for this class.  Filled in by the
	 * runtime.
	 */
	Class                      subclass_list;
	/**
	 * A pointer to the next sibling class to this.  You may find all
	 * subclasses of a given class by following the subclass_list pointer and
	 * then subsequently following the sibling_class pointers in the
	 * subclasses.
	 */
	Class                      sibling_class;

	/**
	 * Metadata describing the protocols adopted by this class.  Not used by
	 * the runtime.
	 */
	struct objc_protocol_list *protocols;
	/**
	 * Linked list of extra data attached to this class.
	 */
	struct reference_list     *extra_data;
	/**
	* New ABI.  The following fields are only available with classes compiled to
	* support the new ABI.  You may test whether any given class supports this
	* ABI by using the CLS_ISNEW_ABI() macro.
	*/

	/**
	* The version of the ABI used for this class.  Zero indicates the ABI first
	* implemented by clang 1.0.  One indicates the presence of bitmaps
	* indicating the offsets of strong, weak, and unretained ivars.  Two
	* indicates that the new ivar structure is used.
	*/
	long                       abi_version;

	/** 
	* Array of pointers to variables where the runtime will store the ivar
	* offset.  These may be used for faster access to non-fragile ivars if all
	* of the code is compiled for the new ABI.  Each of these pointers should
	* have the mangled name __objc_ivar_offset_value_{class name}.{ivar name}
	*
	* When using the compatible non-fragile ABI, this faster form should only be
	* used for classes declared in the same compilation unit.
	*
	* The compiler should also emit symbols of the form 
	* __objc_ivar_offset_{class name}.{ivar name} which are pointers to the
	* offset values.  These should be emitted as weak symbols in every module
	* where they are used.  The legacy-compatible ABI uses these with a double
	* layer of indirection.
	*/
	int                      **ivar_offsets;
	/**
	* List of declared properties on this class (NULL if none).  This contains
	* the accessor methods for each property.
	*/
	struct objc_property_list_gsv1   *properties;

	/**
	 * GC / ARC ABI: Fields below this point only exist if abi_version is >= 1.
	 */

	/**
	 * The location of all strong pointer ivars declared by this class.  
	 *
	 * If the low bit of this field is 0, then this is a pointer to an
	 * objc_bitfield structure.  If the low bit is 1, then the remaining 63
	 * bits are set, from low to high, for each ivar in the object that is a
	 * strong pointer.
	 */
	uintptr_t                  strong_pointers;
	/**
	 * The location of all zeroing weak pointer ivars declared by this class.
	 * The format of this field is the same as the format of the
	 * strong_pointers field.
	 */
	uintptr_t                  weak_pointers;
};

/**
 * Structure representing the GCC ABI class structure.  This is only ever
 * required so that we can take its size - struct objc_class begins with the
 * same fields, and you can test the new abi flag to tell whether it is safe to
 * access the subsequent fields.
 */
struct objc_class_gcc
{
	Class                      isa;
	Class                      super_class;
	const char                *name;
	long                       version;
	unsigned long              info;
	long                       instance_size;
	struct objc_ivar_list_gcc *ivars;
	struct objc_method_list   *methods;
	void                      *dtable;
	Class                      subclass_list;
	Class                      sibling_class;
	struct objc_protocol_list *protocols;
	void                      *gc_object_type;
};


/**
 * An enumerated type describing all of the valid flags that may be used in the
 * info field of a class.
 */
enum objc_class_flags
{
	/** This class structure represents a metaclass. */
	objc_class_flag_meta = (1<<0),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved1 = (1<<1),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved2 = (1<<2),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved3 = (1<<3),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved4 = (1<<4),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved5 = (1<<5),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved6 = (1<<6),
	/** Reserved for future ABI versions. */
	objc_class_flag_reserved7 = (1<<7),
	/**
	 * This class has been sent a +initalize message.  This message is sent
	 * exactly once to every class that is sent a message by the runtime, just
	 * before the first other message is sent.
	 */
	objc_class_flag_initialized = (1<<8),
	/** 
	 * The class has been initialized by the runtime.  Its super_class pointer
	 * should now point to a class, rather than a C string containing the class
	 * name, and its subclass and sibling class links will have been assigned,
	 * if applicable.
	 */
	objc_class_flag_resolved = (1<<9),
	/**
	 * This class was created at run time and may be freed.
	 */
	objc_class_flag_user_created = (1<<10),
	/** 
	 * Instances of this class are provide ARC-safe retain / release /
	 * autorelease implementations.
	 */
	objc_class_flag_fast_arc = (1<<11),
	/**
	 * This class is a hidden class (should not be registered in the class
	 * table nor returned from object_getClass()).
	 */
	objc_class_flag_hidden_class = (1<<12),
	/**
	 * This class is a hidden class used to store associated values.
	 */
	objc_class_flag_assoc_class = (1<<13),
	/**
	 * This class has instances that are never deallocated and are therefore
	 * safe to store directly into weak variables and to skip all reference
	 * count manipulations.
	 */
	objc_class_flag_permanent_instances = (1<<14)
};

/**
 * Sets the specific class flag.  Note: This is not atomic.
 */
static inline void objc_set_class_flag(Class aClass,
                                       enum objc_class_flags flag)
{
	aClass->info |= (unsigned long)flag;
}
/**
 * Unsets the specific class flag.  Note: This is not atomic.
 */
static inline void objc_clear_class_flag(Class aClass,
                                         enum objc_class_flags flag)
{
	aClass->info &= ~(unsigned long)flag;
}
/**
 * Checks whether a specific class flag is set.
 */
static inline BOOL objc_test_class_flag(Class aClass,
                                        enum objc_class_flags flag)
{
	return (aClass->info & (unsigned long)flag) == (unsigned long)flag;
}


/**
 * Adds a class to the class table.
 */
void class_table_insert(Class cls);

/**
 * Removes a class from the class table.  Must be called with the runtime lock
 * held!
 */
void class_table_remove(Class cls);

/**
 * Array of classes used for small objects.  Small objects are embedded in
 * their pointer.  In 32-bit mode, we have one small object class (typically
 * used for storing 31-bit signed integers.  In 64-bit mode then we can have 7,
 * because classes are guaranteed to be word aligned. 
 */
extern Class SmallObjectClasses[7];

static BOOL isSmallObject(id obj)
{
	uintptr_t addr = ((uintptr_t)obj);
	return (addr & OBJC_SMALL_OBJECT_MASK) != 0;
}

__attribute__((always_inline))
static inline Class classForObject(id obj)
{
	if (UNLIKELY(isSmallObject(obj)))
	{
		if (sizeof(Class) == 4)
		{
			return SmallObjectClasses[0];
		}
		else
		{
			uintptr_t addr = ((uintptr_t)obj);
			return SmallObjectClasses[(addr & OBJC_SMALL_OBJECT_MASK)];
		}
	}
	return obj->isa;
}

static inline BOOL classIsOrInherits(Class cls, Class base)
{
	for (Class c = cls ;
		Nil != c ;
		c = c->super_class)
	{
		if (c == base) { return YES; }
	}
	return NO;
}

/**
 * Free the instance variable lists associated with a class.
 */
void freeIvarLists(Class aClass);
/**
 * Free the method lists associated with a class.
 */
void freeMethodLists(Class aClass);

#ifdef __cplusplus
} // extern "C"
#endif
#endif //__OBJC_CLASS_H_INCLUDED
