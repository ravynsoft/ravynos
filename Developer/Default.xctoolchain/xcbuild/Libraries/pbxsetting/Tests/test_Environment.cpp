
#include <gtest/gtest.h>
#include <pbxsetting/Environment.h>

using pbxsetting::Environment;
using pbxsetting::Level;
using pbxsetting::Setting;
using pbxsetting::Value;

TEST(Environment, Layering)
{
    Environment layered;
    layered.insertBack(Level({
        Setting::Parse("LAYERED", "command line, $(LAYERED)"),
    }), false);
    layered.insertBack(Level({
        Setting::Parse("LAYERED", "target, $(LAYERED)"),
    }), false);
    layered.insertBack(Level({
        Setting::Parse("LAYERED", "project, $(LAYERED)"),
    }), false);
    layered.insertBack(Level({
        Setting::Parse("LAYERED", "environment"),
    }), false);
    EXPECT_EQ(layered.resolve("LAYERED"), "command line, target, project, environment");
}

TEST(Environment, Staggered)
{
    Environment staggered;
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "command line, $(LAYERED)"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("STAGGERED", "$(CAPTION): $(LAYERED)"),
        Setting::Parse("LAYERED", "target, $(LAYERED)"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "project, $(LAYERED)"),
        Setting::Parse("CAPTION", "evaluation order"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "environment"),
    }), false);
    EXPECT_EQ(staggered.resolve("STAGGERED"), "evaluation order: command line, target, project, environment");
}

TEST(Environment, StaggeredOverride)
{
    Environment staggered;
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "command line, $(LAYERED)"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("STAGGERED", "$(CAPTION): $(LAYERED)"),
        Setting::Parse("LAYERED", "target, $(LAYERED)"),
        Setting::Parse("CAPTION", "order of evaluation"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "project, $(LAYERED)"),
        Setting::Parse("CAPTION", "evaluation order"),
    }), false);
    staggered.insertBack(Level({
        Setting::Parse("LAYERED", "environment"),
    }), false);
    EXPECT_EQ(staggered.resolve("STAGGERED"), "order of evaluation: command line, target, project, environment");
}

TEST(Environment, Concatenation)
{
    Environment concat;
    concat.insertBack(Level({
        Setting::Parse("CURRENT_PROJECT_VERSION_app", "15.3.9"),
        Setting::Parse("CURRENT_PROJECT_VERSION_xctest", "1.0.0"),
        Setting::Parse("CURRENT_PROJECT_VERSION", "$(CURRENT_PROJECT_VERSION_$(WRAPPER_EXTENSION))"),
    }), false);
    concat.insertBack(Level({
        Setting::Parse("WRAPPER_EXTENSION", "app"),
    }), false);
    EXPECT_EQ(concat.resolve("CURRENT_PROJECT_VERSION"), "15.3.9");
}

TEST(Environment, Inherited)
{
    Environment inherited;
    inherited.insertBack(Level({
        Setting::Parse("OTHER_LDFLAGS", "$(inherited) -framework Security"),
    }), false);
    inherited.insertBack(Level({
        Setting::Parse("OTHER_LDFLAGS", "-ObjC"),
    }), false);
    EXPECT_EQ(inherited.resolve("OTHER_LDFLAGS"), "-ObjC -framework Security");
}

TEST(Environment, InheritedWithLevelInFront)
{
    Environment inherited;
    inherited.insertBack(Level({ }), false);
    inherited.insertBack(Level({
        Setting::Parse("OTHER_LDFLAGS", "$(inherited) -framework Security"),
    }), false);
    inherited.insertBack(Level({
        Setting::Parse("OTHER_LDFLAGS", "-ObjC"),
    }), false);
    EXPECT_EQ(inherited.resolve("OTHER_LDFLAGS"), "-ObjC -framework Security");
}

TEST(Environment, Operations)
{
    Environment environment;
    environment.insertBack(Level({
        Setting::Parse("IDENTIFIER", "$(COMPLEX:identifier)"),
        Setting::Parse("C99IDENTIFIER", "$(COMPLEX:c99extidentifier)"),
        Setting::Parse("RFC1034IDENTIFIER", "$(COMPLEX:rfc1034identifier)"),
        Setting::Parse("QUOTE", "$(COMPLEX:quote)"),
        Setting::Parse("LOWER", "$(BASIC:lower)"),
        Setting::Parse("UPPER", "$(BASIC:upper)"),
        Setting::Parse("BASE", "$(PATH:base)"),
        Setting::Parse("DIR", "$(PATH:dir)"),
        Setting::Parse("FILE", "$(PATH:file)"),
        Setting::Parse("SUFFIX", "$(PATH:suffix)"),
        Setting::Parse("MULTIPLE", "$(COMPLEX:identifier:upper)"),
    }), false);
    environment.insertBack(Level({
        Setting::Parse("BASIC", "Hello, world."),
        Setting::Parse("COMPLEX", "-_'hello%."),
        Setting::Parse("PATH", "/path/to/../file.ext"),
    }), false);
    EXPECT_EQ(environment.resolve("IDENTIFIER"), "___hello__");
    EXPECT_EQ(environment.resolve("C99IDENTIFIER"), "___hello__");
    EXPECT_EQ(environment.resolve("RFC1034IDENTIFIER"), "---hello--");
    EXPECT_EQ(environment.resolve("QUOTE"), "'-_'\"'\"'hello%.'");
    EXPECT_EQ(environment.resolve("LOWER"), "hello, world.");
    EXPECT_EQ(environment.resolve("UPPER"), "HELLO, WORLD.");
    EXPECT_EQ(environment.resolve("BASE"), "file");
    EXPECT_EQ(environment.resolve("DIR"), "/path/to/..");
    EXPECT_EQ(environment.resolve("FILE"), "file.ext");
    EXPECT_EQ(environment.resolve("SUFFIX"), ".ext");
    EXPECT_EQ(environment.resolve("MULTIPLE"), "___HELLO__");
}

TEST(Environment, Value)
{
    Environment env;
    env.insertBack(Level({
        Setting::Parse("ONE", "one"),
        Setting::Parse("TWO", "two"),
    }), false);
    EXPECT_EQ(env.expand(Value::Parse("$(ONE)-$(TWO)")), "one-two");
}

TEST(Environment, Default)
{
    Environment env;
    env.insertBack(Level({
        Setting::Parse("ONE", "one"),
        Setting::Parse("TWO", "two"),
    }), true);
    env.insertFront(Level({
        Setting::Parse("ONE", "1"),
        Setting::Parse("THREE", "three"),
    }), true);
    env.insertBack(Level({
        Setting::Parse("ONE", "one1"),
        Setting::Parse("THREE", "3"),
    }), false);
    env.insertFront(Level({
        Setting::Parse("ONE", "1one"),
    }), false);
    EXPECT_EQ(env.resolve("ONE"), "1one");
    EXPECT_EQ(env.resolve("TWO"), "two");
    EXPECT_EQ(env.resolve("THREE"), "3");
}

