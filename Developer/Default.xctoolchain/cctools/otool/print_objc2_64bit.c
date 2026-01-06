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
#include "stdlib.h"
#include "stddef.h"
#include "string.h"
#include <strings.h> /* cctools-port: For bcmp, bzero ... */
#include "mach-o/loader.h"
#include "mach-o/arm64/reloc.h"
#include "stuff/allocate.h"
#include "stuff/bytesex.h"
#include "stuff/symbol.h"
#include "stuff/reloc.h"
#include "dyld_bind_info.h"
#include "ofile_print.h"

#include <stdarg.h>

extern char *oname;

/*
 * Here we need structures that have the same memory layout and size as the
 * 64-bit Objective-C meta data structures that can be used in this 32-bit
 * program.
 *
 * The real structure definitions come from the objc4 project in the private
 * header file runtime/objc-runtime-new.h in that project.
 */

struct class_t {
    uint64_t isa;		/* class_t * (64-bit pointer) */
    uint64_t superclass;	/* class_t * (64-bit pointer) */
    uint64_t cache;		/* Cache (64-bit pointer) */
    uint64_t vtable;		/* IMP * (64-bit pointer) */
    uint64_t data;		/* class_ro_t * (64-bit pointer) */
};

static
void
swap_class_t(
struct class_t *c,
enum byte_sex target_byte_sex)
{
    c->isa = SWAP_LONG_LONG(c->isa);
    c->superclass = SWAP_LONG_LONG(c->superclass);
    c->cache = SWAP_LONG_LONG(c->cache);
    c->vtable = SWAP_LONG_LONG(c->vtable);
    c->data = SWAP_LONG_LONG(c->data);
}

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
    uint32_t reserved;
    uint64_t ivarLayout;	/* const uint8_t * (64-bit pointer) */
    uint64_t name; 		/* const char * (64-bit pointer) */
    uint64_t baseMethods; 	/* const method_list_t * (64-bit pointer) */
    uint64_t baseProtocols; 	/* const protocol_list_t * (64-bit pointer) */
    uint64_t ivars; 		/* const ivar_list_t * (64-bit pointer) */
    uint64_t weakIvarLayout; 	/* const uint8_t * (64-bit pointer) */
    uint64_t baseProperties; 	/* const struct objc_property_list *
							(64-bit pointer) */
};

/* Values for class_ro_t->flags */
#define RO_META               (1<<0)
#define RO_ROOT               (1<<1)
#define RO_HAS_CXX_STRUCTORS  (1<<2)

static
void
swap_class_ro_t(
struct class_ro_t *cro,
enum byte_sex target_byte_sex)
{
    cro->flags = SWAP_INT(cro->flags);
    cro->instanceStart = SWAP_INT(cro->instanceStart);
    cro->instanceSize = SWAP_INT(cro->instanceSize);
    cro->reserved = SWAP_INT(cro->reserved);
    cro->ivarLayout = SWAP_LONG_LONG(cro->ivarLayout);
    cro->name = SWAP_LONG_LONG(cro->name);
    cro->baseMethods = SWAP_LONG_LONG(cro->baseMethods);
    cro->baseProtocols = SWAP_LONG_LONG(cro->baseProtocols);
    cro->ivars = SWAP_LONG_LONG(cro->ivars);
    cro->weakIvarLayout = SWAP_LONG_LONG(cro->weakIvarLayout);
    cro->baseProperties = SWAP_LONG_LONG(cro->baseProperties);
}

struct method_list_t {
    uint32_t entsize;
    uint32_t count;
    /* struct method_t first;  These structures follow inline */
};

static
void
swap_method_list_t(
struct method_list_t *ml,
enum byte_sex target_byte_sex)
{
    ml->entsize = SWAP_INT(ml->entsize);
    ml->count = SWAP_INT(ml->count);
}

struct method_t {
    uint64_t name;	/* SEL (64-bit pointer) */
    uint64_t types;	/* const char * (64-bit pointer) */
    uint64_t imp;	/* IMP (64-bit pointer) */
};

static
void
swap_method_t(
struct method_t *m,
enum byte_sex target_byte_sex)
{
    m->name = SWAP_LONG_LONG(m->name);
    m->types = SWAP_LONG_LONG(m->types);
    m->imp = SWAP_LONG_LONG(m->imp);
}

struct ivar_list_t {
    uint32_t entsize;
    uint32_t count;
    /* struct ivar_t first;  These structures follow inline */
};

static
void
swap_ivar_list_t(
struct ivar_list_t *il,
enum byte_sex target_byte_sex)
{
    il->entsize = SWAP_INT(il->entsize);
    il->count = SWAP_INT(il->count);
}

struct ivar_t {
    uint64_t offset;	/* uintptr_t * (64-bit pointer) */
    uint64_t name;	/* const char * (64-bit pointer) */
    uint64_t type;	/* const char * (64-bit pointer) */
    uint32_t alignment;
    uint32_t size;
};

static
void
swap_ivar_t(
struct ivar_t *i,
enum byte_sex target_byte_sex)
{
    i->offset = SWAP_LONG_LONG(i->offset);
    i->name = SWAP_LONG_LONG(i->name);
    i->type = SWAP_LONG_LONG(i->type);
    i->alignment = SWAP_INT(i->alignment);
    i->size = SWAP_INT(i->size);
}

struct protocol_list_t {
    uint64_t count;	/* uintptr_t (a 64-bit value) */
    /* struct protocol_t * list[0];  These pointers follow inline */
};

static
void
swap_protocol_list_t(
struct protocol_list_t *pl,
enum byte_sex target_byte_sex)
{
    pl->count = SWAP_LONG_LONG(pl->count);
}

struct protocol_t {
    uint64_t isa;			/* id * (64-bit pointer) */
    uint64_t name;			/* const char * (64-bit pointer) */
    uint64_t protocols;			/* struct protocol_list_t *
							(64-bit pointer) */
    uint64_t instanceMethods;		/* method_list_t * (64-bit pointer) */
    uint64_t classMethods;		/* method_list_t * (64-bit pointer) */
    uint64_t optionalInstanceMethods;	/* method_list_t * (64-bit pointer) */
    uint64_t optionalClassMethods;	/* method_list_t * (64-bit pointer) */
    uint64_t instanceProperties;	/* struct objc_property_list *
							   (64-bit pointer) */
};

static
void
swap_protocol_t(
struct protocol_t *p,
enum byte_sex target_byte_sex)
{
    p->isa = SWAP_LONG_LONG(p->isa);
    p->name = SWAP_LONG_LONG(p->name);
    p->protocols = SWAP_LONG_LONG(p->protocols);
    p->instanceMethods = SWAP_LONG_LONG(p->instanceMethods);
    p->classMethods = SWAP_LONG_LONG(p->classMethods);
    p->optionalInstanceMethods = SWAP_LONG_LONG(p->optionalInstanceMethods);
    p->optionalClassMethods = SWAP_LONG_LONG(p->optionalClassMethods);
    p->instanceProperties = SWAP_LONG_LONG(p->instanceProperties);
}

struct objc_property_list {
    uint32_t entsize;
    uint32_t count;
    /* struct objc_property first;  These structures follow inline */
};

static
void
swap_objc_property_list(
struct objc_property_list *pl,
enum byte_sex target_byte_sex)
{
    pl->entsize = SWAP_INT(pl->entsize);
    pl->count = SWAP_INT(pl->count);
}

struct objc_property {
    uint64_t name;		/* const char * (64-bit pointer) */
    uint64_t attributes;	/* const char * (64-bit pointer) */
};

static
void
swap_objc_property(
struct objc_property *op,
enum byte_sex target_byte_sex)
{
    op->name = SWAP_LONG_LONG(op->name);
    op->attributes = SWAP_LONG_LONG(op->attributes);
}

struct category_t {
    uint64_t name; 		/* const char * (64-bit pointer) */
    uint64_t cls;		/* struct class_t * (64-bit pointer) */
    uint64_t instanceMethods;	/* struct method_list_t * (64-bit pointer) */
    uint64_t classMethods;	/* struct method_list_t * (64-bit pointer) */
    uint64_t protocols;		/* struct protocol_list_t * (64-bit pointer) */
    uint64_t instanceProperties; /* struct objc_property_list *
				    (64-bit pointer) */
};

static
void
swap_category_t(
struct category_t *c,
enum byte_sex target_byte_sex)
{
    c->name = SWAP_LONG_LONG(c->name);
    c->cls = SWAP_LONG_LONG(c->cls);
    c->instanceMethods = SWAP_LONG_LONG(c->instanceMethods);
    c->classMethods = SWAP_LONG_LONG(c->classMethods);
    c->protocols = SWAP_LONG_LONG(c->protocols);
    c->instanceProperties = SWAP_LONG_LONG(c->instanceProperties);
}

struct message_ref {
    uint64_t imp;	/* IMP (64-bit pointer) */
    uint64_t sel;	/* SEL (64-bit pointer) */
};

static
void
swap_message_ref(
struct message_ref *mr,
enum byte_sex target_byte_sex)
{
    mr->imp = SWAP_LONG_LONG(mr->imp);
    mr->sel = SWAP_LONG_LONG(mr->sel);
}

struct objc_image_info {
    uint32_t version;
    uint32_t flags;
};

/* masks for objc_image_info.flags */
#define OBJC_IMAGE_IS_REPLACEMENT (1<<0)
#define OBJC_IMAGE_SUPPORTS_GC (1<<1)

static
void
swap_objc_image_info(
struct objc_image_info *o,
enum byte_sex target_byte_sex)
{
    o->version = SWAP_INT(o->version);
    o->flags = SWAP_INT(o->flags);
}

struct objc_string_object_64 {
    uint64_t isa;		/* class_t * (64-bit pointer) */
    uint64_t characters;	/* char * (64-bit pointer) */
    uint32_t _length;		/* number of non-NULL characters in above */
    uint32_t _pad;		/* unused padding, compiler uses .space 4 */
};

static
void
swap_string_object_64(
struct objc_string_object_64 *string_object,
enum byte_sex target_byte_sex)
{
    string_object->isa = SWAP_LONG_LONG(string_object->isa);
    string_object->characters = SWAP_LONG_LONG(string_object->characters);
    string_object->_length = SWAP_INT(string_object->_length);
    string_object->_pad = SWAP_INT(string_object->_pad);
}

struct cfstring_t {
    uint64_t isa;		/* class_t * (64-bit pointer) */
    uint64_t flags;		/* flag bits */
    uint64_t characters;	/* char * (64-bit pointer) */
    uint64_t length;		/* number of non-NULL characters in above */
};

static
void
swap_cfstring_t(
struct cfstring_t *cfstring,
enum byte_sex target_byte_sex)
{
    cfstring->isa = SWAP_LONG_LONG(cfstring->isa);
    cfstring->flags = SWAP_LONG_LONG(cfstring->flags);
    cfstring->characters = SWAP_LONG_LONG(cfstring->characters);
    cfstring->length = SWAP_LONG_LONG(cfstring->length);
}

#define MAXINDENT 10

struct info {
    char *object_addr;
    uint64_t object_size;
    enum bool swapped;
    enum byte_sex host_byte_sex;
    struct section_info_64 *sections;
    uint32_t nsections;
    cpu_type_t cputype;
    cpu_subtype_t cpusubtype;
    struct nlist_64 *symbols64;
    uint32_t nsymbols;
    char *strings;
    uint32_t strings_size;
    struct symbol *sorted_symbols;
    uint32_t nsorted_symbols;
    uint64_t textbase;
    uint64_t database;
    struct relocation_info *ext_relocs;
    uint32_t next_relocs;
    struct relocation_info *loc_relocs;
    uint32_t nloc_relocs;
    struct dyld_bind_info *dbi;
    uint64_t ndbi;
    enum chain_format_t chain_format;
    enum bool verbose;
    enum bool Vflag;
    uint32_t indent_level;
    uint32_t indent_widths[MAXINDENT];
};

struct section_info_64 {
    char segname[16];
    char sectname[16];
    char *contents;
    uint64_t addr;
    uint64_t size;
    uint32_t offset;
    struct relocation_info *relocs;
    uint32_t nrelocs;
    enum bool cstring;
    enum bool protected;
    enum bool zerofill;
};

static void walk_pointer_list(
    char *listname,
    struct section_info_64 *s,
    struct info *info,
    void (*func)(uint64_t, struct info *));

static void print_class_t(
    uint64_t p,
    struct info *info);

static void print_class_ro_t(
    uint64_t p,
    struct info *info,
    enum bool *is_meta_class);

static void print_layout_map(
    uint64_t p,
    struct info *info);

static void print_method_list_t(
    uint64_t p,
    struct info *info);

static void print_ivar_list_t(
    uint64_t p,
    struct info *info);

static void print_protocol_list_t(
    uint64_t p,
    struct info *info);

static void print_objc_property_list(
    uint64_t p,
    struct info *info);

static void print_category_t(
    uint64_t p,
    struct info *info);

static void print_protocol_t(
    uint64_t p,
    struct info *info);

static void print_message_refs(
    struct section_info_64 *s,
    struct info *info);

static void print_image_info(
    struct section_info_64 *s,
    struct info *info);

static void get_sections_64(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct section_info_64 **sections,
    uint32_t *nsections,
    uint64_t *textbase,
    uint64_t *database);

static struct section_info_64 *get_section_64(
    struct section_info_64 *sections,
    uint32_t nsections,
    char *segname,
    char *sectname);

static void get_cstring_section_64(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct section_info_64 *cstring_section_ptr);

static void *get_pointer_64(
    uint64_t p,
    uint32_t *offset,
    uint32_t *left,
    struct section_info_64 **s,
    struct section_info_64 *sections,
    uint32_t nsections);

static const char *get_symbol_64(
    uint64_t sect_offset,
    uint64_t sect_addr,
    uint64_t textbase,
    uint64_t database,
    uint64_t value,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    struct info *info,
    uint64_t *n_value,
    int64_t *addend);

static void print_field_value(
    uint64_t offset,
    uint64_t pointer,
    enum bool print_data,
    const char* type_name,
    const char* suffix,
    struct info *info,
    struct section_info_64 *s,
    uint64_t *out_n_value,
    int64_t *out_addend);

static void print_field_label(
    struct info *info,
    const char* label,
    ...);

static void print_field_scalar(
    struct info *info,
    const char* label,
    const char* fmt,...);

static void indent_push(
    struct info *info,
    uint32_t width);

static void indent_pop(
    struct info *info);

/*
 * Print the objc2 meta data in 64-bit Mach-O files.
 */
void
print_objc2_64bit(
cpu_type_t cputype,
cpu_subtype_t cpusubtype,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
struct relocation_info *ext_relocs,
uint32_t next_relocs,
struct relocation_info *loc_relocs,
uint32_t nloc_relocs,
struct dyld_bind_info *dbi,
uint64_t ndbi,
enum chain_format_t chain_format,
enum bool verbose,
enum bool Vflag)
{
    struct section_info_64 *s;
    struct info info;
    
    info.object_addr = object_addr;
    info.object_size = object_size;
    info.host_byte_sex = get_host_byte_sex();
    info.swapped = info.host_byte_sex != object_byte_sex;
    info.cputype = cputype;
    info.cpusubtype = cpusubtype;
    info.symbols64 = symbols64;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.sorted_symbols = sorted_symbols;
    info.nsorted_symbols = nsorted_symbols;
    info.ext_relocs = ext_relocs;
    info.next_relocs = next_relocs;
    info.loc_relocs = loc_relocs;
    info.nloc_relocs = nloc_relocs;
    info.dbi = dbi;
    info.ndbi = ndbi;
    info.chain_format = chain_format;
    info.verbose = verbose;
    info.Vflag = Vflag;

    get_sections_64(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &info.sections,
                    &info.nsections, &info.textbase, &info.database);

    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__class_list");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_classlist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_classlist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_classlist");
    walk_pointer_list("class", s, &info, print_class_t);

    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__class_refs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_classrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_classrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_classrefs");
    walk_pointer_list("class refs", s, &info, NULL);

    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__super_refs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_superrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_superrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_superrefs");
    walk_pointer_list("super refs", s, &info, NULL);

    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__category_list");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_catlist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_catlist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_catlist");
    walk_pointer_list("category", s, &info, print_category_t);

    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__protocol_list");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_protolist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_protolist");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_protolist");
    walk_pointer_list("protocol", s, &info, print_protocol_t);
    
    s = get_section_64(info.sections, info.nsections,
                       "__OBJC2", "__message_refs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_msgrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_msgrefs");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_msgrefs");
    print_message_refs(s, &info);
    
    s = get_section_64(info.sections, info.nsections,
                       "__OBJC", "__image_info");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA", "__objc_imageinfo");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_imageinfo");
    if(s == NULL)
        s = get_section_64(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_imageinfo");
    print_image_info(s, &info);
}

static
void
walk_pointer_list(
char *listname,
struct section_info_64 *s,
struct info *info,
void (*func)(uint64_t, struct info *))
{
    uint64_t i, size, left;
    uint64_t p, n_value;
    int64_t addend;
    
    if(s == NULL)
        return;
    
    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);
    for(i = 0; i < s->size; i += sizeof(uint64_t))
    {
        left = s->size - i;
        size = left < sizeof(uint64_t) ?
        left : sizeof(uint64_t);
        if(s->contents + i + size > info->object_addr + info->object_size)
            return;
        
        if(i + sizeof(uint64_t) > s->size)
            printf("%s list pointer extends past end of (%s,%s) "
                   "section\n", listname, s->segname, s->sectname);
        printf("%016llx ", s->addr + i);

        memset(&p, '\0', sizeof(uint64_t));
        memcpy(&p, s->contents + i, size);
        if(info->swapped)
            p = SWAP_LONG_LONG(p);

        print_field_value(i, p, FALSE, NULL, "\n", info, s, &n_value, &addend);
        
        if(func != NULL)
            func(n_value + addend, info);
    }
}

/*
 * get_objc2_64bit_cfstring_name() is used for disassembly and is passed a
 * pointer to a cfstring and returns its name.
 */
char *
get_objc2_64bit_cfstring_name(
uint64_t p,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
cpu_type_t cputype)
{
    struct section_info_64 *sections, *s;
    uint32_t nsections, left, offset;
    uint64_t textbase, database, n_value, cfs_characters;
    int64_t addend;
    struct cfstring_t cfs;
    char *name;
    const char *symbol_name;
    void *r;
    struct info info;

    memset(&info, '\0', sizeof(struct info));
    info.symbols64 = symbols64;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.cputype = cputype;
    info.verbose = TRUE;

    get_sections_64(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &sections, &nsections,
                    &textbase, &database);
    
    r = get_pointer_64(p, &offset, &left, &s, sections, nsections);
    if(r == NULL || left < sizeof(struct cfstring_t))
        return(NULL);

    memcpy(&cfs, r, sizeof(struct cfstring_t));
    if(get_host_byte_sex() != object_byte_sex)
        swap_cfstring_t(&cfs, get_host_byte_sex());

    symbol_name = get_symbol_64(offset +
                                offsetof(struct cfstring_t, characters),
                                s->addr, textbase, database, p, s->relocs,
                                s->nrelocs, &info, &n_value, &addend);
    if(symbol_name == NULL){
        if(sections != NULL)
            free(sections);
        return(NULL);
    }
    cfs_characters = n_value + addend;

    name = get_pointer_64(cfs_characters, NULL, &left, NULL,
                          sections, nsections);

    if(sections != NULL)
        free(sections);

    return(name);
}

/*
 * get_objc2_64bit_class_name() is used for disassembly and is passed a pointer
 * to an Objective-C class and returns the class name.  It is also passed the
 * address of the pointer, so when the pointer is zero as it can be in an .o
 * file, that is use to look for an external relocation entry with a symbol
 * name.
 */
char *
get_objc2_64bit_class_name(
uint64_t p,
uint64_t address_of_p,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
cpu_type_t cputype)
{
    struct section_info_64 *sections, *s;
    uint32_t nsections, left, offset;
    uint64_t textbase, database, n_value;
    int64_t addend;
    struct class_t c;
    struct class_ro_t cro;
    char *name, *class_name;
    const char *symbol_name;
    void *r;
    struct info info;

    memset(&info, '\0', sizeof(struct info));
    info.symbols64 = symbols64;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.cputype = cputype;
    info.verbose = TRUE;

    get_sections_64(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &sections, &nsections,
                    &textbase, &database);

    if(p == 0){
        r = get_pointer_64(address_of_p, &offset, &left, &s, sections,
                           nsections);
        if(r == NULL || left < sizeof(uint64_t)){
            if(sections != NULL)
                free(sections);
            return(NULL);
        }

        symbol_name = get_symbol_64(offset, s->addr, textbase, database,
                                    address_of_p, s->relocs, s->nrelocs,
                                    &info, &n_value, &addend);
        if(symbol_name == NULL){
            if(sections != NULL)
                free(sections);
            return(NULL);
        }

        class_name = rindex(symbol_name, '$');
        if(class_name != NULL &&
           class_name[1] == '_' && class_name[2] != '\0'){
            if(sections != NULL)
                free(sections);
            return(class_name + 2);
        }
        else{
            if(sections != NULL)
                free(sections);
            return(NULL);
        }
    }

    r = get_pointer_64(p, NULL, &left, NULL, sections, nsections);
    if(r == NULL || left < sizeof(struct class_t)){
        if(sections != NULL)
            free(sections);
        return(NULL);
    }

    memcpy(&c, r, sizeof(struct class_t));
    if(get_host_byte_sex() != object_byte_sex)
        swap_class_t(&c, get_host_byte_sex());

    if(c.data == 0){
        if(sections != NULL)
            free(sections);
        return(NULL);
    }
    
    r = get_pointer_64(c.data, NULL, &left, NULL, sections, nsections);
    if(r == NULL || left < sizeof(struct class_ro_t)){
        if(sections != NULL)
            free(sections);
        return(NULL);
    }

    memcpy(&cro, r, sizeof(struct class_ro_t));
    if(get_host_byte_sex() != object_byte_sex)
        swap_class_ro_t(&cro, get_host_byte_sex());

    if(cro.name == 0){
        if(sections != NULL)
            free(sections);
        return(NULL);
    }

    name = get_pointer_64(cro.name, NULL, &left, NULL, sections, nsections);

    if(sections != NULL)
        free(sections);

    return(name);
}

/*
 * get_objc2_64bit_selref() is used for disassembly and is passed a the address
 * of a pointer to an Objective-C selector reference when the pointer value is
 * zero as in a .o file and is likely to have a external relocation entry with
 * who's symbol's n_value is the real pointer to the selector name.  If that is
 * the case the real pointer to the selector name is returned else 0 is
 * returned
 */
uint64_t
get_objc2_64bit_selref(
uint64_t address_of_p,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
cpu_type_t cputype)
{
    struct section_info_64 *sections, *s;
    uint32_t nsections, left, offset;
    uint64_t textbase, database, n_value;
    int64_t addend;
    void *r;
    const char *symbol_name;
    struct info info;

    memset(&info, '\0', sizeof(struct info));
    info.symbols64 = symbols64;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.cputype = cputype;
    info.verbose = TRUE;

    get_sections_64(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &sections, &nsections,
                    &textbase, &database);
    r = get_pointer_64(address_of_p, &offset, &left, &s, sections,
                       nsections);
    if(r == NULL || left < sizeof(uint64_t)){
        if(sections != NULL)
            free(sections);
        return(0);
    }

    symbol_name = get_symbol_64(offset, s->addr, textbase, database,
                                address_of_p, s->relocs, s->nrelocs,
                                &info, &n_value, &addend);

    if(symbol_name == NULL){
        if(sections != NULL)
            free(sections);
        return(0);
    }
    return(n_value);
}

static
void
print_class_t(
uint64_t p,
struct info *info)
{
    struct class_t c;
    void *r;
    uint32_t offset, left;
    struct section_info_64 *s;
    enum bool is_meta_class;
    uint64_t n_value, isa_n_value;
    int64_t addend, isa_addend;
    static uint32_t recursive_depth;

    is_meta_class = FALSE;

    r = get_pointer_64(p, &offset, &left, &s,
                       info->sections, info->nsections);
    if(r == NULL)
        return;

    memset(&c, '\0', sizeof(struct class_t));
    if(left < sizeof(struct class_t)){
        memcpy(&c, r, left);
        printf("   (class_t entends past the end of the section)\n");
    }
    else
        memcpy(&c, r, sizeof(struct class_t));
    if(info->swapped)
        swap_class_t(&c, info->host_byte_sex);

    indent_push(info, sizeof("superclass") - 1);

    print_field_label(info, "isa");
    print_field_value(offset + offsetof(struct class_t, isa), c.isa,
                      FALSE, NULL, "\n", info, s, &isa_n_value, &isa_addend);

    print_field_label(info, "superclass");
    print_field_value(offset + offsetof(struct class_t, superclass),
                      c.superclass, FALSE, NULL, "\n", info, s,
                      &n_value, &addend);

    print_field_label(info, "cache");
    print_field_value(offset + offsetof(struct class_t, cache),
                      c.cache, FALSE, NULL, "\n", info, s, &n_value, &addend);

    print_field_label(info, "vtable");
    print_field_value(offset + offsetof(struct class_t, vtable),
                      c.vtable, FALSE, NULL, "\n", info, s, &n_value, &addend);

    print_field_label(info, "data");
    print_field_value(offset + offsetof(struct class_t, data), c.data, FALSE,
                    "(struct class_ro_t *)", NULL, info, s, &n_value, &addend);
    /*
     * This is a Swift class if some of the low bits of the pointer
     * are set. Note that this value is 3 in 32-bit.
     *
     *   bit 0: is Swift
     *   bit 1: is Swift-stable API
     *   bit 2: has custom retain/release (runtime only)
     */
    if((c.data + n_value) & 0x7)
        printf(" Swift class");
    printf("\n");

    /* Descend into the read only data */
    print_class_ro_t((n_value + addend) & ~0x7, info, &is_meta_class);

    indent_pop(info);

    /* Walk the class hierarchy, but be wary of cycles or bad chains */
    if(is_meta_class == FALSE &&
       isa_n_value + isa_addend != p &&
       isa_n_value + isa_addend != 0 &&
       recursive_depth < 100)
    {
        recursive_depth++;
        printf("Meta Class\n");
        print_class_t(isa_n_value + isa_addend, info);
        recursive_depth--;
    }
}

static
void
print_class_ro_t(
uint64_t p,
struct info *info,
enum bool *is_meta_class)
{
    struct class_ro_t cro;
    void *r;
    uint32_t offset, left;
    struct section_info_64 *s;
    const char *name;
    uint64_t n_value;
    int64_t addend;

    r = get_pointer_64(p, &offset, &left, &s, info->sections,
                       info->nsections);
    if(r == NULL)
        return;

    memset(&cro, '\0', sizeof(struct class_ro_t));
    if(left < sizeof(struct class_ro_t)){
        memcpy(&cro, r, left);
        printf("   (class_ro_t entends past the end of the section)\n");
    }
    else
        memcpy(&cro, r, sizeof(struct class_ro_t));
    if(info->swapped)
        swap_class_ro_t(&cro, info->host_byte_sex);

    indent_push(info, sizeof("weakIvarLayout") - 1);

    print_field_scalar(info, "flags", "0x%x", cro.flags);

    if(info->verbose){
        if(cro.flags & RO_META)
            printf(" RO_META");
        if(cro.flags & RO_ROOT)
            printf(" RO_ROOT");
        if(cro.flags & RO_HAS_CXX_STRUCTORS)
            printf(" RO_HAS_CXX_STRUCTORS");
    }
    printf("\n");

    print_field_scalar(info, "instanceStart", "%u\n", cro.instanceStart);
    print_field_scalar(info, "instanceSize", "%u\n", cro.instanceSize);
    print_field_scalar(info, "reserved", "0x%x\n", cro.reserved);

    print_field_label(info, "ivarLayout");
    print_field_value(offset + offsetof(struct class_ro_t, ivarLayout),
                      cro.ivarLayout, FALSE, NULL, "\n", info, s,
                      &n_value, &addend);
    print_layout_map(n_value + addend, info);

    print_field_label(info, "name");
    print_field_value(offset + offsetof(struct class_ro_t, name),
                      cro.name, FALSE, NULL, NULL, info, s, &n_value, &addend);
    if (info->verbose) {
        name = get_pointer_64(n_value + addend, NULL, &left, NULL,
                              info->sections, info->nsections);
        if (name != NULL)
            printf(" %.*s", (int)left, name);
    }
    printf("\n");

    print_field_label(info, "baseMethods");
    print_field_value(offset + offsetof(struct class_ro_t, baseMethods),
                      cro.baseMethods, FALSE, "(struct method_list_t *)", "\n",
                      info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }

    print_field_label(info, "baseProtocols");
    print_field_value(offset + offsetof(struct class_ro_t, baseProtocols),
                      cro.baseProtocols, FALSE, "(struct protocol_list_t *)",
                      "\n", info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_protocol_list_t(n_value + addend, info);
    }

    print_field_label(info, "ivars");
    print_field_value(offset + offsetof(struct class_ro_t, ivars),
                      cro.ivars, FALSE, "(struct ivar_list_t *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend != 0) {
        print_ivar_list_t(n_value + addend, info);
    }

    print_field_label(info, "weakIvarLayout");
    print_field_value(offset + offsetof(struct class_ro_t, weakIvarLayout),
                      cro.weakIvarLayout, FALSE, NULL, "\n", info, s,
                      &n_value, &addend);
    print_layout_map(n_value + addend, info);

    print_field_label(info, "baseProperties");
    print_field_value(offset + offsetof(struct class_ro_t, baseProperties),
                      cro.baseProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend != 0) {
        print_objc_property_list(n_value + addend, info);
    }

    if(is_meta_class)
        *is_meta_class = (cro.flags & RO_META) ? TRUE : FALSE;

    indent_pop(info);
}

static
void
print_layout_map(
uint64_t p,
struct info *info)
{
    uint32_t offset, left;
    struct section_info_64 *s;
    char *layout_map;

    if(p == 0)
        return;
    
    layout_map = get_pointer_64(p, &offset, &left, &s,
                                info->sections, info->nsections);
    if(layout_map != NULL){
        print_field_label(info, "layout map");
        do{
            printf("0x%02x ", (*layout_map) & 0xff);
            left--;
            layout_map++;
        }while(*layout_map != '\0' && left != 0);
        printf("\n");
    }
}

static
void
print_method_list_t(
uint64_t p,
struct info *info)
{
    struct method_list_t ml;
    struct method_t m;
    void *r;
    uint32_t offset, left, i;
    struct section_info_64 *s;
    uint64_t n_value;
    int64_t addend;
    
    r = get_pointer_64(p, &offset, &left, &s, info->sections, info->nsections);
    if(r == NULL)
        return;

    memset(&ml, '\0', sizeof(struct method_list_t));
    if(left < sizeof(struct method_list_t)){
        memcpy(&ml, r, left);
        print_field_scalar(info, "", "(method_list_t entends past the end "
                           "of the section)\n)");
    }
    else
        memcpy(&ml, r, sizeof(struct method_list_t));
    if(info->swapped)
        swap_method_list_t(&ml, info->host_byte_sex);

    indent_push(info, sizeof("entsize") - 1);

    print_field_scalar(info, "entsize", "%u\n", ml.entsize);
    print_field_scalar(info, "count", "%u\n", ml.count);

    p += sizeof(struct method_list_t);
    offset += sizeof(struct method_list_t);
    for(i = 0; i < ml.count; i++){
        r = get_pointer_64(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            break;

        memset(&m, '\0', sizeof(struct method_t));
        if(left < sizeof(struct method_t)){
            memcpy(&m, r, left);
            print_field_scalar(info, "", "(method_t entends past the end "
                               "of the section)\n)");
        }
        else
            memcpy(&m, r, sizeof(struct method_t));
        if(info->swapped)
            swap_method_t(&m, info->host_byte_sex);

        print_field_label(info, "name");
        print_field_value(offset + offsetof(struct method_t, name),
                          m.name, TRUE, NULL, "\n", info, s, &n_value, &addend);

        print_field_label(info, "types");
        print_field_value(offset + offsetof(struct method_t, types),
                          m.types, TRUE, NULL, "\n", info, s, &n_value,&addend);

        print_field_label(info, "imp");
        print_field_value(offset + offsetof(struct method_t, imp),
                          m.imp, FALSE, NULL, "\n", info, s, &n_value, &addend);

        p += sizeof(struct method_t);
        offset += sizeof(struct method_t);
    }

    indent_pop(info);
}

static
void
print_ivar_list_t(
uint64_t p,
struct info *info)
{
    struct ivar_list_t il;
    struct ivar_t i;
    void *r;
    uint32_t offset, left, j;
    struct section_info_64 *s;
    uint64_t *ivar_offset_p, n_value;
    uint32_t ivar_offset;
    int64_t addend;
    
    r = get_pointer_64(p, &offset, &left, &s, info->sections,
                       info->nsections);
    if(r == NULL)
        return;

    memset(&il, '\0', sizeof(struct ivar_list_t));
    if(left < sizeof(struct ivar_list_t)){
        memcpy(&il, r, left);
        printf("   (ivar_list_t entends past the end of the section)\n");
    }
    else
        memcpy(&il, r, sizeof(struct ivar_list_t));
    if(info->swapped)
        swap_ivar_list_t(&il, info->host_byte_sex);

    indent_push(info, sizeof("alignment") - 1);

    print_field_scalar(info, "entsize", "%u\n", il.entsize);
    print_field_scalar(info, "count", "%u\n", il.count);

    p += sizeof(struct ivar_list_t);
    offset += sizeof(struct ivar_list_t);
    for(j = 0; j < il.count; j++){
        r = get_pointer_64(p, &offset, &left, &s, info->sections,
                           info->nsections);
        if(r == NULL)
            break;

        memset(&i, '\0', sizeof(struct ivar_t));
        if(left < sizeof(struct ivar_t)){
            memcpy(&i, r, left);
            printf("   (ivar_t entends past the end of the section)\n");
        }
        else
            memcpy(&i, r, sizeof(struct ivar_t));
        if(info->swapped)
            swap_ivar_t(&i, info->host_byte_sex);

        print_field_label(info, "offset");
        print_field_value(offset + offsetof(struct ivar_t, offset),
                          i.offset, FALSE, NULL, NULL, info, s,
                          &n_value, &addend);
        if (info->verbose) {
            ivar_offset_p = get_pointer_64(n_value + addend, NULL, &left, NULL,
                                           info->sections, info->nsections);
            if(ivar_offset_p != NULL && left >= sizeof(ivar_offset)){
                memcpy(&ivar_offset, ivar_offset_p, sizeof(ivar_offset));
                if(info->swapped)
                    ivar_offset = SWAP_INT(ivar_offset);
                printf(" %u", ivar_offset);
            }
        }
        printf("\n");

        print_field_label(info, "name");
        print_field_value(offset + offsetof(struct ivar_t, name),
                          i.name, TRUE, NULL, "\n", info, s, &n_value, &addend);

        print_field_label(info, "type");
        print_field_value(offset + offsetof(struct ivar_t, type),
                          i.type, TRUE, NULL, "\n", info, s, &n_value, &addend);

        print_field_scalar(info, "alignment", "%u\n", i.alignment);
        print_field_scalar(info, "size", "%u\n", i.size);

        p += sizeof(struct ivar_t);
        offset += sizeof(struct ivar_t);
    }

    indent_pop(info);
}

static
void
print_protocol_list_t(
uint64_t p,
struct info *info)
{
    struct protocol_list_t pl;
    uint64_t q, n_value;
    int64_t addend;
    void *r;
    uint32_t offset, left, i;
    struct section_info_64 *s;
    static uint32_t recursive_depth;

    r = get_pointer_64(p, &offset, &left, &s, info->sections,
                       info->nsections);
    if(r == NULL)
        return;

    memset(&pl, '\0', sizeof(struct protocol_list_t));
    if(left < sizeof(struct protocol_list_t)){
        memcpy(&pl, r, left);
        printf("   (protocol_list_t entends past the end of the "
               "section)\n");
    }
    else
        memcpy(&pl, r, sizeof(struct protocol_list_t));
    if(info->swapped)
        swap_protocol_list_t(&pl, info->host_byte_sex);

    indent_push(info, sizeof("list[99]") - 1);

    print_field_scalar(info, "count", "%llu\n", pl.count);

    p += sizeof(struct protocol_list_t);
    offset += sizeof(struct protocol_list_t);
    for(i = 0; i < pl.count; i++){
        r = get_pointer_64(p, &offset, &left, &s, info->sections,
                           info->nsections);
        if(r == NULL)
            break;

        q = 0;
        if(left < sizeof(uint64_t)){
            memcpy(&q, r, left);
            printf("   (protocol_t * entends past the end of the "
                   "section)\n");
        }
        else
            memcpy(&q, r, sizeof(uint64_t));
        if(info->swapped)
            q = SWAP_LONG_LONG(q);

        print_field_label(info, "list[%u]", i);
        print_field_value(offset, q, FALSE, "(struct protocol_t *)", "\n",
                          info, s, &n_value, &addend);
        
        if (n_value + addend &&
            recursive_depth < 100)
        {
            recursive_depth += 1;
            print_protocol_t(n_value + addend, info);
            recursive_depth -= 1;
        }
        
        p += sizeof(uint64_t);
        offset += sizeof(uint64_t);
    }

    indent_pop(info);
}

static
void
print_objc_property_list(
uint64_t p,
struct info *info)
{
    struct objc_property_list opl;
    struct objc_property op;
    void *r;
    uint32_t offset, left, j;
    struct section_info_64 *s;
    uint64_t n_value;
    int64_t addend;
    
    r = get_pointer_64(p, &offset, &left, &s, info->sections, info->nsections);
    if(r == NULL)
        return;

    memset(&opl, '\0', sizeof(struct objc_property_list));
    if(left < sizeof(struct objc_property_list)){
        memcpy(&opl, r, left);
        printf("   (objc_property_list entends past the end of the "
               "section)\n");
    }
    else
        memcpy(&opl, r, sizeof(struct objc_property_list));
    if(info->swapped)
        swap_objc_property_list(&opl, info->host_byte_sex);

    indent_push(info, sizeof("attributes") - 1);

    print_field_scalar(info, "entsize", "%u\n", opl.entsize);
    print_field_scalar(info, "count", "%u\n", opl.count);

    p += sizeof(struct objc_property_list);
    offset += sizeof(struct objc_property_list);
    for(j = 0; j < opl.count; j++){
        r = get_pointer_64(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            break;

        memset(&op, '\0', sizeof(struct objc_property));
        if(left < sizeof(struct objc_property)){
            memcpy(&op, r, left);
            printf("   (objc_property entends past the end of the "
                   "section)\n");
        }
        else
            memcpy(&op, r, sizeof(struct objc_property));
        if(info->swapped)
            swap_objc_property(&op, info->host_byte_sex);

        print_field_label(info, "name");
        print_field_value(offset + offsetof(struct objc_property, name),
                          op.name, TRUE, NULL, "\n", info, s,
                          &n_value, &addend);

        print_field_label(info, "attributes");
        print_field_value(offset + offsetof(struct objc_property, attributes),
                          op.attributes, TRUE, NULL, "\n", info, s,
                          &n_value, &addend);

        p += sizeof(struct objc_property);
        offset += sizeof(struct objc_property);
    }

    indent_pop(info);
}

static
void
print_category_t(
uint64_t p,
struct info *info)
{
    struct category_t c;
    void *r;
    uint32_t offset, left;
    struct section_info_64 *s;
    uint64_t n_value;
    int64_t addend;

    r = get_pointer_64(p, &offset, &left, &s,
                       info->sections, info->nsections);
    if(r == NULL)
        return;

    memset(&c, '\0', sizeof(struct category_t));
    if(left < sizeof(struct category_t)){
        memcpy(&c, r, left);
        printf("   (category_t entends past the end of the section)\n");
    }
    else
        memcpy(&c, r, sizeof(struct category_t));
    if(info->swapped)
        swap_category_t(&c, info->host_byte_sex);

    /*
     * The shortest and the longest fields are:
     *    cls
     *    instanceProperties
     * which is just too great. Pick a middle-length field to align this
     * structure, such as "protocols"
     */
    indent_push(info, sizeof("protocols") - 1);

    print_field_label(info, "name");
    print_field_value(offset + offsetof(struct category_t, name),
                      c.name, TRUE, NULL, "\n", info, s, &n_value, &addend);

    print_field_label(info, "cls");
    print_field_value(offset + offsetof(struct category_t, cls),
                      c.cls, FALSE, "(struct class_t *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend != 0) {
        print_class_t(n_value + addend, info);
    }

    print_field_label(info, "instanceMethods");
    print_field_value(offset + offsetof(struct category_t, instanceMethods),
                      c.instanceMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }

    print_field_label(info, "classMethods");
    print_field_value(offset + offsetof(struct category_t, classMethods),
                      c.classMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }

    print_field_label(info, "protocols");
    print_field_value(offset + offsetof(struct category_t, protocols),
                      c.protocols, FALSE, "(struct protocol_list_t *)", "\n",
                      info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_protocol_list_t(n_value + addend, info);
    }

    print_field_label(info, "instanceProperties");
    print_field_value(offset + offsetof(struct category_t, instanceProperties),
                      c.instanceProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend) {
        print_objc_property_list(n_value + addend, info);
    }

    indent_pop(info);
}

void
print_protocol_t(uint64_t p,
                 struct info *info)
{
    struct protocol_t pt;
    void *r;
    uint32_t offset, left;
    struct section_info_64 *s;
    uint64_t n_value;
    int64_t addend;
    
    r = get_pointer_64(p, &offset, &left, &s, info->sections, info->nsections);
    if(r == NULL)
        return;
    
    memset(&pt, '\0', sizeof(struct protocol_t));
    if(left < sizeof(struct protocol_t)){
        memcpy(&pt, r, left);
        printf("   (protocol_t entends past the end of the section)\n");
    }
    else
        memcpy(&pt, r, sizeof(struct protocol_t));
    if(info->swapped)
        swap_protocol_t(&pt, info->host_byte_sex);
    
    /*
     * The shortest and the longest fields are:
     *    isa
     *    optionalInstanceMethods
     * which is just too great. Pick a middle-length field to align this
     * structure, such as "protocols"
     */
    indent_push(info, sizeof("protocols") - 1);

    print_field_label(info, "isa");
    print_field_value(offset + offsetof(struct protocol_t, isa),
                    pt.isa, TRUE, NULL, "\n", info, s, &n_value, &addend);

    print_field_label(info, "name");
    print_field_value(offset + offsetof(struct protocol_t, name),
                    pt.name, TRUE, NULL, "\n", info, s, &n_value, &addend);

    print_field_label(info, "protocols");
    print_field_value(offset + offsetof(struct protocol_t, protocols),
                      pt.protocols, FALSE, "(struct protocol_list_t *)", "\n",
                      info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_protocol_list_t(n_value + addend, info);
    }
    
    print_field_label(info, "instanceMethods");
    print_field_value(offset + offsetof(struct protocol_t, instanceMethods),
                      pt.instanceMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }
    
    print_field_label(info, "classMethods");
    print_field_value(offset + offsetof(struct protocol_t, classMethods),
                      pt.classMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }
    
    print_field_label(info, "optionalInstanceMethods");
    print_field_value(offset + offsetof(struct protocol_t,
                                      optionalInstanceMethods),
                      pt.optionalInstanceMethods, FALSE,
                      "(struct method_list_t *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }
    
    print_field_label(info, "optionalClassMethods");
    print_field_value(offset + offsetof(struct protocol_t,
                                        optionalClassMethods),
                      pt.optionalClassMethods, FALSE,
                      "(struct method_list_t *)", "\n", info, s,
                      &n_value, &addend);
    if(n_value + addend != 0) {
        print_method_list_t(n_value + addend, info);
    }
    
    print_field_label(info, "instanceProperties");
    print_field_value(offset + offsetof(struct protocol_t,
                                      instanceProperties),
                      pt.instanceProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value, &addend);

    if(n_value + addend) {
        print_objc_property_list(n_value + addend, info);
    }

    indent_pop(info);
}

static
void
print_message_refs(
struct section_info_64 *s,
struct info *info)
{
    uint32_t i, left, offset;
    uint64_t p, n_value;
    int64_t addend;
    struct message_ref mr;
    void *r;

    if(s == NULL)
        return;

    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);

    indent_push(info, sizeof("imp") - 1);

    offset = 0;
    for(i = 0; i < s->size; i += sizeof(struct message_ref)){
        p = s->addr + i;
        r = get_pointer_64(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            break;

        memset(&mr, '\0', sizeof(struct message_ref));
        if(left < sizeof(struct message_ref)){
            memcpy(&mr, r, left);
            printf(" (message_ref entends past the end of the section)\n");
        }
        else
            memcpy(&mr, r, sizeof(struct message_ref));
        if(info->swapped)
            swap_message_ref(&mr, info->host_byte_sex);

        print_field_label(info, "imp");
        print_field_value(offset + offsetof(struct message_ref, imp),
                        mr.imp, FALSE, NULL, "\n", info, s, &n_value, &addend);

        print_field_label(info, "sel");
        print_field_value(offset + offsetof(struct message_ref, sel),
                        mr.sel, FALSE, NULL, "\n", info, s, &n_value, &addend);

        offset += sizeof(struct message_ref);
    }

    indent_pop(info);
}

static
void
print_image_info(
struct section_info_64 *s,
struct info *info)
{
    uint32_t left, offset, swift_version;
    uint64_t p;
    struct objc_image_info o;
    void *r;

    if(s == NULL)
        return;

    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);
    p = s->addr;
    r = get_pointer_64(p, &offset, &left, &s,
                       info->sections, info->nsections);
    if(r == NULL)
        return;

    memset(&o, '\0', sizeof(struct objc_image_info));
    if(left < sizeof(struct objc_image_info)){
        memcpy(&o, r, left);
        printf(" (objc_image_info entends past the end of the section)\n");
    }
    else
        memcpy(&o, r, sizeof(struct objc_image_info));
    if(info->swapped)
        swap_objc_image_info(&o, info->host_byte_sex);

    indent_push(info, sizeof("version") - 1);

    print_field_scalar(info, "version", "%u\n", o.version);
    print_field_scalar(info, "flags", "0x%x", o.flags);

    if(o.flags & OBJC_IMAGE_IS_REPLACEMENT)
        printf(" OBJC_IMAGE_IS_REPLACEMENT");
    if(o.flags & OBJC_IMAGE_SUPPORTS_GC)
        printf(" OBJC_IMAGE_SUPPORTS_GC");
    swift_version = (o.flags >> 8) & 0xff;
    if(swift_version != 0){
        if(swift_version == 1)
            printf(" Swift 1.0");
        else if(swift_version == 2)
            printf(" Swift 1.1");
        else if(swift_version == 3)
            printf(" Swift 2.0");
        else if(swift_version == 4)
            printf(" Swift 3.0");
        else if(swift_version == 5)
            printf(" Swift 4.0");
        else if(swift_version == 6)
            printf(" Swift 4.1/4.2");
        else if(swift_version == 7)
            printf(" Swift 5 or later");
        else
            printf(" unknown future Swift version (%d)", swift_version);
    }
    printf("\n");

    indent_pop(info);
}

void
print_objc_string_object_section_64(
char *sectname,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
cpu_type_t cputype,
struct nlist_64 *symbols64,
uint32_t nsymbols,
char *strings,
const uint32_t strings_size,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
enum bool verbose)
{
    struct info info;
    struct section_info_64 *o, cstring_section;
    struct objc_string_object_64 *string_objects, *s, string_object;
    uint64_t string_objects_addr, string_objects_size;
    uint64_t size, left;
    uint32_t left32;
    char *p;
    const char *name;

    printf("Contents of (" SEG_OBJC ",%s) section\n", sectname);
    info.object_addr = object_addr;
    info.object_size = object_size;
    info.host_byte_sex = get_host_byte_sex();
    info.swapped = info.host_byte_sex != object_byte_sex;
    info.cputype = cputype;
    info.symbols64 = symbols64;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.sorted_symbols = sorted_symbols;
    info.nsorted_symbols = nsorted_symbols;
    info.verbose = verbose;
    info.indent_level = 0;
    info.indent_widths[info.indent_level] = 0;

    get_sections_64(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &info.sections,
                    &info.nsections, &info.textbase, &info.database);

    o = get_section_64(info.sections, info.nsections, SEG_OBJC, sectname);
    if(o == NULL)
        return;

    get_cstring_section_64(load_commands, ncmds, sizeofcmds,object_byte_sex,
                           object_addr, object_size, &cstring_section);

    string_objects = (struct objc_string_object_64 *)o->contents;
    string_objects_addr = o->addr;
    string_objects_size = o->size;
    for(s = string_objects;
        (char *)s < (char *)string_objects + string_objects_size;
        s++)
    {
        memset(&string_object, '\0', sizeof(struct objc_string_object_64));
        left = string_objects_size - (s - string_objects);
        size = left < sizeof(struct objc_string_object_64) ?
        left : sizeof(struct objc_string_object_64);
        memcpy(&string_object, s, size);
        if(info.swapped)
            swap_string_object_64(&string_object, info.host_byte_sex);

        if((char *)s + sizeof(struct objc_string_object_64) >
           (char *)s + string_objects_size) {
            printf("String Object extends past end of %s section\n",
                   sectname);
        }

        indent_push(&info, sizeof("characters") - 1);

        printf("String Object 0x%llx\n",
               string_objects_addr + ((char *)s - (char *)string_objects));
        print_field_scalar(&info, "isa", "0x%llx", string_object.isa);

        name = get_symbol_64((uintptr_t)s - (uintptr_t)string_objects,
                             o->addr, info.textbase, info.database,
                             string_object.isa, o->relocs, o->nrelocs,
                             &info, NULL, NULL);
        if(name != NULL)
            printf(" %s\n", name);
        else
            printf("\n");

        print_field_scalar(&info, "characters", "0x%llx",
                           string_object.characters);
        if(verbose){
            p = get_pointer_64(string_object.characters, NULL, &left32,
                               NULL, info.sections, info.nsections);
            if(p != NULL)
                printf(" %.*s", (int)left32, p);
        }
        printf("\n");

        print_field_scalar(&info, "_length", "%u\n", string_object._length);
        print_field_scalar(&info, "_pad", "%u\n", string_object._pad);

        indent_pop(&info);
    }
}

static
void
get_sections_64(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct section_info_64 **sections,
uint32_t *nsections,
uint64_t *textbase,
uint64_t *database) 
{
    enum byte_sex host_byte_sex;
    enum bool swapped, textbase_set, database_set, encrypt_found,
    encrypt64_found;
    uint32_t i, j;
    uint64_t left, size;
    struct load_command lcmd, *lc;
    char *p;
    struct segment_command_64 sg64;
    struct section_64 s64;
    struct encryption_info_command encrypt;
    struct encryption_info_command_64 encrypt64;

    host_byte_sex = get_host_byte_sex();
    swapped = host_byte_sex != object_byte_sex;

    *sections = NULL;
    *nsections = 0;
    textbase_set = FALSE;
    *textbase = 0;
    database_set = FALSE;
    *database = 0;
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
        left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

        switch(lcmd.cmd){
            case LC_SEGMENT_64:
                memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
                size = left < sizeof(struct segment_command_64) ?
                left : sizeof(struct segment_command_64);
                memcpy((char *)&sg64, (char *)lc, size);
                if(swapped)
                    swap_segment_command_64(&sg64, host_byte_sex);
                if((sg64.initprot & VM_PROT_WRITE) == VM_PROT_WRITE &&
                   database_set == FALSE){
                    *database = sg64.vmaddr;
                    database_set = TRUE;
                }
                if((sg64.initprot & VM_PROT_READ) == VM_PROT_READ &&
                   textbase_set == FALSE){
                    *textbase = sg64.vmaddr;
                    textbase_set = TRUE;
                }
                p = (char *)lc + sizeof(struct segment_command_64);
                for(j = 0 ; j < sg64.nsects ; j++){
                    if(p + sizeof(struct section_64) >
                       (char *)load_commands + sizeofcmds){
                        printf("section structure command extends past "
                               "end of load commands\n");
                    }
                    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
                    memset((char *)&s64, '\0', sizeof(struct section_64));
                    size = left < sizeof(struct section_64) ?
                    left : sizeof(struct section_64);
                    memcpy((char *)&s64, p, size);
                    if(swapped)
                        swap_section_64(&s64, 1, host_byte_sex);

                    *sections = reallocate(*sections,
                        sizeof(struct section_info_64) * (*nsections + 1));
                    memcpy((*sections)[*nsections].segname,
                           s64.segname, 16);
                    memcpy((*sections)[*nsections].sectname,
                           s64.sectname, 16);
                    (*sections)[*nsections].addr = s64.addr;
                    (*sections)[*nsections].contents = object_addr + s64.offset;
                    (*sections)[*nsections].offset = s64.offset;
                    (*sections)[*nsections].zerofill =
                    (s64.flags & SECTION_TYPE) == S_ZEROFILL ? TRUE : FALSE;
                    if(s64.offset > object_size){
                        printf("section contents of: (%.16s,%.16s) is past "
                               "end of file\n", s64.segname, s64.sectname);
                        (*sections)[*nsections].size =  0;
                    }
                    else if(s64.offset + s64.size > object_size){
                        printf("part of section contents of: (%.16s,%.16s) "
                               "is past end of file\n",
                               s64.segname, s64.sectname);
                        (*sections)[*nsections].size = object_size - s64.offset;
                    }
                    else
                        (*sections)[*nsections].size = s64.size;
                    if(s64.reloff >= object_size){
                        printf("relocation entries offset for (%.16s,%.16s)"
                               ": is past end of file\n", s64.segname,
                               s64.sectname);
                        (*sections)[*nsections].nrelocs = 0;
                    }
                    else{
                        (*sections)[*nsections].relocs =
                        (struct relocation_info *)(object_addr +
                                                   s64.reloff);
                        if(s64.reloff +
                           s64.nreloc * sizeof(struct relocation_info) >
                           object_size){
                            printf("relocation entries for section (%.16s,"
                                   "%.16s) extends past end of file\n",
                                   s64.segname, s64.sectname);
                            (*sections)[*nsections].nrelocs =
                            (uint32_t)((object_size - s64.reloff) /
                            sizeof(struct relocation_info));
                        }
                        else
                            (*sections)[*nsections].nrelocs = s64.nreloc;
                        if(swapped)
                            swap_relocation_info(
                                (*sections)[*nsections].relocs,
                                (*sections)[*nsections].nrelocs,
                                host_byte_sex);
                    }
                    if(sg64.flags & SG_PROTECTED_VERSION_1)
                        (*sections)[*nsections].protected = TRUE;
                    else
                        (*sections)[*nsections].protected = FALSE;
                    if((s64.flags & SECTION_TYPE) == S_CSTRING_LITERALS)
                        (*sections)[*nsections].cstring = TRUE;
                    else
                        (*sections)[*nsections].cstring = FALSE;
                    (*nsections)++;
                    
                    if(p + sizeof(struct section_64) >
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
        for(i = 0; i < *nsections; i++){
            if((*sections)[i].size > 0 && (*sections)[i].zerofill == FALSE){
                if((*sections)[i].offset >
                   encrypt.cryptoff + encrypt.cryptsize){
                    /* section starts past encryption area */ ;
                }
                else if((*sections)[i].offset + (*sections)[i].size <
                        encrypt.cryptoff){
                    /* section ends before encryption area */ ;
                }
                else{
                    /* section has part in the encrypted area */
                    (*sections)[i].protected = TRUE;
                }
            }
        }
    }
    if(encrypt64_found == TRUE && encrypt64.cryptid != 0){
        for(i = 0; i < *nsections; i++){
            if((*sections)[i].size > 0 && (*sections)[i].zerofill == FALSE){
                if((*sections)[i].offset >
                   encrypt64.cryptoff + encrypt64.cryptsize){
                    /* section starts past encryption area */ ;
                }
                else if((*sections)[i].offset + (*sections)[i].size <
                        encrypt64.cryptoff){
                    /* section ends before encryption area */ ;
                }
                else{
                    /* section has part in the encrypted area */
                    (*sections)[i].protected = TRUE;
                }
            }
        }
    }
}

static
struct section_info_64 *
get_section_64(
struct section_info_64 *sections,
uint32_t nsections,
char *segname,
char *sectname)
{
    uint32_t i;
    
    for(i = 0; i < nsections; i++){
        if(strncmp(sections[i].segname, segname, 16) == 0 &&
           strncmp(sections[i].sectname, sectname, 16) == 0){
            return(sections + i);
        }
    }
    return(NULL);
}

static
void
get_cstring_section_64(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct section_info_64 *cstring_section)
{
    enum byte_sex host_byte_sex;
    enum bool swapped;

    uint32_t i, j;
    uint64_t left, size;
    struct load_command lcmd, *lc;
    char *p;
    struct segment_command_64 sg64;
    struct section_64 s64;

    host_byte_sex = get_host_byte_sex();
    swapped = host_byte_sex != object_byte_sex;

    memset(cstring_section, '\0', sizeof(struct section_info_64));

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
        left = sizeofcmds - (uint32_t)((char *)lc - (char *)load_commands);

        switch(lcmd.cmd){
            case LC_SEGMENT_64:
                memset((char *)&sg64, '\0', sizeof(struct segment_command_64));
                size = left < sizeof(struct segment_command_64) ?
                left : sizeof(struct segment_command_64);
                memcpy((char *)&sg64, (char *)lc, size);
                if(swapped)
                    swap_segment_command_64(&sg64, host_byte_sex);
                
                p = (char *)lc + sizeof(struct segment_command_64);
                for(j = 0 ; j < sg64.nsects ; j++){
                    if(p + sizeof(struct section_64) >
                       (char *)load_commands + sizeofcmds){
                        printf("section structure command extends past "
                               "end of load commands\n");
                    }
                    left = sizeofcmds - (uint32_t)(p - (char *)load_commands);
                    memset((char *)&s64, '\0', sizeof(struct section_64));
                    size = left < sizeof(struct section_64) ?
                    left : sizeof(struct section_64);
                    memcpy((char *)&s64, p, size);
                    if(swapped)
                        swap_section_64(&s64, 1, host_byte_sex);

                    if(strcmp(s64.segname, SEG_TEXT) == 0 &&
                       strcmp(s64.sectname, "__cstring") == 0){
                        cstring_section->addr = s64.addr;
                        cstring_section->contents = object_addr + s64.offset;
                        if(s64.offset > object_size){
                            printf("section contents of: (%.16s,%.16s) is past "
                                   "end of file\n", s64.segname, s64.sectname);
                            cstring_section->size = 0;
                        }
                        else if(s64.offset + s64.size > object_size){
                            printf("part of section contents of: (%.16s,%.16s) "
                                   "is past end of file\n",
                                   s64.segname, s64.sectname);
                            cstring_section->size = object_size - s64.offset;
                        }
                        else
                            cstring_section->size = s64.size;
                        if(sg64.flags & SG_PROTECTED_VERSION_1)
                            cstring_section->protected = TRUE;
                        else
                            cstring_section->protected = FALSE;
                        cstring_section->cstring = TRUE;
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
void *
get_pointer_64(
uint64_t p,
uint32_t *offset,
uint32_t *left,
struct section_info_64 **s,
struct section_info_64 *sections,
uint32_t nsections)
{
    void *r;
    uint64_t addr;
    uint32_t i;
    
    addr = p;
    for(i = 0; i < nsections; i++){
        if(addr >= sections[i].addr &&
           addr < sections[i].addr + sections[i].size){
            if(s != NULL)
                *s = sections + i;
            if(offset != NULL)
                *offset = (uint32_t)(addr - sections[i].addr);
            if(left != NULL)
                *left = (uint32_t)(sections[i].size - (addr-sections[i].addr));
            if(sections[i].protected == TRUE && sections[i].cstring == TRUE)
                r = "some string from a protected section";
            else
                r = sections[i].contents + (addr - sections[i].addr);
            return(r);
        }
    }
    if(s != NULL)
        *s = NULL;
    if(offset != NULL)
        *offset = 0;
    if(left != NULL)
        *left = 0;
    return(NULL);
}

/*
 * get_symbol_64() returns the name of a symbol (or NULL). Based on the
 * relocation information at the specified section offset, address and database
 * or the (pointer) value.  It indirectly returns the symbol's value through
 * *n_value and the relocation's addend through *addend.  Since the later values
 * are needed to walk the pointers it is up to the caller to check the
 * info->verbose flag to print the name or the *n_value + *addend or raw
 * pointer value.
 */
static
const char *
get_symbol_64(
uint64_t sect_offset,
uint64_t sect_addr,
uint64_t textbase,
uint64_t database,
uint64_t value,
struct relocation_info *relocs,
uint32_t nrelocs,
struct info *info,
uint64_t *n_value,
int64_t *addend)
{
    uint32_t i;
    unsigned int r_symbolnum;
    uint32_t n_strx;
    const char *name;
    enum bool has_auth;

    if(n_value != NULL)
        *n_value = 0;
    if(addend != NULL)
        *addend = value;
    has_auth = FALSE;

    /*
     * In the info->verbose == FALSE case we can't simply return now as for
     * the ThreadedRebaseBind case we need to return the real pointer value
     * in "n_value + addend" without the bits from the ThreadedRebaseBind.
     * To do this we need look through the bind entries or in the rebase
     * case move the original pointer value masked with the right bits off
     * into n_value and zero out the addend so the caller can get the real
     * pointer value from n_value + addend and indirect through that.
     * The caller now has to check info->verbose == FALSE to print the
     * original pointer but use the n_value + addend to follow the pointer.
     */

    /*
     * First look in section's relocation entries if it has them which is
     * the .o file case to find the name, n_value and added.
     */
    for(i = 0; i < nrelocs; i++){
        if((uint32_t)relocs[i].r_address == sect_offset){
            r_symbolnum = relocs[i].r_symbolnum;
            if(relocs[i].r_extern){
                if(r_symbolnum >= info->nsymbols)
                    break;
                n_strx = info->symbols64[r_symbolnum].n_un.n_strx;
                if(n_strx <= 0 || n_strx >= info->strings_size)
                    break;
                /*
                 * If this is arm64e and if r_type is a
                 * ARM64_RELOC_AUTHENTICATED_POINTER we need to adjust
                 * addend to just the low 32-bits (signed) of the pointer
                 * value.
                 */
                if(info->cputype == CPU_TYPE_ARM64 &&
                   info->cpusubtype == CPU_SUBTYPE_ARM64E &&
                   relocs[i].r_type == ARM64_RELOC_AUTHENTICATED_POINTER){
                    if(addend != NULL){
                        *addend = 0xffffffffULL & value;
                        if((*addend & 0x80000000ULL) != 0)
                            *addend |= 0xffffffff00000000ULL;
                    }
                }
                if(n_value != NULL)
                    *n_value = info->symbols64[r_symbolnum].n_value;
                return(info->strings + n_strx);
            }
            break;
        }
        if(reloc_has_pair(info->cputype, relocs[i].r_type) == TRUE)
            i++;
    }

    /*
     * Next look in external relocation entries of if it has them which is
     * the original dyld image case to find the name, n_value and added.
     */
    for(i = 0; i < info->next_relocs; i++){
        if((uint32_t)info->ext_relocs[i].r_address ==
           database + sect_offset){
            r_symbolnum = info->ext_relocs[i].r_symbolnum;
            if(info->ext_relocs[i].r_extern){
                if(r_symbolnum >= info->nsymbols)
                    break;
                n_strx = info->symbols64[r_symbolnum].n_un.n_strx;
                if(n_strx <= 0 || n_strx >= info->strings_size)
                    break;
                if(n_value != NULL)
                    *n_value = info->symbols64[r_symbolnum].n_value;
                return(info->strings + n_strx);
            }
            break;
        }
        if(reloc_has_pair(info->cputype, info->ext_relocs[i].r_type) ==TRUE)
            i++;
    }

    /*
     * Lastly look in the dyld bind entries if it has them which is
     * the modern fully linked dyld image case to find the name and added.
     */
    name = get_dyld_bind_info_symbolname(sect_addr + sect_offset,
                                         info->dbi, info->ndbi,
                                         info->chain_format, addend);
    /*
     * If we find a bind entry we return the name which may not be printed
     * if not in verbose mode.  But we needed to make the call above to
     * get the correct addend if info->ThreadedRebaseBind was true.
     */
    if(name != NULL)
        return(name);
    
    /*
     * Fully linked modern images for dyld get will get here if it is has
     * a rebase entry, and the pointer value in "value" would be what this
     * pointer is pointing to in this image normally.
     *
     * But if info->ThreadedRebaseBind is true, to get the correct pointer
     * value we need to know to mask off the upper bits and only keep the
     * low 51-bits.
     */
    /*
     * Unless this is arm64e we have to look for the high authenticated bit
     * to know to use only the low 32-bits as the pointer value.
     */
    /* So at this point, we set n_value as the masked pointer value
     * and zero as the addend for return or the value to call guess_symbol()
     * with for a guess at which symbol has this address.
     */
    value = get_chained_rebase_value(value, info->chain_format, &has_auth);
    if(n_value != NULL)
        *n_value = value;
    if(addend != NULL)
        *addend = 0;
    
    /*
     * We don't guess for symbol values of zero as it is wrong most of the
     * time.
     */
    if(value == 0)
        return(NULL);
    
    /*
     * Remember that authenticated Threaded Rebase Value is a relative
     * vmaddr to the start of text. So a symbol may be encoded as 0x7d70
     * when it really represents 0x0000000100007d70. So we need to add the
     * start of text to "value" before guessing the symbol name.
     *
     * Also note that we're returning the raw, unadjusted value in *n_value
     * so that otool continues to print the bits as they are ...
     */
    if(info->chain_format && has_auth == TRUE){
        value += textbase;
    }
    
    return(guess_symbol(value, info->sorted_symbols, info->nsorted_symbols,
                        info->verbose));
}

/*
 * print_field_scalar() prints a label followed by a formatted value. the label
 * is idented to fit within the info's indent state.
 */
static
void
print_field_scalar(
struct info *info,
const char* label,
const char* fmt,
...)
{
    /* print the label */
    print_field_label(info, label);
    
    /* print the data, if any */
    if (fmt) {
        va_list ap;
        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);
    }
}

/*
 * print_field_label() prints a formatted label. the label is indented to fit
 * within the info's indent state. A single space character will follow the
 * label so that the next value can simply be printed.
 */
static
void
print_field_label(
struct info *info,
const char* label,
...)
{
    va_list ap;
    int width = 0;
    uint32_t label_indent;
    uint32_t label_width;
    
    /* get the current label field width from the indent state */
    label_indent = info->indent_level * 4;
#if 1
    /*
     * use the curent indent width. if the indent level is too deep, just print
     * the value immediately after the label.
     */
    label_width = (info->indent_level < MAXINDENT ?
                   info->indent_widths[info->indent_level] : 0);
#else
    /*
     * use the current indent width unless that would cause the value at this
     * level to print to the left of the previous value. In practice, we need
     * to loop over all the indent widths, compute the right edge of the label
     * field, and use the largest such value.
     */
    uint32_t right = 0;
    for (uint32_t i = 0; i < MAXINDENT; ++i) {
        if (i > info->indent_level)
            break;
        
        uint32_t r = i * 4 + info->indent_widths[i];
        if (r > right)
            right = r;
    }
    label_width = right - label_indent;
#endif
    
    /* measure the width of the string data */
    va_start(ap, label);
    if (label) {
        width = vsnprintf(NULL, 0, label, ap);
    }
    va_end(ap);
    
    /* adjust the width to represent the space following the label */
    width = width < label_width ? label_width - width : 0;
    
    /* print the indent spaces */
    printf("%*s", label_indent, "");
    
    /* print the label */
    if (label) {
        va_start(ap, label);
        vprintf(label, ap);
        va_end(ap);
    }
    
    /* print right padding */
    printf("%*s", width + 1, "");
}

/*
 * print_field_value() prints the following information:
 *
 *   ( pointer | n_value [ + addend ] [symbol]) [data] [type] [suffix]
 *
 * pointer - the raw pointer on disk, only displaying when -v is not specified.
 * n_value - the adjusted pointer view, correcting for chained-fixups and
 *           authenticated pointers.
 * symbol  - the symbol name corresponding to pointer
 * data    - C-string data within the file pointed to by the n_value / addend.
 * type    - an optional C-string that is printed only if supplied and if
 *           n_value + addend != 0.
 * suffix  - an optional C-string that prints after the other field elements,
 *           often used to print a newline.
 *
 * print_field_value usually follows a call to print_field_label, but this is
 * not required.
 */
static
void
print_field_value(
uint64_t offset,
uint64_t p,
enum bool print_data,
const char* type_name,
const char* suffix,
struct info *info,
struct section_info_64 *s,
uint64_t *out_n_value,
int64_t *out_addend)
{
    uint64_t n_value;
    int64_t addend;
    const char* sym_name;

    /* read the symbol name, n_value, and addend. */
    sym_name = get_symbol_64(offset, s->addr, info->textbase, info->database,
                             p, s->relocs, s->nrelocs, info, &n_value, &addend);

    /* print the numeric pointer value */
    if (info->verbose) {
        printf("0x%llx", n_value);
        if (addend)
            printf(" + 0x%llx", addend);
        if (sym_name)
            printf(" %s", sym_name);
    }
    else {
        printf("0x%llx", p);
    }

    /* print the pointer data if any, if requested */
    if (info->verbose && print_data) {
        const char* ptr_data;
        ptr_data = get_pointer_64(n_value + addend, NULL, NULL, NULL,
                                  info->sections, info->nsections);
        if (ptr_data)
            printf(" %s", ptr_data);
    }

#if 0
    if (info->verbose && type_name && (n_value + addend > 0)) {
        printf(" %s", type_name);
    }
#endif

    /* print the suffix field */
    if (suffix)
        printf("%s", suffix);

    /* return the n_value and addend */
    if (out_n_value)
        *out_n_value = n_value;
    if (out_addend)
        *out_addend = addend;
}

void
indent_push(
struct info *info,
uint32_t width)
{
    info->indent_level += 1;
    if (info->indent_level < MAXINDENT)
        info->indent_widths[info->indent_level] = width;
}

void
indent_pop(
struct info *info)
{
    if (info->indent_level)
        info->indent_level -= 1;
}
