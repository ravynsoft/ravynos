// TEST_CONFIG

#include "test.h"
#include "testroot.i"

@interface One : TestRoot @end
@implementation One @end

@interface Two : TestRoot @end
@implementation Two @end

@interface Both : TestRoot @end
@implementation Both @end

@interface None : TestRoot @end
@implementation None @end


objc_hook_getImageName OnePreviousHook;
BOOL GetImageNameHookOne(Class cls, const char **outName)
{
    if (0 == strcmp(class_getName(cls), "One")) {
        *outName = "Image One";
        return YES;
    } else if (0 == strcmp(class_getName(cls), "Both")) {
        *outName = "Image Both via One";
        return YES;
    } else {
        return OnePreviousHook(cls, outName);
    }
}

objc_hook_getImageName TwoPreviousHook;
BOOL GetImageNameHookTwo(Class cls, const char **outName)
{
    if (0 == strcmp(class_getName(cls), "Two")) {
        *outName = "Image Two";
        return YES;
    } else if (0 == strcmp(class_getName(cls), "Both")) {
        *outName = "Image Both via Two";
        return YES;
    } else {
        return TwoPreviousHook(cls, outName);
    }
}

int main()
{

    // before hooks: main executable is the image name for four classes
    testassert(strstr(class_getImageName([One class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([Two class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([Both class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([None class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([NSObject class]), "libobjc"));

    // install hook One
    objc_setHook_getImageName(GetImageNameHookOne, &OnePreviousHook);

    // two classes are in Image One with hook One in place
    testassert(strstr(class_getImageName([One class]), "Image One"));
    testassert(strstr(class_getImageName([Two class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([Both class]), "Image Both via One"));
    testassert(strstr(class_getImageName([None class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([NSObject class]), "libobjc"));

    // install hook Two which chains to One
    objc_setHook_getImageName(GetImageNameHookTwo, &TwoPreviousHook);

    // two classes are in Image Two and one in One with both hooks in place
    testassert(strstr(class_getImageName([One class]), "Image One"));
    testassert(strstr(class_getImageName([Two class]), "Image Two"));
    testassert(strstr(class_getImageName([Both class]), "Image Both via Two"));
    testassert(strstr(class_getImageName([None class]), "getImageNameHook"));
    testassert(strstr(class_getImageName([NSObject class]), "libobjc"));

    succeed(__FILE__);
}
