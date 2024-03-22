/*
 * Copyright 2016 Patrick Rudolph <siro@das-labor.org>
 *
 * Permission is hereby granted, free of charge, f, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, f, copy, modify, merge, f, publish, distribute, f, sub
 * license, f, and/or sell copies of the Software, f, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISe, f, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE. */

/* get number of arguments with __NARG__ */
#define __NARG__(...)  __NARG_I_(__VA_ARGS__,__RSEQ_N())
#define __NARG_I_(...) __ARG_N(__VA_ARGS__)
#define __ARG_N( \
      _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
     _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
     _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
     _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
     _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
     _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
     _61,_62,_63,_64,_65,_66,_67,_68,_69,_70,N,...) N
#define __RSEQ_N() \
     70,69,68,67,66,65,64,63,62,61,60, \
     59,58,57,56,55,54,53,52,51,50, \
     49,48,47,46,45,44,43,42,41,40, \
     39,38,37,36,35,34,33,32,31,30, \
     29,28,27,26,25,24,23,22,21,20, \
     19,18,17,16,15,14,13,12,11,10, \
     9,8,7,6,5,4,3,2,1,0


#define _args_for_bypass_1(a) a
#define _args_for_bypass_7(a, b, c, d, e, f, g) ,g
#define _args_for_bypass_14(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_7(__VA_ARGS__)
#define _args_for_bypass_21(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_14(__VA_ARGS__)
#define _args_for_bypass_28(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_21(__VA_ARGS__)
#define _args_for_bypass_35(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_28(__VA_ARGS__)
#define _args_for_bypass_42(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_35(__VA_ARGS__)
#define _args_for_bypass_49(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_42(__VA_ARGS__)
#define _args_for_bypass_56(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_49(__VA_ARGS__)
#define _args_for_bypass_63(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_56(__VA_ARGS__)
#define _args_for_bypass_70(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_63(__VA_ARGS__)
#define _args_for_bypass_77(a, b, c, d, e, f, g, ...) ,g _args_for_bypass_70(__VA_ARGS__)

#define _GFUNC_(n) _args_for_bypass_##n
#define _GFUNC(n) _GFUNC_(n)

#define ARGS_FOR_BYPASS(...) _GFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_mem_1(a) a;
#define _args_for_mem_7(a, b, c, d, e, f, g) f;
#define _args_for_mem_14(a, b, c, d, e, f, g, ...) f; _args_for_mem_7(__VA_ARGS__)
#define _args_for_mem_21(a, b, c, d, e, f, g, ...) f; _args_for_mem_14(__VA_ARGS__)
#define _args_for_mem_28(a, b, c, d, e, f, g, ...) f; _args_for_mem_21(__VA_ARGS__)
#define _args_for_mem_35(a, b, c, d, e, f, g, ...) f; _args_for_mem_28(__VA_ARGS__)
#define _args_for_mem_42(a, b, c, d, e, f, g, ...) f; _args_for_mem_35(__VA_ARGS__)
#define _args_for_mem_49(a, b, c, d, e, f, g, ...) f; _args_for_mem_42(__VA_ARGS__)
#define _args_for_mem_56(a, b, c, d, e, f, g, ...) f; _args_for_mem_49(__VA_ARGS__)
#define _args_for_mem_63(a, b, c, d, e, f, g, ...) f; _args_for_mem_56(__VA_ARGS__)
#define _args_for_mem_70(a, b, c, d, e, f, g, ...) f; _args_for_mem_63(__VA_ARGS__)
#define _args_for_mem_77(a, b, c, d, e, f, g, ...) f; _args_for_mem_70(__VA_ARGS__)

#define _FFUNC_(n) _args_for_mem_##n
#define _FFUNC(n) _FFUNC_(n)

#define ARGS_FOR_MEM(...) _FFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_unbind_1(a) a;
#define _args_for_unbind_7(a, b, c, d, e, f, g) e;
#define _args_for_unbind_14(a, b, c, d, e, f, g, ...) e; _args_for_unbind_7(__VA_ARGS__)
#define _args_for_unbind_21(a, b, c, d, e, f, g, ...) e; _args_for_unbind_14(__VA_ARGS__)
#define _args_for_unbind_28(a, b, c, d, e, f, g, ...) e; _args_for_unbind_21(__VA_ARGS__)
#define _args_for_unbind_35(a, b, c, d, e, f, g, ...) e; _args_for_unbind_28(__VA_ARGS__)
#define _args_for_unbind_42(a, b, c, d, e, f, g, ...) e; _args_for_unbind_35(__VA_ARGS__)
#define _args_for_unbind_49(a, b, c, d, e, f, g, ...) e; _args_for_unbind_42(__VA_ARGS__)
#define _args_for_unbind_56(a, b, c, d, e, f, g, ...) e; _args_for_unbind_49(__VA_ARGS__)
#define _args_for_unbind_63(a, b, c, d, e, f, g, ...) e; _args_for_unbind_56(__VA_ARGS__)
#define _args_for_unbind_70(a, b, c, d, e, f, g, ...) e; _args_for_unbind_63(__VA_ARGS__)
#define _args_for_unbind_77(a, b, c, d, e, f, g, ...) e; _args_for_unbind_70(__VA_ARGS__)

#define _EFUNC_(n) _args_for_unbind_##n
#define _EFUNC(n) _EFUNC_(n)

#define ARGS_FOR_UNBIND(...) _EFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_call_1(a) a
#define _args_for_call_7(a, b, c, d, e, f, g) ,d
#define _args_for_call_14(a, b, c, d, e, f, g, ...) ,d _args_for_call_7(__VA_ARGS__)
#define _args_for_call_21(a, b, c, d, e, f, g, ...) ,d _args_for_call_14(__VA_ARGS__)
#define _args_for_call_28(a, b, c, d, e, f, g, ...) ,d _args_for_call_21(__VA_ARGS__)
#define _args_for_call_35(a, b, c, d, e, f, g, ...) ,d _args_for_call_28(__VA_ARGS__)
#define _args_for_call_42(a, b, c, d, e, f, g, ...) ,d _args_for_call_35(__VA_ARGS__)
#define _args_for_call_49(a, b, c, d, e, f, g, ...) ,d _args_for_call_42(__VA_ARGS__)
#define _args_for_call_56(a, b, c, d, e, f, g, ...) ,d _args_for_call_49(__VA_ARGS__)
#define _args_for_call_63(a, b, c, d, e, f, g, ...) ,d _args_for_call_56(__VA_ARGS__)
#define _args_for_call_70(a, b, c, d, e, f, g, ...) ,d _args_for_call_63(__VA_ARGS__)
#define _args_for_call_77(a, b, c, d, e, f, g, ...) ,d _args_for_call_70(__VA_ARGS__)

#define _DFUNC_(n) _args_for_call_##n
#define _DFUNC(n) _DFUNC_(n)

#define ARGS_FOR_CALL(...) _DFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_decl_1(a) a
#define _args_for_decl_7(a, b, c, d, e, f, g) ,c
#define _args_for_decl_14(a, b, c, d, e, f, g, ...) ,c _args_for_decl_7(__VA_ARGS__)
#define _args_for_decl_21(a, b, c, d, e, f, g, ...) ,c _args_for_decl_14(__VA_ARGS__)
#define _args_for_decl_28(a, b, c, d, e, f, g, ...) ,c _args_for_decl_21(__VA_ARGS__)
#define _args_for_decl_35(a, b, c, d, e, f, g, ...) ,c _args_for_decl_28(__VA_ARGS__)
#define _args_for_decl_42(a, b, c, d, e, f, g, ...) ,c _args_for_decl_35(__VA_ARGS__)
#define _args_for_decl_49(a, b, c, d, e, f, g, ...) ,c _args_for_decl_42(__VA_ARGS__)
#define _args_for_decl_56(a, b, c, d, e, f, g, ...) ,c _args_for_decl_49(__VA_ARGS__)
#define _args_for_decl_63(a, b, c, d, e, f, g, ...) ,c _args_for_decl_56(__VA_ARGS__)
#define _args_for_decl_70(a, b, c, d, e, f, g, ...) ,c _args_for_decl_63(__VA_ARGS__)
#define _args_for_decl_77(a, b, c, d, e, f, g, ...) ,c _args_for_decl_70(__VA_ARGS__)

#define _CFUNC_(n) _args_for_decl_##n
#define _CFUNC(n) _CFUNC_(n)

#define ARGS_FOR_DECLARATION(...) _CFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_assign_1(a) a
#define _args_for_assign_7(a, b, c, d, e, f, g) b;
#define _args_for_assign_14(a, b, c, d, e, f, g, ...) b; _args_for_assign_7(__VA_ARGS__)
#define _args_for_assign_21(a, b, c, d, e, f, g, ...) b; _args_for_assign_14(__VA_ARGS__)
#define _args_for_assign_28(a, b, c, d, e, f, g, ...) b; _args_for_assign_21(__VA_ARGS__)
#define _args_for_assign_35(a, b, c, d, e, f, g, ...) b; _args_for_assign_28(__VA_ARGS__)
#define _args_for_assign_42(a, b, c, d, e, f, g, ...) b; _args_for_assign_35(__VA_ARGS__)
#define _args_for_assign_49(a, b, c, d, e, f, g, ...) b; _args_for_assign_42(__VA_ARGS__)
#define _args_for_assign_56(a, b, c, d, e, f, g, ...) b; _args_for_assign_49(__VA_ARGS__)
#define _args_for_assign_63(a, b, c, d, e, f, g, ...) b; _args_for_assign_56(__VA_ARGS__)
#define _args_for_assign_70(a, b, c, d, e, f, g, ...) b; _args_for_assign_63(__VA_ARGS__)
#define _args_for_assign_77(a, b, c, d, e, f, g, ...) b; _args_for_assign_70(__VA_ARGS__)

#define _BFUNC_(n) _args_for_assign_##n
#define _BFUNC(n) _BFUNC_(n)

#define ARGS_FOR_ASSIGN(...) _BFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

#define _args_for_struct_1(a) a;
#define _args_for_struct_7(a, b, c, d, e, f, g) a;
#define _args_for_struct_14(a, b, c, d, e, f, g, ...) a; _args_for_struct_7(__VA_ARGS__)
#define _args_for_struct_21(a, b, c, d, e, f, g, ...) a; _args_for_struct_14(__VA_ARGS__)
#define _args_for_struct_28(a, b, c, d, e, f, g, ...) a; _args_for_struct_21(__VA_ARGS__)
#define _args_for_struct_35(a, b, c, d, e, f, g, ...) a; _args_for_struct_28(__VA_ARGS__)
#define _args_for_struct_42(a, b, c, d, e, f, g, ...) a; _args_for_struct_35(__VA_ARGS__)
#define _args_for_struct_49(a, b, c, d, e, f, g, ...) a; _args_for_struct_42(__VA_ARGS__)
#define _args_for_struct_56(a, b, c, d, e, f, g, ...) a; _args_for_struct_49(__VA_ARGS__)
#define _args_for_struct_63(a, b, c, d, e, f, g, ...) a; _args_for_struct_56(__VA_ARGS__)
#define _args_for_struct_70(a, b, c, d, e, f, g, ...) a; _args_for_struct_63(__VA_ARGS__)
#define _args_for_struct_77(a, b, c, d, e, f, g, ...) a; _args_for_struct_70(__VA_ARGS__)

#define _AFUNC_(n) _args_for_struct_##n
#define _AFUNC(n) _AFUNC_(n)

#define ARGS_FOR_STRUCT(...) _AFUNC(__NARG__(__VA_ARGS__)) (__VA_ARGS__)

/* Serialization and deserialization */

#define CSMT_ITEM_NO_WAIT(name, ...) \
\
struct s_##name##_private { \
    struct csmt_instruction instr; \
    ARGS_FOR_STRUCT( __VA_ARGS__ ) \
}; \
\
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) ); \
\
static int \
name##_rx( struct NineDevice9 *device, struct csmt_instruction *instr ) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args = (struct s_##name##_private *)instr; \
    \
    (void) args; \
    (void) ctx; \
    name##_priv( \
        device ARGS_FOR_CALL( __VA_ARGS__ ) \
    ); \
    ARGS_FOR_UNBIND( __VA_ARGS__ ) \
    return 0; \
} \
\
void \
name( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) ) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args; \
    unsigned memsize = sizeof(struct s_##name##_private); \
    unsigned memsize2 = 0; \
    \
    if (!device->csmt_active) { \
        name##_priv( \
            device ARGS_FOR_BYPASS( __VA_ARGS__ ) \
        ); \
        return; \
    } \
    ARGS_FOR_MEM ( __VA_ARGS__ ) \
    args = nine_queue_alloc(ctx->pool, memsize + memsize2); \
    assert(args); \
    args->instr.func = &name##_rx; \
    ARGS_FOR_ASSIGN( __VA_ARGS__ ) \
} \
\
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) )

#define CSMT_ITEM_NO_WAIT_WITH_COUNTER(name, ...) \
\
struct s_##name##_private { \
    struct csmt_instruction instr; \
    unsigned *counter; \
    ARGS_FOR_STRUCT( __VA_ARGS__ ) \
}; \
\
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) ); \
\
static int \
name##_rx( struct NineDevice9 *device, struct csmt_instruction *instr ) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args = (struct s_##name##_private *)instr; \
    \
    (void) args; \
    (void) ctx; \
    name##_priv( \
        device ARGS_FOR_CALL( __VA_ARGS__ ) \
    ); \
    p_atomic_dec(args->counter); \
    ARGS_FOR_UNBIND( __VA_ARGS__ ) \
    return 0; \
} \
\
void \
name( struct NineDevice9 *device, unsigned *counter ARGS_FOR_DECLARATION( __VA_ARGS__ ) ) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args; \
    unsigned memsize = sizeof(struct s_##name##_private); \
    unsigned memsize2 = 0; \
    \
    if (!device->csmt_active) { \
        name##_priv( \
            device ARGS_FOR_BYPASS( __VA_ARGS__ ) \
        ); \
        return; \
    } \
    assert(counter); \
    p_atomic_inc(counter); \
    ARGS_FOR_MEM ( __VA_ARGS__ ) \
    args = nine_queue_alloc(ctx->pool, memsize + memsize2); \
    assert(args); \
    args->instr.func = &name##_rx; \
    args->counter = counter; \
    ARGS_FOR_ASSIGN( __VA_ARGS__ ) \
} \
\
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) )

#define CSMT_ITEM_DO_WAIT(name, ...) \
\
struct s_##name##_private { \
    struct csmt_instruction instr; \
    ARGS_FOR_STRUCT( __VA_ARGS__ ) \
}; \
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) ); \
\
static int \
name##_rx( struct NineDevice9 *device, struct csmt_instruction *instr) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args = (struct s_##name##_private *)instr; \
    \
    (void) args; \
    (void) ctx; \
    name##_priv( \
        device ARGS_FOR_CALL( __VA_ARGS__ ) \
    ); \
    ARGS_FOR_UNBIND( __VA_ARGS__ ) \
    return 1; \
} \
\
void \
name( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) ) \
{ \
    struct csmt_context *ctx = device->csmt_ctx; \
    struct s_##name##_private *args; \
    unsigned memsize = sizeof(struct s_##name##_private); \
    unsigned memsize2 = 0; \
    \
    if (!device->csmt_active) { \
        name##_priv( \
            device ARGS_FOR_BYPASS( __VA_ARGS__ ) \
        ); \
        return; \
    } \
    ARGS_FOR_MEM ( __VA_ARGS__ ) \
    args = nine_queue_alloc(ctx->pool, memsize + memsize2); \
    assert(args); \
    args->instr.func = &name##_rx; \
    ARGS_FOR_ASSIGN( __VA_ARGS__ ) \
    ctx->processed = false; \
    nine_queue_flush(ctx->pool); \
    nine_csmt_wait_processed(ctx); \
} \
\
static void \
name##_priv( struct NineDevice9 *device ARGS_FOR_DECLARATION( __VA_ARGS__ ) )

/* ARGS_FOR_STRUCT, ARGS_FOR_ASSIGN, ARGS_FOR_DECLARATION, ARGS_FOR_CALL, ARGS_FOR_UNBIND, ARGS_FOR_MEM, ARGS_FOR_BYPASS */
#define ARG_VAL(x, y) \
        x _##y ; ,\
        args->_##y = y ; ,\
        x y ,\
        args->_##y ,\
        ,\
        ,\
        y

#define ARG_REF(x, y) \
        x* _##y ; ,\
        args->_##y = y; ,\
        x *y ,\
        args->_##y ,\
        ,\
        ,\
        y

#define ARG_COPY_REF(x, y) \
        x * _##y ; x __##y ; ,\
        if ( y ) { args->_##y = &args->__##y ; args->__##y = *y ; } else { args->_##y = NULL; } ,\
        const x *y ,\
        (const x *)args->_##y ,\
        ,\
        ,\
        (const x *)y

#define ARG_BIND_REF(x, y) \
        x * _##y ,\
        if ( y ) \
            NineUnknown_Bind( (void *)y ); \
        args->_##y = y ; ,\
        x *y ,\
        args->_##y,\
        if (args->_##y) \
            NineUnknown_Unbind((void *)(args->_##y)); \
        args->_##y = NULL; ,\
        ,\
        y

#define ARG_BIND_RES(x, y) \
        x * _##y ,\
        args->_##y = NULL; \
        if (y) \
            pipe_resource_reference(&args->_##y, y); ,\
        x *y ,\
        args->_##y ,\
        if (args->_##y) \
            pipe_resource_reference(&args->_##y, NULL); ,\
        ,\
        y

#define ARG_MEM(x, y) \
        x * _##y ,\
        args->_##y = (void *)args + memsize;\
        memcpy(args->_##y, y, memsize2); ,\
        const x *y ,\
        (const x *)args->_##y ,\
        ,\
        ,\
        (const x *)y

#define ARG_MEM_SIZE(x, y) \
        x _##y ,\
        args->_##y = y; ,\
        x y ,\
        args->_##y ,\
        ,\
        memsize2 = y, \
        y

#define ARG_BIND_BLIT(x, y) \
        x _##y ,\
        memcpy(&args->_##y , y, sizeof(x)); \
        args->_##y.src.resource = NULL; \
        args->_##y.dst.resource = NULL; \
        pipe_resource_reference(&args->_##y.src.resource, y->src.resource); \
        pipe_resource_reference(&args->_##y.dst.resource, y->dst.resource);,\
        x *y ,\
        &args->_##y ,\
        pipe_resource_reference(&args->_##y.src.resource, NULL); \
        pipe_resource_reference(&args->_##y.dst.resource, NULL);,\
        ,\
        y

#define ARG_BIND_VBUF(x, y) \
        x _##y ,\
        memcpy(&args->_##y , y, sizeof(x)); \
        args->_##y.buffer.resource = NULL; \
        pipe_resource_reference(&args->_##y.buffer.resource, y->buffer.resource); ,\
        x *y ,\
        &args->_##y ,\
        pipe_resource_reference(&args->_##y.buffer.resource, NULL); ,\
        ,\
        y

#define ARG_BIND_IBUF(x, y) \
        x _##y ,\
        memcpy(&args->_##y , y, sizeof(x)); \
        args->_##y.buffer = NULL; \
        pipe_resource_reference(&args->_##y.buffer, y->buffer); ,\
        x *y ,\
        &args->_##y ,\
        pipe_resource_reference(&args->_##y.buffer, NULL); ,\
        ,\
        y

#define ARG_BIND_VIEW(x, y) \
        x * _##y ,\
        args->_##y = NULL; \
        if (y) \
            pipe_sampler_view_reference(&args->_##y, y); ,\
        x *y ,\
        args->_##y ,\
        if (args->_##y) \
            pipe_sampler_view_reference(&args->_##y, NULL); ,\
        ,\
        y

