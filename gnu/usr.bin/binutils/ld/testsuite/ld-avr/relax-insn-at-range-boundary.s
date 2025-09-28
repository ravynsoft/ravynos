.section ".text", "ax",@progbits
.global main
main:
backward_target:                ; Exactly -4094 bytes from jmp
  .ds.b 4094, 0
  jmp backward_target
  jmp forward_target
  .ds.b 4094, 0
forward_target:                 ; Exactly 4098 bytes before relax, 4096 bytes after relax
  nop

