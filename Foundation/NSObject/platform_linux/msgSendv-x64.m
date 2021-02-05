#import <Foundation/NSObjCRuntime.h>
#import <objc/runtime.h>

id objc_msgSendv(id self, SEL selector, NSUInteger arg_size, void *arg_frame)
{
    IMP imp = class_getMethodImplementation(object_getClass(self), selector);
    void *result = __builtin_apply((void(*)())imp, arg_frame, (int)arg_size);
    __builtin_return(result);
}
