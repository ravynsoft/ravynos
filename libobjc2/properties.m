#include "objc/runtime.h"
#include "objc/objc-arc.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "class.h"
#include "properties.h"
#include "spinlock.h"
#include "visibility.h"
#include "nsobject.h"
#include "gc_ops.h"
#include "lock.h"

PRIVATE int spinlocks[spinlock_count];

/**
 * Public function for getting a property.  
 */
OBJC_PUBLIC
id objc_getProperty(id obj, SEL _cmd, ptrdiff_t offset, BOOL isAtomic)
{
	if (nil == obj) { return nil; }
	char *addr = (char*)obj;
	addr += offset;
	if (isGCEnabled)
	{
		return *(id*)addr;
	}
	id ret;
	if (isAtomic)
	{
		volatile int *lock = lock_for_pointer(addr);
		lock_spinlock(lock);
		ret = *(id*)addr;
		ret = objc_retain(ret);
		unlock_spinlock(lock);
		ret = objc_autoreleaseReturnValue(ret);
	}
	else
	{
		ret = *(id*)addr;
		ret = objc_retainAutoreleaseReturnValue(ret);
	}
	return ret;
}

OBJC_PUBLIC
void objc_setProperty(id obj, SEL _cmd, ptrdiff_t offset, id arg, BOOL isAtomic, BOOL isCopy)
{
	if (nil == obj) { return; }
	char *addr = (char*)obj;
	addr += offset;

	if (isGCEnabled)
	{
		if (isCopy)
		{
			arg = [arg copy];
		}
		*(id*)addr = arg;
		return;
	}
	if (isCopy)
	{
		arg = [arg copy];
	}
	else
	{
		arg = objc_retain(arg);
	}
	id old;
	if (isAtomic)
	{
		volatile int *lock = lock_for_pointer(addr);
		lock_spinlock(lock);
		old = *(id*)addr;
		*(id*)addr = arg;
		unlock_spinlock(lock);
	}
	else
	{
		old = *(id*)addr;
		*(id*)addr = arg;
	}
	objc_release(old);
}

OBJC_PUBLIC
void objc_setProperty_atomic(id obj, SEL _cmd, id arg, ptrdiff_t offset)
{
	char *addr = (char*)obj;
	addr += offset;
	arg = objc_retain(arg);
	volatile int *lock = lock_for_pointer(addr);
	lock_spinlock(lock);
	id old = *(id*)addr;
	*(id*)addr = arg;
	unlock_spinlock(lock);
	objc_release(old);
}

OBJC_PUBLIC
void objc_setProperty_atomic_copy(id obj, SEL _cmd, id arg, ptrdiff_t offset)
{
	char *addr = (char*)obj;
	addr += offset;

	arg = [arg copy];
	volatile int *lock = lock_for_pointer(addr);
	lock_spinlock(lock);
	id old = *(id*)addr;
	*(id*)addr = arg;
	unlock_spinlock(lock);
	objc_release(old);
}

OBJC_PUBLIC
void objc_setProperty_nonatomic(id obj, SEL _cmd, id arg, ptrdiff_t offset)
{
	char *addr = (char*)obj;
	addr += offset;
	arg = objc_retain(arg);
	id old = *(id*)addr;
	*(id*)addr = arg;
	objc_release(old);
}

OBJC_PUBLIC
void objc_setProperty_nonatomic_copy(id obj, SEL _cmd, id arg, ptrdiff_t offset)
{
	char *addr = (char*)obj;
	addr += offset;
	id old = *(id*)addr;
	*(id*)addr = [arg copy];
	objc_release(old);
}

OBJC_PUBLIC
void objc_copyCppObjectAtomic(void *dest, const void *src,
                              void (*copyHelper) (void *dest, const void *source))
{
	volatile int *lock = lock_for_pointer(src < dest ? src : dest);
	volatile int *lock2 = lock_for_pointer(src < dest ? dest : src);
	lock_spinlock(lock);
	lock_spinlock(lock2);
	copyHelper(dest, src);
	unlock_spinlock(lock);
	unlock_spinlock(lock2);
}

OBJC_PUBLIC
void objc_getCppObjectAtomic(void *dest, const void *src,
                             void (*copyHelper) (void *dest, const void *source))
{
	volatile int *lock = lock_for_pointer(src);
	lock_spinlock(lock);
	copyHelper(dest, src);
	unlock_spinlock(lock);
}

OBJC_PUBLIC
void objc_setCppObjectAtomic(void *dest, const void *src,
                             void (*copyHelper) (void *dest, const void *source))
{
	volatile int *lock = lock_for_pointer(dest);
	lock_spinlock(lock);
	copyHelper(dest, src);
	unlock_spinlock(lock);
}

/**
 * Structure copy function.  This is provided for compatibility with the Apple
 * APIs (it's an ABI function, so it's semi-public), but it's a bad design so
 * it's not used.  The problem is that it does not identify which of the
 * pointers corresponds to the object, which causes some excessive locking to
 * be needed.
 */
OBJC_PUBLIC
void objc_copyPropertyStruct(void *dest,
                             void *src,
                             ptrdiff_t size,
                             BOOL atomic,
                             BOOL strong)
{
	if (atomic)
	{
		volatile int *lock = lock_for_pointer(src < dest ? src : dest);
		volatile int *lock2 = lock_for_pointer(src < dest ? dest : src);
		lock_spinlock(lock);
		lock_spinlock(lock2);
		memcpy(dest, src, size);
		unlock_spinlock(lock);
		unlock_spinlock(lock2);
	}
	else
	{
		memcpy(dest, src, size);
	}
}

/**
 * Get property structure function.  Copies a structure from an ivar to another
 * variable.  Locks on the address of src.
 */
OBJC_PUBLIC
void objc_getPropertyStruct(void *dest,
                            void *src,
                            ptrdiff_t size,
                            BOOL atomic,
                            BOOL strong)
{
	if (atomic)
	{
		volatile int *lock = lock_for_pointer(src);
		lock_spinlock(lock);
		memcpy(dest, src, size);
		unlock_spinlock(lock);
	}
	else
	{
		memcpy(dest, src, size);
	}
}

/**
 * Set property structure function.  Copes a structure to an ivar.  Locks on
 * dest.
 */
OBJC_PUBLIC
void objc_setPropertyStruct(void *dest,
                            void *src,
                            ptrdiff_t size,
                            BOOL atomic,
                            BOOL strong)
{
	if (atomic)
	{
		volatile int *lock = lock_for_pointer(dest);
		lock_spinlock(lock);
		memcpy(dest, src, size);
		unlock_spinlock(lock);
	}
	else
	{
		memcpy(dest, src, size);
	}
}


OBJC_PUBLIC
objc_property_t class_getProperty(Class cls, const char *name)
{
	if (Nil == cls)
	{
		return NULL;
	}
	struct objc_property_list *properties = cls->properties;
	while (NULL != properties)
	{
		for (int i=0 ; i<properties->count ; i++)
		{
			objc_property_t p = property_at_index(properties, i);
			if (strcmp(property_getName(p), name) == 0)
			{
				return p;
			}
		}
		properties = properties->next;
	}
	return NULL;
}

OBJC_PUBLIC
objc_property_t* class_copyPropertyList(Class cls, unsigned int *outCount)
{
	if (Nil == cls)
	{
		if (NULL != outCount) { *outCount = 0; }
		return NULL;
	}
	struct objc_property_list *properties = cls->properties;
	if (!properties)
	{
		if (NULL != outCount) { *outCount = 0; }
		return NULL;
	}
	unsigned int count = 0;
	for (struct objc_property_list *l=properties ; NULL!=l ; l=l->next)
	{
		count += l->count;
	}
	if (NULL != outCount)
	{
		*outCount = count;
	}
	if (0 == count)
	{
		return NULL;
	}
	objc_property_t *list = calloc(sizeof(objc_property_t), count);
	unsigned int out = 0;
	for (struct objc_property_list *l=properties ; NULL!=l ; l=l->next)
	{
		for (int i=0 ; i<l->count ; i++)
		{
			list[out++] = property_at_index(l, i);
		}
	}
	return list;
}
static const char* property_getIVar(objc_property_t property)
{
	const char *iVar = property_getAttributes(property);
	if (iVar != 0)
	{
		while ((*iVar != 0) && (*iVar != 'V'))
		{
			iVar++;
		}
		if (*iVar == 'V')
		{
			return iVar+1;
		}
	}
	return 0;
}

OBJC_PUBLIC
const char *property_getName(objc_property_t property)
{
	if (NULL == property) { return NULL; }

	const char *name = property->name;
	if (NULL == name) { return NULL; }
	if (name[0] == 0)
	{
		name += name[1];
	}
	return name;
}

/*
 * The compiler stores the type encoding of the getter.  We replace this with
 * the type encoding of the property itself.  We use a 0 byte at the start to
 * indicate that the swap has taken place.
 */
static const char *property_getTypeEncoding(objc_property_t property)
{
	if (NULL == property) { return NULL; }
	return property->type;
}

OBJC_PUBLIC
const char *property_getAttributes(objc_property_t property)
{
	if (NULL == property) { return NULL; }
	return property->attributes;
}


OBJC_PUBLIC
objc_property_attribute_t *property_copyAttributeList(objc_property_t property,
                                                      unsigned int *outCount)
{
	if (NULL == property)
	{
		if (NULL != outCount)
		{
			*outCount = 0;
		}
		return NULL;
	}
	objc_property_attribute_t attrs[12];
	int count = 0;

	const char *types = property_getTypeEncoding(property);
	if (NULL != types)
	{
		attrs[count].name = "T";
		attrs[count].value = types;
		count++;
	}
	// If the compiler provides a type encoding string, then it's more
	// informative than the bitfields and should be treated as canonical.  If
	// the compiler didn't provide a type encoding string, then this will
	// create a best-effort one.
	const char *attributes = property_getAttributes(property);
	for (int i=strlen(types)+1 ; attributes[i] != 0 ; i++)
	{
		assert(count<12);
		if (attributes[i] == ',')
		{
			// Comma is never the last character in the string, so this should
			// never push us past the end.
			i++;
		}
		attrs[count].value = "";
		switch (attributes[i])
		{
			case 'R':
				attrs[count].name = "R";
				break;
			case 'C':
				attrs[count].name = "C";
				break;
			case '&':
				attrs[count].name = "&";
				break;
			case 'D':
				attrs[count].name = "D";
				break;
			case 'W':
				attrs[count].name = "W";
				break;
			case 'N':
				attrs[count].name = "N";
				break;
			case 'G':
				attrs[count].name = "G";
				attrs[count].value = sel_getName(property->getter);
				i += strlen(attrs[count].value);
				break;
			case 'S':
				attrs[count].name = "S";
				attrs[count].value = sel_getName(property->setter);
				i += strlen(attrs[count].value);
				break;
			case 'V':
				attrs[count].name = "V";
				attrs[count].value = attributes+i+1;
				i += strlen(attributes+i)-1;
				break;
			default:
				continue;
		}
		count++;
	}
	objc_property_attribute_t *propAttrs = calloc(sizeof(objc_property_attribute_t), count);
	memcpy(propAttrs, attrs, count * sizeof(objc_property_attribute_t));
	if (NULL != outCount)
	{
		*outCount = count;
	}
	return propAttrs;
}

static const objc_property_attribute_t *findAttribute(char attr,
                                                      const objc_property_attribute_t *attributes,
                                                      unsigned int attributeCount)
{
	// This linear scan is N^2 in the worst case, but that's still probably
	// cheaper than sorting the array because N<12
	for (int i=0 ; i<attributeCount ; i++)
	{
		if (attributes[i].name[0] == attr)
		{
			return &attributes[i];
		}
	}
	return NULL;
}
static char *addAttrIfExists(char a,
                             char *buffer,
                             const objc_property_attribute_t *attributes,
                             unsigned int attributeCount)
{
	const objc_property_attribute_t *attr = findAttribute(a, attributes, attributeCount);
	if (attr)
	{
		*(buffer++) = attr->name[0];
		if (attr->value)
		{
			size_t len = strlen(attr->value);
			memcpy(buffer, attr->value, len);
			buffer += len;
		}
		*(buffer++) = ',';
	}
	return buffer;
}

static const char *encodingFromAttrs(const objc_property_attribute_t *attributes,
                                     unsigned int attributeCount)
{
	// Length of the attributes string (initially the number of keys and commas and trailing null)
	size_t attributesSize = 2 * attributeCount;
	for (int i=0 ; i<attributeCount ; i++)
	{
		if (attributes[i].value)
		{
			attributesSize += strlen(attributes[i].value);
		}
	}
	if (attributesSize == 0)
	{
		return NULL;
	}

	char *buffer = malloc(attributesSize);

	char *out = buffer;
	out = addAttrIfExists('T', out, attributes, attributeCount);
	out = addAttrIfExists('R', out, attributes, attributeCount);
	out = addAttrIfExists('&', out, attributes, attributeCount);
	out = addAttrIfExists('C', out, attributes, attributeCount);
	out = addAttrIfExists('W', out, attributes, attributeCount);
	out = addAttrIfExists('D', out, attributes, attributeCount);
	out = addAttrIfExists('N', out, attributes, attributeCount);
	out = addAttrIfExists('G', out, attributes, attributeCount);
	out = addAttrIfExists('S', out, attributes, attributeCount);
	out = addAttrIfExists('V', out, attributes, attributeCount);
	assert(out != buffer);
	out--;
	*out = '\0';

	return buffer;
}
PRIVATE struct objc_property propertyFromAttrs(const objc_property_attribute_t *attributes,
                                               unsigned int attributeCount,
                                               const char *name)
{
	struct objc_property p;
	p.name = strdup(name);
	p.attributes = encodingFromAttrs(attributes, attributeCount);
	p.type = NULL;
	const objc_property_attribute_t *attr = findAttribute('T', attributes, attributeCount);
	if (attr)
	{
		p.type = strdup(attr->value);
	}
	p.getter = NULL;
	attr = findAttribute('G', attributes, attributeCount);
	if (attr)
	{
		// TODO: We should be able to construct the full type encoding if we
		// also have a type, but for now use an untyped selector.
		p.getter = sel_registerName(attr->value);
	}
	p.setter = NULL;
	attr = findAttribute('S', attributes, attributeCount);
	if (attr)
	{
		// TODO: We should be able to construct the full type encoding if we
		// also have a type, but for now use an untyped selector.
		p.setter = sel_registerName(attr->value);
	}
	return p;
}


OBJC_PUBLIC
BOOL class_addProperty(Class cls,
                       const char *name,
                       const objc_property_attribute_t *attributes, 
                       unsigned int attributeCount)
{
	if ((Nil == cls) || (NULL == name) || (class_getProperty(cls, name) != 0)) { return NO; }

	struct objc_property p = propertyFromAttrs(attributes, attributeCount, name);

	struct objc_property_list *l = calloc(1, sizeof(struct objc_property_list)
			+ sizeof(struct objc_property));
	l->count = 1;
	l->size = sizeof(struct objc_property);
	memcpy(&l->properties, &p, sizeof(struct objc_property));
	LOCK_RUNTIME_FOR_SCOPE();
	l->next = cls->properties;
	cls->properties = l;
	return YES;
}

OBJC_PUBLIC
void class_replaceProperty(Class cls,
                           const char *name,
                           const objc_property_attribute_t *attributes,
                           unsigned int attributeCount)
{
	if ((Nil == cls) || (NULL == name)) { return; }
	objc_property_t old = class_getProperty(cls, name);
	if (NULL == old)
	{
		class_addProperty(cls, name, attributes, attributeCount);
		return;
	}
	struct objc_property p = propertyFromAttrs(attributes, attributeCount, name);
	LOCK_RUNTIME_FOR_SCOPE();
	memcpy(old, &p, sizeof(struct objc_property));
}
OBJC_PUBLIC
char *property_copyAttributeValue(objc_property_t property,
                                  const char *attributeName)
{
	if ((NULL == property) || (NULL == attributeName)) { return NULL; }
	const char *attributes = property_getAttributes(property);
	switch (attributeName[0])
	{
		case 'T':
		{
			const char *types = property_getTypeEncoding(property);
			return (NULL == types) ? NULL : strdup(types);
		}
		case 'D':
		case 'R':
		case 'W':
		case 'C':
		case '&':
		case 'N':
		{
			return strchr(attributes, attributeName[0]) ? strdup("") : 0;
		}
		case 'V':
		{
			return strdup(property_getIVar(property));
		}
		case 'S':
		{
			return strdup(sel_getName(property->setter));
		}
		case 'G':
		{
			return strdup(sel_getName(property->getter));
		}
	}
	return 0;
}
