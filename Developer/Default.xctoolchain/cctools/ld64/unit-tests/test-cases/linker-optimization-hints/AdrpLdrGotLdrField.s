
#ifndef TARGET
 #define TARGET _foo
#endif

      .text
      .align 2
_test:
      nop
L1:   adrp	x0, TARGET@GOTPAGE
L2:   ldr   x1, [x0, #TARGET@GOTPAGEOFF]
#if LOAD_GPR_8
L3:   ldr   b2, [x1, #8]
#elif LOAD_GPR_16
L3:   ldr   h2, [x1, #8]
#elif LOAD_GPR_32
L3:   ldr   w2, [x1, #8]
#elif LOAD_GPR_64
L3:   ldr   x2, [x1, #8]
#elif LOAD_FPR_32
L3:   ldr   s2, [x1, #8]
#elif LOAD_FPR_64
L3:   ldr   d2, [x1, #8]
#elif LOAD_VEC_128
L3:   ldr   q2, [x1, #16]
#endif
      nop
  
    .loh AdrpLdrGotLdr L1, L2, L3
    
#if PADDING
_pad:
      .space 1100000
#endif 

#if FOO_AS_CONST
    .const
    .align 4
#endif

#if FOO_AS_DATA
    .data
_makePageOffsetNonZero: .long 0,0,0,0
#endif

#if MISALIGN_DATA
_junk: .byte 0
#endif

_foo:     .long 0,0,0,0



