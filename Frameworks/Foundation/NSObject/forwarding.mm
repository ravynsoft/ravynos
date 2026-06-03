#import <runtime/objc-private.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSInvocation.h>
#include <stdio.h>

#define NSABISizeofRegisterReturn 8
#define NSABIasm_jmp_objc_msgSend __asm__("jmp _objc_msgSend")
#define NSABIasm_jmp_objc_msgSend_stret __asm__("jmp _objc_msgSend_stret")

static void OBJCRaiseException(const char *name,const char *format,...) {
    va_list arguments;
    
    va_start(arguments,format);
    
    fprintf(stderr,"ObjC:%s:",name);
    vfprintf(stderr,format,arguments);
    fprintf(stderr,"\n");
    fflush(stderr);
    va_end(arguments);
}


#if !FOUNDATION_DISALLOW_FORWARDING
@interface NSObject(fastforwarding)
-forwardingTargetForSelector:(SEL)selector;
@end

@interface NSInvocation(_private)
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
#endif

void NSObjCForwardInvocation(void *returnValue,id object,SEL selector,va_list arguments){
   NSMethodSignature *signature=[object methodSignatureForSelector:selector];

   if(signature==nil)
    [object doesNotRecognizeSelector:selector];
   else {
#if FOUNDATION_DISALLOW_FORWARDING
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

//#define objc_msgForward _objc_msgForward


void objc_msgForward_stret(void *result, id object, SEL message, ...)
{
}

#ifdef GCC_RUNTIME_3
// TODO Forwarding currently only works for methods returning types not wider than id. The result
// is cropped to min(sizeof(returntype_of_forward_imp), sizeof(returntype_of_selector || id)).
// TODO Struct returning methods have to be handled separatly.
IMP objc_msg_forward(id rcv, SEL message)
{
    return objc_msgForward;
}
#endif
