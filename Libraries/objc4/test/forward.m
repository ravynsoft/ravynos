// TEST_CONFIG MEM=mrc

#include "test.h"

#include <objc/runtime.h>
#include <objc/message.h>

id ID_RESULT = (id)0x12345678;
long long LL_RESULT = __LONG_LONG_MAX__ - 2LL*__INT_MAX__;
double FP_RESULT = __DBL_MIN__ + __DBL_EPSILON__;
long double LFP_RESULT = __LDBL_MIN__ + __LDBL_EPSILON__;
// STRET_RESULT in test.h


static int state = 0;
static id receiver;

OBJC_ROOT_CLASS
@interface Super { id isa; } @end

@interface Super (Forwarded) 
+(id)idret: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(id)idre2: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(id)idre3: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(long long)llret: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(long long)llre2: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(long long)llre3: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(struct stret)stret: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(struct stret)stre2: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(struct stret)stre3: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(double)fpret: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(double)fpre2: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

+(double)fpre3: 
   (long)i1 :(long)i2 :(long)i3 :(long)i4 :(long)i5 :(long)i6 :(long)i7 :(long)i8 :(long)i9 :(long)i10 :(long)i11 :(long)i12 :(long)i13  :(double)f1 :(double)f2 :(double)f3 :(double)f4 :(double)f5 :(double)f6 :(double)f7 :(double)f8 :(double)f9 :(double)f10 :(double)f11 :(double)f12 :(double)f13 :(double)f14 :(double)f15;

@end


long long forward_handler(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15)
{
#if __arm64__
# if __LP64__
#   define p "x"  // true arm64
# else
#   define p "w"  // arm64_32
# endif
    void *struct_addr;
    __asm__ volatile("mov %"p"0, "p"8" : "=r" (struct_addr) : : p"8");
#endif

    testassert(self == receiver);

    testassert(i1 == 1);
    testassert(i2 == 2);
    testassert(i3 == 3);
    testassert(i4 == 4);
    testassert(i5 == 5);
    testassert(i6 == 6);
    testassert(i7 == 7);
    testassert(i8 == 8);
    testassert(i9 == 9);
    testassert(i10 == 10);
    testassert(i11 == 11);
    testassert(i12 == 12);
    testassert(i13 == 13);

    testassert(f1 == 1.0);
    testassert(f2 == 2.0);
    testassert(f3 == 3.0);
    testassert(f4 == 4.0);
    testassert(f5 == 5.0);
    testassert(f6 == 6.0);
    testassert(f7 == 7.0);
    testassert(f8 == 8.0);
    testassert(f9 == 9.0);
    testassert(f10 == 10.0);
    testassert(f11 == 11.0);
    testassert(f12 == 12.0);
    testassert(f13 == 13.0);
    testassert(f14 == 14.0);
    testassert(f15 == 15.0);

    if (_cmd == @selector(idret::::::::::::::::::::::::::::)  ||  
        _cmd == @selector(idre2::::::::::::::::::::::::::::)  ||  
        _cmd == @selector(idre3::::::::::::::::::::::::::::)) 
    {
        union {
            id idval;
            long long llval;
        } result;
        testassert(state == 11);
        state = 12;
        result.idval = ID_RESULT;
        return result.llval;
    }
    else if (_cmd == @selector(llret::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(llre2::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(llre3::::::::::::::::::::::::::::)) 
    {
        testassert(state == 13);
        state = 14;
        return LL_RESULT;
    }
    else if (_cmd == @selector(fpret::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(fpre2::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(fpre3::::::::::::::::::::::::::::)) 
    {
        testassert(state == 15);
        state = 16;
#if defined(__i386__)
        __asm__ volatile("fldl %0" : : "m" (FP_RESULT));
#elif defined(__x86_64__)
        __asm__ volatile("movsd %0, %%xmm0" : : "m" (FP_RESULT));
#elif defined(__arm64__)
        __asm__ volatile("ldr d0, %0" : : "m" (FP_RESULT));
#elif defined(__arm__)  &&  __ARM_ARCH_7K__
        __asm__ volatile("vld1.64 {d0}, %0" : : "m" (FP_RESULT));
#elif defined(__arm__)
        union {
            double fpval;
            long long llval;
        } result;
        result.fpval = FP_RESULT;
        return result.llval;
#else
#       error unknown architecture
#endif
        return 0;
    }
    else if (_cmd == @selector(stret::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(stre2::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(stre3::::::::::::::::::::::::::::)) 
    {
#if __i386__  ||  __x86_64__  ||  __arm__
        fail("stret message sent to non-stret forward_handler");
#elif __arm64_32__ || __arm64__
        testassert(state == 17);
        state = 18;
        memcpy(struct_addr, &STRET_RESULT, sizeof(STRET_RESULT));
        return 0;
#else
#       error unknown architecture
#endif
    } 
    else {
        fail("unknown selector %s in forward_handler", sel_getName(_cmd));
    }
}


struct stret forward_stret_handler(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15)
{
    testassert(self == receiver);

    testassert(i1 == 1);
    testassert(i2 == 2);
    testassert(i3 == 3);
    testassert(i4 == 4);
    testassert(i5 == 5);
    testassert(i6 == 6);
    testassert(i7 == 7);
    testassert(i8 == 8);
    testassert(i9 == 9);
    testassert(i10 == 10);
    testassert(i11 == 11);
    testassert(i12 == 12);
    testassert(i13 == 13);

    testassert(f1 == 1.0);
    testassert(f2 == 2.0);
    testassert(f3 == 3.0);
    testassert(f4 == 4.0);
    testassert(f5 == 5.0);
    testassert(f6 == 6.0);
    testassert(f7 == 7.0);
    testassert(f8 == 8.0);
    testassert(f9 == 9.0);
    testassert(f10 == 10.0);
    testassert(f11 == 11.0);
    testassert(f12 == 12.0);
    testassert(f13 == 13.0);
    testassert(f14 == 14.0);
    testassert(f15 == 15.0);

    if (_cmd == @selector(idret::::::::::::::::::::::::::::)  ||
        _cmd == @selector(idre2::::::::::::::::::::::::::::)  ||
        _cmd == @selector(idre3::::::::::::::::::::::::::::)  ||
        _cmd == @selector(llret::::::::::::::::::::::::::::)  ||
        _cmd == @selector(llre2::::::::::::::::::::::::::::)  ||
        _cmd == @selector(llre3::::::::::::::::::::::::::::)  ||
        _cmd == @selector(fpret::::::::::::::::::::::::::::)  ||
        _cmd == @selector(fpre2::::::::::::::::::::::::::::)  ||
        _cmd == @selector(fpre3::::::::::::::::::::::::::::))
    {
        fail("non-stret selector %s sent to forward_stret_handler", sel_getName(_cmd));
    }
    else if (_cmd == @selector(stret::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(stre2::::::::::::::::::::::::::::)  ||  
             _cmd == @selector(stre3::::::::::::::::::::::::::::)) 
    {
        testassert(state == 17);
        state = 18;
        return STRET_RESULT;
    }
    else {
        fail("unknown selector %s in forward_stret_handler", sel_getName(_cmd));
    }

}


@implementation Super
+(void)initialize { }
+(id)class { return self; }
@end

typedef id (*id_fn_t)(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15);

typedef long long (*ll_fn_t)(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15);

typedef double (*fp_fn_t)(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15);

typedef struct stret (*st_fn_t)(id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15);

#if __x86_64__
typedef struct stret * (*fake_st_fn_t)(struct stret *, id self, SEL _cmd, long i1, long i2, long i3, long i4, long i5, long i6, long i7, long i8, long i9, long i10, long i11, long i12, long i13, double f1, double f2, double f3, double f4, double f5, double f6, double f7, double f8, double f9, double f10, double f11, double f12, double f13, double f14, double f15);
#endif

__BEGIN_DECLS
extern void *getSP(void);
__END_DECLS

#if defined(__x86_64__)
    asm(".text \n _getSP: movq %rsp, %rax \n retq \n");
#elif defined(__i386__)
    asm(".text \n _getSP: movl %esp, %eax \n ret \n");
#elif defined(__arm__)
    asm(".text \n .thumb \n .thumb_func _getSP \n "
        "_getSP: mov r0, sp \n bx lr \n");
#elif defined(__arm64__)
    asm(".text \n _getSP: mov x0, sp \n ret \n");
#else
#   error unknown architecture
#endif

int main()
{
    id idval;
    long long llval;
    struct stret stval;
#if __x86_64__
    struct stret *stptr;
#endif
    double fpval;
    void *sp1 = (void*)1;
    void *sp2 = (void*)2;

    st_fn_t stret_fwd;
#if __arm64__
    stret_fwd = (st_fn_t)_objc_msgForward;
#else
    stret_fwd = (st_fn_t)_objc_msgForward_stret;
#endif

    receiver = [Super class];

    // Test user-defined forward handler

    objc_setForwardHandler((void*)&forward_handler, (void*)&forward_stret_handler);

    state = 11;
    sp1 = getSP();
    idval = [Super idre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = [Super llre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = [Super fpre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = [Super stre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));

#if __x86_64__
    // check stret return register
    state = 17;
    sp1 = getSP();
    stptr = ((fake_st_fn_t)objc_msgSend_stret)(&stval, [Super class], @selector(stre3::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));
    testassert(stptr == &stval);    
#endif


    // Test user-defined forward handler, cached

    state = 11;
    sp1 = getSP();
    idval = [Super idre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = [Super llre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = [Super fpre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = [Super stre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));

#if __x86_64__
    // check stret return register
    state = 17;
    sp1 = getSP();
    stptr = ((fake_st_fn_t)objc_msgSend_stret)(&stval, [Super class], @selector(stre3::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));
    testassert(stptr == &stval);    
#endif


    // Test user-defined forward handler, uncached but fixed-up

    _objc_flush_caches(nil);

    state = 11;
    sp1 = getSP();
    idval = [Super idre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = [Super llre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = [Super fpre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = [Super stre3:1:2:3:4:5:6:7:8:9:10:11:12:13:1.0:2.0:3.0:4.0:5.0:6.0:7.0:8.0:9.0:10.0:11.0:12.0:13.0:14.0:15.0];
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));

#if __x86_64__
    // check stret return register
    state = 17;
    sp1 = getSP();
    stptr = ((fake_st_fn_t)objc_msgSend_stret)(&stval, [Super class], @selector(stre3::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));
    testassert(stptr == &stval);    
#endif



    // Test user-defined forward handler, manual forwarding

    state = 11;
    sp1 = getSP();
    idval = ((id_fn_t)_objc_msgForward)(receiver, @selector(idre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = ((ll_fn_t)_objc_msgForward)(receiver, @selector(llre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = ((fp_fn_t)_objc_msgForward)(receiver, @selector(fpre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = stret_fwd(receiver, @selector(stre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));


    // Test user-defined forward handler, manual forwarding, cached

    state = 11;
    sp1 = getSP();
    idval = ((id_fn_t)_objc_msgForward)(receiver, @selector(idre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = ((ll_fn_t)_objc_msgForward)(receiver, @selector(llre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = ((fp_fn_t)_objc_msgForward)(receiver, @selector(fpre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = stret_fwd(receiver, @selector(stre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));


    // Test user-defined forward handler, manual forwarding, uncached but fixed-up

    _objc_flush_caches(nil);

    state = 11;
    sp1 = getSP();
    idval = ((id_fn_t)_objc_msgForward)(receiver, @selector(idre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 12);
    testassert(idval == ID_RESULT);

    state = 13;
    sp1 = getSP();
    llval = ((ll_fn_t)_objc_msgForward)(receiver, @selector(llre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 14);
    testassert(llval == LL_RESULT);

    state = 15;
    sp1 = getSP();
    fpval = ((fp_fn_t)_objc_msgForward)(receiver, @selector(fpre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 16);
    testassert(fpval == FP_RESULT);

    state = 17;
    sp1 = getSP();
    stval = stret_fwd(receiver, @selector(stre2::::::::::::::::::::::::::::), 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0);
    sp2 = getSP();
    testassert(sp1 == sp2);
    testassert(state == 18);
    testassert(stret_equal(stval, STRET_RESULT));


    succeed(__FILE__);
}
