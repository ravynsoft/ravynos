 .weak __start_xx
 .weak __stop_xx

 .global _start
_start:
  pld 3,__start_xx@got@pcrel
  pld 4,__stop_xx@got@pcrel

 .section xx,"a",unique,0
 .byte 0

 .section xx,"a",unique,1
 .byte 1

 .section xx,"a",unique,2
 .byte 2
