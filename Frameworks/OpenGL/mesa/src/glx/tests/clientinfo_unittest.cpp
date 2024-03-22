/*
 * Copyright © 2011 Intel Corporation
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
#include <string.h>

#include "glxclient.h"

#include <xcb/glx.h>

#include "mock_xdisplay.h"
#include "fake_glx_screen.h"

/**
 * \name Wrappers around some X structures to make the more usable for tests
 */
/*@{*/
class fake_glx_screen;

class fake_glx_display : public glx_display {
public:
   fake_glx_display(mock_XDisplay *dpy, int major, int minor)
   {
      this->next = 0;
      this->dpy = dpy;
      this->minorVersion = minor;
      this->glXDrawHash = 0;

      this->screens = new glx_screen *[dpy->nscreens];
      memset(this->screens, 0, sizeof(struct glx_screen *) * dpy->nscreens);
   }

   ~fake_glx_display()
   {
      for (int i = 0; i < this->dpy->nscreens; i++) {
         if (this->screens[i] != NULL)
            delete (fake_glx_screen *)this->screens[i];
      }

      delete [] this->screens;
   }

   void init_screen(int i, const char *ext);
};

class glX_send_client_info_test : public ::testing::Test {
public:
   glX_send_client_info_test();
   virtual ~glX_send_client_info_test();
   virtual void SetUp();
   virtual void TearDown();

   void common_protocol_expected_false_test(unsigned major, unsigned minor,
					    const char *glx_ext, bool *value);

   void common_protocol_expected_true_test(unsigned major, unsigned minor,
					   const char *glx_ext, bool *value);

   void create_single_screen_display(unsigned major, unsigned minor,
				     const char *glx_ext);

   void destroy_display();

protected:
   fake_glx_display *glx_dpy;
   mock_XDisplay *display;
};

void
fake_glx_display::init_screen(int i, const char *ext)
{
   if (this->screens[i] != NULL)
      delete this->screens[i];

   this->screens[i] = new fake_glx_screen(this, i, ext);
}
/*@}*/

static const char ext[] = "GL_XXX_dummy";

static bool ClientInfo_was_sent;
static bool SetClientInfoARB_was_sent;
static bool SetClientInfo2ARB_was_sent;
static xcb_connection_t *connection_used;
static int gl_ext_length;
static char *gl_ext_string;
static int glx_ext_length;
static char *glx_ext_string;
static int num_gl_versions;
static uint32_t *gl_versions;
static int glx_major;
static int glx_minor;

extern "C" xcb_connection_t *
XGetXCBConnection(Display *dpy)
{
   return (xcb_connection_t *) 0xdeadbeef;
}

extern "C" xcb_void_cookie_t
xcb_glx_client_info(xcb_connection_t *c,
		    uint32_t major_version,
		    uint32_t minor_version,
		    uint32_t str_len,
		    const char *string)
{
   xcb_void_cookie_t cookie;

   ClientInfo_was_sent = true;
   connection_used = c;

   gl_ext_string = new char[str_len];
   memcpy(gl_ext_string, string, str_len);
   gl_ext_length = str_len;

   glx_major = major_version;
   glx_minor = minor_version;

   cookie.sequence = 0;
   return cookie;
}

extern "C" xcb_void_cookie_t
xcb_glx_set_client_info_arb(xcb_connection_t *c,
			    uint32_t major_version,
			    uint32_t minor_version,
			    uint32_t num_versions,
			    uint32_t gl_str_len,
			    uint32_t glx_str_len,
			    const uint32_t *versions,
			    const char *gl_string,
			    const char *glx_string)
{
   xcb_void_cookie_t cookie;

   SetClientInfoARB_was_sent = true;
   connection_used = c;

   gl_ext_string = new char[gl_str_len];
   memcpy(gl_ext_string, gl_string, gl_str_len);
   gl_ext_length = gl_str_len;

   glx_ext_string = new char[glx_str_len];
   memcpy(glx_ext_string, glx_string, glx_str_len);
   glx_ext_length = glx_str_len;

   gl_versions = new uint32_t[num_versions * 2];
   memcpy(gl_versions, versions, sizeof(uint32_t) * num_versions * 2);
   num_gl_versions = num_versions;

   glx_major = major_version;
   glx_minor = minor_version;

   cookie.sequence = 0;
   return cookie;
}

extern "C" xcb_void_cookie_t
xcb_glx_set_client_info_2arb(xcb_connection_t *c,
			     uint32_t major_version,
			     uint32_t minor_version,
			     uint32_t num_versions,
			     uint32_t gl_str_len,
			     uint32_t glx_str_len,
			     const uint32_t *versions,
			     const char *gl_string,
			     const char *glx_string)
{
   xcb_void_cookie_t cookie;

   SetClientInfo2ARB_was_sent = true;
   connection_used = c;

   gl_ext_string = new char[gl_str_len];
   memcpy(gl_ext_string, gl_string, gl_str_len);
   gl_ext_length = gl_str_len;

   glx_ext_string = new char[glx_str_len];
   memcpy(glx_ext_string, glx_string, glx_str_len);
   glx_ext_length = glx_str_len;

   gl_versions = new uint32_t[num_versions * 3];
   memcpy(gl_versions, versions, sizeof(uint32_t) * num_versions * 3);
   num_gl_versions = num_versions;

   glx_major = major_version;
   glx_minor = minor_version;

   cookie.sequence = 0;
   return cookie;
}

extern "C" char *
__glXGetClientGLExtensionString(int screen)
{
   char *str = (char *) malloc(sizeof(ext));

   memcpy(str, ext, sizeof(ext));
   return str;
}

glX_send_client_info_test::glX_send_client_info_test()
   : glx_dpy(0), display(0)
{
   /* empty */
}

glX_send_client_info_test::~glX_send_client_info_test()
{
   if (glx_dpy)
      delete glx_dpy;

   if (display)
      delete display;
}

void
glX_send_client_info_test::SetUp()
{
   ClientInfo_was_sent = false;
   SetClientInfoARB_was_sent = false;
   SetClientInfo2ARB_was_sent = false;
   connection_used = (xcb_connection_t *) ~0;
   gl_ext_length = 0;
   gl_ext_string = (char *) 0;
   glx_ext_length = 0;
   glx_ext_string = (char *) 0;
   num_gl_versions = 0;
   gl_versions = (uint32_t *) 0;
   glx_major = 0;
   glx_minor = 0;
}

void
glX_send_client_info_test::TearDown()
{
   if (gl_ext_string)
      delete [] gl_ext_string;
   if (glx_ext_string)
      delete [] glx_ext_string;
   if (gl_versions)
      delete [] gl_versions;
}

void
glX_send_client_info_test::create_single_screen_display(unsigned major,
							unsigned minor,
							const char *glx_ext)
{
   this->display = new mock_XDisplay(1);

   this->glx_dpy = new fake_glx_display(this->display, major, minor);
   this->glx_dpy->init_screen(0, glx_ext);
}

void
glX_send_client_info_test::common_protocol_expected_false_test(unsigned major,
							       unsigned minor,
							       const char *glx_ext,
							       bool *value)
{
   create_single_screen_display(major, minor, glx_ext);
   glxSendClientInfo(this->glx_dpy, -1);
   EXPECT_FALSE(*value);
}

void
glX_send_client_info_test::common_protocol_expected_true_test(unsigned major,
							      unsigned minor,
							      const char *glx_ext,
							      bool *value)
{
   create_single_screen_display(major, minor, glx_ext);
   glxSendClientInfo(this->glx_dpy, -1);
   EXPECT_TRUE(*value);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_3)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that only has GLX 1.3 regardless of the extension
    * setting.
    */
   common_protocol_expected_false_test(1, 3,
				       "GLX_ARB_create_context",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_1)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that only has GLX 1.3 regardless of the extension
    * setting.
    */
   common_protocol_expected_false_test(1, 3,
				       "GLX_ARB_create_context",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_4_with_empty_extensions)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 but has an empty extension string
    * (i.e., no extensions at all).
    */
   common_protocol_expected_false_test(1, 4,
				       "",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_4_without_extension)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 but doesn't have the extension.
    */
   common_protocol_expected_false_test(1, 4,
				       "GLX_EXT_texture_from_pixmap",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_4_with_wrong_extension)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 but does not have the extension.
    *
    * This test differs from
    * doesnt_send_SetClientInfoARB_for_1_4_without_extension in that an
    * extension exists that looks like the correct extension but isn't.
    */
   common_protocol_expected_false_test(1, 4,
				       "GLX_ARB_create_context2",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfoARB_for_1_4_with_profile_extension)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that no glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 but does not have the extension.
    *
    * This test differs from
    * doesnt_send_SetClientInfoARB_for_1_4_without_extension in that an
    * extension exists that looks like the correct extension but isn't.
    */
   common_protocol_expected_false_test(1, 4,
				       "GLX_ARB_create_context_profile",
				       &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfo2ARB_for_1_3)
{
   /* The glXSetClientInfo2ARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context_profile extension.  Verify that no
    * glXSetClientInfo2ARB is sent to a GLX server that only has GLX 1.3
    * regardless of the extension setting.
    */
   common_protocol_expected_false_test(1, 3,
				       "GLX_ARB_create_context_profile",
				       &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfo2ARB_for_1_1)
{
   /* The glXSetClientInfo2ARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context_profile extension.  Verify that no
    * glXSetClientInfo2ARB is sent to a GLX server that only has GLX 1.1
    * regardless of the extension setting.
    */
   common_protocol_expected_false_test(1, 1,
				       "GLX_ARB_create_context_profile",
				       &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfo2ARB_for_1_4_with_empty_extensions)
{
   /* The glXSetClientInfo2ARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context_profile extension.  Verify that no
    * glXSetClientInfo2ARB is sent to a GLX server that has GLX 1.4 but has an
    * empty extension string (i.e., no extensions at all).
    */
   common_protocol_expected_false_test(1, 4,
				       "",
				       &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfo2ARB_for_1_4_without_extension)
{
   /* The glXSetClientInfo2ARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context_profile extension.  Verify that no
    * glXSetClientInfo2ARB is sent to a GLX server that has GLX 1.4 but
    * doesn't have the extension.
    */
   common_protocol_expected_false_test(1, 4,
				       "GLX_EXT_texture_from_pixmap",
				       &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, doesnt_send_SetClientInfo2ARB_for_1_4_with_wrong_extension)
{
   /* The glXSetClientInfo2ARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context_profile extension.  Verify that no
    * glXSetClientInfo2ARB is sent to a GLX server that has GLX 1.4 but does
    * not have the extension.
    *
    * This test differs from
    * doesnt_send_SetClientInfo2ARB_for_1_4_without_extension in that an
    * extension exists that looks like the correct extension but isn't.
    */
   common_protocol_expected_false_test(1, 4,
				       "GLX_ARB_create_context_profile2",
				       &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, does_send_ClientInfo_for_1_1)
{
   /* The glXClientInfo protocol was added in GLX 1.1.  Verify that
    * glXClientInfo is sent to a GLX server that has GLX 1.1.
    */
   common_protocol_expected_true_test(1, 1,
				      "",
				      &ClientInfo_was_sent);
}

TEST_F(glX_send_client_info_test, does_send_SetClientInfoARB_for_1_4_with_extension)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 and the extension.
    */
   common_protocol_expected_true_test(1, 4,
				      "GLX_ARB_create_context",
				      &SetClientInfoARB_was_sent);
}

TEST_F(glX_send_client_info_test, does_send_SetClientInfo2ARB_for_1_4_with_just_profile_extension)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 and the extension.
    */
   common_protocol_expected_true_test(1, 4,
				      "GLX_ARB_create_context_profile",
				      &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, does_send_SetClientInfo2ARB_for_1_4_with_both_extensions)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 and the extension.
    */
   common_protocol_expected_true_test(1, 4,
				      "GLX_ARB_create_context "
				      "GLX_ARB_create_context_profile",
				      &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, does_send_SetClientInfo2ARB_for_1_4_with_both_extensions_reversed)
{
   /* The glXSetClientInfoARB protocol was added in GLX 1.4 with the
    * GLX_ARB_create_context extension.  Verify that glXSetClientInfoARB is
    * sent to a GLX server that has GLX 1.4 and the extension.
    */
   common_protocol_expected_true_test(1, 4,
				      "GLX_ARB_create_context_profile "
				      "GLX_ARB_create_context",
				      &SetClientInfo2ARB_was_sent);
}

TEST_F(glX_send_client_info_test, uses_correct_connection)
{
   create_single_screen_display(1, 1, "");
   glxSendClientInfo(this->glx_dpy, -1);
   EXPECT_EQ((xcb_connection_t *) 0xdeadbeef, connection_used);
}

TEST_F(glX_send_client_info_test, sends_correct_gl_extension_string)
{
   create_single_screen_display(1, 1, "");
   glxSendClientInfo(this->glx_dpy, -1);

   ASSERT_EQ((int) sizeof(ext), gl_ext_length);
   ASSERT_NE((char *) 0, gl_ext_string);
   EXPECT_EQ(0, memcmp(gl_ext_string, ext, sizeof(ext)));
}

TEST_F(glX_send_client_info_test, gl_versions_are_sane)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context");
   glxSendClientInfo(this->glx_dpy, -1);

   ASSERT_NE(0, num_gl_versions);

   unsigned versions_below_3_0 = 0;
   for (int i = 0; i < num_gl_versions; i++) {
      EXPECT_LT(0u, gl_versions[i * 2]);
      EXPECT_GE(4u, gl_versions[i * 2]);

      /* Verify that the minor version advertised with the major version makes
       * sense.
       */
      switch (gl_versions[i * 2]) {
      case 1:
	 EXPECT_GE(5u, gl_versions[i * 2 + 1]);
	 versions_below_3_0++;
	 break;
      case 2:
	 EXPECT_GE(1u, gl_versions[i * 2 + 1]);
	 versions_below_3_0++;
	 break;
      case 3:
	 EXPECT_GE(3u, gl_versions[i * 2 + 1]);
	 break;
      case 4:
	 EXPECT_GE(2u, gl_versions[i * 2 + 1]);
	 break;
      }
   }

   /* From the GLX_ARB_create_context spec:
    *
    *     "Only the highest supported version below 3.0 should be sent, since
    *     OpenGL 2.1 is backwards compatible with all earlier versions."
    */
   EXPECT_LE(versions_below_3_0, 1u);
}

TEST_F(glX_send_client_info_test, gl_versions_and_profiles_are_sane)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context_profile");
   glxSendClientInfo(this->glx_dpy, -1);

   ASSERT_NE(0, num_gl_versions);

   const uint32_t all_valid_bits = GLX_CONTEXT_CORE_PROFILE_BIT_ARB
      | GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
   const uint32_t es_bit = GLX_CONTEXT_ES2_PROFILE_BIT_EXT;

   unsigned versions_below_3_0 = 0;

   for (int i = 0; i < num_gl_versions; i++) {
      EXPECT_LT(0u, gl_versions[i * 3]);
      EXPECT_GE(4u, gl_versions[i * 3]);

      /* Verify that the minor version advertised with the major version makes
       * sense.
       */
      switch (gl_versions[i * 3]) {
      case 1:
         if (gl_versions[i * 3 + 2] & es_bit) {
            EXPECT_GE(1u, gl_versions[i * 3 + 1]);
            EXPECT_EQ(es_bit, gl_versions[i * 3 + 2]);
         } else {
            EXPECT_GE(5u, gl_versions[i * 3 + 1]);
            EXPECT_EQ(0u, gl_versions[i * 3 + 2]);
            versions_below_3_0++;
         }
	 break;
      case 2:
         if (gl_versions[i * 3 + 2] & es_bit) {
            EXPECT_EQ(0u, gl_versions[i * 3 + 1]);
            EXPECT_EQ(es_bit, gl_versions[i * 3 + 2]);
         } else {
            EXPECT_GE(1u, gl_versions[i * 3 + 1]);
            EXPECT_EQ(0u, gl_versions[i * 3 + 2]);
            versions_below_3_0++;
         }
	 break;
      case 3:
	 EXPECT_GE(3u, gl_versions[i * 3 + 1]);

	 /* Profiles were not introduced until OpenGL 3.2.
	  */
	 if (gl_versions[i * 3 + 1] < 2) {
	    EXPECT_EQ(0u, gl_versions[i * 3 + 2] & ~(es_bit));
	 } else if (gl_versions[i * 3 + 1] == 2) {
	    EXPECT_EQ(0u, gl_versions[i * 3 + 2] & ~(all_valid_bits | es_bit));
	 } else {
	    EXPECT_EQ(0u, gl_versions[i * 3 + 2] & ~(all_valid_bits));
         }
	 break;
      case 4:
	 EXPECT_GE(6u, gl_versions[i * 3 + 1]);
	 EXPECT_EQ(0u, gl_versions[i * 3 + 2] & ~(all_valid_bits));
	 break;
      }
   }

   /* From the GLX_ARB_create_context_profile spec:
    *
    *     "Only the highest supported version below 3.0 should be sent, since
    *     OpenGL 2.1 is backwards compatible with all earlier versions."
    */
   EXPECT_LE(versions_below_3_0, 1u);
}

TEST_F(glX_send_client_info_test, glx_version_is_1_4_for_1_1)
{
   create_single_screen_display(1, 1, "");
   glxSendClientInfo(this->glx_dpy, -1);

   EXPECT_EQ(1, glx_major);
   EXPECT_EQ(4, glx_minor);
}

TEST_F(glX_send_client_info_test, glx_version_is_1_4_for_1_4)
{
   create_single_screen_display(1, 4, "");
   glxSendClientInfo(this->glx_dpy, -1);

   EXPECT_EQ(1, glx_major);
   EXPECT_EQ(4, glx_minor);
}

TEST_F(glX_send_client_info_test, glx_version_is_1_4_for_1_4_with_ARB_create_context)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context");
   glxSendClientInfo(this->glx_dpy, -1);

   EXPECT_EQ(1, glx_major);
   EXPECT_EQ(4, glx_minor);
}

TEST_F(glX_send_client_info_test, glx_version_is_1_4_for_1_4_with_ARB_create_context_profile)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context_profile");
   glxSendClientInfo(this->glx_dpy, -1);

   EXPECT_EQ(1, glx_major);
   EXPECT_EQ(4, glx_minor);
}

TEST_F(glX_send_client_info_test, glx_version_is_1_4_for_1_5)
{
   create_single_screen_display(1, 5, "");
   glxSendClientInfo(this->glx_dpy, -1);

   EXPECT_EQ(1, glx_major);
   EXPECT_EQ(4, glx_minor);
}

TEST_F(glX_send_client_info_test, glx_extensions_has_GLX_ARB_create_context)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context");
   glxSendClientInfo(this->glx_dpy, -1);

   ASSERT_NE(0, glx_ext_length);
   ASSERT_NE((char *) 0, glx_ext_string);

   bool found_GLX_ARB_create_context = false;
   const char *const needle = "GLX_ARB_create_context";
   const unsigned len = strlen(needle);
   char *haystack = glx_ext_string;
   while (haystack != NULL) {
      char *match = strstr(haystack, needle);

      if (match[len] == '\0' || match[len] == ' ') {
	 found_GLX_ARB_create_context = true;
	 break;
      }

      haystack = match + len;
   }

   EXPECT_TRUE(found_GLX_ARB_create_context);
}

TEST_F(glX_send_client_info_test, glx_extensions_has_GLX_ARB_create_context_profile)
{
   create_single_screen_display(1, 4, "GLX_ARB_create_context_profile");
   glxSendClientInfo(this->glx_dpy, -1);

   ASSERT_NE(0, glx_ext_length);
   ASSERT_NE((char *) 0, glx_ext_string);

   bool found_GLX_ARB_create_context_profile = false;
   const char *const needle = "GLX_ARB_create_context_profile";
   const unsigned len = strlen(needle);
   char *haystack = glx_ext_string;
   while (haystack != NULL) {
      char *match = strstr(haystack, needle);

      if (match[len] == '\0' || match[len] == ' ') {
	 found_GLX_ARB_create_context_profile = true;
	 break;
      }

      haystack = match + len;
   }

   EXPECT_TRUE(found_GLX_ARB_create_context_profile);
}
