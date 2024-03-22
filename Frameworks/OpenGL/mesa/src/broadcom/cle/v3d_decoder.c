/*
 * Copyright © 2016 Intel Corporation
 * Copyright © 2017 Broadcom
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "v3d_decoder.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#ifdef WITH_LIBEXPAT
#include <expat.h>
#endif
#include <inttypes.h>
#include <zlib.h>

#include <util/macros.h>
#include <util/ralloc.h>
#include <util/u_debug.h>

#include "v3d_packet_helpers.h"
#include "v3d_xml.h"
#include "broadcom/clif/clif_private.h"

struct v3d_spec {
        uint32_t ver;

        int ncommands;
        struct v3d_group *commands[256];
        int nstructs;
        struct v3d_group *structs[256];
        int nregisters;
        struct v3d_group *registers[256];
        int nenums;
        struct v3d_enum *enums[256];
};

#ifdef WITH_LIBEXPAT

struct location {
        const char *filename;
        int line_number;
};

struct parser_context {
        XML_Parser parser;
        const struct v3d_device_info *devinfo;
        int foo;
        struct location loc;

        struct v3d_group *group;
        struct v3d_enum *enoom;

        int nvalues;
        struct v3d_value *values[256];

        struct v3d_spec *spec;

        int parse_depth;
        int parse_skip_depth;
};

#endif /* WITH_LIBEXPAT */

const char *
v3d_group_get_name(struct v3d_group *group)
{
        return group->name;
}

uint8_t
v3d_group_get_opcode(struct v3d_group *group)
{
        return group->opcode;
}

struct v3d_group *
v3d_spec_find_struct(struct v3d_spec *spec, const char *name)
{
        for (int i = 0; i < spec->nstructs; i++)
                if (strcmp(spec->structs[i]->name, name) == 0)
                        return spec->structs[i];

        return NULL;
}

struct v3d_group *
v3d_spec_find_register(struct v3d_spec *spec, uint32_t offset)
{
        for (int i = 0; i < spec->nregisters; i++)
                if (spec->registers[i]->register_offset == offset)
                        return spec->registers[i];

        return NULL;
}

struct v3d_group *
v3d_spec_find_register_by_name(struct v3d_spec *spec, const char *name)
{
        for (int i = 0; i < spec->nregisters; i++) {
                if (strcmp(spec->registers[i]->name, name) == 0)
                        return spec->registers[i];
        }

        return NULL;
}

struct v3d_enum *
v3d_spec_find_enum(struct v3d_spec *spec, const char *name)
{
        for (int i = 0; i < spec->nenums; i++)
                if (strcmp(spec->enums[i]->name, name) == 0)
                        return spec->enums[i];

        return NULL;
}

#ifdef WITH_LIBEXPAT

static void __attribute__((noreturn))
fail(struct location *loc, const char *msg, ...)
{
        va_list ap;

        va_start(ap, msg);
        fprintf(stderr, "%s:%d: error: ",
                loc->filename, loc->line_number);
        vfprintf(stderr, msg, ap);
        fprintf(stderr, "\n");
        va_end(ap);
        exit(EXIT_FAILURE);
}

static void *
fail_on_null(void *p)
{
        if (p == NULL) {
                fprintf(stderr, "aubinator: out of memory\n");
                exit(EXIT_FAILURE);
        }

        return p;
}

static char *
xstrdup(const char *s)
{
        return fail_on_null(strdup(s));
}

static void *
zalloc(size_t s)
{
        return calloc(s, 1);
}

static void *
xzalloc(size_t s)
{
        return fail_on_null(zalloc(s));
}

/* We allow fields to have either a bit index, or append "b" for a byte index.
 */
static bool
is_byte_offset(const char *value)
{
        return value[strlen(value) - 1] == 'b';
}

static void
get_group_offset_count(const char **atts, uint32_t *offset, uint32_t *count,
                       uint32_t *size, bool *variable)
{
        char *p;
        int i;

        for (i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "count") == 0) {
                        *count = strtoul(atts[i + 1], &p, 0);
                        if (*count == 0)
                                *variable = true;
                } else if (strcmp(atts[i], "start") == 0) {
                        *offset = strtoul(atts[i + 1], &p, 0);
                } else if (strcmp(atts[i], "size") == 0) {
                        *size = strtoul(atts[i + 1], &p, 0);
                }
        }
        return;
}

static struct v3d_group *
create_group(struct parser_context *ctx,
             const char *name,
             const char **atts,
             struct v3d_group *parent)
{
        struct v3d_group *group;

        group = xzalloc(sizeof(*group));
        if (name)
                group->name = xstrdup(name);

        group->spec = ctx->spec;
        group->group_offset = 0;
        group->group_count = 0;
        group->variable = false;

        if (parent) {
                group->parent = parent;
                get_group_offset_count(atts,
                                       &group->group_offset,
                                       &group->group_count,
                                       &group->group_size,
                                       &group->variable);
        }

        return group;
}

static struct v3d_enum *
create_enum(struct parser_context *ctx, const char *name, const char **atts)
{
        struct v3d_enum *e;

        e = xzalloc(sizeof(*e));
        if (name)
                e->name = xstrdup(name);

        e->nvalues = 0;

        return e;
}

static void
get_register_offset(const char **atts, uint32_t *offset)
{
        char *p;
        int i;

        for (i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "num") == 0)
                        *offset = strtoul(atts[i + 1], &p, 0);
        }
        return;
}

static struct v3d_type
string_to_type(struct parser_context *ctx, const char *s)
{
        int i, f;
        struct v3d_group *g;
        struct v3d_enum *e;

        if (strcmp(s, "int") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_INT };
        else if (strcmp(s, "uint") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_UINT };
        else if (strcmp(s, "bool") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_BOOL };
        else if (strcmp(s, "float") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_FLOAT };
        else if (strcmp(s, "f187") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_F187 };
        else if (strcmp(s, "address") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_ADDRESS };
        else if (strcmp(s, "offset") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_OFFSET };
        else if (sscanf(s, "u%d.%d", &i, &f) == 2)
                return (struct v3d_type) { .kind = V3D_TYPE_UFIXED, .i = i, .f = f };
        else if (sscanf(s, "s%d.%d", &i, &f) == 2)
                return (struct v3d_type) { .kind = V3D_TYPE_SFIXED, .i = i, .f = f };
        else if (g = v3d_spec_find_struct(ctx->spec, s), g != NULL)
                return (struct v3d_type) { .kind = V3D_TYPE_STRUCT, .v3d_struct = g };
        else if (e = v3d_spec_find_enum(ctx->spec, s), e != NULL)
                return (struct v3d_type) { .kind = V3D_TYPE_ENUM, .v3d_enum = e };
        else if (strcmp(s, "mbo") == 0)
                return (struct v3d_type) { .kind = V3D_TYPE_MBO };
        else
                fail(&ctx->loc, "invalid type: %s", s);
}

static struct v3d_field *
create_field(struct parser_context *ctx, const char **atts)
{
        struct v3d_field *field;
        char *p;
        int i;
        uint32_t size = 0;

        field = xzalloc(sizeof(*field));

        for (i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "name") == 0)
                        field->name = xstrdup(atts[i + 1]);
                else if (strcmp(atts[i], "start") == 0) {
                        field->start = strtoul(atts[i + 1], &p, 0);
                        if (is_byte_offset(atts[i + 1]))
                                field->start *= 8;
                } else if (strcmp(atts[i], "end") == 0) {
                        field->end = strtoul(atts[i + 1], &p, 0) - 1;
                        if (is_byte_offset(atts[i + 1]))
                                field->end *= 8;
                } else if (strcmp(atts[i], "size") == 0) {
                        size = strtoul(atts[i + 1], &p, 0);
                        if (is_byte_offset(atts[i + 1]))
                                size *= 8;
                } else if (strcmp(atts[i], "type") == 0)
                        field->type = string_to_type(ctx, atts[i + 1]);
                else if (strcmp(atts[i], "default") == 0) {
                        field->has_default = true;
                        field->default_value = strtoul(atts[i + 1], &p, 0);
                } else if (strcmp(atts[i], "minus_one") == 0) {
                        assert(strcmp(atts[i + 1], "true") == 0);
                        field->minus_one = true;
                }
        }

        if (size)
                field->end = field->start + size - 1;

        return field;
}

static struct v3d_value *
create_value(struct parser_context *ctx, const char **atts)
{
        struct v3d_value *value = xzalloc(sizeof(*value));

        for (int i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "name") == 0)
                        value->name = xstrdup(atts[i + 1]);
                else if (strcmp(atts[i], "value") == 0)
                        value->value = strtoul(atts[i + 1], NULL, 0);
        }

        return value;
}

static void
create_and_append_field(struct parser_context *ctx,
                        const char **atts)
{
        if (ctx->group->nfields == ctx->group->fields_size) {
                ctx->group->fields_size = MAX2(ctx->group->fields_size * 2, 2);
                ctx->group->fields =
                        (struct v3d_field **) realloc(ctx->group->fields,
                                                      sizeof(ctx->group->fields[0]) *
                                                      ctx->group->fields_size);
        }

        ctx->group->fields[ctx->group->nfields++] = create_field(ctx, atts);
}

static void
set_group_opcode(struct v3d_group *group, const char **atts)
{
        char *p;
        int i;

        for (i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "code") == 0)
                        group->opcode = strtoul(atts[i + 1], &p, 0);
        }
        return;
}

static bool
ver_in_range(int ver, int min_ver, int max_ver)
{
        return ((min_ver == 0 || ver >= min_ver) &&
                (max_ver == 0 || ver <= max_ver));
}

static bool
skip_if_ver_mismatch(struct parser_context *ctx, int min_ver, int max_ver)
{
        if (!ctx->parse_skip_depth && !ver_in_range(ctx->devinfo->ver,
                                                    min_ver, max_ver)) {
                assert(ctx->parse_depth != 0);
                ctx->parse_skip_depth = ctx->parse_depth;
        }

        return ctx->parse_skip_depth;
}

static void
start_element(void *data, const char *element_name, const char **atts)
{
        struct parser_context *ctx = data;
        int i;
        const char *name = NULL;
        const char *ver = NULL;
        int min_ver = 0;
        int max_ver = 0;

        ctx->loc.line_number = XML_GetCurrentLineNumber(ctx->parser);

        for (i = 0; atts[i]; i += 2) {
                if (strcmp(atts[i], "shortname") == 0)
                        name = atts[i + 1];
                else if (strcmp(atts[i], "name") == 0 && !name)
                        name = atts[i + 1];
                else if (strcmp(atts[i], "gen") == 0)
                        ver = atts[i + 1];
                else if (strcmp(atts[i], "min_ver") == 0)
                        min_ver = strtoul(atts[i + 1], NULL, 0);
                else if (strcmp(atts[i], "max_ver") == 0)
                        max_ver = strtoul(atts[i + 1], NULL, 0);
        }

        if (skip_if_ver_mismatch(ctx, min_ver, max_ver))
                goto skip;

        if (strcmp(element_name, "vcxml") == 0) {
                if (ver == NULL)
                        fail(&ctx->loc, "no ver given");

                /* Make sure that we picked an XML that matched our version.
                 */
                assert(ver_in_range(ctx->devinfo->ver, min_ver, max_ver));

                int major, minor;
                int n = sscanf(ver, "%d.%d", &major, &minor);
                if (n == 0)
                        fail(&ctx->loc, "invalid ver given: %s", ver);
                if (n == 1)
                        minor = 0;

                ctx->spec->ver = major * 10 + minor;
        } else if (strcmp(element_name, "packet") == 0 ||
                   strcmp(element_name, "struct") == 0) {
                ctx->group = create_group(ctx, name, atts, NULL);

                if (strcmp(element_name, "packet") == 0)
                        set_group_opcode(ctx->group, atts);
        } else if (strcmp(element_name, "register") == 0) {
                ctx->group = create_group(ctx, name, atts, NULL);
                get_register_offset(atts, &ctx->group->register_offset);
        } else if (strcmp(element_name, "group") == 0) {
                struct v3d_group *previous_group = ctx->group;
                while (previous_group->next)
                        previous_group = previous_group->next;

                struct v3d_group *group = create_group(ctx, "", atts,
                                                       ctx->group);
                previous_group->next = group;
                ctx->group = group;
        } else if (strcmp(element_name, "field") == 0) {
                create_and_append_field(ctx, atts);
        } else if (strcmp(element_name, "enum") == 0) {
                ctx->enoom = create_enum(ctx, name, atts);
        } else if (strcmp(element_name, "value") == 0) {
                ctx->values[ctx->nvalues++] = create_value(ctx, atts);
                assert(ctx->nvalues < ARRAY_SIZE(ctx->values));
        }

skip:
        ctx->parse_depth++;
}

static int
field_offset_compare(const void *a, const void *b)
{
        return ((*(const struct v3d_field **)a)->start -
                (*(const struct v3d_field **)b)->start);
}

static void
end_element(void *data, const char *name)
{
        struct parser_context *ctx = data;
        struct v3d_spec *spec = ctx->spec;

        ctx->parse_depth--;

        if (ctx->parse_skip_depth) {
                if (ctx->parse_skip_depth == ctx->parse_depth)
                        ctx->parse_skip_depth = 0;
                return;
        }

        if (strcmp(name, "packet") == 0 ||
            strcmp(name, "struct") == 0 ||
            strcmp(name, "register") == 0) {
                struct v3d_group *group = ctx->group;

                ctx->group = ctx->group->parent;

                if (strcmp(name, "packet") == 0) {
                        spec->commands[spec->ncommands++] = group;

                        /* V3D packet XML has the packet contents with offsets
                         * starting from the first bit after the opcode, to
                         * match the spec.  Shift the fields up now.
                         */
                        for (int i = 0; i < group->nfields; i++) {
                                group->fields[i]->start += 8;
                                group->fields[i]->end += 8;
                        }
                }
                else if (strcmp(name, "struct") == 0)
                        spec->structs[spec->nstructs++] = group;
                else if (strcmp(name, "register") == 0)
                        spec->registers[spec->nregisters++] = group;

                /* Sort the fields in increasing offset order.  The XML might
                 * be specified in any order, but we'll want to iterate from
                 * the bottom.
                 */
                qsort(group->fields, group->nfields, sizeof(*group->fields),
                      field_offset_compare);

                assert(spec->ncommands < ARRAY_SIZE(spec->commands));
                assert(spec->nstructs < ARRAY_SIZE(spec->structs));
                assert(spec->nregisters < ARRAY_SIZE(spec->registers));
        } else if (strcmp(name, "group") == 0) {
                ctx->group = ctx->group->parent;
        } else if (strcmp(name, "field") == 0) {
                assert(ctx->group->nfields > 0);
                struct v3d_field *field = ctx->group->fields[ctx->group->nfields - 1];
                size_t size = ctx->nvalues * sizeof(ctx->values[0]);
                field->inline_enum.values = xzalloc(size);
                field->inline_enum.nvalues = ctx->nvalues;
                memcpy(field->inline_enum.values, ctx->values, size);
                ctx->nvalues = 0;
        } else if (strcmp(name, "enum") == 0) {
                struct v3d_enum *e = ctx->enoom;
                size_t size = ctx->nvalues * sizeof(ctx->values[0]);
                e->values = xzalloc(size);
                e->nvalues = ctx->nvalues;
                memcpy(e->values, ctx->values, size);
                ctx->nvalues = 0;
                ctx->enoom = NULL;
                spec->enums[spec->nenums++] = e;
        }
}

static void
character_data(void *data, const XML_Char *s, int len)
{
}

static uint32_t zlib_inflate(const void *compressed_data,
                             uint32_t compressed_len,
                             void **out_ptr)
{
        struct z_stream_s zstream;
        void *out;

        memset(&zstream, 0, sizeof(zstream));

        zstream.next_in = (unsigned char *)compressed_data;
        zstream.avail_in = compressed_len;

        if (inflateInit(&zstream) != Z_OK)
                return 0;

        out = malloc(4096);
        zstream.next_out = out;
        zstream.avail_out = 4096;

        do {
                switch (inflate(&zstream, Z_SYNC_FLUSH)) {
                case Z_STREAM_END:
                        goto end;
                case Z_OK:
                        break;
                default:
                        inflateEnd(&zstream);
                        return 0;
                }

                if (zstream.avail_out)
                        break;

                out = realloc(out, 2*zstream.total_out);
                if (out == NULL) {
                        inflateEnd(&zstream);
                        return 0;
                }

                zstream.next_out = (unsigned char *)out + zstream.total_out;
                zstream.avail_out = zstream.total_out;
        } while (1);
 end:
        inflateEnd(&zstream);
        *out_ptr = out;
        return zstream.total_out;
}

#endif /* WITH_LIBEXPAT */

struct v3d_spec *
v3d_spec_load(const struct v3d_device_info *devinfo)
{
        struct v3d_spec *spec = calloc(1, sizeof(struct v3d_spec));
        if (!spec)
                return NULL;

#ifdef WITH_LIBEXPAT
        struct parser_context ctx;
        void *buf;
        uint8_t *text_data = NULL;
        uint32_t text_offset = 0, text_length = 0;
        ASSERTED uint32_t total_length;

        for (int i = 0; i < ARRAY_SIZE(genxml_files_table); i++) {
                if (i != 0) {
                        assert(genxml_files_table[i - 1].ver_10 <
                               genxml_files_table[i].ver_10);
                }

                if (genxml_files_table[i].ver_10 <= devinfo->ver) {
                        text_offset = genxml_files_table[i].offset;
                        text_length = genxml_files_table[i].length;
                }
        }

        if (text_length == 0) {
                fprintf(stderr, "unable to find gen (%u) data\n", devinfo->ver);
                free(spec);
                return NULL;
        }

        memset(&ctx, 0, sizeof ctx);
        ctx.parser = XML_ParserCreate(NULL);
        ctx.devinfo = devinfo;
        XML_SetUserData(ctx.parser, &ctx);
        if (ctx.parser == NULL) {
                fprintf(stderr, "failed to create parser\n");
                free(spec);
                return NULL;
        }

        XML_SetElementHandler(ctx.parser, start_element, end_element);
        XML_SetCharacterDataHandler(ctx.parser, character_data);

        ctx.spec = spec;

        total_length = zlib_inflate(compress_genxmls,
                                    sizeof(compress_genxmls),
                                    (void **) &text_data);
        assert(text_offset + text_length <= total_length);

        buf = XML_GetBuffer(ctx.parser, text_length);
        memcpy(buf, &text_data[text_offset], text_length);

        if (XML_ParseBuffer(ctx.parser, text_length, true) == 0) {
                fprintf(stderr,
                        "Error parsing XML at line %ld col %ld byte %ld/%u: %s\n",
                        XML_GetCurrentLineNumber(ctx.parser),
                        XML_GetCurrentColumnNumber(ctx.parser),
                        XML_GetCurrentByteIndex(ctx.parser), text_length,
                        XML_ErrorString(XML_GetErrorCode(ctx.parser)));
                XML_ParserFree(ctx.parser);
                free(text_data);
                free(spec);
                return NULL;
        }

        XML_ParserFree(ctx.parser);
        free(text_data);

        return ctx.spec;
#else /* !WITH_LIBEXPAT */
        debug_warn_once("CLIF dumping not supported due to missing libexpat");
        return spec;
#endif /* !WITH_LIBEXPAT */
}

struct v3d_group *
v3d_spec_find_instruction(struct v3d_spec *spec, const uint8_t *p)
{
        uint8_t opcode = *p;

        for (int i = 0; i < spec->ncommands; i++) {
                struct v3d_group *group = spec->commands[i];

                if (opcode != group->opcode)
                        continue;

                /* If there's a "sub-id" field, make sure that it matches the
                 * instruction being decoded.
                 */
                struct v3d_field *subid = NULL;
                for (int j = 0; j < group->nfields; j++) {
                        struct v3d_field *field = group->fields[j];
                        if (strcmp(field->name, "sub-id") == 0) {
                                subid = field;
                                break;
                        }
                }

                if (subid && (__gen_unpack_uint(p, subid->start, subid->end) !=
                              subid->default_value)) {
                        continue;
                }

                return group;
        }

        return NULL;
}

/** Returns the size of a V3D packet. */
int
v3d_group_get_length(struct v3d_group *group)
{
        int last_bit = 0;
        for (int i = 0; i < group->nfields; i++) {
                struct v3d_field *field = group->fields[i];

                last_bit = MAX2(last_bit, field->end);
        }
        return last_bit / 8 + 1;
}

void
v3d_field_iterator_init(struct v3d_field_iterator *iter,
                        struct v3d_group *group,
                        const uint8_t *p)
{
        memset(iter, 0, sizeof(*iter));

        iter->group = group;
        iter->p = p;
}

static const char *
v3d_get_enum_name(struct v3d_enum *e, uint64_t value)
{
        for (int i = 0; i < e->nvalues; i++) {
                if (e->values[i]->value == value) {
                        return e->values[i]->name;
                }
        }
        return NULL;
}

static bool
iter_more_fields(const struct v3d_field_iterator *iter)
{
        return iter->field_iter < iter->group->nfields;
}

static uint32_t
iter_group_offset_bits(const struct v3d_field_iterator *iter,
                       uint32_t group_iter)
{
        return iter->group->group_offset + (group_iter *
                                            iter->group->group_size);
}

static bool
iter_more_groups(const struct v3d_field_iterator *iter)
{
        if (iter->group->variable) {
                return iter_group_offset_bits(iter, iter->group_iter + 1) <
                        (v3d_group_get_length(iter->group) * 8);
        } else {
                return (iter->group_iter + 1) < iter->group->group_count ||
                        iter->group->next != NULL;
        }
}

static void
iter_advance_group(struct v3d_field_iterator *iter)
{
        if (iter->group->variable)
                iter->group_iter++;
        else {
                if ((iter->group_iter + 1) < iter->group->group_count) {
                        iter->group_iter++;
                } else {
                        iter->group = iter->group->next;
                        iter->group_iter = 0;
                }
        }

        iter->field_iter = 0;
}

static bool
iter_advance_field(struct v3d_field_iterator *iter)
{
        while (!iter_more_fields(iter)) {
                if (!iter_more_groups(iter))
                        return false;

                iter_advance_group(iter);
        }

        iter->field = iter->group->fields[iter->field_iter++];
        if (iter->field->name)
                snprintf(iter->name, sizeof(iter->name), "%s", iter->field->name);
        else
                memset(iter->name, 0, sizeof(iter->name));
        iter->offset = iter_group_offset_bits(iter, iter->group_iter) / 8 +
                iter->field->start / 8;
        iter->struct_desc = NULL;

        return true;
}

bool
v3d_field_iterator_next(struct clif_dump *clif, struct v3d_field_iterator *iter)
{
        if (!iter_advance_field(iter))
                return false;

        const char *enum_name = NULL;

        int group_member_offset =
                iter_group_offset_bits(iter, iter->group_iter);
        int s = group_member_offset + iter->field->start;
        int e = group_member_offset + iter->field->end;

        assert(!iter->field->minus_one ||
               iter->field->type.kind == V3D_TYPE_INT ||
               iter->field->type.kind == V3D_TYPE_UINT);

        switch (iter->field->type.kind) {
        case V3D_TYPE_UNKNOWN:
        case V3D_TYPE_INT: {
                uint32_t value = __gen_unpack_sint(iter->p, s, e);
                if (iter->field->minus_one)
                        value++;
                snprintf(iter->value, sizeof(iter->value), "%d", value);
                enum_name = v3d_get_enum_name(&iter->field->inline_enum, value);
                break;
        }
        case V3D_TYPE_UINT: {
                uint32_t value = __gen_unpack_uint(iter->p, s, e);
                if (iter->field->minus_one)
                        value++;
                if (strcmp(iter->field->name, "Vec size") == 0 && value == 0)
                        value = 1 << (e - s + 1);
                snprintf(iter->value, sizeof(iter->value), "%u", value);
                enum_name = v3d_get_enum_name(&iter->field->inline_enum, value);
                break;
        }
        case V3D_TYPE_BOOL:
                snprintf(iter->value, sizeof(iter->value), "%s",
                         __gen_unpack_uint(iter->p, s, e) ?
                         "1 /* true */" : "0 /* false */");
                break;
        case V3D_TYPE_FLOAT:
                snprintf(iter->value, sizeof(iter->value), "%f",
                         __gen_unpack_float(iter->p, s, e));
                break;

        case V3D_TYPE_F187:
                snprintf(iter->value, sizeof(iter->value), "%f",
                         __gen_unpack_f187(iter->p, s, e));
                break;

        case V3D_TYPE_ADDRESS: {
                uint32_t addr =
                        __gen_unpack_uint(iter->p, s, e) << (31 - (e - s));
                struct clif_bo *bo = clif_lookup_bo(clif, addr);
                if (bo) {
                        snprintf(iter->value, sizeof(iter->value),
                                 "[%s+0x%08x] /* 0x%08x */",
                                 bo->name, addr - bo->offset, addr);
                } else if (addr) {
                        snprintf(iter->value, sizeof(iter->value),
                                 "/* XXX: BO unknown */ 0x%08x", addr);
                } else {
                        snprintf(iter->value, sizeof(iter->value),
                                 "[null]");
                }

                break;
        }

        case V3D_TYPE_OFFSET:
                snprintf(iter->value, sizeof(iter->value), "0x%08"PRIx64,
                         __gen_unpack_uint(iter->p, s, e) << (31 - (e - s)));
                break;
        case V3D_TYPE_STRUCT:
                snprintf(iter->value, sizeof(iter->value), "<struct %s>",
                         iter->field->type.v3d_struct->name);
                iter->struct_desc =
                        v3d_spec_find_struct(iter->group->spec,
                                             iter->field->type.v3d_struct->name);
                break;
        case V3D_TYPE_SFIXED:
                if (clif->pretty) {
                        snprintf(iter->value, sizeof(iter->value), "%f",
                                 __gen_unpack_sfixed(iter->p, s, e,
                                                     iter->field->type.f));
                } else {
                        snprintf(iter->value, sizeof(iter->value), "%u",
                                 (unsigned)__gen_unpack_uint(iter->p, s, e));
                }
                break;
        case V3D_TYPE_UFIXED:
                if (clif->pretty) {
                        snprintf(iter->value, sizeof(iter->value), "%f",
                                 __gen_unpack_ufixed(iter->p, s, e,
                                                     iter->field->type.f));
                } else {
                        snprintf(iter->value, sizeof(iter->value), "%u",
                                 (unsigned)__gen_unpack_uint(iter->p, s, e));
                }
                break;
        case V3D_TYPE_MBO:
                break;
        case V3D_TYPE_ENUM: {
                uint32_t value = __gen_unpack_uint(iter->p, s, e);
                snprintf(iter->value, sizeof(iter->value), "%d", value);
                enum_name = v3d_get_enum_name(iter->field->type.v3d_enum, value);
                break;
        }
        }

        if (strlen(iter->group->name) == 0) {
                int length = strlen(iter->name);
                snprintf(iter->name + length, sizeof(iter->name) - length,
                         "[%i]", iter->group_iter);
        }

        if (enum_name) {
                int length = strlen(iter->value);
                snprintf(iter->value + length, sizeof(iter->value) - length,
                         " /* %s */", enum_name);
        }

        return true;
}

void
v3d_print_group(struct clif_dump *clif, struct v3d_group *group,
                uint64_t offset, const uint8_t *p)
{
        struct v3d_field_iterator iter;

        v3d_field_iterator_init(&iter, group, p);
        while (v3d_field_iterator_next(clif, &iter)) {
                /* Clif parsing uses the packet name, and expects no
                 * sub-id.
                 */
                if (strcmp(iter.field->name, "sub-id") == 0 ||
                    strcmp(iter.field->name, "unused") == 0 ||
                    strcmp(iter.field->name, "Pad") == 0)
                        continue;

                if (clif->pretty) {
                        fprintf(clif->out, "    %s: %s\n",
                                iter.name, iter.value);
                } else {
                        fprintf(clif->out, "  /* %30s: */ %s\n",
                                iter.name, iter.value);
                }
                if (iter.struct_desc) {
                        uint64_t struct_offset = offset + iter.offset;
                        v3d_print_group(clif, iter.struct_desc,
                                        struct_offset,
                                        &p[iter.offset]);
                }
        }
}
