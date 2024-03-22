/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include <gtest/gtest.h>
#include <signal.h>
#include <setjmp.h>

#if defined(GLX_DIRECT_RENDERING) && !defined(GLX_USE_APPLEGL)

#include "glxclient.h"
#include "glx_error.h"
#include "dri2.h"
#include "GL/internal/dri_interface.h"
#include "dri2_priv.h"

namespace {
   struct attribute_test_vector {
      const char *glx_string;
      const char *dri_string;
      int glx_attribute;
      int dri_attribute;
   };
}

#define E(g, d) { # g, # d, g, d }

static bool got_sigsegv;
static jmp_buf jmp;

static void
sigsegv_handler(int sig)
{
   (void) sig;
   got_sigsegv = true;
   longjmp(jmp, 1);
}

class dri2_query_renderer_string_test : public ::testing::Test {
public:
   virtual void SetUp();
   virtual void TearDown();

   struct sigaction sa;
   struct sigaction old_sa;
};

class dri2_query_renderer_integer_test :
   public dri2_query_renderer_string_test {
};

static bool queryString_called = false;
static int queryString_attribute = -1;

static bool queryInteger_called = false;
static int queryInteger_attribute = -1;

static int
fake_queryInteger(__DRIscreen *screen, int attribute, unsigned int *val)
{
   (void) screen;

   queryInteger_attribute = attribute;
   queryInteger_called = true;

   switch (attribute) {
   case __DRI2_RENDERER_VENDOR_ID:
      *val = ~__DRI2_RENDERER_VENDOR_ID;
      return 0;
   case __DRI2_RENDERER_DEVICE_ID:
      *val = ~__DRI2_RENDERER_DEVICE_ID;
      return 0;
   case __DRI2_RENDERER_VERSION:
      *val = ~__DRI2_RENDERER_VERSION;
      return 0;
   case __DRI2_RENDERER_ACCELERATED:
      *val = ~__DRI2_RENDERER_ACCELERATED;
      return 0;
   case __DRI2_RENDERER_VIDEO_MEMORY:
      *val = ~__DRI2_RENDERER_VIDEO_MEMORY;
      return 0;
   case __DRI2_RENDERER_UNIFIED_MEMORY_ARCHITECTURE:
      *val = ~__DRI2_RENDERER_UNIFIED_MEMORY_ARCHITECTURE;
      return 0;
   case __DRI2_RENDERER_PREFERRED_PROFILE:
      *val = ~__DRI2_RENDERER_PREFERRED_PROFILE;
      return 0;
   case __DRI2_RENDERER_OPENGL_CORE_PROFILE_VERSION:
      *val = ~__DRI2_RENDERER_OPENGL_CORE_PROFILE_VERSION;
      return 0;
   case __DRI2_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION:
      *val = ~__DRI2_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION;
      return 0;
   case __DRI2_RENDERER_OPENGL_ES_PROFILE_VERSION:
      *val = ~__DRI2_RENDERER_OPENGL_ES_PROFILE_VERSION;
      return 0;
   case __DRI2_RENDERER_OPENGL_ES2_PROFILE_VERSION:
      *val = ~__DRI2_RENDERER_OPENGL_ES2_PROFILE_VERSION;
      return 0;
   }

   return -1;
}

static int
fake_queryString(__DRIscreen *screen, int attribute, const char **val)
{
   (void) screen;

   queryString_attribute = attribute;
   queryString_called = true;

   switch (attribute) {
   case __DRI2_RENDERER_VENDOR_ID:
      *val = "__DRI2_RENDERER_VENDOR_ID";
      return 0;
   case __DRI2_RENDERER_DEVICE_ID:
      *val = "__DRI2_RENDERER_DEVICE_ID";
      return 0;
   }

   return -1;
}

static const __DRI2rendererQueryExtension rendererQueryExt = {
   { __DRI2_RENDERER_QUERY, 1 },

   fake_queryInteger,
   fake_queryString
};

void dri2_query_renderer_string_test::SetUp()
{
   got_sigsegv = false;

   sa.sa_handler = sigsegv_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = 0;
   sigaction(SIGSEGV, &sa, &old_sa);
}

void dri2_query_renderer_string_test::TearDown()
{
   sigaction(SIGSEGV, &old_sa, NULL);
}

/**
 * dri2_query_renderer_string will return an error if the rendererQuery
 * extension is not present.  It will also not segfault.
 */
TEST_F(dri2_query_renderer_string_test, DRI2_RENDERER_QUERY_not_supported)
{
   struct dri2_screen dsc;

   memset(&dsc, 0, sizeof(dsc));

   if (setjmp(jmp) == 0) {
      static const char original_value[] = "0xDEADBEEF";
      const char *value = original_value;
      const int success =
         dri2_query_renderer_string(&dsc.base,
                                    GLX_RENDERER_VENDOR_ID_MESA, &value);

      EXPECT_EQ(-1, success);
      EXPECT_EQ(original_value, value);
   } else {
      EXPECT_FALSE(got_sigsegv);
   }
}

/**
 * dri2_query_renderer_string will call queryString with the correct DRI2 enum
 * for each GLX attribute value.
 *
 * \note
 * This test does \b not perform any checking for invalid GLX attribute values.
 * Other unit tests verify that invalid values are filtered before
 * dri2_query_renderer_string is called.
 */
TEST_F(dri2_query_renderer_string_test, valid_attribute_mapping)
{
   struct dri2_screen dsc;
   struct attribute_test_vector valid_attributes[] = {
      E(GLX_RENDERER_VENDOR_ID_MESA,
        __DRI2_RENDERER_VENDOR_ID),
      E(GLX_RENDERER_DEVICE_ID_MESA,
        __DRI2_RENDERER_DEVICE_ID),
   };

   memset(&dsc, 0, sizeof(dsc));
   dsc.rendererQuery = &rendererQueryExt;

   if (setjmp(jmp) == 0) {
      for (unsigned i = 0; i < ARRAY_SIZE(valid_attributes); i++) {
         static const char original_value[] = "original value";
         const char *value = original_value;
         const int success =
            dri2_query_renderer_string(&dsc.base,
                                       valid_attributes[i].glx_attribute,
                                       &value);

         EXPECT_EQ(0, success);
         EXPECT_EQ(valid_attributes[i].dri_attribute, queryString_attribute)
            << valid_attributes[i].glx_string;
         EXPECT_STREQ(valid_attributes[i].dri_string, value)
            << valid_attributes[i].glx_string;
      }
   } else {
      EXPECT_FALSE(got_sigsegv);
   }
}

/**
 * dri2_query_renderer_integer will return an error if the rendererQuery
 * extension is not present.  It will also not segfault.
 */
TEST_F(dri2_query_renderer_integer_test, DRI2_RENDERER_QUERY_not_supported)
{
   struct dri2_screen dsc;

   memset(&dsc, 0, sizeof(dsc));

   if (setjmp(jmp) == 0) {
      unsigned int value = 0xDEADBEEF;
      const int success =
         dri2_query_renderer_integer(&dsc.base,
                                    GLX_RENDERER_VENDOR_ID_MESA, &value);

      EXPECT_EQ(-1, success);
      EXPECT_EQ(0xDEADBEEF, value);
   } else {
      EXPECT_FALSE(got_sigsegv);
   }
}

/**
 * dri2_query_renderer_integer will call queryInteger with the correct DRI2 enum
 * for each GLX attribute value.
 *
 * \note
 * This test does \b not perform any checking for invalid GLX attribute values.
 * Other unit tests verify that invalid values are filtered before
 * dri2_query_renderer_integer is called.
 */
TEST_F(dri2_query_renderer_integer_test, valid_attribute_mapping)
{
   struct dri2_screen dsc;
   struct attribute_test_vector valid_attributes[] = {
      E(GLX_RENDERER_VENDOR_ID_MESA,
        __DRI2_RENDERER_VENDOR_ID),
      E(GLX_RENDERER_DEVICE_ID_MESA,
        __DRI2_RENDERER_DEVICE_ID),
      E(GLX_RENDERER_VERSION_MESA,
        __DRI2_RENDERER_VERSION),
      E(GLX_RENDERER_ACCELERATED_MESA,
        __DRI2_RENDERER_ACCELERATED),
      E(GLX_RENDERER_VIDEO_MEMORY_MESA,
        __DRI2_RENDERER_VIDEO_MEMORY),
      E(GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA,
        __DRI2_RENDERER_UNIFIED_MEMORY_ARCHITECTURE),
      E(GLX_RENDERER_PREFERRED_PROFILE_MESA,
        __DRI2_RENDERER_PREFERRED_PROFILE),
      E(GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA,
        __DRI2_RENDERER_OPENGL_CORE_PROFILE_VERSION),
      E(GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA,
        __DRI2_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION),
      E(GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA,
        __DRI2_RENDERER_OPENGL_ES_PROFILE_VERSION),
      E(GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA,
        __DRI2_RENDERER_OPENGL_ES2_PROFILE_VERSION),
   };

   memset(&dsc, 0, sizeof(dsc));
   dsc.rendererQuery = &rendererQueryExt;

   if (setjmp(jmp) == 0) {
      for (unsigned i = 0; i < ARRAY_SIZE(valid_attributes); i++) {
         unsigned int value = 0xDEADBEEF;
         const int success =
            dri2_query_renderer_integer(&dsc.base,
                                       valid_attributes[i].glx_attribute,
                                       &value);

         EXPECT_EQ(0, success);
         EXPECT_EQ(valid_attributes[i].dri_attribute, queryInteger_attribute)
            << valid_attributes[i].glx_string;
         EXPECT_EQ((unsigned int) ~valid_attributes[i].dri_attribute, value)
            << valid_attributes[i].glx_string;
      }
   } else {
      EXPECT_FALSE(got_sigsegv);
   }
}

#endif /* GLX_DIRECT_RENDERING */
