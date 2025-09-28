  /* .tbss */
  .comm foo[UL],8
  .lcomm foo2_l,8,foo2[UL]

  /* .tdata */
  .globl bar[TL]
  .csect bar[TL]
  .long 1
