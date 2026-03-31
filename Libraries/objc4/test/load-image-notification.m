/*
TEST_BUILD
  $C{COMPILE} -DCLASSNAME=Class1 $DIR/load-image-notification-dylib.m -o load-image-notification1.dylib -dynamiclib
  $C{COMPILE} -DCLASSNAME=Class2 $DIR/load-image-notification-dylib.m -o load-image-notification2.dylib -dynamiclib
  $C{COMPILE} -DCLASSNAME=Class3 $DIR/load-image-notification-dylib.m -o load-image-notification3.dylib -dynamiclib
  $C{COMPILE} -DCLASSNAME=Class4 $DIR/load-image-notification-dylib.m -o load-image-notification4.dylib -dynamiclib
  $C{COMPILE} -DCLASSNAME=Class5 $DIR/load-image-notification-dylib.m -o load-image-notification5.dylib -dynamiclib
  $C{COMPILE} $DIR/load-image-notification.m -o load-image-notification.exe
END
*/

#include "test.h"

#include <dlfcn.h>

#define ADD_IMAGE_CALLBACK(n)                                        \
int called ## n = 0;                                                 \
static void add_image ## n(const struct mach_header * mh __unused) { \
    called ## n++;                                                   \
}

ADD_IMAGE_CALLBACK(1)
ADD_IMAGE_CALLBACK(2)
ADD_IMAGE_CALLBACK(3)
ADD_IMAGE_CALLBACK(4)
ADD_IMAGE_CALLBACK(5)

int main()
{
    objc_addLoadImageFunc(add_image1);
    testassert(called1 > 0);
    int oldcalled = called1;
    void *handle = dlopen("load-image-notification1.dylib", RTLD_LAZY);
    testassert(handle);
    testassert(called1 > oldcalled);
    
    objc_addLoadImageFunc(add_image2);
    testassert(called2 == called1);
    oldcalled = called1;
    handle = dlopen("load-image-notification2.dylib", RTLD_LAZY);
    testassert(handle);
    testassert(called1 > oldcalled);
    testassert(called2 == called1);
    
    objc_addLoadImageFunc(add_image3);
    testassert(called3 == called1);
    oldcalled = called1;
    handle = dlopen("load-image-notification3.dylib", RTLD_LAZY);
    testassert(handle);
    testassert(called1 > oldcalled);
    testassert(called2 == called1);
    testassert(called3 == called1);
    
    objc_addLoadImageFunc(add_image4);
    testassert(called4 == called1);
    oldcalled = called1;
    handle = dlopen("load-image-notification4.dylib", RTLD_LAZY);
    testassert(handle);
    testassert(called1 > oldcalled);
    testassert(called2 == called1);
    testassert(called3 == called1);
    testassert(called4 == called1);
    
    objc_addLoadImageFunc(add_image5);
    testassert(called5 == called1);
    oldcalled = called1;
    handle = dlopen("load-image-notification5.dylib", RTLD_LAZY);
    testassert(handle);
    testassert(called1 > oldcalled);
    testassert(called2 == called1);
    testassert(called3 == called1);
    testassert(called4 == called1);
    testassert(called5 == called1);
    
    succeed(__FILE__);
}
