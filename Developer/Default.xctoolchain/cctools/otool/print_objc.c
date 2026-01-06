/*
 * Copyright © 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1.  Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * @APPLE_LICENSE_HEADER_END@
 */
#include "stdio.h"
#include "string.h"
#include "stdint.h"            /* cctools-port: intptr_t */
#include "mach-o/loader.h"
#include "objc/runtime.h"      /* cctools-port:
				  objc/objc-runtime.h -> objc/runtime.h */
#include "stuff/allocate.h"
#include "stuff/bytesex.h"
#include "stuff/symbol.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"

/*
 * Here we need structures that have the same memory layout and size as the
 * 32-bit Objective-C 1 meta data structures.
 *
 * The real structure definitions come from the header file objc/objc-runtime.h
 * and those it includes like objc/objc-class.h, etc.  But since this program
 * must also run on 64-bit hosts that can't be used.
 */

struct objc_module_t {
    uint32_t version;
    uint32_t size;
    uint32_t name;	/* char * (32-bit pointer) */
    uint32_t symtab;	/* struct objc_symtab * (32-bit pointer) */
};

struct objc_symtab_t {
    uint32_t sel_ref_cnt; 
    uint32_t refs;		/* SEL * (32-bit pointer) */
    uint16_t cls_def_cnt;
    uint16_t cat_def_cnt;
    uint32_t defs[1];		/* void * (32-bit pointer) variable size */
};

struct objc_class_t {
    uint32_t isa;	  /* struct objc_class * (32-bit pointer) */
    uint32_t super_class; /* struct objc_class * (32-bit pointer) */
    uint32_t name; 	  /* const char * (32-bit pointer) */
    int32_t version;
    int32_t info;
    int32_t instance_size;
    uint32_t ivars; 	  /* struct objc_ivar_list * (32-bit pointer) */
    uint32_t methodLists; /* struct objc_method_list ** (32-bit pointer) */
    uint32_t cache; 	  /* struct objc_cache * (32-bit pointer) */
    uint32_t protocols;   /* struct objc_protocol_list * (32-bit pointer) */
};

struct objc_category_t {
    uint32_t category_name;	/* char * (32-bit pointer) */
    uint32_t class_name;	/* char * (32-bit pointer) */
    uint32_t instance_methods;	/* struct objc_method_list * (32-bit pointer) */
    uint32_t class_methods;	/* struct objc_method_list * (32-bit pointer) */
    uint32_t protocols;		/* struct objc_protocol_list * (32-bit ptr) */
};

struct objc_ivar_t {
    uint32_t ivar_name;		/* char * (32-bit pointer) */
    uint32_t ivar_type;		/* char * (32-bit pointer) */
    int32_t ivar_offset;
};

struct objc_ivar_list_t {
    int32_t ivar_count;
    struct objc_ivar_t ivar_list[1];          /* variable length structure */
};

struct objc_method_t {
    uint32_t method_name; /* SEL, aka struct objc_selector * (32-bit pointer) */
    uint32_t method_types; /* char * (32-bit pointer) */
    uint32_t method_imp; /* IMP, aka function pointer, (*IMP)(id, SEL, ...)
			    (32-bit pointer) */
};

struct objc_method_list_t {
    uint32_t obsolete;		/* struct objc_method_list * (32-bit pointer) */
    int32_t method_count;
    struct objc_method_t method_list[1];      /* variable length structure */
};

struct objc_protocol_t {
    uint32_t isa;		/* struct objc_class * (32-bit pointer) */
    uint32_t protocol_name;	/* char * (32-bit pointer) */
    uint32_t protocol_list;	/* struct objc_protocol_list *
				   (32-bit pointer) */
    uint32_t instance_methods;	/* struct objc_method_description_list *
				   (32-bit pointer) */
    uint32_t class_methods;	/* struct objc_method_description_list *
				   (32-bit pointer) */
};

struct objc_protocol_list_t {
    uint32_t next;	/* struct objc_protocol_list * (32-bit pointer) */
    int32_t count;
    uint32_t list[1];	/* Protocol *, aka struct objc_protocol_t * (
			   (32-bit pointer) */
};

struct objc_method_description_t {
    uint32_t name;	/* SEL, aka struct objc_selector * (32-bit pointer) */
    uint32_t types;	/* char * (32-bit pointer) */
};

struct objc_method_description_list_t {
    int32_t count;
    struct objc_method_description_t list[1];
};


/*
 * The header file "objc/NXString.h" has gone away and there is no real public
 * header file to get this definition from anymore.
 */
struct objc_string_object_t {
    uint32_t isa;		/* struct objc_class * (32-bit pointer) */
    uint32_t characters;	/* char * (32-bit pointer) */
    uint32_t _length;
};
typedef struct objc_string_object_t NXConstantString;

#define SIZEHASHTABLE 821
struct _hashEntry_t {
    uint32_t next;	/* struct _hashEntry * (32-bit pointer) */
    uint32_t sel;	/* char * (32-bit pointer) */
};

struct imageInfo_t {
    uint32_t version;
    uint32_t flags;
};

static
void
swap_imageInfo_t(
struct imageInfo_t *o,
enum byte_sex target_byte_sex)
{
	o->version = SWAP_INT(o->version);
	o->flags = SWAP_INT(o->flags);
}

void
swap_objc_module_t(
struct objc_module_t *module,
enum byte_sex target_byte_sex)
{
	module->version = SWAP_INT(module->version);
	module->size = SWAP_INT(module->size);
	module->name = SWAP_INT(module->name);
	module->symtab = SWAP_INT(module->symtab);
}

void
swap_objc_symtab_t(
struct objc_symtab_t *symtab,
enum byte_sex target_byte_sex)
{
	symtab->sel_ref_cnt = SWAP_INT(symtab->sel_ref_cnt);
	symtab->refs = SWAP_INT(symtab->refs);
	symtab->cls_def_cnt = SWAP_SHORT(symtab->cls_def_cnt);
	symtab->cat_def_cnt = SWAP_SHORT(symtab->cat_def_cnt);
}

void
swap_objc_class_t(
struct objc_class_t *objc_class,
enum byte_sex target_byte_sex)
{
	objc_class->isa = SWAP_INT(objc_class->isa);
	objc_class->super_class = SWAP_INT(objc_class->super_class);
	objc_class->name = SWAP_INT(objc_class->name);		
	objc_class->version = SWAP_INT(objc_class->version);
	objc_class->info = SWAP_INT(objc_class->info);
	objc_class->instance_size = SWAP_INT(objc_class->instance_size);
	objc_class->ivars = SWAP_INT(objc_class->ivars);
	objc_class->methodLists = SWAP_INT(objc_class->methodLists);
	objc_class->cache = SWAP_INT(objc_class->cache);
	objc_class->protocols = SWAP_INT(objc_class->protocols);
}

void
swap_objc_category_t(
struct objc_category_t *objc_category,
enum byte_sex target_byte_sex)
{
	objc_category->category_name = SWAP_INT(objc_category->category_name);
	objc_category->class_name = SWAP_INT(objc_category->class_name);
	objc_category->instance_methods =
		SWAP_INT(objc_category->instance_methods);
	objc_category->class_methods =
		SWAP_INT(objc_category->class_methods);
	objc_category->protocols =
		SWAP_INT(objc_category->protocols);
}

void
swap_objc_ivar_list_t(
struct objc_ivar_list_t *objc_ivar_list,
enum byte_sex target_byte_sex)
{
	objc_ivar_list->ivar_count = SWAP_INT(objc_ivar_list->ivar_count);
}

void
swap_objc_ivar_t(
struct objc_ivar_t *objc_ivar,
enum byte_sex target_byte_sex)
{
	objc_ivar->ivar_name = SWAP_INT(objc_ivar->ivar_name);
	objc_ivar->ivar_type = SWAP_INT(objc_ivar->ivar_type);
	objc_ivar->ivar_offset = SWAP_INT(objc_ivar->ivar_offset);
}

void
swap_objc_method_list_t(
struct objc_method_list_t *method_list,
enum byte_sex target_byte_sex)
{
	method_list->obsolete = SWAP_INT(method_list->obsolete);
	method_list->method_count = SWAP_INT(method_list->method_count);
}

void
swap_objc_method_t(
struct objc_method_t *method,
enum byte_sex target_byte_sex)
{
	method->method_name = SWAP_INT(method->method_name);
	method->method_types = SWAP_INT(method->method_types);
	method->method_imp = SWAP_INT(method->method_imp);
}

void
swap_objc_protocol_list_t(
struct objc_protocol_list_t *protocol_list,
enum byte_sex target_byte_sex)
{
	protocol_list->next = SWAP_INT(protocol_list->next);
	protocol_list->count = SWAP_INT(protocol_list->count);
}

void
swap_objc_protocol_t(
struct objc_protocol_t *protocol,
enum byte_sex target_byte_sex)
{
	protocol->isa = SWAP_INT(protocol->isa);
	protocol->protocol_name = SWAP_INT(protocol->protocol_name);
	protocol->protocol_list = SWAP_INT(protocol->protocol_list);
	protocol->instance_methods = SWAP_INT(protocol->instance_methods);
	protocol->class_methods = SWAP_INT(protocol->class_methods);
}

void
swap_objc_method_description_list_t(
struct objc_method_description_list_t *mdl,
enum byte_sex target_byte_sex)
{
	mdl->count = SWAP_INT(mdl->count);
}

void
swap_objc_method_description_t(
struct objc_method_description_t *md,
enum byte_sex target_byte_sex)
{
	md->name = SWAP_INT(md->name);
	md->types = SWAP_INT(md->types);
}

void
swap_string_object_t(
struct objc_string_object_t *string_object,
enum byte_sex target_byte_sex)
{
	string_object->isa = SWAP_INT(string_object->isa);
	string_object->characters = SWAP_INT(string_object->characters);
	string_object->_length = SWAP_INT(string_object->_length);
}

void
swap_hashEntry_t(
struct _hashEntry_t *_hashEntry,
enum byte_sex target_byte_sex)
{
	_hashEntry->next = SWAP_INT(_hashEntry->next);
	_hashEntry->sel = SWAP_INT(_hashEntry->sel);
}

struct section_info {
    char *contents;
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    enum bool protected;
    enum bool zerofill;
};

static void get_objc_sections(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct section_info **objc_sections,
    uint32_t *nobjc_sections,
    char *sectname,
    char **sect,
    uint32_t *sect_addr,
    uint32_t *sect_size);

static void get_cstring_section(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct section_info *cstring_section_ptr);

static enum bool print_method_list(
    uint32_t addr,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr,
    enum byte_sex host_byte_sex,
    enum bool swapped,
    struct symbol *sorted_symbols,
    uint32_t nsorted_symbols,
    enum bool verbose);

static enum bool print_protocol_list(
    uint32_t indent,
    uint32_t addr,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr,
    enum byte_sex host_byte_sex,
    enum bool swapped,
    enum bool verbose);

static void print_protocol(
    uint32_t indent,
    struct objc_protocol_t *protocol,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr,
    enum byte_sex host_byte_sex,
    enum bool swapped,
    enum bool verbose);

static enum bool print_method_description_list(
    uint32_t indent,
    uint32_t addr,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr,
    enum byte_sex host_byte_sex,
    enum bool swapped,
    enum bool verbose);

static enum bool print_PHASH(
    uint32_t indent,
    uint32_t addr,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr,
    enum byte_sex host_byte_sex,
    enum bool swapped,
    enum bool verbose);

static void print_indent(
    uint32_t indent);

static void *get_pointer(
    uint32_t addr,
    uint32_t *left,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    struct section_info *cstring_section_ptr);

static enum bool get_symtab(
    uint32_t addr,
    struct objc_symtab_t *symtab,
    uint32_t **defs,
    uint32_t *left,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_objc_class(
    uint32_t addr,
    struct objc_class_t *objc_class,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_objc_category(
    uint32_t addr,
    struct objc_category_t *objc_category,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_ivar_list(
    uint32_t addr,
    struct objc_ivar_list_t *objc_ivar_list,
    struct objc_ivar_t **ivar_list,
    uint32_t *left,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_method_list(
    uint32_t addr,
    struct objc_method_list_t *method_list,
    struct objc_method_t **methods,
    uint32_t *left,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_protocol_list(
    uint32_t addr,
    struct objc_protocol_list_t *protocol_list,
    uint32_t **list,
    uint32_t *left,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_protocol(
    uint32_t addr,
    struct objc_protocol_t *protocol,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_method_description_list(
    uint32_t addr,
    struct objc_method_description_list_t *mdl,
    struct objc_method_description_t **list,
    uint32_t *left,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

static enum bool get_hashEntry(
    uint32_t addr,
    struct _hashEntry_t *_hashEntry,
    enum bool *trunc,
    struct section_info *objc_sections,
    uint32_t nobjc_sections,
    enum byte_sex host_byte_sex,
    enum bool swapped);

/*
 * Print the objc segment.
 */
enum bool
print_objc_segment(
cpu_type_t mh_cputype,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped, trunc;
    uint32_t i, j, left, size, defs_left, def, ivar_list_left;
    char *p;
    struct section_info *objc_sections;
    uint32_t nobjc_sections;
    struct section_info cstring_section;

    struct objc_module_t *modules, *m, module;
    uint32_t modules_addr, modules_size;
    struct objc_symtab_t symtab;
    uint32_t *defs;
    struct objc_class_t objc_class;
    struct objc_ivar_list_t objc_ivar_list;
    struct objc_ivar_t *ivar_list, ivar;
    struct objc_category_t objc_category;

    struct imageInfo_t *imageInfo, info;
    uint32_t imageInfo_addr, imageInfo_size;

	printf("Objective-C segment\n");
	get_objc_sections(load_commands, ncmds, sizeofcmds, object_byte_sex,
			  object_addr, object_size, &objc_sections,
			  &nobjc_sections, SECT_OBJC_MODULES, (char **)&modules,
			  &modules_addr, &modules_size);

	if(modules == NULL){
	    if(mh_cputype == CPU_TYPE_I386)
		return(FALSE);
	    printf("can't print objective-C information no (" SEG_OBJC ","
		   SECT_OBJC_MODULES ") section\n");
	    return(TRUE);
	}

    if (verbose)
        get_cstring_section(load_commands, ncmds, sizeofcmds, object_byte_sex,
                            object_addr, object_size, &cstring_section);

    host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	memset(&module, '\0', sizeof(struct objc_module_t));

	for(m = modules;
	    (char *)m < (char *)modules + modules_size;
	    m = (struct objc_module_t *)((char *)m + module.size) ){

	    memset(&module, '\0', sizeof(struct objc_module_t));
	    left = (uint32_t)(modules_size - (m - modules));
	    size = left < sizeof(struct objc_module_t) ?
		   left : sizeof(struct objc_module_t);
	    memcpy(&module, m, size);
	    if(swapped)
		swap_objc_module_t(&module, host_byte_sex);

	    if((char *)m + module.size > (char *)m + modules_size)
		printf("module extends past end of " SECT_OBJC_MODULES
		       " section\n");
	    printf("Module 0x%x\n", (unsigned int)
		   (modules_addr + (char *)m - (char *)modules));

	    printf("    version %u\n", module.version);
	    printf("       size %u\n", module.size);
	    if(verbose){
		p = get_pointer(module.name, &left,
		    objc_sections, nobjc_sections, &cstring_section);
		if(p != NULL)
		    printf("       name %.*s\n", (int)left, p);
		else
		    printf("       name 0x%08x (not in an " SEG_OBJC
			   " section)\n", (unsigned int)module.name);
	    }
	    else
		printf("       name 0x%08x\n", (unsigned int)(module.name));

	    if(get_symtab(module.symtab, &symtab, &defs, &defs_left, &trunc,
		    objc_sections, nobjc_sections,
		    host_byte_sex, swapped) == FALSE){
		printf("     symtab 0x%08x (not in an " SEG_OBJC
		       " section)\n", (unsigned int)module.symtab);
		continue;
	    }
	    printf("     symtab 0x%08x\n", (unsigned int)module.symtab);
	    if(trunc == TRUE)
		printf("\tsymtab extends past end of an " SEG_OBJC
		       " section\n");
	    printf("\tsel_ref_cnt %u\n", symtab.sel_ref_cnt);
	    p = get_pointer(symtab.refs, &left,
                     objc_sections, nobjc_sections, &cstring_section);
	    if(p != NULL)
		printf("\trefs 0x%08x", symtab.refs);
	    else
		printf("\trefs 0x%08x (not in an " SEG_OBJC " section)\n",
		       symtab.refs);

	    printf("\tcls_def_cnt %d\n", symtab.cls_def_cnt);
	    printf("\tcat_def_cnt %d\n", symtab.cat_def_cnt);
	    if(symtab.cls_def_cnt > 0)
		printf("\tClass Definitions\n");
	    for(i = 0; i < symtab.cls_def_cnt; i++){
		if((i + 1) * sizeof(uint32_t) > defs_left){
		    printf("\t(remaining class defs entries entends past "
			   "the end of the section)\n");
		    break;
		}
	    
		memcpy(&def, defs + i, sizeof(uint32_t));
		if(swapped)
		    def = SWAP_INT(def);

		if(get_objc_class(def, &objc_class, &trunc, objc_sections,
			  nobjc_sections, host_byte_sex, swapped) == TRUE){
		    printf("\tdefs[%u] 0x%08x", i, def);
print_objc_class:
		    if(trunc == TRUE)
			printf(" (entends past the end of the section)\n");
		    else
			printf("\n");
		    printf("\t\t      isa 0x%08x", objc_class.isa);

		    /* cctools-port: added (const char*)(intptr_t) */
		    if(verbose && objc_getMetaClass((const char*)(intptr_t)objc_class.name)){
			p = get_pointer(objc_class.isa, &left, objc_sections,
					nobjc_sections, &cstring_section);
			if(p != NULL)
			    printf(" %.*s\n", (int)left, p);
			else
			    printf(" (not in an " SEG_OBJC " section)\n");
		    }
		    else
			printf("\n");

		    printf("\t      super_class 0x%08x",objc_class.super_class);
		    if(verbose){
			p = get_pointer(objc_class.super_class, &left,
					objc_sections, nobjc_sections, &cstring_section);
			if(p != NULL)
			    printf(" %.*s\n", (int)left, p);
			else
			    printf(" (not in an " SEG_OBJC " section)\n");
		    }
		    else
			printf("\n");

		    printf("\t\t     name 0x%08x", objc_class.name);
		    if(verbose){
			p = get_pointer(objc_class.name, &left,
					objc_sections, nobjc_sections, &cstring_section);
			if(p != NULL)
			    printf(" %.*s\n", (int)left, p);
			else
			    printf(" (not in an " SEG_OBJC " section)\n");
		    }
		    else
			printf("\n");
		    printf("\t\t  version 0x%08x\n",
			   (unsigned int)objc_class.version);
		    printf("\t\t     info 0x%08x",
			   (unsigned int)objc_class.info);
		    if(verbose){
			/* cctools-port: added (const char*)(intptr_t) */
			if(objc_getClass((const char*)(intptr_t)objc_class.name))
			    printf(" CLS_CLASS\n");
			/* cctools-port: added (const char*)(intptr_t) */
			else if(objc_getMetaClass((const char*)(intptr_t)objc_class.name))
			    printf(" CLS_META\n");
			else
			    printf("\n");
		    }
		    else
			printf("\n");
		    printf("\t    instance_size 0x%08x\n",
			   (unsigned int)objc_class.instance_size);

		    if(get_ivar_list(objc_class.ivars, &objc_ivar_list,
			    &ivar_list, &ivar_list_left, &trunc,
			    objc_sections, nobjc_sections, host_byte_sex,
			    swapped) == TRUE){
			printf("\t\t    ivars 0x%08x\n",
			       (unsigned int)objc_class.ivars);
			if(trunc == TRUE)
			    printf("\t\t objc_ivar_list extends past end "
				   "of " SECT_OBJC_SYMBOLS " section\n");
			printf("\t\t       ivar_count %d\n", 
				    objc_ivar_list.ivar_count);
			for(j = 0;
			    j < (uint32_t)objc_ivar_list.ivar_count;
			    j++){
			    if((j + 1) * sizeof(struct objc_ivar_t) >
			       ivar_list_left){
				printf("\t\t remaining ivar's extend past "
				       "the of the section\n");
				break;
			    }
			    memcpy(&ivar, ivar_list + j,
				   sizeof(struct objc_ivar_t));
			    if(swapped)
				swap_objc_ivar_t(&ivar, host_byte_sex);

			    printf("\t\t\tivar_name 0x%08x",
				   (unsigned int)ivar.ivar_name);
			    if(verbose){
				p = get_pointer(ivar.ivar_name, &left,
					    objc_sections, nobjc_sections, &cstring_section);
				if(p != NULL)
				    printf(" %.*s\n", (int)left, p);
				else
				    printf(" (not in an " SEG_OBJC
					   " section)\n");
			    }
			    else
				printf("\n");
			    printf("\t\t\tivar_type 0x%08x",
				   (unsigned int)ivar.ivar_type);
			    if(verbose){
				p = get_pointer(ivar.ivar_type, &left,
					    objc_sections, nobjc_sections, &cstring_section);
				if(p != NULL)
				    printf(" %.*s\n", (int)left, p);
				else
				    printf(" (not in an " SEG_OBJC
					   " section)\n");
			    }
			    else
				printf("\n");
			    printf("\t\t      ivar_offset 0x%08x\n",
				   (unsigned int)ivar.ivar_offset);
			}
		    }
		    else{
			printf("\t\t    ivars 0x%08x (not in an " SEG_OBJC
			       " section)\n",
			       (unsigned int)objc_class.ivars);
		    }

		    printf("\t\t  methods 0x%08x",
			   (unsigned int)objc_class.methodLists);
		    if(print_method_list(objc_class.methodLists,
					 objc_sections, nobjc_sections,
					 &cstring_section,
					 host_byte_sex, swapped, sorted_symbols,
					 nsorted_symbols, verbose) == FALSE)
			printf(" (not in an " SEG_OBJC " section)\n");

		    printf("\t\t    cache 0x%08x\n",
			   (unsigned int)objc_class.cache);

		    printf("\t\tprotocols 0x%08x",
			   (unsigned int)objc_class.protocols);
		    if(print_protocol_list(16, objc_class.protocols,
			objc_sections, nobjc_sections, &cstring_section,
			host_byte_sex, swapped, verbose) == FALSE)
			printf(" (not in an " SEG_OBJC " section)\n");

		    /* cctools-port: added (const char*)(intptr_t) */
		    if(objc_getClass((const char*)(intptr_t)objc_class.name)){
			printf("\tMeta Class");
			if(get_objc_class((uint32_t)objc_class.isa,
			     &objc_class, &trunc, objc_sections, nobjc_sections,
			     host_byte_sex, swapped) == TRUE){
			    goto print_objc_class;
			}
			else
			    printf(" (not in " SECT_OBJC_SYMBOLS
				   " section)\n");
		    }
		}
		else
		    printf("\tdefs[%u] 0x%08x (not in an " SEG_OBJC
			   " section)\n", i, (unsigned int)def);
	    }
	    if(symtab.cat_def_cnt > 0)
		printf("\tCategory Definitions\n");
	    for(i = 0; i < symtab.cat_def_cnt; i++){
		if((i + symtab.cls_def_cnt + 1) * sizeof(uint32_t) >
							      defs_left){
		    printf("\t(remaining category defs entries entends "
			   "past the end of the section)\n");
		    break;
		}
	    
		memcpy(&def, defs + i + symtab.cls_def_cnt, sizeof(uint32_t));
		if(swapped)
		    def = SWAP_INT(def);

		if(get_objc_category(def, &objc_category, &trunc,
			  objc_sections, nobjc_sections,
              host_byte_sex, swapped) == TRUE){
		    printf("\tdefs[%u] 0x%08x", i + symtab.cls_def_cnt,
			   (unsigned int)def);
		    if(trunc == TRUE)
			printf(" (entends past the end of the section)\n");
		    else
			printf("\n");
		    printf("\t       category name 0x%08x",
			   objc_category.category_name);
		    if(verbose){
			p = get_pointer(objc_category.category_name, &left,
					objc_sections, nobjc_sections,
					&cstring_section);
			if(p != NULL)
			    printf(" %.*s\n", (int)left, p);
			else
			    printf(" (not in an " SEG_OBJC " section)\n");
		    }
		    else
			printf("\n");

		    printf("\t\t  class name 0x%08x", objc_category.class_name);
		    if(verbose){
			p = get_pointer(objc_category.class_name, &left,
					objc_sections, nobjc_sections,
					&cstring_section);
			if(p != NULL)
			    printf(" %.*s\n", (int)left, p);
			else
			    printf(" (not in an " SEG_OBJC " section)\n");
		    }
		    else
			printf("\n");

		    printf("\t    instance methods 0x%08x",
			   objc_category.instance_methods);
		    if(print_method_list(objc_category.instance_methods,
					 objc_sections, nobjc_sections,
					 &cstring_section,
					 host_byte_sex, swapped,
					 sorted_symbols, nsorted_symbols,
					 verbose) == FALSE)
			printf(" (not in an " SEG_OBJC " section)\n");

		    printf("\t       class methods 0x%08x",
			   objc_category.class_methods);
		    if(print_method_list(objc_category.class_methods,
			objc_sections, nobjc_sections, &cstring_section,
			host_byte_sex, swapped, sorted_symbols, nsorted_symbols,
			verbose) == FALSE)
			printf(" (not in an " SEG_OBJC " section)\n");
		}
		else
		    printf("\tdefs[%u] 0x%08x (not in an " SEG_OBJC
			   " section)\n", i + symtab.cls_def_cnt,
			   (unsigned int)def);
	    }
	}

	printf("Contents of (%s,%s) section\n", SEG_OBJC, "__image_info");
	get_objc_sections(load_commands, ncmds, sizeofcmds, object_byte_sex,
			  object_addr, object_size, &objc_sections,
			  &nobjc_sections, "__image_info", (char **)&imageInfo,
			  &imageInfo_addr, &imageInfo_size);
	memset(&info, '\0', sizeof(struct imageInfo_t));
	if(imageInfo_size < sizeof(struct imageInfo_t)){
	    memcpy(&info, imageInfo, imageInfo_size);
	    printf(" (imageInfo entends past the end of the section)\n");
	}
	else
	    memcpy(&info, imageInfo, sizeof(struct imageInfo_t));
	if(swapped)
	    swap_imageInfo_t(&info, host_byte_sex);
	printf("  version %u\n", info.version);
	printf("    flags 0x%x", info.flags);
	if(info.flags & 0x1)
	    printf("  F&C");
	if(info.flags & 0x2)
	    printf(" GC");
	if(info.flags & 0x4)
	    printf(" GC-only");
	else
	    printf(" RR");
	printf("\n");
	return(TRUE);
}

void
print_objc_protocol_section(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct section_info *objc_sections, cstring_section;
    uint32_t nobjc_sections;
    struct objc_protocol_t *protocols, *p, protocol;
    uint32_t protocols_addr, protocols_size;
    uint32_t size, left;

	printf("Contents of (" SEG_OBJC ",__protocol) section\n");
	get_objc_sections(load_commands, ncmds, sizeofcmds, object_byte_sex,
			  object_addr, object_size, &objc_sections,
			  &nobjc_sections, "__protocol", (char **)&protocols,
			  &protocols_addr, &protocols_size);

    if (verbose)
        get_cstring_section(load_commands, ncmds, sizeofcmds, object_byte_sex,
                            object_addr, object_size, &cstring_section);

    host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	for(p = protocols; (char *)p < (char *)protocols + protocols_size; p++){

	    memset(&protocol, '\0', sizeof(struct objc_protocol_t));
	    left = (uint32_t)(protocols_size - (p - protocols));
	    size = left < sizeof(struct objc_protocol_t) ?
		   left : sizeof(struct objc_protocol_t);
	    memcpy(&protocol, p, size);

	    if((char *)p + sizeof(struct objc_protocol_t) >
	       (char *)p + protocols_size)
		printf("Protocol extends past end of __protocol section\n");
	    printf("Protocol 0x%x\n", (unsigned int)
		   (protocols_addr + (char *)p - (char *)protocols));

	    print_protocol(0, &protocol,
			      objc_sections, nobjc_sections, &cstring_section,
			      host_byte_sex, swapped, verbose);
	}
}

void
print_objc_string_object_section(
char *sectname,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
enum bool verbose)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct section_info *objc_sections;
    uint32_t nobjc_sections;
    struct section_info cstring_section;
    struct objc_string_object_t *string_objects, *s, string_object;
    uint32_t string_objects_addr, string_objects_size;
    uint32_t size, left;
    char *p;

	printf("Contents of (" SEG_OBJC ",%s) section\n", sectname);
	get_objc_sections(load_commands, ncmds, sizeofcmds, object_byte_sex,
			  object_addr, object_size, &objc_sections,
			  &nobjc_sections, sectname, (char **)&string_objects,
			  &string_objects_addr, &string_objects_size);

	get_cstring_section(load_commands, ncmds, sizeofcmds, object_byte_sex,
			    object_addr, object_size, &cstring_section);

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	for(s = string_objects;
	    (char *)s < (char *)string_objects + string_objects_size;
	    s++){

	    memset(&string_object, '\0', sizeof(struct objc_string_object_t));
	    left = (uint32_t)(string_objects_size - (s - string_objects));
	    size = left < sizeof(struct objc_string_object_t) ?
		   left : sizeof(struct objc_string_object_t);
	    memcpy(&string_object, s, size);

	    if((char *)s + sizeof(struct objc_string_object_t) >
	       (char *)s + string_objects_size)
		printf("String Object extends past end of %s section\n",
		       sectname);
	    printf("String Object 0x%x\n", (unsigned int)
		   (string_objects_addr + (char *)s - (char *)string_objects));

	    if(swapped)
		swap_string_object_t(&string_object, host_byte_sex);
	    printf("           isa 0x%x\n", string_object.isa);
	    printf("    characters 0x%x",
		   (unsigned int)string_object.characters);
	    if(verbose){
		p = get_pointer(string_object.characters, &left,
			        &cstring_section, 1, &cstring_section);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in the (" SEG_TEXT ",__cstring) section)\n");
	    }
	    else
		printf("\n");
	    printf("       _length %u\n", string_object._length);
	}
}

/*
 * PHASH[SIZEHASHTABLE];
 * HASH[?]; variable sized (computed from size of section).
 */
void
print_objc_runtime_setup_section(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
enum bool verbose)
{

    enum byte_sex host_byte_sex;
    enum bool swapped;
    struct section_info *objc_sections, cstring_section;
    uint32_t i, nobjc_sections, left;
    struct _hashEntry_t **PHASH, *HASH, _hashEntry;
    char *sect, *p;
    uint32_t sect_addr, sect_size, phash;

	printf("Contents of (" SEG_OBJC ",__runtime_setup) section\n");
	get_objc_sections(load_commands, ncmds, sizeofcmds, object_byte_sex,
			  object_addr, object_size, &objc_sections,
			  &nobjc_sections, "__runtime_setup", &sect, &sect_addr,
			  &sect_size);

    if (verbose)
        get_cstring_section(load_commands, ncmds, sizeofcmds, object_byte_sex,
                            object_addr, object_size, &cstring_section);

    host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	PHASH = (struct _hashEntry_t **)sect;
	for(i = 0;
	    i < SIZEHASHTABLE && (i + 1) * sizeof(uint32_t) < sect_size;
	    i++){

	    memcpy(&phash, PHASH + i, sizeof(uint32_t));
	    if(swapped)
		phash = SWAP_INT(phash);

	    if(phash == 0)
		continue;

	    printf("PHASH[%3u] 0x%x", i, (unsigned int)phash);
	    if(print_PHASH(4, phash,
			   objc_sections, nobjc_sections, &cstring_section,
			   host_byte_sex, swapped, verbose) == FALSE)
		printf(" (not in an " SEG_OBJC " section)\n");
	}

	HASH = (struct _hashEntry_t *)(PHASH + SIZEHASHTABLE);
	for(i = 0; (char *)(HASH + i) < sect + sect_size; i++){
	    memcpy((char *)&_hashEntry, HASH + i, sizeof(struct _hashEntry_t));
	    if(swapped)
		swap_hashEntry_t(&_hashEntry, host_byte_sex);

	    printf("HASH at 0x%08x\n",
		   (unsigned int)(sect_addr + (char *)(HASH + i) - sect));
	    printf("     sel 0x%08x", (unsigned int)_hashEntry.sel);
	    if(verbose){
		p = get_pointer(_hashEntry.sel, &left, objc_sections,
				nobjc_sections, &cstring_section);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in an " SEG_OBJC " section)\n");
	    }
	    else
		printf("\n");

	    printf("    next 0x%08x", (unsigned int)_hashEntry.next);
	    if(_hashEntry.next == 0){
		printf("\n");
	    }
	    else{
		if((uint32_t)_hashEntry.next < sect_addr || 
		   (uint32_t)_hashEntry.next >= sect_addr + sect_size)
		    printf(" (not in the ("SEG_OBJC ",__runtime_setup "
			   "section)\n");
		else
		    printf("\n");
	    }
	}
}

static
void
get_objc_sections(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct section_info **objc_sections,
uint32_t *nobjc_sections,
char *sectname,
char **sect,
uint32_t *sect_addr,
uint32_t *sect_size)
{
    enum byte_sex host_byte_sex;
    enum bool swapped, encrypt_found, encrypt64_found;

    uint32_t i, j, left, size;
    struct load_command lcmd, *lc;
    char *p;
    struct segment_command sg;
    struct section s;
    struct encryption_info_command encrypt;
    struct encryption_info_command_64 encrypt64;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	*objc_sections = NULL;
	*nobjc_sections = 0;
	*sect = NULL;
	*sect_addr = 0;
	*sect_size = 0;
	encrypt_found = FALSE;
	encrypt64_found = FALSE;

	lc = load_commands;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&lcmd, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&lcmd, host_byte_sex);
	    if(lcmd.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    if((char *)lc + lcmd.cmdsize >
	       (char *)load_commands + sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = (uint32_t)(sizeofcmds - ((char *)lc-(char *)load_commands));

	    switch(lcmd.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);

		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past "
			       "end of load commands\n");
		    }
		    left = (uint32_t)(sizeofcmds - (p - (char *)load_commands));
		    memset((char *)&s, '\0', sizeof(struct section));
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);

		    if(strcmp(s.segname, SEG_OBJC) == 0){
			*objc_sections = reallocate(*objc_sections,
			   sizeof(struct section_info) * (*nobjc_sections + 1));
			(*objc_sections)[*nobjc_sections].addr = s.addr;
			(*objc_sections)[*nobjc_sections].contents = 
							 object_addr + s.offset;
			(*objc_sections)[*nobjc_sections].offset = s.offset;
		        (*objc_sections)[*nobjc_sections].zerofill =
			  (s.flags & SECTION_TYPE) == S_ZEROFILL ? TRUE : FALSE;
			if(s.offset > object_size){
			    printf("section contents of: (%.16s,%.16s) is past "
				   "end of file\n", s.segname, s.sectname);
			    (*objc_sections)[*nobjc_sections].size =  0;
			}
			else if(s.offset + s.size > object_size){
			    printf("part of section contents of: (%.16s,%.16s) "
				   "is past end of file\n",
				   s.segname, s.sectname);
			    (*objc_sections)[*nobjc_sections].size =
				(uint32_t)(object_size - s.offset);
			}
			else
			    (*objc_sections)[*nobjc_sections].size = s.size;

			if(strncmp(s.sectname, sectname, 16) == 0){
			    if(*sect != NULL)
				printf("more than one (" SEG_OBJC ",%s) "
				       "section\n", sectname);
			    else{
				*sect = (object_addr + s.offset);
				*sect_size = s.size;
				*sect_addr = s.addr;
			    }
			}
			if(sg.flags & SG_PROTECTED_VERSION_1)
			    (*objc_sections)[*nobjc_sections].protected = TRUE;
			else
			    (*objc_sections)[*nobjc_sections].protected = FALSE;
			(*nobjc_sections)++;
		    }

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		}
		break;
	    case LC_ENCRYPTION_INFO:
		memset((char *)&encrypt, '\0',
		       sizeof(struct encryption_info_command));
		size = left < sizeof(struct encryption_info_command) ?
		       left : sizeof(struct encryption_info_command);
		memcpy((char *)&encrypt, (char *)lc, size);
		if(swapped)
		    swap_encryption_command(&encrypt, host_byte_sex);
		encrypt_found = TRUE;
		break;
	    case LC_ENCRYPTION_INFO_64:
		memset((char *)&encrypt64, '\0',
		       sizeof(struct encryption_info_command_64));
		size = left < sizeof(struct encryption_info_command_64) ?
		       left : sizeof(struct encryption_info_command_64);
		memcpy((char *)&encrypt64, (char *)lc, size);
		if(swapped)
		    swap_encryption_command_64(&encrypt64, host_byte_sex);
		encrypt64_found = TRUE;
		break;
	    }
	    if(lcmd.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lcmd.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}

	if(encrypt_found == TRUE && encrypt.cryptid != 0){
	    for(i = 0; i < *nobjc_sections; i++){
		if((*objc_sections)[i].size > 0 &&
		   (*objc_sections)[i].zerofill == FALSE){
		    if((*objc_sections)[i].offset >
		       encrypt.cryptoff + encrypt.cryptsize){
			/* section starts past encryption area */ ;
		    }
		    else if((*objc_sections)[i].offset +
			    (*objc_sections)[i].size < encrypt.cryptoff){
			/* section ends before encryption area */ ;
		    }
		    else{
			/* section has part in the encrypted area */
			(*objc_sections)[i].protected = TRUE;
		    }
		}
	    }
	}
	if(encrypt64_found == TRUE && encrypt64.cryptid != 0){
	    for(i = 0; i < *nobjc_sections; i++){
		if((*objc_sections)[i].size > 0 &&
		   (*objc_sections)[i].zerofill == FALSE){
		    if((*objc_sections)[i].offset >
		       encrypt64.cryptoff + encrypt64.cryptsize){
			/* section starts past encryption area */ ;
		    }
		    else if((*objc_sections)[i].offset +
			    (*objc_sections)[i].size < encrypt64.cryptoff){
			/* section ends before encryption area */ ;
		    }
		    else{
			/* section has part in the encrypted area */
			(*objc_sections)[i].protected = TRUE;
		    }
		}
	    }
	}
}

static
void
get_cstring_section(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct section_info *cstring_section)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;

    uint32_t i, j, left, size;
    struct load_command lcmd, *lc;
    char *p;
    struct segment_command sg;
    struct section s;

	host_byte_sex = get_host_byte_sex();
	swapped = host_byte_sex != object_byte_sex;

	memset(cstring_section, '\0', sizeof(struct section_info));

	lc = load_commands;
	for(i = 0 ; i < ncmds; i++){
	    memcpy((char *)&lcmd, (char *)lc, sizeof(struct load_command));
	    if(swapped)
		swap_load_command(&lcmd, host_byte_sex);
	    if(lcmd.cmdsize % sizeof(int32_t) != 0)
		printf("load command %u size not a multiple of "
		       "sizeof(int32_t)\n", i);
	    if((char *)lc + lcmd.cmdsize >
	       (char *)load_commands + sizeofcmds)
		printf("load command %u extends past end of load "
		       "commands\n", i);
	    left = (uint32_t)(sizeofcmds - ((char *)lc-(char *)load_commands));

	    switch(lcmd.cmd){
	    case LC_SEGMENT:
		memset((char *)&sg, '\0', sizeof(struct segment_command));
		size = left < sizeof(struct segment_command) ?
		       left : sizeof(struct segment_command);
		memcpy((char *)&sg, (char *)lc, size);
		if(swapped)
		    swap_segment_command(&sg, host_byte_sex);

		p = (char *)lc + sizeof(struct segment_command);
		for(j = 0 ; j < sg.nsects ; j++){
		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds){
			printf("section structure command extends past "
			       "end of load commands\n");
		    }
		    left = (uint32_t)(sizeofcmds - (p - (char *)load_commands));
		    memset((char *)&s, '\0', sizeof(struct section));
		    size = left < sizeof(struct section) ?
			   left : sizeof(struct section);
		    memcpy((char *)&s, p, size);
		    if(swapped)
			swap_section(&s, 1, host_byte_sex);

		    if(strcmp(s.segname, SEG_TEXT) == 0 &&
		       strcmp(s.sectname, "__cstring") == 0){
			cstring_section->addr = s.addr;
			cstring_section->contents = object_addr + s.offset;
			if(s.offset > object_size){
			    printf("section contents of: (%.16s,%.16s) is past "
				   "end of file\n", s.segname, s.sectname);
			    cstring_section->size = 0;
			}
			else if(s.offset + s.size > object_size){
			    printf("part of section contents of: (%.16s,%.16s) "
				   "is past end of file\n",
				   s.segname, s.sectname);
			    cstring_section->size =
				(uint32_t)(object_size - s.offset);
			}
			else
			    cstring_section->size = s.size;
			if(sg.flags & SG_PROTECTED_VERSION_1)
			    cstring_section->protected = TRUE;
			else
			    cstring_section->protected = FALSE;
			return;
		    }

		    if(p + sizeof(struct section) >
		       (char *)load_commands + sizeofcmds)
			break;
		    p += size;
		}
		break;
	    }
	    if(lcmd.cmdsize == 0){
		printf("load command %u size zero (can't advance to other "
		       "load commands)\n", i);
		break;
	    }
	    lc = (struct load_command *)((char *)lc + lcmd.cmdsize);
	    if((char *)lc > (char *)load_commands + sizeofcmds)
		break;
	}
}

static
enum bool
print_method_list(
uint32_t addr,
struct section_info *objc_sections,
uint32_t nobjc_sections,
struct section_info *cstring_section_ptr,
enum byte_sex host_byte_sex,
enum bool swapped,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
enum bool verbose)
{
    struct objc_method_t *methods, method;
    struct objc_method_list_t method_list;
    enum bool trunc;
    uint32_t i, methods_left, left;
    char *p;

	if(get_method_list(addr, &method_list, &methods, &methods_left, &trunc,
			   objc_sections, nobjc_sections,
			   host_byte_sex, swapped) == FALSE)
	    return(FALSE);
	
	printf("\n");
	if(trunc == TRUE)
	    printf("\t\t objc_method_list extends past end of the section\n");

	printf("\t\t         obsolete 0x%08x\n",
	       (unsigned int)method_list.obsolete);
	printf("\t\t     method_count %d\n",
	       method_list.method_count);
	
	for(i = 0; i < (uint32_t)method_list.method_count; i++){
	    if((i + 1) * sizeof(struct objc_method_t) > methods_left){
		printf("\t\t remaining method's extend past the of the "
		       "section\n");
		break;
	    }
	    memcpy(&method, methods + i, sizeof(struct objc_method_t));
	    if(swapped)
		swap_objc_method_t(&method, host_byte_sex);

	    printf("\t\t      method_name 0x%08x", method.method_name);
	    if(verbose){
		p = get_pointer(method.method_name, &left, objc_sections,
				nobjc_sections, cstring_section_ptr);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in an " SEG_OBJC " section)\n");
	    }
	    else
		printf("\n");

	    printf("\t\t     method_types 0x%08x", method.method_types);
	    if(verbose){
		p = get_pointer(method.method_types, &left, objc_sections,
				nobjc_sections, cstring_section_ptr);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in an " SEG_OBJC " section)\n");
	    }
	    else
		printf("\n");
	    printf("\t\t       method_imp 0x%08x ", method.method_imp);
	    if(verbose)
		print_label(method.method_imp, FALSE, sorted_symbols,
			    nsorted_symbols);
	    printf("\n");
	}
	return(TRUE);
}

static
enum bool
print_protocol_list(
uint32_t indent,
uint32_t addr,
struct section_info *objc_sections,
uint32_t nobjc_sections,
struct section_info *cstring_section_ptr,
enum byte_sex host_byte_sex,
enum bool swapped,
enum bool verbose)
{
    uint32_t *list;
    struct objc_protocol_t protocol;
    struct objc_protocol_list_t protocol_list;
    enum bool trunc;
    uint32_t l, i, list_left;

	if(get_protocol_list(addr, &protocol_list, &list, &list_left, &trunc,
			     objc_sections, nobjc_sections,
			     host_byte_sex, swapped) == FALSE)
	    return(FALSE);

	printf("\n");
	if(trunc == TRUE){
	    print_indent(indent);
	    printf(" objc_protocol_list extends past end of the section\n");
	}

	print_indent(indent);
	printf("         next 0x%08x\n",
	       (unsigned int)protocol_list.next);
	print_indent(indent);
	printf("        count %u\n", protocol_list.count);
	
	for(i = 0; i < protocol_list.count; i++){
	    if((i + 1) * sizeof(uint32_t) > list_left){
		print_indent(indent);
		printf(" remaining list entries extend past the of the "
		       "section\n");
		break;
	    }
	    memcpy(&l, list + i, sizeof(uint32_t));
	    if(swapped)
		l = SWAP_INT(l);

	    print_indent(indent);
	    printf("      list[%u] 0x%08x", i, l);
	    if(get_protocol(l, &protocol, &trunc, objc_sections, nobjc_sections,
			host_byte_sex, swapped) == FALSE){
		printf(" (not in an " SEG_OBJC " section)\n");
		continue;
	    }
	    printf("\n");
	    if(trunc == TRUE){
		print_indent(indent);
		printf("            Protocol extends past end of the "
			"section\n");
	    }
	    print_protocol(indent, &protocol,
		    objc_sections, nobjc_sections, cstring_section_ptr,
		    host_byte_sex, swapped, verbose);
	}
	return(TRUE);
}

static
void
print_protocol(
uint32_t indent,
struct objc_protocol_t *protocol,
struct section_info *objc_sections,
uint32_t nobjc_sections,
struct section_info *cstring_section_ptr,
enum byte_sex host_byte_sex,
enum bool swapped,
enum bool verbose)
{
    uint32_t left;
    char *p;

	print_indent(indent);
	printf("              isa 0x%08x\n",
		(unsigned int)protocol->isa);
	print_indent(indent);
	printf("    protocol_name 0x%08x",
		(unsigned int)protocol->protocol_name);
	if(verbose){
	    p = get_pointer(protocol->protocol_name, &left,
		    objc_sections, nobjc_sections, cstring_section_ptr);
	    if(p != NULL)
		printf(" %.*s\n", (int)left, p);
	    else
		printf(" (not in an " SEG_OBJC " section)\n");
	}
	else
	    printf("\n");
	print_indent(indent);
	printf("    protocol_list 0x%08x",
		(unsigned int)protocol->protocol_list);
	if(print_protocol_list(indent + 4, protocol->protocol_list,
		    objc_sections, nobjc_sections, cstring_section_ptr,
		    host_byte_sex, swapped, verbose) == FALSE)
	    printf(" (not in an " SEG_OBJC " section)\n");

	print_indent(indent);
	printf(" instance_methods 0x%08x",
		(unsigned int)protocol->instance_methods);
	if(print_method_description_list(indent, protocol->instance_methods,
		    objc_sections, nobjc_sections, cstring_section_ptr,
		    host_byte_sex, swapped, verbose) == FALSE)
	    printf(" (not in an " SEG_OBJC " section)\n");

	print_indent(indent);
	printf("    class_methods 0x%08x",
		(unsigned int)protocol->class_methods);
	if(print_method_description_list(indent, protocol->class_methods,
		    objc_sections, nobjc_sections, cstring_section_ptr,
		    host_byte_sex, swapped, verbose) == FALSE)
	    printf(" (not in an " SEG_OBJC " section)\n");
}

static
enum bool
print_method_description_list(
	uint32_t indent,
	uint32_t addr,
	struct section_info *objc_sections,
	uint32_t nobjc_sections,
	struct section_info *cstring_section_ptr,
	enum byte_sex host_byte_sex,
	enum bool swapped,
	enum bool verbose)
{
    struct objc_method_description_list_t mdl;
    struct objc_method_description_t *list, md;
    enum bool trunc;
    uint32_t i, list_left, left;
    char *p;

    if(get_method_description_list(addr, &mdl, &list, &list_left,
		&trunc, objc_sections, nobjc_sections,
		host_byte_sex, swapped) == FALSE)
	return(FALSE);

    printf("\n");
    if(trunc == TRUE){
	print_indent(indent);
	printf(" objc_method_description_list extends past end of the "
		"section\n");
    }

    print_indent(indent);
    printf("        count %d\n", mdl.count);

    for(i = 0; i < (uint32_t)mdl.count; i++){
	if((i + 1) * sizeof(struct objc_method_description_t) > list_left){
		print_indent(indent);
		printf(" remaining list entries extend past the of the "
		       "section\n");
		break;
	    }
	    print_indent(indent);
	    printf("        list[%u]\n", i);
	    memcpy(&md, list + i, sizeof(struct objc_method_description_t));
	    if(swapped)
		swap_objc_method_description_t(&md, host_byte_sex);

	    print_indent(indent);
	    printf("             name 0x%08x", (unsigned int)md.name);
	    if(verbose){
		p = get_pointer(md.name, &left,
		    objc_sections, nobjc_sections, cstring_section_ptr);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in an " SEG_OBJC " section)\n");
	    }
	    else
		printf("\n");

	    print_indent(indent);
	    printf("            types 0x%08x", (unsigned int)md.types);
	    if(verbose){
		p = get_pointer(md.types, &left,
		    objc_sections, nobjc_sections, cstring_section_ptr);
		if(p != NULL)
		    printf(" %.*s\n", (int)left, p);
		else
		    printf(" (not in an " SEG_OBJC " section)\n");
	    }
	    else
		printf("\n");
	}
	return(TRUE);
}

static enum bool
print_PHASH(
uint32_t indent,
uint32_t addr,
struct section_info *objc_sections,
uint32_t nobjc_sections,
struct section_info *cstring_section_ptr,
enum byte_sex host_byte_sex,
enum bool swapped,
enum bool verbose)
{
    struct _hashEntry_t _hashEntry;
    enum bool trunc;
    uint32_t left;
    char *p;

	if(get_hashEntry(addr, &_hashEntry, &trunc, objc_sections,
	    nobjc_sections, host_byte_sex, swapped) == FALSE)
	    return(FALSE);

	printf("\n");
	if(trunc == TRUE){
	    print_indent(indent);
	    printf("_hashEntry extends past end of the section\n");
	}

	print_indent(indent);
	printf(" sel 0x%08x", (unsigned int)_hashEntry.sel);
	if(verbose){
	    p = get_pointer(_hashEntry.sel, &left,
    	    objc_sections, nobjc_sections, cstring_section_ptr);
	    if(p != NULL)
		printf(" %.*s\n", (int)left, p);
	    else
		printf(" (not in an " SEG_OBJC " section)\n");
	}
	else
	    printf("\n");

	print_indent(indent);
	printf("next 0x%08x", (unsigned int)_hashEntry.next);
	if(_hashEntry.next == 0){
	    printf("\n");
	    return(TRUE);
	}
	if(print_PHASH(indent+4, _hashEntry.next,
	    objc_sections, nobjc_sections, cstring_section_ptr,
	    host_byte_sex, swapped, verbose) == FALSE)
	    printf(" (not in an " SEG_OBJC " section)\n");
	return(TRUE);
}

static
void
print_indent(
uint32_t indent)
{
     uint32_t i;
	
	for(i = 0; i < indent; ){
	    if(indent - i >= 8){
		printf("\t");
		i += 8;
	    }
	    else{
		printf("%.*s", (int)(indent - i), "        ");
		return;
	    }
	}
}

static
void *
get_pointer(
uint32_t addr,
uint32_t *left,
struct section_info *objc_sections,
uint32_t nobjc_sections,
struct section_info *cstring_section_ptr)
{
    void* returnValue = NULL;
    uint32_t i;

	if(addr >= cstring_section_ptr->addr &&
	   addr < cstring_section_ptr->addr + cstring_section_ptr->size){
	    *left = cstring_section_ptr->size -
	    (addr - cstring_section_ptr->addr);
	    if(cstring_section_ptr->protected == TRUE)
		returnValue = "some string from a protected section";
	    else
		returnValue = (cstring_section_ptr->contents +
			       (addr - cstring_section_ptr->addr));
	}
	for(i = 0; returnValue != NULL && i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){
		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		returnValue = (objc_sections[i].contents +
		       (addr - objc_sections[i].addr));
	    }
	}
	return(returnValue);
}

static
enum bool
get_symtab(
uint32_t addr,
struct objc_symtab_t *symtab,
uint32_t **defs,
uint32_t *left,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t i;

	memset(symtab, '\0', sizeof(struct objc_symtab_t));
	if(addr == 0)
	    return(0);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(*left >= sizeof(struct objc_symtab_t) - sizeof(uint32_t)){
		    memcpy(symtab,
			   objc_sections[i].contents +
			       (addr - objc_sections[i].addr),
			   sizeof(struct objc_symtab_t) - sizeof(uint32_t));
		    *left -= sizeof(struct objc_symtab_t) - sizeof(uint32_t);
		    *defs = (uint32_t *)(objc_sections[i].contents +
				     (addr - objc_sections[i].addr) +
			   	     sizeof(struct objc_symtab_t) - 
				     sizeof(uint32_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(symtab,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   *left);
		    *left = 0;
		    *defs = NULL;
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_symtab_t(symtab, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_objc_class(
uint32_t addr,
struct objc_class_t *objc_class,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t left, i;

	memset(objc_class, '\0', sizeof(struct objc_class_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(left >= sizeof(struct objc_class_t)){
		    memcpy(objc_class,
			   objc_sections[i].contents +
			       (addr - objc_sections[i].addr),
			   sizeof(struct objc_class_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(objc_class,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   left);
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_class_t(objc_class, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_objc_category(
uint32_t addr,
struct objc_category_t *objc_category,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t left, i;

	memset(objc_category, '\0', sizeof(struct objc_category_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(left >= sizeof(struct objc_category_t)){
		    memcpy(objc_category,
			   objc_sections[i].contents +
			       (addr - objc_sections[i].addr),
			   sizeof(struct objc_category_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(objc_category,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   left);
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_category_t(objc_category, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_ivar_list(
uint32_t addr,
struct objc_ivar_list_t *objc_ivar_list,
struct objc_ivar_t **ivar_list,
uint32_t *left,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t i;

	memset(objc_ivar_list, '\0', sizeof(struct objc_ivar_list_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(*left >= sizeof(struct objc_ivar_list_t) -
			    sizeof(struct objc_ivar_t)){
		    memcpy(objc_ivar_list,
			   objc_sections[i].contents +
				(addr - objc_sections[i].addr),
			   sizeof(struct objc_ivar_list_t) -
				sizeof(struct objc_ivar_t));
		    *left -= sizeof(struct objc_ivar_list_t) -
			     sizeof(struct objc_ivar_t);
		    *ivar_list = (struct objc_ivar_t *)
				 (objc_sections[i].contents +
				     (addr - objc_sections[i].addr) +
				  sizeof(struct objc_ivar_list_t) -
				      sizeof(struct objc_ivar_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(objc_ivar_list,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   *left);
		    *left = 0;
		    *ivar_list = NULL;
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_ivar_list_t(objc_ivar_list, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_method_list(
uint32_t addr,
struct objc_method_list_t *method_list,
struct objc_method_t **methods,
uint32_t *left,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t i;

	memset(method_list, '\0', sizeof(struct objc_method_list_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(*left >= sizeof(struct objc_method_list_t) -
			    sizeof(struct objc_method_t)){
		    memcpy(method_list,
			   objc_sections[i].contents +
				(addr - objc_sections[i].addr),
			   sizeof(struct objc_method_list_t) -
				sizeof(struct objc_method_t));
		    *left -= sizeof(struct objc_method_list_t) -
			     sizeof(struct objc_method_t);
		    *methods = (struct objc_method_t *)
				 (objc_sections[i].contents +
				     (addr - objc_sections[i].addr) +
				  sizeof(struct objc_method_list_t) -
				      sizeof(struct objc_method_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(method_list,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   *left);
		    *left = 0;
		    *methods = NULL;
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_method_list_t(method_list, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_protocol_list(
uint32_t addr,
struct objc_protocol_list_t *protocol_list,
uint32_t **list,
uint32_t *left,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t i;

	memset(protocol_list, '\0', sizeof(struct objc_protocol_list_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(*left >= sizeof(struct objc_protocol_list_t) -
			    sizeof(uint32_t)){
		    memcpy(protocol_list,
			   objc_sections[i].contents +
				(addr - objc_sections[i].addr),
			   sizeof(struct objc_protocol_list_t) -
				sizeof(uint32_t));
		    *left -= sizeof(struct objc_protocol_list_t) -
			     sizeof(uint32_t);
		    *list = (uint32_t *)
				 (objc_sections[i].contents +
				     (addr - objc_sections[i].addr) +
				  sizeof(struct objc_protocol_list_t) -
				      sizeof(uint32_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(protocol_list,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   *left);
		    *left = 0;
		    *list = NULL;
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_protocol_list_t(protocol_list, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_protocol(
uint32_t addr,
struct objc_protocol_t *protocol,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t left, i;

	memset(protocol, '\0', sizeof(struct objc_protocol_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(left >= sizeof(struct objc_protocol_t)){
		    memcpy(protocol,
			   objc_sections[i].contents +
			       (addr - objc_sections[i].addr),
			   sizeof(struct objc_protocol_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(protocol,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   left);
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_protocol_t(protocol, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_method_description_list(
uint32_t addr,
struct objc_method_description_list_t *mdl,
struct objc_method_description_t **list,
uint32_t *left,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t i;

	memset(mdl, '\0', sizeof(struct objc_method_description_list_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		*left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(*left >= sizeof(struct objc_method_description_list_t) -
			    sizeof(struct objc_method_description_t)){
		    memcpy(mdl,
			   objc_sections[i].contents +
				(addr - objc_sections[i].addr),
			   sizeof(struct objc_method_description_list_t) -
				sizeof(struct objc_method_description_t));
		    *left -= sizeof(struct objc_method_description_list_t) -
			     sizeof(struct objc_method_description_t);
		    *list = (struct objc_method_description_t *)
				 (objc_sections[i].contents +
				     (addr - objc_sections[i].addr) +
				 sizeof(struct objc_method_description_list_t) -
				      sizeof(struct objc_method_description_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(mdl,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   *left);
		    *left = 0;
		    *list = NULL;
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_objc_method_description_list_t(mdl, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}

static
enum bool
get_hashEntry(
uint32_t addr,
struct _hashEntry_t *_hashEntry,
enum bool *trunc,
struct section_info *objc_sections,
uint32_t nobjc_sections,
enum byte_sex host_byte_sex,
enum bool swapped)
{
    uint32_t left, i;

	memset(_hashEntry, '\0', sizeof(struct _hashEntry_t));
	if(addr == 0)
	    return(FALSE);
	for(i = 0; i < nobjc_sections; i++){
	    if(addr >= objc_sections[i].addr &&
	       addr < objc_sections[i].addr + objc_sections[i].size){

		left = objc_sections[i].size -
		        (addr - objc_sections[i].addr);
		if(left >= sizeof(struct _hashEntry_t)){
		    memcpy(_hashEntry,
			   objc_sections[i].contents +
			       (addr - objc_sections[i].addr),
			   sizeof(struct _hashEntry_t));
		    *trunc = FALSE;
		}
		else{
		    memcpy(_hashEntry,
			   objc_sections[i].contents +
			        (addr - objc_sections[i].addr),
			   left);
		    *trunc = TRUE;
		}
		if(swapped)
		    swap_hashEntry_t(_hashEntry, host_byte_sex);
		return(TRUE);
	    }
	}
	return(FALSE);
}
