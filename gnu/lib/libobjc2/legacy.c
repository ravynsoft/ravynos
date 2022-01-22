#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "objc/runtime.h"
#include "objc/encoding.h"
#include "legacy.h"
#include "properties.h"
#include "class.h"
#include "loader.h"

PRIVATE size_t lengthOfTypeEncoding(const char *types);

enum objc_class_flags_gsv1
{
	/** This class structure represents a class. */
	objc_class_flag_class_gsv1 = (1<<0),
	/** This class structure represents a metaclass. */
	objc_class_flag_meta_gsv1 = (1<<1),
	/** 
	 * The class uses the new, Objective-C 2, runtime ABI.  This ABI defines an
	 * ABI version field inside the class, and so will be used for all
	 * subsequent versions that retain some degree of compatibility.
	 */
	objc_class_flag_new_abi_gsv1 = (1<<4)
};

static inline BOOL objc_test_class_flag_gsv1(struct objc_class_gsv1 *aClass,
                                               enum objc_class_flags_gsv1 flag)
{
	return (aClass->info & (unsigned long)flag) == (unsigned long)flag;
}
/**
 * Checks the version of a class.  Return values are:
 * 0. Legacy GCC ABI compatible class.
 * 1. First release of GNUstep ABI.
 * 2. Second release of the GNUstep ABI, adds strong / weak ivar bitmaps.
 * 3. Third release of the GNUstep ABI.  Many cleanups.
 */
static inline int objc_get_class_version_gsv1(struct objc_class_gsv1 *aClass)
{
	if (!objc_test_class_flag_gsv1(aClass, objc_class_flag_new_abi_gsv1))
	{
		return 0;
	}
	return aClass->abi_version + 1;
}

static objc_ivar_ownership ownershipForIvar(struct objc_class_gsv1 *cls, int idx)
{
	if (objc_get_class_version_gsv1(cls) < 2)
	{
		return ownership_unsafe;
	}
	if (objc_bitfield_test(cls->strong_pointers, idx))
	{
		return ownership_strong;
	}
	if (objc_bitfield_test(cls->weak_pointers, idx))
	{
		return ownership_weak;
	}
	return ownership_unsafe;
}

static struct objc_ivar_list *upgradeIvarList(struct objc_class_gsv1 *cls)
{
	struct objc_ivar_list_gcc *l = cls->ivars;
	if (l == NULL)
	{
		return NULL;
	}
	struct objc_ivar_list *n = calloc(1, sizeof(struct objc_ivar_list) +
			l->count*sizeof(struct objc_ivar));
	n->size = sizeof(struct objc_ivar);
	n->count = l->count;
	for (int i=0 ; i<l->count ; i++)
	{
		BOOL isBitfield = NO;
		int bitfieldSize = 0;
		int nextOffset;
		// Bitfields have the same offset, but should have their size set to
		// the size of the bitfield.  We calculate the size of the bitfield by
		// looking for the next ivar after the current one that has a different
		// offset.
		if (i+1 < l->count)
		{
			nextOffset = l->ivar_list[i+1].offset;
			if (l->ivar_list[i].offset == l->ivar_list[i+1].offset)
			{
				isBitfield = YES;
				for (int j=i+2 ; j<l->count ; j++)
				{
					if (l->ivar_list[i].offset != l->ivar_list[j].offset)
					{
						bitfieldSize = l->ivar_list[j].offset - l->ivar_list[i].offset;
						break;
					}
				}
				if (bitfieldSize == 0)
				{
					bitfieldSize = cls->instance_size - l->ivar_list[i].offset;
				}
			}
		}
		else
		{
			nextOffset = cls->instance_size;
		}
		if (nextOffset < 0)
		{
			nextOffset = -nextOffset;
		}
		const char *type = l->ivar_list[i].type;
		int size = nextOffset - l->ivar_list[i].offset;
		n->ivar_list[i].name = l->ivar_list[i].name;
		n->ivar_list[i].type = type;
		n->ivar_list[i].size = isBitfield ? bitfieldSize : size;
		if (objc_test_class_flag_gsv1(cls, objc_class_flag_new_abi_gsv1))
		{
			n->ivar_list[i].offset = cls->ivar_offsets[i];
		}
		else
		{
			n->ivar_list[i].offset = &l->ivar_list[i].offset;
		}
		ivarSetAlign(&n->ivar_list[i], ((type == NULL) || type[0] == 0) ? __alignof__(void*) : objc_alignof_type(type));
		if (type[0] == '\0')
		{
			ivarSetAlign(&n->ivar_list[i], size);
		}
		ivarSetOwnership(&n->ivar_list[i], ownershipForIvar(cls, i));
	}
	return n;
}

static struct objc_method_list *upgradeMethodList(struct objc_method_list_gcc *old)
{
	if (old == NULL)
	{
		return NULL;
	}
	if (old->count == 0)
	{
		return NULL;
	}
	struct objc_method_list *l = calloc(sizeof(struct objc_method_list) + old->count * sizeof(struct objc_method), 1);
	l->count = old->count;
	if (old->next)
	{
		l->next = upgradeMethodList(old->next);
	}
	l->size = sizeof(struct objc_method);
	for (int i=0 ; i<old->count ; i++)
	{
		l->methods[i].imp = old->methods[i].imp;
		l->methods[i].selector = old->methods[i].selector;
		l->methods[i].types = old->methods[i].types;
	}
	return l;
}

static inline BOOL checkAttribute(char field, int attr)
{
	return (field & attr) == attr;
}

static void upgradeProperty(struct objc_property *n, struct objc_property_gsv1 *o)
{
	char *typeEncoding;
	ptrdiff_t typeSize;
	if (o->name[0] == '\0')
	{
		n->name = o->name + o->name[1];
		n->attributes = o->name + 2;
		// If we have an attribute string, then it will contain a more accurate
		// version of the types than we'll find in the getter (qualifiers such
		// as _Atomic and volatile may be dropped)
		assert(n->attributes[0] == 'T');
		const char *type_start = &n->attributes[1];
		const char *type_end = strchr(type_start, ',');
		if (type_end == NULL)
		{
			type_end = type_start + strlen(type_start);
		}
		typeSize = type_end - type_start;
		typeEncoding = malloc(typeSize  + 1);
		memcpy(typeEncoding, type_start, typeSize);
		typeEncoding[typeSize] = 0;
	}
	else
	{
		typeSize = (ptrdiff_t)lengthOfTypeEncoding(o->getter_types);
		typeEncoding = malloc(typeSize + 1);
		memcpy(typeEncoding, o->getter_types, typeSize);
		typeEncoding[typeSize] = 0;
	}
	n->type = typeEncoding;

	if (o->getter_name)
	{
		n->getter = sel_registerTypedName_np(o->getter_name, o->getter_types);
	}
	if (o->setter_name)
	{
		n->setter = sel_registerTypedName_np(o->setter_name, o->setter_types);
	}

	if (o->name[0] == '\0')
	{
		return;
	}

	n->name = o->name;

	const char *name = o->name;
	size_t nameSize = (NULL == name) ? 0 : strlen(name);
	// Encoding is T{type},V{name}, so 4 bytes for the "T,V" that we always
	// need.  We also need two bytes for the leading null and the length.
	size_t encodingSize = typeSize + nameSize + 6;
	char flags[20];
	size_t i = 0;
	// Flags that are a comma then a character
	if (checkAttribute(o->attributes, OBJC_PR_readonly))
	{
		flags[i++] = ',';
		flags[i++] = 'R';
	}
	if (checkAttribute(o->attributes, OBJC_PR_retain))
	{
		flags[i++] = ',';
		flags[i++] = '&';
	}
	if (checkAttribute(o->attributes, OBJC_PR_copy))
	{
		flags[i++] = ',';
		flags[i++] = 'C';
	}
	if (checkAttribute(o->attributes2, OBJC_PR_weak))
	{
		flags[i++] = ',';
		flags[i++] = 'W';
	}
	if (checkAttribute(o->attributes2, OBJC_PR_dynamic))
	{
		flags[i++] = ',';
		flags[i++] = 'D';
	}
	if ((o->attributes & OBJC_PR_nonatomic) == OBJC_PR_nonatomic)
	{
		flags[i++] = ',';
		flags[i++] = 'N';
	}
	encodingSize += i;
	flags[i] = '\0';
	size_t setterLength = 0;
	size_t getterLength = 0;
	if ((o->attributes & OBJC_PR_getter) == OBJC_PR_getter)
	{
		getterLength = strlen(o->getter_name);
		encodingSize += 2 + getterLength;
	}
	if ((o->attributes & OBJC_PR_setter) == OBJC_PR_setter)
	{
		setterLength = strlen(o->setter_name);
		encodingSize += 2 + setterLength;
	}
	unsigned char *encoding = malloc(encodingSize);
	// Set the leading 0 and the offset of the name
	unsigned char *insert = encoding;
	BOOL needsComma = NO;
	*(insert++) = 0;
	*(insert++) = 0;
	// Set the type encoding
	*(insert++) = 'T';
	memcpy(insert, typeEncoding, typeSize);
	insert += typeSize;
	needsComma = YES;
	// Set the flags
	memcpy(insert, flags, i);
	insert += i;
	if ((o->attributes & OBJC_PR_getter) == OBJC_PR_getter)
	{
		if (needsComma)
		{
			*(insert++) = ',';
		}
		i++;
		needsComma = YES;
		*(insert++) = 'G';
		memcpy(insert, o->getter_name, getterLength);
		insert += getterLength;
	}
	if ((o->attributes & OBJC_PR_setter) == OBJC_PR_setter)
	{
		if (needsComma)
		{
			*(insert++) = ',';
		}
		i++;
		needsComma = YES;
		*(insert++) = 'S';
		memcpy(insert, o->setter_name, setterLength);
		insert += setterLength;
	}
	if (needsComma)
	{
		*(insert++) = ',';
	}
	*(insert++) = 'V';
	memcpy(insert, name, nameSize);
	insert += nameSize;
	*(insert++) = '\0';

	n->attributes = (const char*)encoding;
}

static struct objc_property_list *upgradePropertyList(struct objc_property_list_gsv1 *l)
{
	if (l == NULL)
	{
		return NULL;
	}
	size_t data_size = l->count * sizeof(struct objc_property);
	struct objc_property_list *n = calloc(1, sizeof(struct objc_property_list) + data_size);
	n->count = l->count;
	n->size = sizeof(struct objc_property);
	for (int i=0 ; i<l->count ; i++)
	{
		upgradeProperty(&n->properties[i], &l->properties[i]);
	}
	return n;
}

static int legacy_key;

PRIVATE struct objc_class_gsv1* objc_legacy_class_for_class(Class cls)
{
	return (struct objc_class_gsv1*)objc_getAssociatedObject((id)cls, &legacy_key);
}

PRIVATE Class objc_upgrade_class(struct objc_class_gsv1 *oldClass)
{
	Class cls = calloc(sizeof(struct objc_class), 1);
	cls->isa = oldClass->isa;
	// super_class is left nil and we upgrade it later.
	cls->name = oldClass->name;
	cls->version = oldClass->version;
	cls->info = objc_class_flag_meta;
	cls->instance_size = oldClass->instance_size;
	cls->ivars = upgradeIvarList(oldClass);
	cls->methods = upgradeMethodList(oldClass->methods);
	cls->protocols = oldClass->protocols;
	cls->abi_version = oldClass->abi_version;
	cls->properties = upgradePropertyList(oldClass->properties);
	objc_register_selectors_from_class(cls);
	if (!objc_test_class_flag_gsv1(oldClass, objc_class_flag_meta_gsv1))
	{
		cls->info = 0;
		cls->isa = objc_upgrade_class((struct objc_class_gsv1*)cls->isa);
		objc_setAssociatedObject((id)cls, &legacy_key, (id)oldClass, OBJC_ASSOCIATION_ASSIGN);
	}
	return cls;
}
PRIVATE struct objc_category *objc_upgrade_category(struct objc_category_gcc *old)
{
	struct objc_category *cat = calloc(1, sizeof(struct objc_category));
	memcpy(cat, old, sizeof(struct objc_category_gcc));
	cat->instance_methods = upgradeMethodList(old->instance_methods);
	cat->class_methods = upgradeMethodList(old->class_methods);
	if (cat->instance_methods != NULL)
	{
		objc_register_selectors_from_list(cat->instance_methods);
	}
	if (cat->class_methods != NULL)
	{
		objc_register_selectors_from_list(cat->class_methods);
	}
	for (int i=0 ; i<cat->protocols->count ; i++)
	{
		objc_init_protocols(cat->protocols);
	}
	return cat;
}

static struct objc_protocol_method_description_list*
upgrade_protocol_method_list_gcc(struct objc_protocol_method_description_list_gcc *l)
{
	if ((l == NULL) || (l->count == 0))
	{
		return NULL;
	}
	struct objc_protocol_method_description_list *n =
		malloc(sizeof(struct objc_protocol_method_description_list) +
			l->count * sizeof(struct objc_protocol_method_description));
	n->count = l->count;
	n->size = sizeof(struct objc_protocol_method_description);
	for (int i=0 ; i<n->count ; i++)
	{
		n->methods[i].selector = sel_registerTypedName_np(l->methods[i].name, l->methods[i].types);
		n->methods[i].types = l->methods[i].types;
	}
	return n;
}

PRIVATE struct objc_protocol *objc_upgrade_protocol_gcc(struct objc_protocol_gcc *p)
{
	// If the protocol has already been upgraded, the don't try to upgrade it twice.
	if (p->isa == objc_getClass("ProtocolGCC"))
	{
		return objc_getProtocol(p->name);
	}
	p->isa = objc_getClass("ProtocolGCC");
	Protocol *proto =
		(Protocol*)class_createInstance((Class)objc_getClass("Protocol"),
				sizeof(struct objc_protocol) - sizeof(id));
	proto->name = p->name;
	// Aliasing of this between the new and old structures means that when this
	// returns these will all be updated.
	proto->protocol_list = p->protocol_list;
	proto->instance_methods = upgrade_protocol_method_list_gcc(p->instance_methods);
	proto->class_methods = upgrade_protocol_method_list_gcc(p->class_methods);
	assert(proto->isa);
	return proto;
}

PRIVATE struct objc_protocol *objc_upgrade_protocol_gsv1(struct objc_protocol_gsv1 *p)
{
	// If the protocol has already been upgraded, the don't try to upgrade it twice.
	if (p->isa == objc_getClass("ProtocolGSv1"))
	{
		return objc_getProtocol(p->name);
	}
	Protocol *n =
		(Protocol*)class_createInstance((Class)objc_getClass("Protocol"),
				sizeof(struct objc_protocol) - sizeof(id));
	n->instance_methods = upgrade_protocol_method_list_gcc(p->instance_methods);
	// Aliasing of this between the new and old structures means that when this
	// returns these will all be updated.
	n->name = p->name;
	n->protocol_list = p->protocol_list;
	n->class_methods = upgrade_protocol_method_list_gcc(p->class_methods);
	n->properties = upgradePropertyList(p->properties);
	n->optional_properties = upgradePropertyList(p->optional_properties);
	n->isa = objc_getClass("Protocol");
	// We do in-place upgrading of these, because they might be referenced
	// directly
	p->instance_methods = (struct objc_protocol_method_description_list_gcc*)n->instance_methods;
	p->class_methods = (struct objc_protocol_method_description_list_gcc*)n->class_methods;
	p->properties = (struct objc_property_list_gsv1*)n->properties;
	p->optional_properties = (struct objc_property_list_gsv1*)n->optional_properties;
	p->isa = objc_getClass("ProtocolGSv1");
	assert(p->isa);
	return n;
}

