  .text
  .globl  foo
  .p2align  4
foo:
  movl  %eax, %fs:0x1
  pushq  %rbp
  pushq  %rbp
  movq  %rsp, %rbp
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  ret
  pushq  %rbp
  movl  %eax, %fs:0x1
  pushq  %rbp
  pushq  %rbp
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  ret $30
  pushq  %rbp
