/*
 * Mesa 3-D graphics library
 *
 * Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file program_binary.c
 *
 * Helper functions for serializing a binary program.
 */


#include "compiler/glsl/serialize.h"
#include "main/errors.h"
#include "main/mtypes.h"
#include "main/shaderapi.h"
#include "util/bitscan.h"
#include "util/blob.h"
#include "util/crc32.h"
#include "program_binary.h"
#include "program/prog_parameter.h"

#include "state_tracker/st_shader_cache.h"

/**
 * Mesa supports one binary format, but it must differentiate between formats
 * produced by different drivers and different Mesa versions.
 *
 * Mesa uses a uint32_t value to specify an internal format. The only format
 * defined has one uint32_t value of 0, followed by 20 bytes specifying a sha1
 * that uniquely identifies the Mesa driver type and version.
 */

struct program_binary_header {
   /* If internal_format is 0, it must be followed by the 20 byte sha1 that
    * identifies the Mesa driver and version supported. If we want to support
    * something besides a sha1, then a new internal_format value can be added.
    */
   uint32_t internal_format;
   uint8_t sha1[20];
   /* Fields following sha1 can be changed since the sha1 will guarantee that
    * the binary only works with the same Mesa version.
    */
   uint32_t size;
   uint32_t crc32;
};

/**
 * Returns the header size needed for a binary
 */
static unsigned
get_program_binary_header_size(void)
{
   return sizeof(struct program_binary_header);
}

static bool
write_program_binary(const void *payload, unsigned payload_size,
                     const void *sha1, void *binary, unsigned binary_size,
                     GLenum *binary_format)
{
   struct program_binary_header *hdr = binary;

   if (binary_size < sizeof(*hdr))
      return false;

   /* binary_size is the size of the buffer provided by the application.
    * Make sure our program (payload) will fit in the buffer.
    */
   if (payload_size > binary_size - sizeof(*hdr))
      return false;

   hdr->internal_format = 0;
   memcpy(hdr->sha1, sha1, sizeof(hdr->sha1));
   memcpy(hdr + 1, payload, payload_size);
   hdr->size = payload_size;

   hdr->crc32 = util_hash_crc32(hdr + 1, payload_size);
   *binary_format = GL_PROGRAM_BINARY_FORMAT_MESA;

   return true;
}

static bool
simple_header_checks(const struct program_binary_header *hdr, unsigned length)
{
   if (hdr == NULL || length < sizeof(*hdr))
      return false;

   if (hdr->internal_format != 0)
      return false;

   return true;
}

static bool
check_crc32(const struct program_binary_header *hdr, unsigned length)
{
   uint32_t crc32;
   unsigned crc32_len;

   crc32_len = hdr->size;
   if (crc32_len > length - sizeof(*hdr))
      return false;

   crc32 = util_hash_crc32(hdr + 1, crc32_len);
   if (hdr->crc32 != crc32)
      return false;

   return true;
}

static bool
is_program_binary_valid(GLenum binary_format, const void *sha1,
                        const struct program_binary_header *hdr,
                        unsigned length)
{
   if (binary_format != GL_PROGRAM_BINARY_FORMAT_MESA)
      return false;

   if (!simple_header_checks(hdr, length))
      return false;

   if (memcmp(hdr->sha1, sha1, sizeof(hdr->sha1)) != 0)
      return false;

   if (!check_crc32(hdr, length))
      return false;

   return true;
}

/**
 * Returns the payload within the binary.
 *
 * If NULL is returned, then the binary not supported. If non-NULL is
 * returned, it will be a pointer contained within the specified `binary`
 * buffer.
 *
 * This can be used to access the payload of `binary` during the
 * glProgramBinary call.
 */
static const void*
get_program_binary_payload(GLenum binary_format, const void *sha1,
                           const void *binary, unsigned length)
{
   const struct program_binary_header *hdr = binary;
   if (!is_program_binary_valid(binary_format, sha1, hdr, length))
      return NULL;
   return (const uint8_t*)binary + sizeof(*hdr);
}

static void
write_program_payload(struct gl_context *ctx, struct blob *blob,
                      struct gl_shader_program *sh_prog)
{
   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct gl_linked_shader *shader = sh_prog->_LinkedShaders[stage];
      if (shader)
         ctx->Driver.ProgramBinarySerializeDriverBlob(ctx, sh_prog,
                                                      shader->Program);
   }

   blob_write_uint32(blob, sh_prog->SeparateShader);

   serialize_glsl_program(blob, ctx, sh_prog);

   for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
      struct gl_linked_shader *shader = sh_prog->_LinkedShaders[stage];
      if (shader) {
         struct gl_program *prog = sh_prog->_LinkedShaders[stage]->Program;
         ralloc_free(prog->driver_cache_blob);
         prog->driver_cache_blob = NULL;
         prog->driver_cache_blob_size = 0;
      }
   }
}

static bool
read_program_payload(struct gl_context *ctx, struct blob_reader *blob,
                     GLenum binary_format, struct gl_shader_program *sh_prog)
{
   sh_prog->SeparateShader = blob_read_uint32(blob);

   if (!deserialize_glsl_program(blob, ctx, sh_prog))
      return false;

   unsigned int stage;
   for (stage = 0; stage < ARRAY_SIZE(sh_prog->_LinkedShaders); stage++) {
      struct gl_linked_shader *shader = sh_prog->_LinkedShaders[stage];
      if (!shader)
         continue;

      ctx->Driver.ProgramBinaryDeserializeDriverBlob(ctx, sh_prog,
                                                     shader->Program);
   }

   return true;
}

void
_mesa_get_program_binary_length(struct gl_context *ctx,
                                struct gl_shader_program *sh_prog,
                                GLint *length)
{
   struct blob blob;
   blob_init_fixed(&blob, NULL, SIZE_MAX);
   write_program_payload(ctx, &blob, sh_prog);
   *length = get_program_binary_header_size() + blob.size;
   blob_finish(&blob);
}

void
_mesa_get_program_binary(struct gl_context *ctx,
                       struct gl_shader_program *sh_prog,
                       GLsizei buf_size, GLsizei *length,
                       GLenum *binary_format, GLvoid *binary)
{
   struct blob blob;
   uint8_t driver_sha1[20];
   unsigned header_size = get_program_binary_header_size();

   st_get_program_binary_driver_sha1(ctx, driver_sha1);

   blob_init(&blob);

   if (buf_size < header_size)
      goto fail;

   write_program_payload(ctx, &blob, sh_prog);
   if (blob.size + header_size > buf_size ||
       blob.out_of_memory)
      goto fail;

   bool written = write_program_binary(blob.data, blob.size, driver_sha1,
                                      binary, buf_size, binary_format);
   if (!written || blob.out_of_memory)
      goto fail;

   *length = header_size + blob.size;

   blob_finish(&blob);
   return;

fail:
   _mesa_error(ctx, GL_INVALID_OPERATION,
               "glGetProgramBinary(buffer too small)");
   *length = 0;
   blob_finish(&blob);
}

void
_mesa_program_binary(struct gl_context *ctx, struct gl_shader_program *sh_prog,
                     GLenum binary_format, const GLvoid *binary,
                     GLsizei length)
{
   uint8_t driver_sha1[20];
   unsigned header_size = get_program_binary_header_size();

   st_get_program_binary_driver_sha1(ctx, driver_sha1);

   const void *payload = get_program_binary_payload(binary_format, driver_sha1,
                                                    binary, length);

   if (payload == NULL) {
      sh_prog->data->LinkStatus = LINKING_FAILURE;
      return;
   }

   struct blob_reader blob;
   blob_reader_init(&blob, payload, length - header_size);

   unsigned programs_in_use = 0;
   if (ctx->_Shader)
      for (unsigned stage = 0; stage < MESA_SHADER_STAGES; stage++) {
         if (ctx->_Shader->CurrentProgram[stage] &&
             ctx->_Shader->CurrentProgram[stage]->Id == sh_prog->Name) {
            programs_in_use |= 1 << stage;
         }
   }

   if (!read_program_payload(ctx, &blob, binary_format, sh_prog)) {
      sh_prog->data->LinkStatus = LINKING_FAILURE;
      return;
   }

   _mesa_create_program_resource_hash(sh_prog);

   /* From section 7.3 (Program Objects) of the OpenGL 4.5 spec:
    *
    *    "If LinkProgram or ProgramBinary successfully re-links a program
    *     object that is active for any shader stage, then the newly generated
    *     executable code will be installed as part of the current rendering
    *     state for all shader stages where the program is active.
    *     Additionally, the newly generated executable code is made part of
    *     the state of any program pipeline for all stages where the program
    *     is attached."
    */
   while (programs_in_use) {
      const int stage = u_bit_scan(&programs_in_use);

      struct gl_program *prog = NULL;
      if (sh_prog->_LinkedShaders[stage])
         prog = sh_prog->_LinkedShaders[stage]->Program;

      _mesa_use_program(ctx, stage, sh_prog, prog, ctx->_Shader);
   }

   sh_prog->data->LinkStatus = LINKING_SKIPPED;
}
