/**************************************************************************
 *
 * Copyright © 2010 Luca Barbieri
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include <stdio.h>
#include "translate/translate.h"
#include "util/u_memory.h"
#include "util/format/u_format.h"
#include "util/half_float.h"
#include "util/u_cpu_detect.h"

/* don't use this for serious use */
static double rand_double()
{
   const double rm = (double)RAND_MAX + 1;
   double div = 1;
   double v = 0;
   unsigned i;
   for(i = 0; i < 4; ++i)
   {
      div *= rm;
      v += (double)rand() / div;
   }
   return v;
}

char cpu_caps_override_env[128];

int main(int argc, char** argv)
{
   struct translate *(*create_fn)(const struct translate_key *key) = 0;

   struct translate_key key;
   unsigned output_format;
   unsigned input_format;
   unsigned buffer_size = 4096;
   unsigned char* buffer[5];
   unsigned char* byte_buffer;
   float* float_buffer;
   double* double_buffer;
   uint16_t *half_buffer;
   unsigned * elts;
   unsigned count = 4;
   unsigned i, j, k;
   unsigned passed = 0;
   unsigned total = 0;
   const float error = 0.03125;

   create_fn = 0;

   if (argc <= 1 ||
       !strcmp(argv[1], "default") )
      create_fn = translate_create;
   else if (!strcmp(argv[1], "generic"))
      create_fn = translate_generic_create;
   else if (!strcmp(argv[1], "x86"))
      create_fn = translate_sse2_create;
   else
   {
      const char *translate_options[] = {
         "nosse", "sse", "sse2", "sse3", "ssse3", "sse4.1", "avx",
         NULL
      };
      const char **option;
      for (option = translate_options; *option; ++option)
      {
         if (!strcmp(argv[1], *option))
         {
            create_fn = translate_sse2_create;
            break;
         }
      }
      if (create_fn) {
         snprintf(cpu_caps_override_env, sizeof(cpu_caps_override_env), "GALLIUM_OVERRIDE_CPU_CAPS=%s", argv[1]);
         putenv(cpu_caps_override_env);
      }
   }

   if (!create_fn)
   {
      printf("Usage: ./translate_test [default|generic|x86|nosse|sse|sse2|sse3|ssse3|sse4.1|avx]\n");
      return 2;
   }

   for (i = 1; i < ARRAY_SIZE(buffer); ++i)
      buffer[i] = align_malloc(buffer_size, 4096);

   byte_buffer = align_malloc(buffer_size, 4096);
   float_buffer = align_malloc(buffer_size, 4096);
   double_buffer = align_malloc(buffer_size, 4096);
   half_buffer = align_malloc(buffer_size, 4096);

   elts = align_malloc(count * sizeof *elts, 4096);

   key.nr_elements = 1;
   key.element[0].input_buffer = 0;
   key.element[0].input_offset = 0;
   key.element[0].output_offset = 0;
   key.element[0].type = TRANSLATE_ELEMENT_NORMAL;
   key.element[0].instance_divisor = 0;

   srand(4359025);

   /* avoid negative values that work badly when converted to unsigned format*/
   for (i = 0; i < buffer_size; ++i)
      byte_buffer[i] = rand() & 0x7f7f7f7f;

   for (i = 0; i < buffer_size / sizeof(float); ++i)
      float_buffer[i] = (float)rand_double();

   for (i = 0; i < buffer_size / sizeof(double); ++i)
      double_buffer[i] = rand_double();

   for (i = 0; i < buffer_size / sizeof(double); ++i)
      half_buffer[i] = _mesa_float_to_half((float) rand_double());

   for (i = 0; i < count; ++i)
      elts[i] = i;

   for (output_format = 1; output_format < PIPE_FORMAT_COUNT; ++output_format)
   {
      const struct util_format_description* output_format_desc = util_format_description(output_format);
      const struct util_format_pack_description* output_format_pack = util_format_pack_description(output_format);
      util_format_fetch_rgba_func_ptr fetch_rgba =
         util_format_fetch_rgba_func(output_format);
      unsigned output_format_size;
      unsigned output_normalized = 0;

      if (!fetch_rgba
          || !output_format_pack->pack_rgba_float
          || output_format_desc->colorspace != UTIL_FORMAT_COLORSPACE_RGB
          || output_format_desc->layout != UTIL_FORMAT_LAYOUT_PLAIN
          || !translate_is_output_format_supported(output_format))
         continue;

      for(i = 0; i < output_format_desc->nr_channels; ++i)
      {
         if(output_format_desc->channel[i].type != UTIL_FORMAT_TYPE_FLOAT)
            output_normalized |= (1 << output_format_desc->channel[i].normalized);
      }

      output_format_size = util_format_get_stride(output_format, 1);

      for (input_format = 1; input_format < PIPE_FORMAT_COUNT; ++input_format)
      {
         const struct util_format_description* input_format_desc = util_format_description(input_format);
         const struct util_format_pack_description* input_format_pack = util_format_pack_description(input_format);
         util_format_fetch_rgba_func_ptr fetch_rgba =
            util_format_fetch_rgba_func(input_format);
         unsigned input_format_size;
         struct translate* translate[2];
         unsigned fail = 0;
         unsigned used_generic = 0;
         unsigned input_normalized = 0;
         bool input_is_float = false;

         if (!fetch_rgba
             || !input_format_pack->pack_rgba_float
             || input_format_desc->colorspace != UTIL_FORMAT_COLORSPACE_RGB
             || input_format_desc->layout != UTIL_FORMAT_LAYOUT_PLAIN
             || !translate_is_output_format_supported(input_format))
            continue;

         input_format_size = util_format_get_stride(input_format, 1);

         for(i = 0; i < input_format_desc->nr_channels; ++i)
         {
            if(input_format_desc->channel[i].type == UTIL_FORMAT_TYPE_FLOAT)
            {
               input_is_float = 1;
               input_normalized |= 1 << 1;
            }
            else
               input_normalized |= (1 << input_format_desc->channel[i].normalized);
         }

         if(((input_normalized | output_normalized) == 3)
               || ((input_normalized & 1) && (output_normalized & 1)
                     && input_format_size * output_format_desc->nr_channels > output_format_size * input_format_desc->nr_channels))
            continue;

         key.element[0].input_format = input_format;
         key.element[0].output_format = output_format;
         key.output_stride = output_format_size;
         translate[0] = create_fn(&key);
         if (!translate[0])
            continue;

         key.element[0].input_format = output_format;
         key.element[0].output_format = input_format;
         key.output_stride = input_format_size;
         translate[1] = create_fn(&key);
         if(!translate[1])
         {
            used_generic = 1;
            translate[1] = translate_generic_create(&key);
            if(!translate[1])
               continue;
         }

         for(i = 1; i < 5; ++i)
            memset(buffer[i], 0xcd - (0x22 * i), 4096);

         if(input_is_float && input_format_desc->channel[0].size == 32)
            buffer[0] = (unsigned char*)float_buffer;
         else if(input_is_float && input_format_desc->channel[0].size == 64)
            buffer[0] = (unsigned char*)double_buffer;
         else if(input_is_float && input_format_desc->channel[0].size == 16)
            buffer[0] = (unsigned char*)half_buffer;
         else if(input_is_float)
            abort();
         else
            buffer[0] = byte_buffer;

         translate[0]->set_buffer(translate[0], 0, buffer[0], input_format_size, count - 1);
         translate[0]->run_elts(translate[0], elts, count, 0, 0, buffer[1]);
         translate[1]->set_buffer(translate[1], 0, buffer[1], output_format_size, count - 1);
         translate[1]->run_elts(translate[1], elts, count, 0, 0, buffer[2]);
         translate[0]->set_buffer(translate[0], 0, buffer[2], input_format_size, count - 1);
         translate[0]->run_elts(translate[0], elts, count, 0, 0, buffer[3]);
         translate[1]->set_buffer(translate[1], 0, buffer[3], output_format_size, count - 1);
         translate[1]->run_elts(translate[1], elts, count, 0, 0, buffer[4]);

         for (i = 0; i < count; ++i)
         {
            float a[4];
            float b[4];
            fetch_rgba(a, buffer[2] + i * input_format_size, 0, 0);
            fetch_rgba(b, buffer[4] + i * input_format_size, 0, 0);

            for (j = 0; j < count; ++j)
            {
               float d = a[j] - b[j];
               if (d > error || d < -error)
               {
                  fail = 1;
                  break;
               }
            }
         }

         printf("%s%s: %s -> %s -> %s -> %s -> %s\n",
               fail ? "FAIL" : "PASS",
               used_generic ? "[GENERIC]" : "",
               input_format_desc->name, output_format_desc->name, input_format_desc->name, output_format_desc->name, input_format_desc->name);

         if (1)
         {
            for (i = 0; i < ARRAY_SIZE(buffer); ++i)
            {
               unsigned format_size = (i & 1) ? output_format_size : input_format_size;
               printf("%c ", (i == 2 || i == 4) ? '*' : ' ');
               for (j = 0; j < count; ++j)
               {
                  for (k = 0; k < format_size; ++k)
                  {
                     printf("%02x", buffer[i][j * format_size + k]);
                  }
                  printf(" ");
               }
               printf("\n");
            }
         }

         if (!fail)
            ++passed;
         ++total;

         if(translate[1])
            translate[1]->release(translate[1]);
         translate[0]->release(translate[0]);
      }
   }

   printf("%u/%u tests passed for translate_%s\n", passed, total, argv[1]);

   for (i = 1; i < ARRAY_SIZE(buffer); ++i)
      align_free(buffer[i]);

   align_free(byte_buffer);
   align_free(float_buffer);
   align_free(double_buffer);
   align_free(half_buffer);
   align_free(elts);
   return passed != total;
}
