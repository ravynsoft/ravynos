/*
 * Copyright © 2020 Google LLC
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

#include <gtest/gtest.h>
#include <driconf.h>
#include <xmlconfig.h>

class xmlconfig_test : public ::testing::Test {
protected:
   xmlconfig_test();
   ~xmlconfig_test();

   driOptionCache drirc_init(const char *driver, const char *drm,
                             const char *exec_name,
                             const char *app, int appver,
                             const char *engine, int enginever);

   driOptionCache options;
};

xmlconfig_test::xmlconfig_test()
{
   /* Unset variables from the envrionment to prevent user settings from
    * impacting the tests.
    */
   unsetenv("glsl_zero_init");
   unsetenv("always_have_depth_buffer");
   unsetenv("opt");
   unsetenv("vblank_mode");
   unsetenv("not_present");
   unsetenv("mesa_b_option");
   unsetenv("mesa_s_option");
   unsetenv("mest_test_unknown_option");
   unsetenv("mest_drirc_option");

   options = {};
}

xmlconfig_test::~xmlconfig_test()
{
   driDestroyOptionInfo(&options);
}

/* wraps a DRI_CONF_OPT_* in the required xml bits */
#define DRI_CONF_TEST_OPT(x) x

TEST_F(xmlconfig_test, bools)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_GLSL_ZERO_INIT(false)
      DRI_CONF_ALWAYS_HAVE_DEPTH_BUFFER(true)
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(driQueryOptionb(&options, "glsl_zero_init"), false);
   EXPECT_EQ(driQueryOptionb(&options, "always_have_depth_buffer"), true);
}

TEST_F(xmlconfig_test, ints)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_OPT_I(opt, 2, 0, 999, "option")
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(driQueryOptioni(&options, "opt"), 2);
}

TEST_F(xmlconfig_test, floats)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_OPT_F(opt, 2.0, 1.0, 2.0, "option")
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(driQueryOptionf(&options, "opt"), 2.0);
}

TEST_F(xmlconfig_test, enums)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_VBLANK_MODE(DRI_CONF_VBLANK_DEF_INTERVAL_1)
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(driQueryOptioni(&options, "vblank_mode"), DRI_CONF_VBLANK_DEF_INTERVAL_1);
}

TEST_F(xmlconfig_test, enums_from_env)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_VBLANK_MODE(DRI_CONF_VBLANK_DEF_INTERVAL_1)
   };

   setenv("vblank_mode", "0", 1);
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(0, driQueryOptioni(&options, "vblank_mode"));
}

TEST_F(xmlconfig_test, string)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_OPT_S(opt, value, "option")
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_STREQ(driQueryOptionstr(&options, "opt"), "value");
}

TEST_F(xmlconfig_test, check_option)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_GLSL_ZERO_INIT(true)
      DRI_CONF_ALWAYS_HAVE_DEPTH_BUFFER(true)
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   EXPECT_EQ(driCheckOption(&options, "glsl_zero_init", DRI_BOOL), true);

   EXPECT_EQ(driCheckOption(&options, "glsl_zero_init", DRI_ENUM), false);
   EXPECT_EQ(driCheckOption(&options, "glsl_zero_init", DRI_INT), false);
   EXPECT_EQ(driCheckOption(&options, "glsl_zero_init", DRI_FLOAT), false);
   EXPECT_EQ(driCheckOption(&options, "glsl_zero_init", DRI_STRING), false);

   EXPECT_EQ(driCheckOption(&options, "not_present", DRI_BOOL), false);
}

TEST_F(xmlconfig_test, copy_cache)
{
   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_OPT_B(mesa_b_option, true, "description")
      DRI_CONF_OPT_S(mesa_s_option, value, "description")
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   driOptionCache cache;

   /* This tries to parse user config files.  We've called our option
    * "mesa_test_option" so the test shouldn't end up with something from the
    * user's homedir/environment that would override us.
    */
   driParseConfigFiles(&cache, &options,
                       0, "driver", "drm", NULL,
                       NULL, 0,
                       NULL, 0);

   /* Can we inspect the cache? */
   EXPECT_EQ(driCheckOption(&cache, "mesa_b_option", DRI_BOOL), true);
   EXPECT_EQ(driCheckOption(&cache, "mesa_s_option", DRI_STRING), true);
   EXPECT_EQ(driCheckOption(&cache, "mesa_test_unknown_option", DRI_BOOL), false);

   /* Did the value get copied? */
   EXPECT_EQ(driQueryOptionb(&cache, "mesa_b_option"), true);
   EXPECT_STREQ(driQueryOptionstr(&cache, "mesa_s_option"), "value");

   driDestroyOptionCache(&cache);
}

driOptionCache
xmlconfig_test::drirc_init(const char *driver, const char *drm,
                           const char *exec_name,
                           const char *app, int appver,
                           const char *engine, int enginever)
{
   driInjectExecName(exec_name);

   driOptionDescription driconf[] = {
      DRI_CONF_SECTION_MISCELLANEOUS
      DRI_CONF_OPT_I(mesa_drirc_option, 0, 0, 200, "description")
   };
   driParseOptionInfo(&options, driconf, ARRAY_SIZE(driconf));

   driOptionCache cache;

   /* This should parse the "user" drirc files under ./tests/drirc_test/,
    * based on the setting of $HOME by meson.build.
    */
   driParseConfigFiles(&cache, &options,
                       0, driver, drm, NULL,
                       app, appver,
                       engine, enginever);

   return cache;
}

TEST_F(xmlconfig_test, drirc_app)
{
   driOptionCache cache = drirc_init("driver", "drm",
                                     "app1",
                                     NULL, 0,
                                     NULL, 0);
#if WITH_XMLCONFIG
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 1);
#else
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 0);
#endif
   driDestroyOptionCache(&cache);
}

TEST_F(xmlconfig_test, drirc_user_app)
{
   driOptionCache cache = drirc_init("driver", "drm",
                                     "app3",
                                     NULL, 0,
                                     NULL, 0);
#if WITH_XMLCONFIG
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 10);
#else
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 0);
#endif
   driDestroyOptionCache(&cache);
}

TEST_F(xmlconfig_test, drirc_env_override)
{
   setenv("mesa_drirc_option", "7", 1);
   driOptionCache cache = drirc_init("driver", "drm",
                                     "app1",
                                     NULL, 0,
                                     NULL, 0);
   /* env var takes precedence over config files */
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 7);
   unsetenv("mesa_drirc_option");
   driDestroyOptionCache(&cache);
}

#if WITH_XMLCONFIG
TEST_F(xmlconfig_test, drirc_app_versioned)
{
   driOptionCache cache = drirc_init("driver", "drm",
                                     NULL,
                                     "Versioned App Name", 1,
                                     NULL, 0);
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 3);
   driDestroyOptionCache(&cache);
}

TEST_F(xmlconfig_test, drirc_engine_versioned)
{
   driOptionCache cache = drirc_init("driver", "drm",
                                     NULL,
                                     "unknownapp", 0,
                                     "Versioned Engine Name", 1);
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 5);
   driDestroyOptionCache(&cache);
}

TEST_F(xmlconfig_test, drirc_exec_regexp)
{
   driOptionCache cache = drirc_init("driver", "drm",
                                     "app2v4",
                                     NULL, 0,
                                     NULL, 0);
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 7);
   driDestroyOptionCache(&cache);
}

TEST_F(xmlconfig_test, drirc_exec_override)
{
   putenv("MESA_DRICONF_EXECUTABLE_OVERRIDE=app1");
   driOptionCache cache = drirc_init("driver", "drm",
                                     NULL,
                                     NULL, 0,
                                     NULL, 0);
   EXPECT_EQ(driQueryOptioni(&cache, "mesa_drirc_option"), 1);
   driDestroyOptionCache(&cache);
}
#endif
