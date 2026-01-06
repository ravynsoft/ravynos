
#ifndef ADDEND
 #define ADDEND 0
#endif

      .text
      .align 2
_test:
      nop
L1:   adrp	x0, _foo@PAGE
L2:   add   x0, x0, _foo@PAGEOFF
#if LOAD_GPR_8
L3:   str   b1, [x0, #ADDEND]
#elif LOAD_GPR_16
L3:   str   h1, [x0, #ADDEND]
#elif LOAD_GPR_32
L3:   str   w1, [x0, #ADDEND]
#elif LOAD_GPR_64
L3:   str   x1, [x0, #ADDEND]
#elif LOAD_FPR_32
L3:   str   s1, [x0, #ADDEND]
#elif LOAD_FPR_64
L3:   str   d1, [x0, #ADDEND]
#elif LOAD_VEC_128
L3:   str   q1, [x0, #ADDEND]
#endif
      nop
  
    .loh AdrpAddStr L1, L2, L3

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


