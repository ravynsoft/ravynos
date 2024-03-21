        .text
        .global _start, dest
_start:
  jmp dest
  .align	1
dest:
  nop
  rjmp dest

