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
#include "stddef.h"
#include "string.h"
#include "mach-o/loader.h"
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
 * 32-bit Objective-C 2 meta data structures.
 *
 * The real structure definitions come from the objc4 project in the private
 * header file runtime/objc-runtime-new.h in that project.
 */

struct class_t {
    uint32_t isa;		/* class_t * (32-bit pointer) */
    uint32_t superclass;	/* class_t * (32-bit pointer) */
    uint32_t cache;		/* Cache (32-bit pointer) */
    uint32_t vtable;		/* IMP * (32-bit pointer) */
    uint32_t data;		/* class_ro_t * (32-bit pointer) */
};

static
void
swap_class_t(
struct class_t *c,
enum byte_sex target_byte_sex)
{
    c->isa = SWAP_INT(c->isa);
    c->superclass = SWAP_INT(c->superclass);
    c->cache = SWAP_INT(c->cache);
    c->vtable = SWAP_INT(c->vtable);
    c->data = SWAP_INT(c->data);
}

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
    uint32_t ivarLayout;	/* const uint8_t * (32-bit pointer) */
    uint32_t name; 		/* const char * (32-bit pointer) */
    uint32_t baseMethods; 	/* const method_list_t * (32-bit pointer) */
    uint32_t baseProtocols; 	/* const protocol_list_t * (32-bit pointer) */
    uint32_t ivars; 		/* const ivar_list_t * (32-bit pointer) */
    uint32_t weakIvarLayout; 	/* const uint8_t * (32-bit pointer) */
    uint32_t baseProperties; 	/* const struct objc_property_list *
							(32-bit pointer) */
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
    cro->ivarLayout = SWAP_INT(cro->ivarLayout);
    cro->name = SWAP_INT(cro->name);
    cro->baseMethods = SWAP_INT(cro->baseMethods);
    cro->baseProtocols = SWAP_INT(cro->baseProtocols);
    cro->ivars = SWAP_INT(cro->ivars);
    cro->weakIvarLayout = SWAP_INT(cro->weakIvarLayout);
    cro->baseProperties = SWAP_INT(cro->baseProperties);
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
    uint32_t name;	/* SEL (32-bit pointer) */
    uint32_t types;	/* const char * (32-bit pointer) */
    uint32_t imp;	/* IMP (32-bit pointer) */
};

static
void
swap_method_t(
struct method_t *m,
enum byte_sex target_byte_sex)
{
    m->name = SWAP_INT(m->name);
    m->types = SWAP_INT(m->types);
    m->imp = SWAP_INT(m->imp);
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
    uint32_t offset;	/* uintptr_t * (32-bit pointer) */
    uint32_t name;	/* const char * (32-bit pointer) */
    uint32_t type;	/* const char * (32-bit pointer) */
    uint32_t alignment;
    uint32_t size;
};

static
void
swap_ivar_t(
struct ivar_t *i,
enum byte_sex target_byte_sex)
{
    i->offset = SWAP_INT(i->offset);
    i->name = SWAP_INT(i->name);
    i->type = SWAP_INT(i->type);
    i->alignment = SWAP_INT(i->alignment);
    i->size = SWAP_INT(i->size);
}

struct protocol_list_t {
    uint32_t count;	/* uintptr_t (a 32-bit value) */
    /* struct protocol_t * list[0];  These pointers follow inline */
};

static
void
swap_protocol_list_t(
struct protocol_list_t *pl,
enum byte_sex target_byte_sex)
{
    pl->count = SWAP_INT(pl->count);
}

struct protocol_t {
    uint32_t isa;			/* id * (32-bit pointer) */
    uint32_t name;			/* const char * (32-bit pointer) */
    uint32_t protocols;			/* struct protocol_list_t *
							(32-bit pointer) */
    uint32_t instanceMethods;		/* method_list_t * (32-bit pointer) */
    uint32_t classMethods;		/* method_list_t * (32-bit pointer) */
    uint32_t optionalInstanceMethods;	/* method_list_t * (32-bit pointer) */
    uint32_t optionalClassMethods;	/* method_list_t * (32-bit pointer) */
    uint32_t instanceProperties;	/* struct objc_property_list *
							   (32-bit pointer) */
};

static
void
swap_protocol_t(
struct protocol_t *p,
enum byte_sex target_byte_sex)
{
    p->isa = SWAP_INT(p->isa);
    p->name = SWAP_INT(p->name);
    p->protocols = SWAP_INT(p->protocols);
    p->instanceMethods = SWAP_INT(p->instanceMethods);
    p->classMethods = SWAP_INT(p->classMethods);
    p->optionalInstanceMethods = SWAP_INT(p->optionalInstanceMethods);
    p->optionalClassMethods = SWAP_INT(p->optionalClassMethods);
    p->instanceProperties = SWAP_INT(p->instanceProperties);
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
    uint32_t name;		/* const char * (32-bit pointer) */
    uint32_t attributes;	/* const char * (32-bit pointer) */
};

static
void
swap_objc_property(
struct objc_property *op,
enum byte_sex target_byte_sex)
{
    op->name = SWAP_INT(op->name);
    op->attributes = SWAP_INT(op->attributes);
}

struct category_t {
    uint32_t name; 		/* const char * (32-bit pointer) */
    uint32_t cls;		/* struct class_t * (32-bit pointer) */
    uint32_t instanceMethods;	/* struct method_list_t * (32-bit pointer) */
    uint32_t classMethods;	/* struct method_list_t * (32-bit pointer) */
    uint32_t protocols;		/* struct protocol_list_t * (32-bit pointer) */
    uint32_t instanceProperties; /* struct objc_property_list *
				    (32-bit pointer) */
};

static
void
swap_category_t(
struct category_t *c,
enum byte_sex target_byte_sex)
{
    c->name = SWAP_INT(c->name);
    c->cls = SWAP_INT(c->cls);
    c->instanceMethods = SWAP_INT(c->instanceMethods);
    c->classMethods = SWAP_INT(c->classMethods);
    c->protocols = SWAP_INT(c->protocols);
    c->instanceProperties = SWAP_INT(c->instanceProperties);
}

struct message_ref {
    uint32_t imp;	/* IMP (32-bit pointer) */
    uint32_t sel;	/* SEL (32-bit pointer) */
};

static
void
swap_message_ref(
struct message_ref *mr,
enum byte_sex target_byte_sex)
{
    mr->imp = SWAP_INT(mr->imp);
    mr->sel = SWAP_INT(mr->sel);
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

#define MAXINDENT 10

struct info {
    enum bool swapped;
    enum byte_sex host_byte_sex;
    struct section_info_32 *sections;
    uint32_t nsections;
    cpu_type_t cputype;
    struct nlist *symbols;
    uint32_t nsymbols;
    char *strings;
    uint32_t strings_size;
    struct symbol *sorted_symbols;
    uint32_t nsorted_symbols;
    uint32_t database;
    struct relocation_info *ext_relocs;
    uint32_t next_relocs;
    struct relocation_info *loc_relocs;
    uint32_t nloc_relocs;
    enum bool verbose;
    struct dyld_bind_info *dbi;
    uint64_t ndbi;
    enum chain_format_t chain_format;
    uint32_t indent_level;
    uint32_t indent_widths[MAXINDENT];
};

struct section_info_32 {
    char segname[16];
    char sectname[16];
    char *contents;
    uint32_t addr;
    uint32_t size;
    uint32_t offset;
    struct relocation_info *relocs;
    uint32_t nrelocs;
    enum bool protected;
    enum bool zerofill;
};

static void walk_pointer_list(
    char *listname,
    struct section_info_32 *s,
    struct info *info,
    void (*func)(uint32_t, struct info *));

static void print_class_t(
    uint32_t p,
    struct info *info);

static void print_class_ro_t(
    uint32_t p,
    struct info *info,
    enum bool *is_meta_class);

static void print_layout_map(
    uint32_t p,
    struct info *info);

static void print_method_list_t(
    uint32_t p,
    struct info *info);

static void print_ivar_list_t(
    uint32_t p,
    struct info *info);

static void print_protocol_list_t(
    uint32_t p,
    struct info *info);

static void print_objc_property_list(
    uint32_t p,
    struct info *info);

static void print_category_t(
    uint32_t p,
    struct info *info);

static void print_protocol_t(
    uint32_t p,
    struct info *info);

static void print_message_refs(
    struct section_info_32 *s,
    struct info *info);

static void print_image_info(
    struct section_info_32 *s,
    struct info *info);

static void get_sections_32(
    struct load_command *load_commands,
    uint32_t ncmds,
    uint32_t sizeofcmds,
    enum byte_sex object_byte_sex,
    char *object_addr,
    uint64_t object_size,
    struct section_info_32 **sections,
    uint32_t *nsections,
    uint32_t *database);

static struct section_info_32 *get_section_32(
    struct section_info_32 *sections,
    uint32_t nsections,
    char *segname,
    char *sectname);

static void *get_pointer_32(
    uint32_t p,
    uint32_t *offset,
    uint32_t *left,
    struct section_info_32 **s,
    struct section_info_32 *sections,
    uint32_t nsections);

static const char *get_symbol_32(
    uint32_t sect_offset,
    uint32_t sect_addr,
    uint32_t database_offset,
    uint64_t value,
    struct relocation_info *relocs,
    uint32_t nrelocs,
    struct info *info,
    uint32_t* n_value);

static
void
print_field_scalar(
    struct info *info,
    const char* label,
    const char* fmt,
    ...);

static
void
print_field_label(
    struct info *info,
    const char* label,
    ...);

static
void
print_field_value(
    uint32_t offset,
    uint32_t p,
    enum bool print_data,
    const char* type_name,
    const char* suffix,
    struct info *info,
    struct section_info_32 *s,
    uint32_t *out_n_value);

static void indent_push(
    struct info *info,
    uint32_t width);

static void indent_pop(
    struct info *info);

/*
 * Print the objc2 meta data in 32-bit Mach-O files.
 */
void
print_objc2_32bit(
cpu_type_t cputype,
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct nlist *symbols,
uint32_t nsymbols,
char *strings,
uint32_t strings_size,
struct symbol *sorted_symbols,
uint32_t nsorted_symbols,
struct relocation_info *ext_relocs,
uint32_t next_relocs,
struct relocation_info *loc_relocs,
uint32_t nloc_relocs,
struct dyld_bind_info* dbi,
uint64_t ndbi,
enum chain_format_t chain_format,
enum bool verbose)
{
    struct section_info_32 *s;
    struct info info;
    
    info.host_byte_sex = get_host_byte_sex();
    info.swapped = info.host_byte_sex != object_byte_sex;
    info.cputype = cputype;
    info.symbols = symbols;
    info.nsymbols = nsymbols;
    info.strings = strings;
    info.strings_size = strings_size;
    info.sorted_symbols = sorted_symbols;
    info.nsorted_symbols = nsorted_symbols;
    info.ext_relocs = ext_relocs;
    info.next_relocs = next_relocs;
    info.loc_relocs = loc_relocs;
    info.nloc_relocs = nloc_relocs;
    info.verbose = verbose;
    info.dbi = dbi;
    info.ndbi = ndbi;
    info.chain_format = chain_format;

    get_sections_32(load_commands, ncmds, sizeofcmds, object_byte_sex,
                    object_addr, object_size, &info.sections,
                    &info.nsections, &info.database);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__class_list");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_classlist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_classlist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_classlist");
    walk_pointer_list("class", s, &info, print_class_t);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__class_refs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_classrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_classrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_classrefs");
    walk_pointer_list("class refs", s, &info, NULL);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__super_refs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_superrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_superrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_superrefs");
    walk_pointer_list("super refs", s, &info, NULL);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__category_list");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_catlist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_catlist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_catlist");
    walk_pointer_list("category", s, &info, print_category_t);
    
    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__protocol_list");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_protolist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_protolist");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_protolist");
    walk_pointer_list("protocol", s, &info, print_protocol_t);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC2", "__message_refs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_msgrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_msgrefs");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_msgrefs");
    print_message_refs(s, &info);

    s = get_section_32(info.sections, info.nsections,
                       "__OBJC", "__image_info");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA", "__objc_imageinfo");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_CONST", "__objc_imageinfo");
    if(s == NULL)
        s = get_section_32(info.sections, info.nsections,
                           "__DATA_DIRTY", "__objc_imageinfo");
    print_image_info(s, &info);
}

static
void
walk_pointer_list(
char *listname,
struct section_info_32 *s,
struct info *info,
void (*func)(uint32_t, struct info *))
{
    uint32_t i, size, left;
    uint32_t p;
    uint32_t n_value;

    if(s == NULL)
        return;

    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);
    for(i = 0; i < s->size; i += sizeof(uint32_t)){

        memset(&p, '\0', sizeof(uint32_t));
        left = s->size - i;
        size = left < sizeof(uint32_t) ?
        left : sizeof(uint32_t);
        memcpy(&p, s->contents + i, size);

        if(i + sizeof(uint32_t) > s->size)
            printf("%s list pointer extends past end of (%.16s,%.16s) "
                   "section\n", listname, s->segname, s->sectname);
        printf("%08x ", s->addr + i);

        if(info->swapped)
            p = SWAP_INT(p);

        print_field_value(i, p, FALSE, NULL, "\n", info, s, &n_value);

        if(func != NULL)
            func(n_value, info);
    }
}

static
void
print_class_t(
uint32_t p,
struct info *info)
{
    struct class_t c;
    void *r;
    uint32_t offset, left;
    struct section_info_32 *s;
    enum bool is_meta_class;
    uint32_t n_value, isa_n_value;
    static uint32_t recursion_depth;

    is_meta_class = FALSE;
    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
                      FALSE, NULL, "\n", info, s, &isa_n_value);
    
    print_field_label(info, "superclass");
    print_field_value(offset + offsetof(struct class_t, superclass),
                      c.superclass, FALSE, NULL, "\n", info, s,
                      &n_value);
    
    print_field_label(info, "cache");
    print_field_value(offset + offsetof(struct class_t, cache),
                      c.cache, FALSE, NULL, "\n", info, s, &n_value);
    
    print_field_label(info, "vtable");
    print_field_value(offset + offsetof(struct class_t, vtable),
                      c.vtable, FALSE, NULL, "\n", info, s, &n_value);
    
    print_field_label(info, "data");
    print_field_value(offset + offsetof(struct class_t, data), c.data, FALSE,
                      "(struct class_ro_t *)", NULL, info, s, &n_value);
    /*
     * This is a Swift class if some of the low bits of the pointer
     * are set. Note that this value is 7 in 64-bit.
     *
     *   bit 0: is Swift
     *   bit 1: is Swift-stable API
     *   bit 2: has custom retain/release (runtime only)
     */
    if(n_value & 0x3)
        printf(" Swift class");
    printf("\n");

    /* Descend into the read only data */
    print_class_ro_t(n_value & ~0x3, info, &is_meta_class);

    indent_pop(info);

    /* Walk the class hierarchy, but be wary of cycles or bad chains */
    if (!is_meta_class &&
        isa_n_value != p &&
        isa_n_value != 0 &&
        recursion_depth < 100)
    {
        recursion_depth++;
        printf("Meta Class\n");
        print_class_t(isa_n_value, info);
        recursion_depth--;
    }
}

static
void
print_class_ro_t(
uint32_t p,
struct info *info,
enum bool *is_meta_class)
{
    struct class_ro_t cro;
    void *r;
    uint32_t offset, left;
    struct section_info_32 *s;
    const char *name;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections,
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

    print_field_label(info, "ivarLayout");
    print_field_value(offset + offsetof(struct class_ro_t, ivarLayout),
                      cro.ivarLayout, FALSE, NULL, "\n", info, s,
                      &n_value);
    print_layout_map(n_value, info);

    print_field_label(info, "name");
    print_field_value(offset + offsetof(struct class_ro_t, name),
                      cro.name, FALSE, NULL, NULL, info, s, &n_value);
    if (info->verbose) {
        name = get_pointer_32(n_value, NULL, &left, NULL,
                              info->sections, info->nsections);
        if (name != NULL)
            printf(" %.*s", (int)left, name);
    }
    printf("\n");

    print_field_label(info, "baseMethods");
    print_field_value(offset + offsetof(struct class_ro_t, baseMethods),
                      cro.baseMethods, FALSE, "(struct method_list_t *)", "\n",
                      info, s, &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }

    print_field_label(info, "baseProtocols");
    print_field_value(offset + offsetof(struct class_ro_t, baseProtocols),
                      cro.baseProtocols, FALSE, "(struct protocol_list_t *)",
                      "\n", info, s, &n_value);
    if(n_value != 0) {
        print_protocol_list_t(n_value, info);
    }

    print_field_label(info, "ivars");
    print_field_value(offset + offsetof(struct class_ro_t, ivars),
                      cro.ivars, FALSE, "(struct ivar_list_t *)", "\n", info, s,
                      &n_value);
    if(n_value != 0) {
        print_ivar_list_t(n_value, info);
    }

    print_field_label(info, "weakIvarLayout");
    print_field_value(offset + offsetof(struct class_ro_t, weakIvarLayout),
                      cro.weakIvarLayout, FALSE, NULL, "\n", info, s,
                      &n_value);
    print_layout_map(n_value, info);
    
    print_field_label(info, "baseProperties");
    print_field_value(offset + offsetof(struct class_ro_t, baseProperties),
                      cro.baseProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value);
    if(n_value != 0) {
        print_objc_property_list(n_value, info);
    }

    if (is_meta_class)
        *is_meta_class = (cro.flags & RO_META) ? TRUE : FALSE;

    indent_pop(info);
}

static
void
print_layout_map(
uint32_t p,
struct info *info)
{
    uint32_t offset, left;
    struct section_info_32 *s;
    char *layout_map;
    
    if(p == 0)
        return;
    layout_map = get_pointer_32(p, &offset, &left, &s,
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
uint32_t p,
struct info *info)
{
    struct method_list_t ml;
    struct method_t m;
    void *r;
    uint32_t offset, left, i;
    struct section_info_32 *s;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
        r = get_pointer_32(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            return;

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
                          m.name, TRUE, NULL, "\n", info, s, &n_value);
        
        print_field_label(info, "types");
        print_field_value(offset + offsetof(struct method_t, types),
                          m.types, TRUE, NULL, "\n", info, s, &n_value);
        
        print_field_label(info, "imp");
        print_field_value(offset + offsetof(struct method_t, imp),
                          m.imp, FALSE, NULL, "\n", info, s, &n_value);

        p += sizeof(struct method_t);
        offset += sizeof(struct method_t);
    }

    indent_pop(info);
}

static
void
print_ivar_list_t(
uint32_t p,
struct info *info)
{
    struct ivar_list_t il;
    struct ivar_t i;
    void *r;
    uint32_t offset, left, j;
    struct section_info_32 *s;
    uint32_t *ivar_offset_p, ivar_offset;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
        r = get_pointer_32(p, &offset, &left, &s,
                           info->sections, info->nsections);
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
                          &n_value);
        if (info->verbose) {
            ivar_offset_p = get_pointer_32(n_value, NULL, &left, NULL,
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
                          i.name, TRUE, NULL, "\n", info, s, &n_value);

        print_field_label(info, "type");
        print_field_value(offset + offsetof(struct ivar_t, type),
                          i.type, TRUE, NULL, "\n", info, s, &n_value);
        
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
uint32_t p,
struct info *info)
{
    struct protocol_list_t pl;
    uint32_t q;
    void *r;
    uint32_t offset, left, i;
    struct section_info_32 *s;
    uint32_t n_value;
    static uint32_t recursive_depth;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
        r = get_pointer_32(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            break;

        q = 0;
        if(left < sizeof(uint32_t)){
            memcpy(&q, r, left);
            printf("   (protocol_t * entends past the end of the "
                   "section)\n");
        }
        else
            memcpy(&q, r, sizeof(uint32_t));
        if(info->swapped)
            q = SWAP_INT(q);

        print_field_label(info, "list[%u]", i);
        print_field_value(offset, q, FALSE, "(struct protocol_t *)", "\n",
                          info, s, &n_value);
        
        if (n_value &&
            recursive_depth < 100)
        {
            recursive_depth += 1;
            print_protocol_t(n_value, info);
            recursive_depth -= 1;
        }

        p += sizeof(uint32_t);
        offset += sizeof(uint32_t);
    }

    indent_pop(info);
}

static
void
print_objc_property_list(
uint32_t p,
struct info *info)
{
    struct objc_property_list opl;
    struct objc_property op;
    void *r;
    uint32_t offset, left, j;
    struct section_info_32 *s;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
        r = get_pointer_32(p, &offset, &left, &s,
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
                          &n_value);
        
        print_field_label(info, "attributes");
        print_field_value(offset + offsetof(struct objc_property, attributes),
                          op.attributes, TRUE, NULL, "\n", info, s,
                          &n_value);

        p += sizeof(struct objc_property);
        offset += sizeof(struct objc_property);
    }

    indent_pop(info);
}

static
void
print_category_t(
uint32_t p,
struct info *info)
{
    struct category_t c;
    void *r;
    uint32_t offset, left;
    struct section_info_32 *s;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
                      c.name, TRUE, NULL, "\n", info, s, &n_value);
    
    print_field_label(info, "cls");
    print_field_value(offset + offsetof(struct category_t, cls),
                      c.cls, FALSE, "(struct class_t *)", "\n", info, s,
                      &n_value);
    if(n_value != 0) {
        print_class_t(n_value, info);
    }
    
    print_field_label(info, "instanceMethods");
    print_field_value(offset + offsetof(struct category_t, instanceMethods),
                      c.instanceMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "classMethods");
    print_field_value(offset + offsetof(struct category_t, classMethods),
                      c.classMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "protocols");
    print_field_value(offset + offsetof(struct category_t, protocols),
                      c.protocols, FALSE, "(struct protocol_list_t *)", "\n",
                      info, s, &n_value);
    if(n_value != 0) {
        print_protocol_list_t(n_value, info);
    }
    
    print_field_label(info, "instanceProperties");
    print_field_value(offset + offsetof(struct category_t, instanceProperties),
                      c.instanceProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value);
    if(n_value) {
        print_objc_property_list(n_value, info);
    }

    indent_pop(info);
}

static void
print_protocol_t(uint32_t p,
                 struct info *info)
{
    struct protocol_t pt;
    void *r;
    uint32_t offset, left;
    struct section_info_32 *s;
    uint32_t n_value;

    r = get_pointer_32(p, &offset, &left, &s, info->sections, info->nsections);
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
                      pt.isa, TRUE, NULL, "\n", info, s, &n_value);
    
    print_field_label(info, "name");
    print_field_value(offset + offsetof(struct protocol_t, name),
                      pt.name, TRUE, NULL, "\n", info, s, &n_value);
    
    print_field_label(info, "protocols");
    print_field_value(offset + offsetof(struct protocol_t, protocols),
                      pt.protocols, FALSE, "(struct protocol_list_t *)", "\n",
                      info, s, &n_value);
    if(n_value != 0) {
        print_protocol_list_t(n_value, info);
    }
    
    print_field_label(info, "instanceMethods");
    print_field_value(offset + offsetof(struct protocol_t, instanceMethods),
                      pt.instanceMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "classMethods");
    print_field_value(offset + offsetof(struct protocol_t, classMethods),
                      pt.classMethods, FALSE, "(struct method_list_t *)",
                      "\n", info, s, &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "optionalInstanceMethods");
    print_field_value(offset + offsetof(struct protocol_t,
                                        optionalInstanceMethods),
                      pt.optionalInstanceMethods, FALSE,
                      "(struct method_list_t *)", "\n", info, s,
                      &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "optionalClassMethods");
    print_field_value(offset + offsetof(struct protocol_t,
                                        optionalClassMethods),
                      pt.optionalClassMethods, FALSE,
                      "(struct method_list_t *)", "\n", info, s,
                      &n_value);
    if(n_value != 0) {
        print_method_list_t(n_value, info);
    }
    
    print_field_label(info, "instanceProperties");
    print_field_value(offset + offsetof(struct protocol_t,
                                        instanceProperties),
                      pt.instanceProperties, FALSE,
                      "(struct objc_property_list *)", "\n", info, s,
                      &n_value);
    
    if(n_value) {
        print_objc_property_list(n_value, info);
    }

    indent_pop(info);
}

static
void
print_message_refs(
struct section_info_32 *s,
struct info *info)
{
    uint32_t i, left, offset;
    uint32_t p;
    struct message_ref mr;
    void *r;
    uint32_t n_value;
    
    if(s == NULL)
        return;

    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);

    indent_push(info, sizeof("imp") - 1);

    offset = 0;
    for(i = 0; i < s->size; i += sizeof(struct message_ref)){
        p = s->addr + i;
        r = get_pointer_32(p, &offset, &left, &s,
                           info->sections, info->nsections);
        if(r == NULL)
            return;
        
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
                          mr.imp, FALSE, NULL, "\n", info, s, &n_value);

        print_field_label(info, "sel");
        print_field_value(offset + offsetof(struct message_ref, sel),
                          mr.sel, FALSE, NULL, "\n", info, s, &n_value);

        offset += sizeof(struct message_ref);
    }

    indent_pop(info);
}

static
void
print_image_info(
struct section_info_32 *s,
struct info *info)
{
    uint32_t left, offset, swift_version;
    uint32_t p;
    struct objc_image_info o;
    void *r;
    
    if(s == NULL)
        return;

    info->indent_level = 0;
    info->indent_widths[info->indent_level] = 0;

    printf("Contents of (%.16s,%.16s) section\n", s->segname, s->sectname);
    p = s->addr;
    r = get_pointer_32(p, &offset, &left, &s,
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

static
void
get_sections_32(
struct load_command *load_commands,
uint32_t ncmds,
uint32_t sizeofcmds,
enum byte_sex object_byte_sex,
char *object_addr,
uint64_t object_size,
struct section_info_32 **sections,
uint32_t *nsections,
uint32_t *database) 
{
    enum byte_sex host_byte_sex;
    enum bool swapped, database_set, zerobased, encrypt_found, encrypt64_found;
    
    uint32_t i, j, left, size;
    struct load_command lcmd, *lc;
    char *p;
    struct segment_command sg;
    struct section s;
    struct encryption_info_command encrypt;
    struct encryption_info_command_64 encrypt64;
    
    host_byte_sex = get_host_byte_sex();
    swapped = host_byte_sex != object_byte_sex;
    
    *sections = NULL;
    *nsections = 0;
    database_set = FALSE;
    *database = 0;
    zerobased = FALSE;
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
        left = (uint32_t)(sizeofcmds - ((char *)lc - (char *)load_commands));
        
        switch(lcmd.cmd){
            case LC_SEGMENT:
                memset((char *)&sg, '\0', sizeof(struct segment_command));
                size = left < sizeof(struct segment_command) ?
                left : sizeof(struct segment_command);
                memcpy((char *)&sg, (char *)lc, size);
                if(swapped)
                    swap_segment_command(&sg, host_byte_sex);
                if((sg.initprot & VM_PROT_WRITE) == VM_PROT_WRITE &&
                   database_set == FALSE){
                    *database = sg.vmaddr;
                    database_set = TRUE;
                }
                if((sg.initprot & VM_PROT_READ) == VM_PROT_READ &&
                   sg.vmaddr == 0)
                    zerobased = TRUE;
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
                    
                    *sections = reallocate(*sections,
                        sizeof(struct section_info_32) * (*nsections + 1));
                    memcpy((*sections)[*nsections].segname,
                           s.segname, 16);
                    memcpy((*sections)[*nsections].sectname,
                           s.sectname, 16);
                    (*sections)[*nsections].addr = s.addr;
                    (*sections)[*nsections].contents = object_addr + s.offset;
                    (*sections)[*nsections].offset = s.offset;
                    (*sections)[*nsections].zerofill = (s.flags & SECTION_TYPE)
                    == S_ZEROFILL ? TRUE : FALSE;
                    if(s.offset > object_size){
                        printf("section contents of: (%.16s,%.16s) is past "
                               "end of file\n", s.segname, s.sectname);
                        (*sections)[*nsections].size =  0;
                    }
                    else if(s.offset + s.size > object_size){
                        printf("part of section contents of: (%.16s,%.16s) "
                               "is past end of file\n",
                               s.segname, s.sectname);
                        (*sections)[*nsections].size = (uint32_t)(object_size
								  - s.offset);
                    }
                    else
                        (*sections)[*nsections].size = s.size;
                    if(s.reloff >= object_size){
                        printf("relocation entries offset for (%.16s,%.16s)"
                               ": is past end of file\n", s.segname,
                               s.sectname);
                        (*sections)[*nsections].nrelocs = 0;
                    }
                    else{
                        (*sections)[*nsections].relocs =
                        (struct relocation_info *)(object_addr +
                                                   s.reloff);
                        if(s.reloff +
                           s.nreloc * sizeof(struct relocation_info) >
                           object_size){
                            printf("relocation entries for section (%.16s,"
                                   "%.16s) extends past end of file\n",
                                   s.segname, s.sectname);
                            (*sections)[*nsections].nrelocs =
                            (uint32_t)((object_size - s.reloff) /
                            sizeof(struct relocation_info));
                        }
                        else
                            (*sections)[*nsections].nrelocs = s.nreloc;
                        if(swapped)
                            swap_relocation_info(
                                (*sections)[*nsections].relocs,
                                (*sections)[*nsections].nrelocs,
                                host_byte_sex);
                    }
                    if(sg.flags & SG_PROTECTED_VERSION_1)
                        (*sections)[*nsections].protected = TRUE;
                    else
                        (*sections)[*nsections].protected = FALSE;
                    (*nsections)++;
                    
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
    if(zerobased == TRUE)
        *database = 0;
    
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
struct section_info_32 *
get_section_32(
struct section_info_32 *sections,
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
void *
get_pointer_32(
uint32_t p,
uint32_t *offset,
uint32_t *left,
struct section_info_32 **s,
struct section_info_32 *sections,
uint32_t nsections)
{
    void *r;
    uint32_t addr;
    uint32_t i;
    
    addr = p;
    for(i = 0; i < nsections; i++){
        if(addr >= sections[i].addr &&
           addr < sections[i].addr + sections[i].size){
            if(s != NULL)
                *s = sections + i;
            if(offset != NULL)
                *offset = addr - sections[i].addr;
            if(left != NULL)
                *left = sections[i].size - (addr - sections[i].addr);
            if(sections[i].protected == TRUE)
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
 * get_symbol() returns the name of a symbol (or NULL). Based on the relocation
 * information at the specified section offset or the value.
 */
static
const char *
get_symbol_32(
uint32_t sect_offset,
uint32_t sect_addr,
uint32_t database_offset,
uint64_t value,
struct relocation_info *relocs,
uint32_t nrelocs,
struct info *info,
uint32_t* n_value)
{
    uint32_t i;
    unsigned int r_symbolnum;
    uint32_t n_strx;
    const char* name;
    
    if(n_value != NULL)
        *n_value = (uint32_t)0;
    if(info->verbose == FALSE)
        return(NULL);
    
    for(i = 0; i < nrelocs; i++){
        if((uint32_t)relocs[i].r_address == sect_offset){
            r_symbolnum = relocs[i].r_symbolnum;
            if(relocs[i].r_extern){
                if(r_symbolnum >= info->nsymbols)
                    break;
                n_strx = info->symbols[r_symbolnum].n_un.n_strx;
                if(n_strx <= 0 || n_strx >= info->strings_size)
                    break;
                return(info->strings + n_strx);
            }
            break;
        }
        if(reloc_has_pair(info->cputype, relocs[i].r_type) == TRUE)
            i++;
    }
    for(i = 0; i < info->next_relocs; i++){
        if((uint32_t)info->ext_relocs[i].r_address ==
           database_offset + sect_offset){
            r_symbolnum = info->ext_relocs[i].r_symbolnum;
            if(info->ext_relocs[i].r_extern){
                if(r_symbolnum >= info->nsymbols)
                    break;
                n_strx = info->symbols[r_symbolnum].n_un.n_strx;
                if(n_strx <= 0 || n_strx >= info->strings_size)
                    break;
                return(info->strings + n_strx);
            }
            break;
        }
        if(reloc_has_pair(info->cputype, info->ext_relocs[i].r_type) ==TRUE)
            i++;
    }
    
    /*
     * If this is a chained format, the symbol information may be part of
     * the dyld info. If we find one, return early before hitting the
     * symbol table.
     */
    name = get_dyld_bind_info_symbolname(sect_addr + sect_offset,
                                         info->dbi, info->ndbi,
                                         info->chain_format, NULL);
    if (name)
        return name;
    
    /*
     * If this is a chained rebase, we will need to convert the value on
     * disk into its proper logical value, before guessing at the symbol
     * name.
     */
    value = get_chained_rebase_value(value, info->chain_format, NULL);
    if(n_value != NULL)
        *n_value = (uint32_t)value;

    return(guess_symbol(value, info->sorted_symbols, info->nsorted_symbols,
                        info->verbose));
}

/*
 * print_field_scalar() prints a label followed by a formatted value. the label
 * is idented to fit within the info's indent state.
 */
/* This is identical to the 64-bit implementaton. */
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
/* This is identical to the 64-bit implementaton. */
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
 *   ( pointer | n_value [symbol]) [data] [type] [suffix]
 *
 * pointer - the raw pointer on disk, only displaying when -v is not specified.
 * n_value - the adjusted pointer view, correcting for chained-fixups and
 *           authenticated pointers.
 * symbol  - the symbol name corresponding to pointer
 * data    - C-string data within the file pointed to by the n_value / addend.
 * type    - an optional C-string that is printed only if supplied and if
 *           n_value != 0.
 * suffix  - an optional C-string that prints after the other field elements,
 *           often used to print a newline.
 *
 * print_field_value usually follows a call to print_field_label, but this is
 * not required.
 */
/*
 * The primary difference from the 64-bit version, other than the datatypes,
 * is print_field_value here does not print or return an addend. Also, the
 * arguments to get_symbol_32 do not match those required by get_symbol_64.
 */
static
void
print_field_value(
uint32_t offset,
uint32_t p,
enum bool print_data,
const char* type_name,
const char* suffix,
struct info *info,
struct section_info_32 *s,
uint32_t *out_n_value)
{
    uint32_t n_value;
    const char* sym_name;
    
    /* read the symbol name, n_value, and addend. */
    sym_name = get_symbol_32(offset, s->addr, info->database, p,
                             s->relocs, s->nrelocs, info, &n_value);
    
    /* print the numeric pointer value */
    if (info->verbose) {
        printf("0x%x", n_value);
        if (sym_name)
            printf(" %s", sym_name);
    }
    else {
        printf("0x%x", p);
    }
    
    /* print the pointer data if any, if requested */
    if (info->verbose && print_data) {
        const char* ptr_data;
        ptr_data = get_pointer_32(n_value, NULL, NULL, NULL,
                                  info->sections, info->nsections);
        if (ptr_data)
            printf(" %s", ptr_data);
    }

#if 0
    if (info->verbose && type_name && (n_value > 0)) {
        printf(" %s", type_name);
    }
#endif

    /* print the suffix field */
    if (suffix)
        printf("%s", suffix);
    
    /* return the n_value */
    if (out_n_value)
        *out_n_value = n_value;
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
