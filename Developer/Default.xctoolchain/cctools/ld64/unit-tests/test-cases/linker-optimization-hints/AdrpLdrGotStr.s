
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
L3:   str   b2, [x1]
#elif LOAD_GPR_16
L3:   str   h2, [x1]
#elif LOAD_GPR_32
L3:   str   w2, [x1]
#elif LOAD_GPR_64
L3:   str   x2, [x1]
#elif LOAD_FPR_32
L3:   str   s2, [x1]
#elif LOAD_FPR_64
L3:   str   d2, [x1]
#elif LOAD_VEC_128
L3:   str   q2, [x1]
#endif
      nop
  
    .loh AdrpLdrGotStr L1, L2, L3
    
#if PADDING
_pad:
      .space 1100000
#endif 
    
    .data
_makePageOffsetNonZero: .long 0,0,0,0

#if MISALIGN_DATA
_junk: .byte 0
#endif

_foo:     .long 0
          .long 0
_8foo8:   .long 0
          .long 0
_16foo16: .long 0


