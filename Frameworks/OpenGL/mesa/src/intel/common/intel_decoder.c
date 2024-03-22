/*
 * Copyright © 2016 Intel Corporation
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

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <expat.h>
#include <inttypes.h>
#include <zlib.h>

#include <util/list.h>
#include <util/macros.h>
#include <util/os_file.h>
#include <util/ralloc.h>
#include <util/u_math.h>

#include "intel_decoder.h"

#include "isl/isl.h"
#include "genxml/genX_xml.h"

#define XML_BUFFER_SIZE 4096
#define MAX_VALUE_ITEMS 128

struct location {
   const char *filename;
   int line_number;
};

struct genxml_import_exclusion {
   struct list_head link;
   char *name;
};

struct genxml_import {
   struct list_head link;
   struct list_head exclusions;
   char *name;
};

struct parser_context {
   XML_Parser parser;
   int foo;
   struct location loc;

   struct intel_group *group;
   struct intel_enum *enoom;
   const char *dirname;
   struct genxml_import import;

   int n_values, n_allocated_values;
   struct intel_value **values;

   struct intel_field *last_field;

   struct intel_spec *spec;
};

const char *
intel_group_get_name(const struct intel_group *group)
{
   return group->name;
}

uint32_t
intel_group_get_opcode(const struct intel_group *group)
{
   return group->opcode;
}

struct intel_group *
intel_spec_find_struct(struct intel_spec *spec, const char *name)
{
   struct hash_entry *entry = _mesa_hash_table_search(spec->structs,
                                                      name);
   return entry ? entry->data : NULL;
}

struct intel_group *
intel_spec_find_register(struct intel_spec *spec, uint32_t offset)
{
   struct hash_entry *entry =
      _mesa_hash_table_search(spec->registers_by_offset,
                              (void *) (uintptr_t) offset);
   return entry ? entry->data : NULL;
}

struct intel_group *
intel_spec_find_register_by_name(struct intel_spec *spec, const char *name)
{
   struct hash_entry *entry =
      _mesa_hash_table_search(spec->registers_by_name, name);
   return entry ? entry->data : NULL;
}

struct intel_enum *
intel_spec_find_enum(struct intel_spec *spec, const char *name)
{
   struct hash_entry *entry = _mesa_hash_table_search(spec->enums,
                                                      name);
   return entry ? entry->data : NULL;
}

uint32_t
intel_spec_get_gen(struct intel_spec *spec)
{
   return spec->gen;
}

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

static void
get_array_offset_count(const char **atts, uint32_t *offset, uint32_t *count,
                       uint32_t *size, bool *variable)
{
   for (int i = 0; atts[i]; i += 2) {
      char *p;

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

static struct intel_group *
create_group(struct parser_context *ctx,
             const char *name,
             const char **atts,
             struct intel_group *parent,
             bool fixed_length)
{
   struct intel_group *group;

   group = rzalloc(ctx->spec, struct intel_group);
   if (name)
      group->name = ralloc_strdup(group, name);

   group->spec = ctx->spec;
   group->variable = false;
   group->fixed_length = fixed_length;
   group->dword_length_field = NULL;
   group->dw_length = 0;
   group->engine_mask = INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_RENDER) |
                        INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_COMPUTE) |
                        INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_VIDEO) |
                        INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_COPY);
   group->bias = 1;

   for (int i = 0; atts[i]; i += 2) {
      char *p;
      if (strcmp(atts[i], "length") == 0) {
         group->dw_length = strtoul(atts[i + 1], &p, 0);
      } else if (strcmp(atts[i], "bias") == 0) {
         group->bias = strtoul(atts[i + 1], &p, 0);
      } else if (strcmp(atts[i], "engine") == 0) {
         void *mem_ctx = ralloc_context(NULL);
         char *tmp = ralloc_strdup(mem_ctx, atts[i + 1]);
         char *save_ptr;
         char *tok = strtok_r(tmp, "|", &save_ptr);

         group->engine_mask = 0;
         while (tok != NULL) {
            if (strcmp(tok, "render") == 0) {
               group->engine_mask |= INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_RENDER);
            } else if (strcmp(tok, "compute") == 0) {
               group->engine_mask |= INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_COMPUTE);
            } else if (strcmp(tok, "video") == 0) {
               group->engine_mask |= INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_VIDEO);
            } else if (strcmp(tok, "blitter") == 0) {
               group->engine_mask |= INTEL_ENGINE_CLASS_TO_MASK(INTEL_ENGINE_CLASS_COPY);
            } else {
               fprintf(stderr, "unknown engine class defined for instruction \"%s\": %s\n", name, atts[i + 1]);
            }

            tok = strtok_r(NULL, "|", &save_ptr);
         }

         ralloc_free(mem_ctx);
      }
   }

   if (parent) {
      group->parent = parent;
      get_array_offset_count(atts,
                             &group->array_offset,
                             &group->array_count,
                             &group->array_item_size,
                             &group->variable);
   }

   return group;
}

static struct intel_enum *
create_enum(struct parser_context *ctx, const char *name, const char **atts)
{
   struct intel_enum *e;

   e = rzalloc(ctx->spec, struct intel_enum);
   if (name)
      e->name = ralloc_strdup(e, name);

   return e;
}

static void
get_register_offset(const char **atts, uint32_t *offset)
{
   for (int i = 0; atts[i]; i += 2) {
      char *p;

      if (strcmp(atts[i], "num") == 0)
         *offset = strtoul(atts[i + 1], &p, 0);
   }
   return;
}

static void
get_start_end_pos(int *start, int *end)
{
   /* start value has to be mod with 32 as we need the relative
    * start position in the first DWord. For the end position, add
    * the length of the field to the start position to get the
    * relative position in the 64 bit address.
    */
   if (*end - *start > 32) {
      int len = *end - *start;
      *start = *start % 32;
      *end = *start + len;
   } else {
      *start = *start % 32;
      *end = *end % 32;
   }

   return;
}

static inline uint64_t
mask(int start, int end)
{
   uint64_t v;

   v = ~0ULL >> (63 - end + start);

   return v << start;
}

static inline uint64_t
field_value(uint64_t value, int start, int end)
{
   get_start_end_pos(&start, &end);
   return (value & mask(start, end)) >> (start);
}

static struct intel_type
string_to_type(struct parser_context *ctx, const char *s)
{
   int i, f;
   struct intel_group *g;
   struct intel_enum *e;

   if (strcmp(s, "int") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_INT };
   else if (strcmp(s, "uint") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_UINT };
   else if (strcmp(s, "bool") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_BOOL };
   else if (strcmp(s, "float") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_FLOAT };
   else if (strcmp(s, "address") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_ADDRESS };
   else if (strcmp(s, "offset") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_OFFSET };
   else if (sscanf(s, "u%d.%d", &i, &f) == 2)
      return (struct intel_type) { .kind = INTEL_TYPE_UFIXED, .i = i, .f = f };
   else if (sscanf(s, "s%d.%d", &i, &f) == 2)
      return (struct intel_type) { .kind = INTEL_TYPE_SFIXED, .i = i, .f = f };
   else if (g = intel_spec_find_struct(ctx->spec, s), g != NULL)
      return (struct intel_type) { .kind = INTEL_TYPE_STRUCT, .intel_struct = g };
   else if (e = intel_spec_find_enum(ctx->spec, s), e != NULL)
      return (struct intel_type) { .kind = INTEL_TYPE_ENUM, .intel_enum = e };
   else if (strcmp(s, "mbo") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_MBO };
   else if (strcmp(s, "mbz") == 0)
      return (struct intel_type) { .kind = INTEL_TYPE_MBZ };
   else
      fail(&ctx->loc, "invalid type: %s", s);
}

static struct intel_field *
create_field(struct parser_context *ctx, const char **atts)
{
   struct intel_field *field;

   field = rzalloc(ctx->group, struct intel_field);
   field->parent = ctx->group;

   for (int i = 0; atts[i]; i += 2) {
      char *p;

      if (strcmp(atts[i], "name") == 0) {
         field->name = ralloc_strdup(field, atts[i + 1]);
         if (strcmp(field->name, "DWord Length") == 0) {
            field->parent->dword_length_field = field;
         }
      } else if (strcmp(atts[i], "start") == 0) {
         field->start = strtoul(atts[i + 1], &p, 0);
      } else if (strcmp(atts[i], "end") == 0) {
         field->end = strtoul(atts[i + 1], &p, 0);
      } else if (strcmp(atts[i], "type") == 0) {
         field->type = string_to_type(ctx, atts[i + 1]);
      } else if (strcmp(atts[i], "default") == 0 &&
               field->start >= 16 && field->end <= 31) {
         field->has_default = true;
         field->default_value = strtoul(atts[i + 1], &p, 0);
      }
   }

   return field;
}

static struct intel_field *
create_array_field(struct parser_context *ctx, struct intel_group *array)
{
   struct intel_field *field;

   field = rzalloc(ctx->group, struct intel_field);
   field->parent = ctx->group;

   field->array = array;
   field->start = field->array->array_offset;

   return field;
}

static struct intel_value *
create_value(struct parser_context *ctx, const char **atts)
{
   struct intel_value *value = rzalloc(ctx->values, struct intel_value);

   for (int i = 0; atts[i]; i += 2) {
      if (strcmp(atts[i], "name") == 0)
         value->name = ralloc_strdup(value, atts[i + 1]);
      else if (strcmp(atts[i], "value") == 0)
         value->value = strtoul(atts[i + 1], NULL, 0);
   }

   return value;
}

static struct intel_field *
create_and_append_field(struct parser_context *ctx,
                        const char **atts,
                        struct intel_group *array)
{
   struct intel_field *field = array ?
      create_array_field(ctx, array) : create_field(ctx, atts);
   struct intel_field *prev = NULL, *list = ctx->group->fields;

   while (list && field->start > list->start) {
      prev = list;
      list = list->next;
   }

   field->next = list;
   if (prev == NULL)
      ctx->group->fields = field;
   else
      prev->next = field;

   return field;
}

static bool
start_genxml_import(struct parser_context *ctx, const char **atts)
{
   assert(ctx->import.name == NULL);
   assert(list_is_empty(&ctx->import.exclusions));
   list_inithead(&ctx->import.exclusions);

   for (int i = 0; atts[i]; i += 2) {
      if (strcmp(atts[i], "name") == 0) {
         ctx->import.name = ralloc_strdup(ctx->spec, atts[i + 1]);
      }
   }

   if (ctx->import.name == NULL)
      fail(&ctx->loc, "import without name");

   return ctx->import.name != NULL;
}

static struct genxml_import_exclusion *
add_genxml_import_exclusion(struct parser_context *ctx, const char **atts)
{
   struct genxml_import_exclusion *exclusion;

   if (ctx->import.name == NULL) {
      fail(&ctx->loc, "exclude found without a named import");
      return NULL;
   }

   exclusion = rzalloc(ctx->import.name, struct genxml_import_exclusion);

   for (int i = 0; atts[i]; i += 2) {
      if (strcmp(atts[i], "name") == 0) {
         exclusion->name = ralloc_strdup(exclusion, atts[i + 1]);
      }
   }

   if (exclusion->name != NULL) {
      list_addtail(&exclusion->link, &ctx->import.exclusions);
   } else {
      ralloc_free(exclusion);
      exclusion = NULL;
   }

   return exclusion;
}

static void
move_group_to_spec(struct intel_spec *new_spec, struct intel_spec *old_spec,
                   struct intel_group *group);

static void
move_field_to_spec(struct intel_spec *new_spec, struct intel_spec *old_spec,
                   struct intel_field *field)
{
   while (field != NULL) {
      if (field->array != NULL && field->array->spec == old_spec)
         move_group_to_spec(new_spec, old_spec, field->array);
      if (field->type.kind == INTEL_TYPE_STRUCT &&
          field->type.intel_struct->spec == old_spec)
         move_group_to_spec(new_spec, old_spec, field->type.intel_struct);
      if (field->type.kind == INTEL_TYPE_ENUM)
         ralloc_steal(new_spec, field->type.intel_enum);
      field = field->next;
   }
}

static void
move_group_to_spec(struct intel_spec *new_spec, struct intel_spec *old_spec,
                   struct intel_group *group)
{
   struct intel_group *g = group;
   while (g != NULL) {
      if (g->spec == old_spec) {
         if (ralloc_parent(g) == old_spec)
            ralloc_steal(new_spec, g);
         g->spec = new_spec;
      }
      g = g->next;
   }
   move_field_to_spec(new_spec, old_spec, group->fields);
   move_field_to_spec(new_spec, old_spec, group->dword_length_field);
}

static bool
finish_genxml_import(struct parser_context *ctx)
{
   struct intel_spec *spec = ctx->spec;
   struct genxml_import *import = &ctx->import;

   if (import->name == NULL) {
      fail(&ctx->loc, "import without name");
      return false;
   }

   struct intel_spec *imported_spec =
      intel_spec_load_filename(ctx->dirname, import->name);
   if (import->name == NULL) {
      fail(&ctx->loc, "failed to load %s for importing", import->name);
      return false;
   }

   assert(_mesa_hash_table_num_entries(imported_spec->access_cache) == 0);

   list_for_each_entry(struct genxml_import_exclusion, exclusion,
                       &import->exclusions, link) {
      struct hash_entry *entry;
      entry = _mesa_hash_table_search(imported_spec->commands,
                                      exclusion->name);
      if (entry != NULL) {
         _mesa_hash_table_remove(imported_spec->commands, entry);
      }
      entry = _mesa_hash_table_search(imported_spec->structs,
                                      exclusion->name);
      if (entry != NULL) {
         _mesa_hash_table_remove(imported_spec->structs, entry);
      }
      entry = _mesa_hash_table_search(imported_spec->registers_by_name,
                                      exclusion->name);
      if (entry != NULL) {
         struct intel_group *group = entry->data;
         _mesa_hash_table_remove(imported_spec->registers_by_name, entry);
         entry = _mesa_hash_table_search(imported_spec->registers_by_offset,
                                         (void *) (uintptr_t) group->register_offset);
         if (entry != NULL)
            _mesa_hash_table_remove(imported_spec->registers_by_offset, entry);
      }
      entry = _mesa_hash_table_search(imported_spec->enums,
                                      exclusion->name);
      if (entry != NULL) {
         _mesa_hash_table_remove(imported_spec->enums, entry);
      }
   }

   hash_table_foreach(imported_spec->commands, entry) {
      struct intel_group *group = entry->data;
      move_group_to_spec(spec, imported_spec, group);
      _mesa_hash_table_insert(spec->commands, group->name, group);
   }
   hash_table_foreach(imported_spec->structs, entry) {
      struct intel_group *group = entry->data;
      move_group_to_spec(spec, imported_spec, group);
      _mesa_hash_table_insert(spec->structs, group->name, group);
   }
   hash_table_foreach(imported_spec->registers_by_name, entry) {
      struct intel_group *group = entry->data;
      move_group_to_spec(spec, imported_spec, group);
      _mesa_hash_table_insert(spec->registers_by_name, group->name, group);
      _mesa_hash_table_insert(spec->registers_by_offset,
                              (void *) (uintptr_t) group->register_offset,
                              group);
   }
   hash_table_foreach(imported_spec->enums, entry) {
      struct intel_enum *enoom = entry->data;
      ralloc_steal(spec, enoom);
      _mesa_hash_table_insert(spec->enums, enoom->name, enoom);
   }

   intel_spec_destroy(imported_spec);
   ralloc_free(ctx->import.name); /* also frees exclusions */
   ctx->import.name = NULL;
   list_inithead(&ctx->import.exclusions);

   return true;
}

static void
start_element(void *data, const char *element_name, const char **atts)
{
   struct parser_context *ctx = data;
   const char *name = NULL;
   const char *gen = NULL;

   ctx->loc.line_number = XML_GetCurrentLineNumber(ctx->parser);

   for (int i = 0; atts[i]; i += 2) {
      if (strcmp(atts[i], "name") == 0)
         name = atts[i + 1];
      else if (strcmp(atts[i], "gen") == 0)
         gen = atts[i + 1];
   }

   if (strcmp(element_name, "genxml") == 0) {
      if (name == NULL)
         fail(&ctx->loc, "no platform name given");
      if (gen == NULL)
         fail(&ctx->loc, "no gen given");

      int major, minor;
      int n = sscanf(gen, "%d.%d", &major, &minor);
      if (n == 0)
         fail(&ctx->loc, "invalid gen given: %s", gen);
      if (n == 1)
         minor = 0;

      ctx->spec->gen = intel_make_gen(major, minor);
   } else if (strcmp(element_name, "instruction") == 0) {
      ctx->group = create_group(ctx, name, atts, NULL, false);
   } else if (strcmp(element_name, "struct") == 0) {
      ctx->group = create_group(ctx, name, atts, NULL, true);
   } else if (strcmp(element_name, "register") == 0) {
      ctx->group = create_group(ctx, name, atts, NULL, true);
      get_register_offset(atts, &ctx->group->register_offset);
   } else if (strcmp(element_name, "group") == 0) {
      struct intel_group *group = create_group(ctx, "", atts, ctx->group, false);
      ctx->last_field = create_and_append_field(ctx, NULL, group);
      ctx->group = group;
   } else if (strcmp(element_name, "field") == 0) {
      ctx->last_field = create_and_append_field(ctx, atts, NULL);
   } else if (strcmp(element_name, "enum") == 0) {
      ctx->enoom = create_enum(ctx, name, atts);
   } else if (strcmp(element_name, "value") == 0) {
      if (ctx->n_values >= ctx->n_allocated_values) {
         ctx->n_allocated_values = MAX2(2, ctx->n_allocated_values * 2);
         ctx->values = reralloc_array_size(ctx->spec, ctx->values,
                                           sizeof(struct intel_value *),
                                           ctx->n_allocated_values);
      }
      assert(ctx->n_values < ctx->n_allocated_values);
      ctx->values[ctx->n_values++] = create_value(ctx, atts);
   } else if (strcmp(element_name, "import") == 0) {
      start_genxml_import(ctx, atts);
   } else if (strcmp(element_name, "exclude") == 0) {
      add_genxml_import_exclusion(ctx, atts);
   }

}

static void
end_element(void *data, const char *name)
{
   struct parser_context *ctx = data;
   struct intel_spec *spec = ctx->spec;

   if (strcmp(name, "instruction") == 0 ||
       strcmp(name, "struct") == 0 ||
       strcmp(name, "register") == 0) {
      struct intel_group *group = ctx->group;
      struct intel_field *list = group->fields;

      ctx->group = ctx->group->parent;

      if (strcmp(name, "instruction") == 0) {
         while (list && list->end <= 31) {
            if (list->start >= 16 && list->has_default) {
               group->opcode_mask |=
                  mask(list->start % 32, list->end % 32);
               group->opcode |= list->default_value << list->start;
            }
            list = list->next;
         }
      }

      if (strcmp(name, "instruction") == 0)
         _mesa_hash_table_insert(spec->commands, group->name, group);
      else if (strcmp(name, "struct") == 0)
         _mesa_hash_table_insert(spec->structs, group->name, group);
      else if (strcmp(name, "register") == 0) {
         _mesa_hash_table_insert(spec->registers_by_name, group->name, group);
         _mesa_hash_table_insert(spec->registers_by_offset,
                                 (void *) (uintptr_t) group->register_offset,
                                 group);
      }
   } else if (strcmp(name, "group") == 0) {
      ctx->group = ctx->group->parent;
   } else if (strcmp(name, "field") == 0) {
      struct intel_field *field = ctx->last_field;
      ctx->last_field = NULL;
      field->inline_enum.values = ctx->values;
      ralloc_steal(field, ctx->values);
      field->inline_enum.nvalues = ctx->n_values;
      ctx->values = ralloc_array(ctx->spec, struct intel_value*, ctx->n_allocated_values = 2);
      ctx->n_values = 0;
   } else if (strcmp(name, "enum") == 0) {
      struct intel_enum *e = ctx->enoom;
      e->values = ctx->values;
      ralloc_steal(e, ctx->values);
      e->nvalues = ctx->n_values;
      ctx->values = ralloc_array(ctx->spec, struct intel_value*, ctx->n_allocated_values = 2);
      ctx->n_values = 0;
      ctx->enoom = NULL;
      _mesa_hash_table_insert(spec->enums, e->name, e);
   } else if (strcmp(name, "import") == 0) {
      finish_genxml_import(ctx);
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

static uint32_t _hash_uint32(const void *key)
{
   return (uint32_t) (uintptr_t) key;
}

static struct intel_spec *
intel_spec_init(void)
{
   struct intel_spec *spec;
   spec = rzalloc(NULL, struct intel_spec);
   if (spec == NULL)
      return NULL;

   spec->commands =
      _mesa_hash_table_create(spec, _mesa_hash_string, _mesa_key_string_equal);
   spec->structs =
      _mesa_hash_table_create(spec, _mesa_hash_string, _mesa_key_string_equal);
   spec->registers_by_name =
      _mesa_hash_table_create(spec, _mesa_hash_string, _mesa_key_string_equal);
   spec->registers_by_offset =
      _mesa_hash_table_create(spec, _hash_uint32, _mesa_key_pointer_equal);
   spec->enums =
      _mesa_hash_table_create(spec, _mesa_hash_string, _mesa_key_string_equal);
   spec->access_cache =
      _mesa_hash_table_create(spec, _mesa_hash_string, _mesa_key_string_equal);

   return spec;
}

static bool
get_xml_data_dir(const char *dirname, const char *filename,
                 void **data, size_t *data_len)
{
   size_t fullname_len = strlen(dirname) + strlen(filename) + 2;
   char *fullname = malloc(fullname_len);

   if (fullname == NULL)
      return NULL;

   ASSERTED size_t len = snprintf(fullname, fullname_len, "%s/%s",
                                  dirname, filename);
   assert(len < fullname_len);

   *data = (void*)os_read_file(fullname, data_len);
   free(fullname);
   return *data != NULL;
}

static bool
get_embedded_xml_data(int verx10, void **data, size_t *data_len)
{
   uint8_t *text_data = NULL;
   uint32_t text_offset = 0, text_length = 0;
   ASSERTED uint32_t total_length;

   for (int i = 0; i < ARRAY_SIZE(genxml_files_table); i++) {
      if (genxml_files_table[i].ver_10 == verx10) {
         text_offset = genxml_files_table[i].offset;
         text_length = genxml_files_table[i].length;
         break;
      }
   }

   if (text_length == 0) {
      fprintf(stderr, "unable to find gen (%u) data\n", verx10);
      return false;
   }

   total_length = zlib_inflate(compress_genxmls,
                               sizeof(compress_genxmls),
                               (void **) &text_data);
   assert(text_offset + text_length <= total_length);

   *data = malloc(text_length);
   if (*data == NULL) {
      free(text_data);
      return false;
   }

   memcpy(*data, &text_data[text_offset], text_length);
   free(text_data);
   *data_len = text_length;
   return true;
}

static bool
get_embedded_xml_data_by_name(const char *filename,
                              void **data, size_t *data_len)
{
   int filename_len = strlen(filename);
   if (filename_len < 8 || filename_len > 10)
      return false;

   if (strncmp(filename, "gen", 3) != 0 ||
       strcmp(filename + filename_len - 4, ".xml") != 0)
      return false;

   char *numstr = strndup(filename + 3, filename_len - 7);
   char *endptr;
   long num = strtol(numstr, &endptr, 10);
   if (*endptr != '\0') {
      free(numstr);
      return false;
   }
   /* convert ver numbers to verx10 */
   if (num < 45)
      num = num * 10;

   free(numstr);
   return get_embedded_xml_data(num, data, data_len);
}

static bool
get_xml_data(int verx10, const char *dirname, const char *filename,
             void **data, size_t *data_len)
{
   if (dirname != NULL)
      return get_xml_data_dir(dirname, filename, data, data_len);
   else if (filename != NULL)
      return get_embedded_xml_data_by_name(filename, data, data_len);
   else
      return get_embedded_xml_data(verx10, data, data_len);
}

static struct intel_spec *
intel_spec_load_common(int verx10, const char *dirname, const char *filename)
{
   struct parser_context ctx;
   void *xmlbuf, *data;
   size_t data_len;

   if (!get_xml_data(verx10, dirname, filename, &data, &data_len))
      return NULL;

   memset(&ctx, 0, sizeof ctx);
   ctx.dirname = dirname;
   list_inithead(&ctx.import.exclusions);
   ctx.parser = XML_ParserCreate(NULL);
   XML_SetUserData(ctx.parser, &ctx);
   if (ctx.parser == NULL) {
      free(data);
      fprintf(stderr, "failed to create parser\n");
      return NULL;
   }

   XML_SetElementHandler(ctx.parser, start_element, end_element);
   XML_SetCharacterDataHandler(ctx.parser, character_data);

   ctx.spec = intel_spec_init();
   if (ctx.spec == NULL) {
      free(data);
      fprintf(stderr, "Failed to create intel_spec\n");
      return NULL;
   }

   xmlbuf = XML_GetBuffer(ctx.parser, data_len);
   memcpy(xmlbuf, data, data_len);
   free(data);
   data = NULL;

   if (XML_ParseBuffer(ctx.parser, data_len, true) == 0) {
      fprintf(stderr,
              "Error parsing XML at line %ld col %ld byte %ld/%zu: %s\n",
              XML_GetCurrentLineNumber(ctx.parser),
              XML_GetCurrentColumnNumber(ctx.parser),
              XML_GetCurrentByteIndex(ctx.parser), data_len,
              XML_ErrorString(XML_GetErrorCode(ctx.parser)));
      XML_ParserFree(ctx.parser);
      return NULL;
   }

   XML_ParserFree(ctx.parser);
   assert(ctx.import.name == NULL);

   return ctx.spec;
}

struct intel_spec *
intel_spec_load(const struct intel_device_info *devinfo)
{
   return intel_spec_load_common(devinfo->verx10, NULL, NULL);
}

struct intel_spec *
intel_spec_load_filename(const char *dir, const char *name)
{
   return intel_spec_load_common(0, dir, name);
}

struct intel_spec *
intel_spec_load_from_path(const struct intel_device_info *devinfo,
                          const char *path)
{
   char filename[20];
   int xml_file_num = devinfo->verx10 % 10 ? devinfo->verx10 : devinfo->ver;

   ASSERTED size_t len = snprintf(filename, ARRAY_SIZE(filename), "gen%i.xml",
                                  xml_file_num);
   assert(len < ARRAY_SIZE(filename));

   return intel_spec_load_common(devinfo->verx10, path, filename);
}

void intel_spec_destroy(struct intel_spec *spec)
{
   ralloc_free(spec);
}

struct intel_group *
intel_spec_find_instruction(struct intel_spec *spec,
                            enum intel_engine_class engine,
                            const uint32_t *p)
{
   hash_table_foreach(spec->commands, entry) {
      struct intel_group *command = entry->data;
      uint32_t opcode = *p & command->opcode_mask;
      if ((command->engine_mask & INTEL_ENGINE_CLASS_TO_MASK(engine)) &&
           opcode == command->opcode)
         return command;
   }

   return NULL;
}

struct intel_field *
intel_group_find_field(struct intel_group *group, const char *name)
{
   char path[256];
   snprintf(path, sizeof(path), "%s/%s", group->name, name);

   struct intel_spec *spec = group->spec;
   struct hash_entry *entry = _mesa_hash_table_search(spec->access_cache,
                                                      path);
   if (entry)
      return entry->data;

   struct intel_field *field = group->fields;
   while (field) {
      if (strcmp(field->name, name) == 0) {
         _mesa_hash_table_insert(spec->access_cache,
                                 ralloc_strdup(spec, path),
                                 field);
         return field;
      }
      field = field->next;
   }

   return NULL;
}

int
intel_group_get_length(const struct intel_group *group, const uint32_t *p)
{
   if (group) {
      if (group->fixed_length)
         return group->dw_length;
      else {
         struct intel_field *field = group->dword_length_field;
         if (field) {
            return field_value(p[0], field->start, field->end) + group->bias;
         }
      }
   }

   uint32_t h = p[0];
   uint32_t type = field_value(h, 29, 31);

   switch (type) {
   case 0: /* MI */ {
      uint32_t opcode = field_value(h, 23, 28);
      if (opcode < 16)
         return 1;
      else
         return field_value(h, 0, 7) + 2;
      break;
   }

   case 2: /* BLT */ {
      return field_value(h, 0, 7) + 2;
   }

   case 3: /* Render */ {
      uint32_t subtype = field_value(h, 27, 28);
      uint32_t opcode = field_value(h, 24, 26);
      uint16_t whole_opcode = field_value(h, 16, 31);
      switch (subtype) {
      case 0:
         if (whole_opcode == 0x6104 /* PIPELINE_SELECT_965 */)
            return 1;
         else if (opcode < 2)
            return field_value(h, 0, 7) + 2;
         else
            return -1;
      case 1:
         if (opcode < 2)
            return 1;
         else
            return -1;
      case 2: {
         if (opcode == 0)
            return field_value(h, 0, 7) + 2;
         else if (opcode < 3)
            return field_value(h, 0, 15) + 2;
         else
            return -1;
      }
      case 3:
         if (whole_opcode == 0x780b)
            return 1;
         else if (opcode < 4)
            return field_value(h, 0, 7) + 2;
         else
            return -1;
      }
   }
   }

   return -1;
}

static const char *
intel_get_enum_name(struct intel_enum *e, uint64_t value)
{
   for (int i = 0; i < e->nvalues; i++) {
      if (e->values[i]->value == value) {
         return e->values[i]->name;
      }
   }
   return NULL;
}

static bool
iter_more_fields(const struct intel_field_iterator *iter)
{
   return iter->field != NULL && iter->field->next != NULL;
}

static uint32_t
iter_array_offset_bits(const struct intel_field_iterator *iter)
{
   if (iter->level == 0)
      return 0;

   uint32_t offset = 0;
   const struct intel_group *group = iter->groups[1];
   for (int level = 1; level <= iter->level; level++, group = iter->groups[level]) {
      uint32_t array_idx = iter->array_iter[level];
      offset += group->array_offset + array_idx * group->array_item_size;
   }

   return offset;
}

/* Checks whether we have more items in the array to iterate, or more arrays to
 * iterate through.
 */
/* descend into a non-array field */
static void
iter_push_array(struct intel_field_iterator *iter)
{
   assert(iter->level >= 0);

   iter->group = iter->field->array;
   iter->level++;
   assert(iter->level < DECODE_MAX_ARRAY_DEPTH);
   iter->groups[iter->level] = iter->group;
   iter->array_iter[iter->level] = 0;

   assert(iter->group->fields != NULL); /* an empty <group> makes no sense */
   iter->field = iter->group->fields;
   iter->fields[iter->level] = iter->field;
}

static void
iter_pop_array(struct intel_field_iterator *iter)
{
   assert(iter->level > 0);

   iter->level--;
   iter->field = iter->fields[iter->level];
   iter->group = iter->groups[iter->level];
}

static void
iter_start_field(struct intel_field_iterator *iter, struct intel_field *field)
{
   iter->field = field;
   iter->fields[iter->level] = field;

   while (iter->field->array)
      iter_push_array(iter);

   int array_member_offset = iter_array_offset_bits(iter);

   iter->start_bit = array_member_offset + iter->field->start;
   iter->end_bit = array_member_offset + iter->field->end;
   iter->struct_desc = NULL;
}

static void
iter_advance_array(struct intel_field_iterator *iter)
{
   assert(iter->level > 0);
   int lvl = iter->level;

   if (iter->group->variable)
      iter->array_iter[lvl]++;
   else {
      if ((iter->array_iter[lvl] + 1) < iter->group->array_count) {
         iter->array_iter[lvl]++;
      }
   }

   iter_start_field(iter, iter->group->fields);
}

static bool
iter_more_array_elems(const struct intel_field_iterator *iter)
{
   int lvl = iter->level;
   assert(lvl >= 0);

   if (iter->group->variable) {
      int length = intel_group_get_length(iter->group, iter->p);
      assert(length >= 0 && "error the length is unknown!");
      return iter_array_offset_bits(iter) + iter->group->array_item_size <
         (length * 32);
   } else {
      return (iter->array_iter[lvl] + 1) < iter->group->array_count;
   }
}

static bool
iter_advance_field(struct intel_field_iterator *iter)
{
   /* Keep looping while we either have more fields to look at, or we are
    * inside a <group> and can go up a level.
    */
   while (iter_more_fields(iter) || iter->level > 0) {
      if (iter_more_fields(iter)) {
         iter_start_field(iter, iter->field->next);
         return true;
      }

      assert(iter->level >= 0);

      if (iter_more_array_elems(iter)) {
         iter_advance_array(iter);
         return true;
      }

      /* At this point, we reached the end of the <group> and were on the last
       * iteration. So it's time to go back to the parent and then advance the
       * field.
       */
      iter_pop_array(iter);
   }

   return false;
}

static bool
iter_decode_field_raw(struct intel_field_iterator *iter, uint64_t *qw)
{
   *qw = 0;

   int field_start = iter->p_bit + iter->start_bit;
   int field_end = iter->p_bit + iter->end_bit;

   const uint32_t *p = iter->p + (iter->start_bit / 32);
   if (iter->p_end && p >= iter->p_end)
      return false;

   if ((field_end - field_start) > 32) {
      if (!iter->p_end || (p + 1) < iter->p_end)
         *qw = ((uint64_t) p[1]) << 32;
      *qw |= p[0];
   } else
      *qw = p[0];

   *qw = field_value(*qw, field_start, field_end);

   /* Address & offset types have to be aligned to dwords, their start bit is
    * a reminder of the alignment requirement.
    */
   if (iter->field->type.kind == INTEL_TYPE_ADDRESS ||
       iter->field->type.kind == INTEL_TYPE_OFFSET)
      *qw <<= field_start % 32;

   return true;
}

static bool
iter_decode_field(struct intel_field_iterator *iter)
{
   union {
      uint64_t qw;
      float f;
   } v;

   if (iter->field->name)
      snprintf(iter->name, sizeof(iter->name), "%s", iter->field->name);
   else
      memset(iter->name, 0, sizeof(iter->name));

   memset(&v, 0, sizeof(v));

   if (!iter_decode_field_raw(iter, &iter->raw_value))
      return false;

   const char *enum_name = NULL;

   v.qw = iter->raw_value;
   switch (iter->field->type.kind) {
   case INTEL_TYPE_UNKNOWN:
   case INTEL_TYPE_INT: {
      snprintf(iter->value, sizeof(iter->value), "%"PRId64, v.qw);
      enum_name = intel_get_enum_name(&iter->field->inline_enum, v.qw);
      break;
   }
   case INTEL_TYPE_MBZ:
   case INTEL_TYPE_UINT: {
      snprintf(iter->value, sizeof(iter->value), "%"PRIu64, v.qw);
      enum_name = intel_get_enum_name(&iter->field->inline_enum, v.qw);
      break;
   }
   case INTEL_TYPE_BOOL: {
      const char *true_string =
         iter->print_colors ? "\e[0;35mtrue\e[0m" : "true";
      snprintf(iter->value, sizeof(iter->value), "%s",
               v.qw ? true_string : "false");
      break;
   }
   case INTEL_TYPE_FLOAT:
      snprintf(iter->value, sizeof(iter->value), "%f", v.f);
      break;
   case INTEL_TYPE_ADDRESS:
   case INTEL_TYPE_OFFSET:
      snprintf(iter->value, sizeof(iter->value), "0x%08"PRIx64, v.qw);
      break;
   case INTEL_TYPE_STRUCT:
      snprintf(iter->value, sizeof(iter->value), "<struct %s>",
               iter->field->type.intel_struct->name);
      iter->struct_desc =
         intel_spec_find_struct(iter->group->spec,
                                iter->field->type.intel_struct->name);
      break;
   case INTEL_TYPE_UFIXED:
      snprintf(iter->value, sizeof(iter->value), "%f",
               (float) v.qw / (1 << iter->field->type.f));
      break;
   case INTEL_TYPE_SFIXED: {
      /* Sign extend before converting */
      int bits = iter->field->type.i + iter->field->type.f + 1;
      int64_t v_sign_extend = util_mask_sign_extend(v.qw, bits);
      snprintf(iter->value, sizeof(iter->value), "%f",
               (float) v_sign_extend / (1 << iter->field->type.f));
      break;
   }
   case INTEL_TYPE_MBO:
       break;
   case INTEL_TYPE_ENUM: {
      snprintf(iter->value, sizeof(iter->value), "%"PRId64, v.qw);
      enum_name = intel_get_enum_name(iter->field->type.intel_enum, v.qw);
      break;
   }
   }

   if (strlen(iter->group->name) == 0) {
      int length = strlen(iter->name);
      assert(iter->level >= 0);

      int level = 1;
      char *buf = iter->name + length;
      while (level <= iter->level) {
         int printed = snprintf(buf, sizeof(iter->name) - length,
                                "[%i]", iter->array_iter[level]);
         level++;
         length += printed;
         buf += printed;
      }
   }

   if (enum_name) {
      int length = strlen(iter->value);
      snprintf(iter->value + length, sizeof(iter->value) - length,
               " (%s)", enum_name);
   } else if (strcmp(iter->name, "Surface Format") == 0 ||
              strcmp(iter->name, "Source Element Format") == 0) {
      if (isl_format_is_valid((enum isl_format)v.qw)) {
         const char *fmt_name = isl_format_get_name((enum isl_format)v.qw);
         int length = strlen(iter->value);
         snprintf(iter->value + length, sizeof(iter->value) - length,
                  " (%s)", fmt_name);
      }
   }

   return true;
}

void
intel_field_iterator_init(struct intel_field_iterator *iter,
                          struct intel_group *group,
                          const uint32_t *p, int p_bit,
                          bool print_colors)
{
   memset(iter, 0, sizeof(*iter));

   iter->groups[iter->level] = group;
   iter->group = group;
   iter->p = p;
   iter->p_bit = p_bit;

   int length = intel_group_get_length(iter->group, iter->p);
   assert(length >= 0 && "error the length is unknown!");
   iter->p_end = length >= 0 ? &p[length] : NULL;
   iter->print_colors = print_colors;
}

bool
intel_field_iterator_next(struct intel_field_iterator *iter)
{
   /* Initial condition */
   if (!iter->field) {
      if (iter->group->fields)
         iter_start_field(iter, iter->group->fields);

      bool result = iter_decode_field(iter);
      if (!result && iter->p_end) {
         /* We're dealing with a non empty struct of length=0 (BLEND_STATE on
          * Gen 7.5)
          */
         assert(iter->group->dw_length == 0);
      }

      return result;
   }

   if (!iter_advance_field(iter))
      return false;

   if (!iter_decode_field(iter))
      return false;

   return true;
}

static void
print_dword_header(FILE *outfile,
                   struct intel_field_iterator *iter,
                   uint64_t offset, uint32_t dword)
{
   fprintf(outfile, "0x%08"PRIx64":  0x%08x : Dword %d\n",
           offset + 4 * dword, iter->p[dword], dword);
}

bool
intel_field_is_header(struct intel_field *field)
{
   uint32_t bits;

   /* Instructions are identified by the first DWord. */
   if (field->start >= 32 ||
       field->end >= 32)
      return false;

   bits = (1ULL << (field->end - field->start + 1)) - 1;
   bits <<= field->start;

   return (field->parent->opcode_mask & bits) != 0;
}

void
intel_print_group(FILE *outfile, struct intel_group *group, uint64_t offset,
                  const uint32_t *p, int p_bit, bool color)
{
   struct intel_field_iterator iter;
   int last_dword = -1;

   intel_field_iterator_init(&iter, group, p, p_bit, color);
   while (intel_field_iterator_next(&iter)) {
      int iter_dword = iter.end_bit / 32;
      if (last_dword != iter_dword) {
         for (int i = last_dword + 1; i <= iter_dword; i++)
            print_dword_header(outfile, &iter, offset, i);
         last_dword = iter_dword;
      }
      if (!intel_field_is_header(iter.field)) {
         fprintf(outfile, "    %s: %s\n", iter.name, iter.value);
         if (iter.struct_desc) {
            int struct_dword = iter.start_bit / 32;
            uint64_t struct_offset = offset + 4 * struct_dword;
            intel_print_group(outfile, iter.struct_desc, struct_offset,
                              &p[struct_dword], iter.start_bit % 32, color);
         }
      }
   }
}
