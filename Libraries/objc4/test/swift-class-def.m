#include <sys/cdefs.h>

#if __LP64__
#   define PTR " .quad " 
#   define PTRSIZE "8"
#   define LOGPTRSIZE "3"
#   define ONLY_LP64(x) x
#else
#   define PTR " .long " 
#   define PTRSIZE "4"
#   define LOGPTRSIZE "2"
#   define ONLY_LP64(x)
#endif

#if __has_feature(ptrauth_calls)
#   define SIGNED_METHOD_LIST_IMP "@AUTH(ia,0,addr) "
#   define SIGNED_STUB_INITIALIZER "@AUTH(ia,0xc671,addr) "
#else
#   define SIGNED_METHOD_LIST_IMP
#   define SIGNED_STUB_INITIALIZER
#endif

#define str(x) #x
#define str2(x) str(x)

// Swift metadata initializers. Define these in the test.
EXTERN_C Class initSuper(Class cls, void *arg);
EXTERN_C Class initSub(Class cls, void *arg);

@interface SwiftSuper : NSObject @end
@interface SwiftSub : SwiftSuper @end

__BEGIN_DECLS
// not id to avoid ARC operations because the class doesn't implement RR methods
void* nop(void* self) { return self; }
__END_DECLS

#define SWIFT_CLASS(name, superclass, swiftInit) \
asm(                                               \
    ".globl _OBJC_CLASS_$_" #name             "\n" \
    ".section __DATA,__objc_data               \n" \
    ".align 3                                  \n" \
    "_OBJC_CLASS_$_" #name ":                  \n" \
    PTR "_OBJC_METACLASS_$_" #name            "\n" \
    PTR "_OBJC_CLASS_$_" #superclass          "\n" \
    PTR "__objc_empty_cache                    \n" \
    PTR "0 \n"                                     \
    PTR "L_" #name "_ro + 2 \n"                    \
    /* Swift class fields. */                      \
    ".long 0 \n"   /* flags */                     \
    ".long 0 \n"   /* instanceAddressOffset */     \
    ".long 16 \n"  /* instanceSize */              \
    ".short 15 \n" /* instanceAlignMask */         \
    ".short 0 \n"  /* reserved */                  \
    ".long 256 \n" /* classSize */                 \
    ".long 0 \n"   /* classAddressOffset */        \
    PTR "0 \n"     /* description */               \
    /* pad to OBJC_MAX_CLASS_SIZE */               \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
                                                   \
    "_OBJC_METACLASS_$_" #name ":              \n" \
    PTR "_OBJC_METACLASS_$_" #superclass      "\n" \
    PTR "_OBJC_METACLASS_$_" #superclass      "\n" \
    PTR "__objc_empty_cache                    \n" \
    PTR "0 \n"                                     \
    PTR "L_" #name "_meta_ro \n"                   \
    /* pad to OBJC_MAX_CLASS_SIZE */               \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
                                                   \
    "L_" #name "_ro: \n"                           \
    ".long (1<<6) \n"                              \
    ".long 0 \n"                                   \
    ".long " PTRSIZE " \n"                         \
    ONLY_LP64(".long 0 \n")                        \
    PTR "0 \n"                                     \
    PTR "L_" #name "_name \n"                      \
    PTR "L_" #name "_methods \n"                   \
    PTR "0 \n"                                     \
    PTR "L_" #name "_ivars \n"                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "_" #swiftInit SIGNED_METHOD_LIST_IMP "\n" \
                                                   \
    "L_" #name "_meta_ro: \n"                      \
    ".long 1 \n"                                   \
    ".long 40 \n"                                  \
    ".long 40 \n"                                  \
    ONLY_LP64(".long 0 \n")                        \
    PTR "0 \n"                                     \
    PTR "L_" #name "_name \n"                      \
    PTR "L_" #name "_meta_methods \n"              \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
    PTR "0 \n"                                     \
                                                   \
    "L_" #name "_methods: \n"                      \
    "L_" #name "_meta_methods: \n"                 \
    ".long 3*" PTRSIZE "\n"                        \
    ".long 1 \n"                                   \
    PTR "L_" #name "_self \n"                      \
    PTR "L_" #name "_self \n"                      \
    PTR "_nop" SIGNED_METHOD_LIST_IMP "\n"         \
                                                   \
    "L_" #name "_ivars: \n"                        \
    ".long 4*" PTRSIZE " \n"                       \
    ".long 1 \n"                                   \
    PTR "L_" #name "_ivar_offset \n"               \
    PTR "L_" #name "_ivar_name \n"                 \
    PTR "L_" #name "_ivar_type \n"                 \
    ".long " LOGPTRSIZE "\n"                       \
    ".long " PTRSIZE "\n"                          \
                                                   \
    "L_" #name "_ivar_offset: \n"                  \
    ".long 0 \n"                                   \
                                                   \
    ".cstring \n"                                  \
    "L_" #name "_name: .ascii \"" #name "\\0\" \n" \
    "L_" #name "_self: .ascii \"self\\0\" \n"      \
    "L_" #name "_ivar_name: "                      \
    "  .ascii \"" #name "_ivar\\0\" \n"            \
    "L_" #name "_ivar_type: .ascii \"c\\0\" \n"    \
                                                   \
                                                   \
    ".text \n"                                     \
);                                                 \
extern char OBJC_CLASS_$_ ## name;                 \
Class Raw ## name = (Class)&OBJC_CLASS_$_ ## name

#define SWIFT_STUB_CLASSREF(name)  \
extern char OBJC_CLASS_$_ ## name; \
static Class name ## Classref = (Class)(&OBJC_CLASS_$_ ## name + 1)

#define SWIFT_STUB_CLASS(name, initializer)        \
asm(                                               \
    ".globl _OBJC_CLASS_$_" #name "\n"             \
    ".section __DATA,__objc_data \n"               \
    ".align 3 \n"                                  \
    "_dummy" #name ": \n"                          \
    PTR "0 \n"                                     \
    ".alt_entry _OBJC_CLASS_$_" #name "\n"         \
    "_OBJC_CLASS_$_" #name ": \n"                  \
    PTR "1 \n"                                     \
    PTR "_" #initializer SIGNED_STUB_INITIALIZER "\n" \
    ".text"                                        \
);                                                 \
extern char OBJC_CLASS_$_ ## name;                 \
Class Raw ## name = (Class)&OBJC_CLASS_$_ ## name; \
SWIFT_STUB_CLASSREF(name)
    

void fn(void) { }
