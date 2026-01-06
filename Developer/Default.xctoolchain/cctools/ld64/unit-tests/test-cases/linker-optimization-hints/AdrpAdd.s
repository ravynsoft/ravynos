

      .text
      .align 2
_test:
      nop
L1:   adrp	x0, _foo@PAGE
L2:   add   x0, x0, _foo@PAGEOFF
      nop
  
    .loh AdrpAdd L1, L2
    
#if PADDING
_pad:
      .space 1100000
#endif 
  
#if FOO_AS_CONST
    .const
#endif

#if FOO_AS_DATA
    .data
#endif

_foo: .long 0

