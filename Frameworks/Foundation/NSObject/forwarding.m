#import <objc/runtime.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSInvocation.h>
#include <stdio.h>

#define NSABISizeofRegisterReturn 8
#ifndef GCC_RUNTIME_3
#define NSABIasm_jmp_objc_msgSend __asm__("jmp _objc_msgSend")
#define NSABIasm_jmp_objc_msgSend_stret __asm__("jmp _objc_msgSend_stret")
#endif
// 64-bit freebsd, FIX
// #define NSABIasm_jmp_objc_msgSend __asm__("jmp _objc_msgSend@PLT")
// #define NSABIasm_jmp_objc_msgSend_stret __asm__("jmp _objc_msgSend_stret@PLT");

static void OBJCRaiseException(const char *name,const char *format,...) {
    va_list arguments;
    
    va_start(arguments,format);
    
    fprintf(stderr,"ObjC:%s:",name);
    vfprintf(stderr,format,arguments);
    fprintf(stderr,"\n");
    fflush(stderr);
    va_end(arguments);
}


#if !COCOTRON_DISALLOW_FORWARDING
@interface NSObject(fastforwarding)
-forwardingTargetForSelector:(SEL)selector;
@end

@interface NSInvocation(private)
+(NSInvocation *)invocationWithMethodSignature:(NSMethodSignature *)signature arguments:(void *)arguments;
@end

#ifndef GCC_RUNTIME_3
id NSObjCGetFastForwardTarget(id object,SEL selector){
   id check=nil;

   if([object respondsToSelector:@selector(forwardingTargetForSelector:)])
    if((check=[object forwardingTargetForSelector:selector])==object)
     check=nil;

   return check;
}
#endif

void NSObjCForwardInvocation(void *returnValue,id object,SEL selector,va_list arguments){
   NSMethodSignature *signature=[object methodSignatureForSelector:selector];

   if(signature==nil)
    [object doesNotRecognizeSelector:selector];
   else {
#if COCOTRON_DISALLOW_FORWARDING
    Class class = object_getClass(object);
    OBJCRaiseException("ForwardingDisallowed", "%c[%s %s(%d)]", class_isMetaClass(class) ? '+' : '-', class_getName(class) , sel_getName(selector), selector);
#else
    NSInvocation *invocation=[NSInvocation invocationWithMethodSignature:signature arguments:arguments];
#endif

    [object forwardInvocation:invocation];
    [invocation getReturnValue:returnValue];
   }
}

void NSObjCForward(id object,SEL selector,...){
#ifndef GCC_RUNTIME_3
   id check=NSObjCGetFastForwardTarget(object,selector);

   if(check!=nil){
    object=check;
    NSABIasm_jmp_objc_msgSend;
   }
#endif

   uint8_t returnValue[NSABISizeofRegisterReturn];

   va_list arguments;

   va_start(arguments,selector);

   NSObjCForwardInvocation(returnValue,object,selector,arguments);

   va_end(arguments);
}

void NSObjCForward_stret(void *returnValue,id object,SEL selector,...){
#ifndef GCC_RUNTIME_3
   id check=NSObjCGetFastForwardTarget(object,selector);

   if(check!=nil){
    object=check;
    NSABIasm_jmp_objc_msgSend_stret;
   }
#endif

   va_list arguments;

   va_start(arguments,selector);

   NSObjCForwardInvocation(returnValue,object,selector,arguments);

   va_end(arguments);
}
#endif


// both of these suck, we should be using NSMethodSignature types to extract the frame and create the NSInvocation here
#ifdef __sparc__
id objc_msgForward(id object, SEL message, ...)
{
    Class class = object_getClass(object);

    struct objc_method *method;
    va_list arguments;
    unsigned i, frameLength, limit;
    unsigned *frame;

    if ((method = class_getInstanceMethod(class, @selector(_frameLengthForSelector:))) == NULL) {
        OBJCRaiseException("OBJCDoesNotRecognizeSelector", "%c[%s %s(%d)]", class_isMetaClass(class) ? '+' : '-', class_getName(class) , sel_getName(message), message);
        return nil;
    }
    IMP imp = method_getImplementation(method);
    frameLength = imp(object, @selector(_frameLengthForSelector:), message);
    frame = __builtin_alloca(2 * sizeof(unsigned) + frameLength);
    va_start(arguments, message);
    frame[0] = object;
    frame[1] = message;
    for (i = 0; i < frameLength / sizeof(unsigned); i++) {
        frame[i+2] = va_arg(arguments, unsigned);
    }

    if ((method = class_getInstanceMethod(class, @selector(forwardSelector:arguments:))) != NULL) {
        imp = method_getImplementation(method);

        return imp(object, @selector(forwardSelector:arguments:), message, frame);
    } else {
        OBJCRaiseException("OBJCDoesNotRecognizeSelector", "%c[%s %s(%d)]", class_isMetaClass(class) ? '+' : '-', class_getName(class), sel_getName(message), message);
        return nil;
    }
}

void objc_msgForward_stret(void *result, id object, SEL message, ...)
{
}

#else

id objc_msgForward(id object, SEL message, ...)
{
    Class class = object_getClass(object);
    struct objc_slot *slot;
    void *arguments = &object;

    if ((slot = objc_get_slot(class, @selector(forwardSelector:arguments:))) != NULL) {
        // handle nil receiver when forwarding
        if(slot->owner == nil)
            return objc_msg_lookup_sender(object,@selector(forwardSelector:arguments:),object);

        IMP imp = method_getImplementation(slot->method);

        return imp(object, @selector(forwardSelector:arguments:), message, arguments);
    } else {
        OBJCRaiseException("OBJCDoesNotRecognizeSelector", "%c[%s %s(%d)]", class_isMetaClass(class) ? '+' : '-', class_getName(class), sel_getName(message), message);
        return nil;
    }
}


void objc_msgForward_stret(void *result, id object, SEL message, ...)
{
}

#endif

#ifdef GCC_RUNTIME_3
// TODO Forwarding currently only works for methods returning types not wider than id. The result
// is cropped to min(sizeof(returntype_of_forward_imp), sizeof(returntype_of_selector || id)).
// TODO Struct returning methods have to be handled separatly.
IMP objc_msg_forward(id rcv, SEL message)
{
    return objc_msgForward;
}
#endif
