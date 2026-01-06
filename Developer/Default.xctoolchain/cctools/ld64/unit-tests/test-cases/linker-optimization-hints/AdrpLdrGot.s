
#ifndef TARGET
 #define TARGET _malloc
#endif

      .text
      .align 2
_test:
      nop
L1:   adrp	x0, TARGET@GOTPAGE
L2:   ldr   x1, [x0, #TARGET@GOTPAGEOFF]
      nop
  
    .loh AdrpLdrGot L1, L2

#if PADDING
_pad:
      .space 1100000
#endif 
    
_fooCode:
      nop
  

    .data
_makePageOffsetNonZero: .long 0,0,0,0

_fooData:     .long 0


