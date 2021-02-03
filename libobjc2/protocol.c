#include "objc/runtime.h"
#include "protocol.h"
#include "properties.h"
#include "class.h"
#include "lock.h"
#include "legacy.h"
#include <stdlib.h>
#include <assert.h>

#define BUFFER_TYPE struct objc_protocol_list *
#include "buffer.h"

// Get the functions for string hashing
#include "string_hash.h"

static int protocol_compare(const char *name,
                            const struct objc_protocol *protocol)
{
	return string_compare(name, protocol->name);
}
static int protocol_hash(const struct objc_protocol *protocol)
{
	return string_hash(protocol->name);
}
#define MAP_TABLE_NAME protocol
#define MAP_TABLE_COMPARE_FUNCTION protocol_compare
#define MAP_TABLE_HASH_KEY string_hash
#define MAP_TABLE_HASH_VALUE protocol_hash
#include "hash_table.h"

static protocol_table *known_protocol_table;
mutex_t protocol_table_lock;

PRIVATE void init_protocol_table(void)
{
	protocol_initialize(&known_protocol_table, 128);
	INIT_LOCK(protocol_table_lock);
}

static void protocol_table_insert(const struct objc_protocol *protocol)
{
	protocol_insert(known_protocol_table, (void*)protocol);
}

struct objc_protocol *protocol_for_name(const char *name)
{
	return protocol_table_get(known_protocol_table, name);
}

static id incompleteProtocolClass(void)
{
	static id IncompleteProtocolClass = 0;
	if (IncompleteProtocolClass == nil)
	{
		IncompleteProtocolClass = objc_getClass("__IncompleteProtocol");
	}
	return IncompleteProtocolClass;
}

/**
 * Class used for legacy GCC protocols (`ProtocolGCC`).
 */
static id protocol_class_gcc;
/**
 * Class used for legacy GNUstep V1 ABI  protocols (`ProtocolGSv1`).
 */
static id protocol_class_gsv1;
/**
 * Class used for protocols (`Protocol`).
 */
static id protocol_class_gsv2;

static BOOL init_protocol_classes(void)
{
	if (nil == protocol_class_gcc)
	{
		protocol_class_gcc = objc_getClass("ProtocolGCC");
	}
	if (nil == protocol_class_gsv1)
	{
		protocol_class_gsv1 = objc_getClass("ProtocolGSv1");
	}
	if (nil == protocol_class_gsv2)
	{
		protocol_class_gsv2 = objc_getClass("Protocol");
	}
	if ((nil == protocol_class_gcc) ||
	    (nil == protocol_class_gsv1) ||
	    (nil == protocol_class_gsv2))
	{
		return NO;
	}
	return YES;
}

static BOOL protocol_hasClassProperties(struct objc_protocol *p)
{
	if (!init_protocol_classes())
	{
		return NO;
	}
	return p->isa == protocol_class_gsv2;
}

static BOOL protocol_hasOptionalMethodsAndProperties(struct objc_protocol *p)
{
	if (!init_protocol_classes())
	{
		return NO;
	}
	if (p->isa == protocol_class_gcc)
	{
		return NO;
	}
	return YES;
}

static int isEmptyProtocol(struct objc_protocol *aProto)
{
	int isEmpty =
		((aProto->instance_methods == NULL) ||
			(aProto->instance_methods->count == 0)) &&
		((aProto->class_methods == NULL) ||
			(aProto->class_methods->count == 0)) &&
		((aProto->protocol_list == NULL) ||
		  (aProto->protocol_list->count == 0));
	if (protocol_hasOptionalMethodsAndProperties(aProto))
	{
		isEmpty &= (aProto->optional_instance_methods == NULL) ||
			(aProto->optional_instance_methods->count == 0);
		isEmpty &= (aProto->optional_class_methods == NULL) ||
			(aProto->optional_class_methods->count == 0);
		isEmpty &= (aProto->properties == 0) || (aProto->properties->count == 0);
		isEmpty &= (aProto->optional_properties == 0) || (aProto->optional_properties->count == 0);
	}
	return isEmpty;
}

// FIXME: Make p1 adopt all of the stuff in p2
static void makeProtocolEqualToProtocol(struct objc_protocol *p1,
                                        struct objc_protocol *p2)
{
#define COPY(x) p1->x = p2->x
	COPY(instance_methods);
	COPY(class_methods);
	COPY(protocol_list);
	if (protocol_hasOptionalMethodsAndProperties(p1) &&
		protocol_hasOptionalMethodsAndProperties(p2))
	{
		COPY(optional_instance_methods);
		COPY(optional_class_methods);
		COPY(properties);
		COPY(optional_properties);
	}
#undef COPY
}

static struct objc_protocol *unique_protocol(struct objc_protocol *aProto)
{
	struct objc_protocol *oldProtocol =
		protocol_for_name(aProto->name);
	if (NULL == oldProtocol)
	{
		// This is the first time we've seen this protocol, so add it to the
		// hash table and ignore it.
		protocol_table_insert(aProto);
		return aProto;
	}
	if (isEmptyProtocol(oldProtocol))
	{
		if (isEmptyProtocol(aProto))
		{
			return aProto;
			// Add protocol to a list somehow.
		}
		else
		{
			// This protocol is not empty, so we use its definitions
			makeProtocolEqualToProtocol(oldProtocol, aProto);
			return aProto;
		}
	}
	else
	{
		if (isEmptyProtocol(aProto))
		{
			makeProtocolEqualToProtocol(aProto, oldProtocol);
			return oldProtocol;
		}
		else
		{
			return oldProtocol;
			//FIXME: We should really perform a check here to make sure the
			//protocols are actually the same.
		}
	}
}

static BOOL init_protocols(struct objc_protocol_list *protocols)
{
	if (!init_protocol_classes())
	{
		return NO;
	}

	for (unsigned i=0 ; i<protocols->count ; i++)
	{
		struct objc_protocol *aProto = protocols->list[i];
		// Don't initialise a protocol twice
		if ((aProto->isa == protocol_class_gcc) ||
		    (aProto->isa == protocol_class_gsv1) ||
		    (aProto->isa == protocol_class_gsv2))
		{
			continue;
		}

		// Protocols in the protocol list have their class pointers set to the
		// version of the protocol class that they expect.
		enum protocol_version version =
			(enum protocol_version)(uintptr_t)aProto->isa;
		switch (version)
		{
			default:
				fprintf(stderr, "Unknown protocol version");
				abort();
#ifdef OLDABI_COMPAT
			case protocol_version_gcc:
				protocols->list[i] = objc_upgrade_protocol_gcc((struct objc_protocol_gcc *)aProto);
				assert(aProto->isa == protocol_class_gcc);
				assert(protocols->list[i]->isa == protocol_class_gsv2);
				aProto = protocols->list[i];
				break;
			case protocol_version_gsv1:
				protocols->list[i] = objc_upgrade_protocol_gsv1((struct objc_protocol_gsv1 *)aProto);
				assert(aProto->isa == protocol_class_gsv1);
				assert(protocols->list[i]->isa == protocol_class_gsv2);
				aProto = protocols->list[i];
				break;
#endif
			case protocol_version_gsv2:
				aProto->isa = protocol_class_gsv2;
				break;
		}
		// Initialize all of the protocols that this protocol refers to
		if (NULL != aProto->protocol_list)
		{
			init_protocols(aProto->protocol_list);
		}
		// Replace this protocol with a unique version of it.
		protocols->list[i] = unique_protocol(aProto);
	}
	return YES;
}

PRIVATE void objc_init_protocols(struct objc_protocol_list *protocols)
{
	LOCK_FOR_SCOPE(&protocol_table_lock);
	if (!init_protocols(protocols))
	{
		set_buffered_object_at_index(protocols, buffered_objects++);
		return;
	}
	if (buffered_objects == 0) { return; }

	// If we can load one protocol, then we can load all of them.
	for (unsigned i=0 ; i<buffered_objects ; i++)
	{
		struct objc_protocol_list *c = buffered_object_at_index(i);
		if (NULL != c)
		{
			init_protocols(c);
			set_buffered_object_at_index(NULL, i);
		}
	}
	compact_buffer();
}

// Public functions:
Protocol *objc_getProtocol(const char *name)
{
	if (NULL == name) { return NULL; }
	LOCK_FOR_SCOPE(&protocol_table_lock);
	return (Protocol*)protocol_for_name(name);
}

BOOL protocol_conformsToProtocol(Protocol *p1, Protocol *p2)
{
	if (NULL == p1 || NULL == p2) { return NO; }

	// A protocol trivially conforms to itself
	if (strcmp(p1->name, p2->name) == 0) { return YES; }

	for (struct objc_protocol_list *list = p1->protocol_list ;
		list != NULL ; list = list->next)
	{
		for (int i=0 ; i<list->count ; i++)
		{
			if (strcmp(list->list[i]->name, p2->name) == 0)
			{
				return YES;
			}
			if (protocol_conformsToProtocol((Protocol*)list->list[i], p2))
			{
				return YES;
			}
		}
	}
	return NO;
}

BOOL class_conformsToProtocol(Class cls, Protocol *protocol)
{
	if (Nil == cls || NULL == protocol) { return NO; }
	for ( ; Nil != cls ; cls = class_getSuperclass(cls))
	{
		for (struct objc_protocol_list *protocols = cls->protocols;
			protocols != NULL ; protocols = protocols->next)
		{
			for (int i=0 ; i<protocols->count ; i++)
			{
				Protocol *p1 = (Protocol*)protocols->list[i];
				if (protocol_conformsToProtocol(p1, protocol))
				{
					return YES;
				}
			}
		}
	}
	return NO;
}

static struct objc_protocol_method_description_list *
get_method_list(Protocol *p,
                BOOL isRequiredMethod,
                BOOL isInstanceMethod)
{
	struct objc_protocol_method_description_list *list;
	if (isRequiredMethod)
	{
		if (isInstanceMethod)
		{
			list = p->instance_methods;
		}
		else
		{
			list = p->class_methods;
		}
	}
	else
	{
		if (!protocol_hasOptionalMethodsAndProperties(p)) { return NULL; }

		if (isInstanceMethod)
		{
			list = p->optional_instance_methods;
		}
		else
		{
			list = p->optional_class_methods;
		}
	}
	return list;
}

struct objc_method_description *protocol_copyMethodDescriptionList(Protocol *p,
	BOOL isRequiredMethod, BOOL isInstanceMethod, unsigned int *count)
{
	if ((NULL == p) || (NULL == count)){ return NULL; }
	struct objc_protocol_method_description_list *list =
		get_method_list(p, isRequiredMethod, isInstanceMethod);
	*count = 0;
	if (NULL == list || list->count == 0) { return NULL; }

	*count = list->count;
	struct objc_method_description *out =
		calloc(sizeof(struct objc_method_description), list->count);
	for (int i=0 ; i < (list->count) ; i++)
	{
		out[i].name = protocol_method_at_index(list, i)->selector;
		out[i].types = sel_getType_np(protocol_method_at_index(list, i)->selector);
	}
	return out;
}

Protocol*__unsafe_unretained* protocol_copyProtocolList(Protocol *p, unsigned int *count)
{
	if (NULL == p) { return NULL; }
	*count = 0;
	if (p->protocol_list == NULL || p->protocol_list->count ==0)
	{
		return NULL;
	}

	*count  = p->protocol_list->count;
	Protocol **out = calloc(sizeof(Protocol*), p->protocol_list->count);
	for (int i=0 ; i<p->protocol_list->count ; i++)
	{
		out[i] = (Protocol*)p->protocol_list->list[i];
	}
	return out;
}

objc_property_t *protocol_copyPropertyList2(Protocol *p, unsigned int *outCount,
		BOOL isRequiredProperty, BOOL isInstanceProperty)
{
	struct objc_property_list *properties =
	    isInstanceProperty ?
	        (isRequiredProperty ? p->properties : p->optional_properties) :
	        (isRequiredProperty ? p->class_properties : p->optional_class_properties);
	if (NULL == p) { return NULL; }
	// If it's an old protocol, it won't have any of the other options.
	if (!isRequiredProperty && !isInstanceProperty &&
	    !protocol_hasOptionalMethodsAndProperties(p))
	{
		return NULL;
	}
	if (properties == NULL)
	{
		return NULL;
	}
	unsigned int count = 0;
	for (struct objc_property_list *l=properties ; l!=NULL ; l=l->next)
	{
		count += l->count;
	}
	if (0 == count)
	{
		return NULL;
	}
	objc_property_t *list = calloc(sizeof(objc_property_t), count);
	unsigned int out = 0;
	for (struct objc_property_list *l=properties ; l!=NULL ; l=l->next)
	{
		for (int i=0 ; i<l->count ; i++)
		{
			list[out++] = property_at_index(l, i);
		}
	}
	*outCount = count;
	return list;
}

objc_property_t *protocol_copyPropertyList(Protocol *p,
                                           unsigned int *outCount)
{
	return protocol_copyPropertyList2(p, outCount, YES, YES);
}

objc_property_t protocol_getProperty(Protocol *p,
                                     const char *name,
                                     BOOL isRequiredProperty,
                                     BOOL isInstanceProperty)
{
	if (NULL == p) { return NULL; }
	if (!protocol_hasOptionalMethodsAndProperties(p))
	{
		return NULL;
	}
	if (!isInstanceProperty && !protocol_hasClassProperties(p))
	{
		return NULL;
	}
	struct objc_property_list *properties =
	    isInstanceProperty ?
	        (isRequiredProperty ? p->properties : p->optional_properties) :
	        (isRequiredProperty ? p->class_properties : p->optional_class_properties);
	while (NULL != properties)
	{
		for (int i=0 ; i<properties->count ; i++)
		{
			objc_property_t prop = property_at_index(properties, i);
			if (strcmp(property_getName(prop), name) == 0)
			{
				return prop;
			}
		}
		properties = properties->next;
	}
	return NULL;
}

static struct objc_protocol_method_description *
get_method_description(Protocol *p,
                       SEL aSel,
                       BOOL isRequiredMethod,
                       BOOL isInstanceMethod)
{
	struct objc_protocol_method_description_list *list =
		get_method_list(p, isRequiredMethod, isInstanceMethod);
	if (NULL == list)
	{
		return NULL;
	}
	for (int i=0 ; i<list->count ; i++)
	{
		SEL s = protocol_method_at_index(list, i)->selector;
		if (sel_isEqual(s, aSel))
		{
			return protocol_method_at_index(list, i);
		}
	}
	return NULL;
}

struct objc_method_description
protocol_getMethodDescription(Protocol *p,
                              SEL aSel,
                              BOOL isRequiredMethod,
                              BOOL isInstanceMethod)
{
	struct objc_method_description d = {0,0};
	struct objc_protocol_method_description *m = 
		get_method_description(p, aSel, isRequiredMethod, isInstanceMethod);
	if (m != NULL)
	{
		SEL s = m->selector;
		d.name = s;
		d.types = sel_getType_np(s);
	}
	return d;
}

const char *_protocol_getMethodTypeEncoding(Protocol *p,
                                            SEL aSel,
                                            BOOL isRequiredMethod,
                                            BOOL isInstanceMethod)
{
	struct objc_protocol_method_description *m = 
		get_method_description(p, aSel, isRequiredMethod, isInstanceMethod);
	if (m != NULL)
	{
		return m->types;
	}
	return NULL;
}


const char *protocol_getName(Protocol *p)
{
	if (NULL != p)
	{
		return p->name;
	}
	return NULL;
}

BOOL protocol_isEqual(Protocol *p, Protocol *other)
{
	if (NULL == p || NULL == other)
	{
		return NO;
	}
	if (p == other ||
		p->name == other->name ||
		0 == strcmp(p->name, other->name))
	{
		return YES;
	}
	return NO;
}

Protocol*__unsafe_unretained* objc_copyProtocolList(unsigned int *outCount)
{
	LOCK_FOR_SCOPE(&protocol_table_lock);
	unsigned int total = known_protocol_table->table_used;
	Protocol **p = calloc(sizeof(Protocol*), known_protocol_table->table_used);

	struct protocol_table_enumerator *e = NULL;
	Protocol *next;

	unsigned int count = 0;
	while ((count < total) && (next = protocol_next(known_protocol_table, &e)))
	{
		p[count++] = next;
	}
	if (NULL != outCount)
	{
		*outCount = total;
	}
	return p;
}


Protocol *objc_allocateProtocol(const char *name)
{
	if (objc_getProtocol(name) != NULL) { return NULL; }
	// Create this as an object and add extra space at the end for the properties.
	Protocol *p = (Protocol*)class_createInstance((Class)incompleteProtocolClass(),
			sizeof(struct objc_protocol) - sizeof(id));
	p->name = strdup(name);
	return p;
}
void objc_registerProtocol(Protocol *proto)
{
	if (NULL == proto) { return; }
	LOCK_FOR_SCOPE(&protocol_table_lock);
	if (objc_getProtocol(proto->name) != NULL) { return; }
	if (incompleteProtocolClass() != proto->isa) { return; }
	init_protocol_classes();
	proto->isa = protocol_class_gsv2;
	protocol_table_insert(proto);
}
PRIVATE void registerProtocol(Protocol *proto)
{
	init_protocol_classes();
	LOCK_FOR_SCOPE(&protocol_table_lock);
	proto->isa = protocol_class_gsv2;
	if (protocol_for_name(proto->name) == NULL)
	{
		protocol_table_insert(proto);
	}
}
void protocol_addMethodDescription(Protocol *aProtocol,
                                   SEL name,
                                   const char *types,
                                   BOOL isRequiredMethod,
                                   BOOL isInstanceMethod)
{
	if ((NULL == aProtocol) || (NULL == name) || (NULL == types)) { return; }
	if (incompleteProtocolClass() != aProtocol->isa) { return; }
	struct objc_protocol_method_description_list **listPtr;
	if (isInstanceMethod)
	{
		if (isRequiredMethod)
		{
			listPtr = &aProtocol->instance_methods;
		}
		else
		{
			listPtr = &aProtocol->optional_instance_methods;
		}
	}
	else
	{
		if (isRequiredMethod)
		{
			listPtr = &aProtocol->class_methods;
		}
		else
		{
			listPtr = &aProtocol->optional_class_methods;
		}
	}
	if (NULL == *listPtr)
	{
		// FIXME: Factor this out, we do the same thing in multiple places.
		*listPtr = calloc(1, sizeof(struct objc_protocol_method_description_list) +
				sizeof(struct objc_protocol_method_description));
		(*listPtr)->count = 1;
		(*listPtr)->size = sizeof(struct objc_protocol_method_description);
	}
	else
	{
		(*listPtr)->count++;
		*listPtr = realloc(*listPtr, sizeof(struct objc_protocol_method_description_list) +
				sizeof(struct objc_protocol_method_description) * (*listPtr)->count);
	}
	struct objc_protocol_method_description_list *list = *listPtr;
	int index = list->count-1;
	protocol_method_at_index(list, index)->selector = sel_registerTypedName_np(sel_getName(name), types);
	protocol_method_at_index(list, index)->types = types;
}
void protocol_addProtocol(Protocol *aProtocol, Protocol *addition)
{
	if ((NULL == aProtocol) || (NULL == addition)) { return; }
	if (incompleteProtocolClass() != aProtocol->isa) { return; }
	if (NULL == aProtocol->protocol_list)
	{
		aProtocol->protocol_list = calloc(1, sizeof(struct objc_property_list) + sizeof(Protocol*));
		aProtocol->protocol_list->count = 1;
	}
	else
	{
		aProtocol->protocol_list->count++;
		aProtocol->protocol_list = realloc(aProtocol->protocol_list, sizeof(struct objc_property_list) +
				aProtocol->protocol_list->count * sizeof(Protocol*));
	}
	aProtocol->protocol_list->list[aProtocol->protocol_list->count-1] = (Protocol*)addition;
}
void protocol_addProperty(Protocol *aProtocol,
                          const char *name,
                          const objc_property_attribute_t *attributes,
                          unsigned int attributeCount,
                          BOOL isRequiredProperty,
                          BOOL isInstanceProperty)
{
	if ((NULL == aProtocol) || (NULL == name)) { return; }
	if (incompleteProtocolClass() != aProtocol->isa) { return; }
	if (!isInstanceProperty) { return; }
	struct objc_property_list **listPtr = 
	    isInstanceProperty ?
	        (isRequiredProperty ? &aProtocol->properties : &aProtocol->optional_properties) :
	        (isRequiredProperty ? &aProtocol->class_properties : &aProtocol->optional_class_properties);
	if (NULL == *listPtr)
	{
		*listPtr = calloc(1, sizeof(struct objc_property_list) + sizeof(struct objc_property));
		(*listPtr)->size = sizeof(struct objc_property);
		(*listPtr)->count = 1;
	}
	else
	{
		(*listPtr)->count++;
		*listPtr = realloc(*listPtr, sizeof(struct objc_property_list) +
				sizeof(struct objc_property) * (*listPtr)->count);
	}
	struct objc_property_list *list = *listPtr;
	int index = list->count-1;
	struct objc_property p = propertyFromAttrs(attributes, attributeCount, name);
	assert(list->size == sizeof(p));
	memcpy(&(list->properties[index]), &p, sizeof(p));
}

