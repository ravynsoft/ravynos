
#ifndef ADDEND
 #define ADDEND  
#endif

      .text
      .align 2
_test:
      nop
L1:   adrp	x0, _foo ADDEND@PAGE
#if LOAD_GPR_8
L3:   ldr   b1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_16
L3:   ldr   h1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_32
L3:   ldr   w1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_32_S16
L3:   ldrsh w1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_32_S8
L3:   ldrsb w1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_64
L3:   ldr   x1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_64_S32
L3:   ldrsw x1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_64_S16
L3:   ldrsh x1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_GPR_64_S8
L3:   ldrsb x1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_FPR_32
L3:   ldr   s1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_FPR_64
L3:   ldr   d1, [x0,  _foo ADDEND@PAGEOFF]
#elif LOAD_VEC_128
L3:   ldr   q1, [x0,  _foo ADDEND@PAGEOFF]
#endif
      nop
  
    .loh AdrpLdr L1, L3
    
#if PADDING
_pad:
      .space 1100000
#endif 

#if FOO_AS_CONSTANT
    .literal4
#endif

#if FOO_AS_DATA
    .data
_makePageOffsetNonZero: .long 0,0,0,0
#endif

#if MISALIGN_DATA
_junk: .byte 0
#endif

_foo:     .long 0
          .long 0
_8foo8:   .long 0
          .long 0
_16foo16: .long 0


