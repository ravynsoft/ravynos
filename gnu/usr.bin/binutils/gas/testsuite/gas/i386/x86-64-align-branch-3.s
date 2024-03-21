  .text
  .globl  foo
  .p2align  4
foo:
  movl  %eax, %fs:0x1
  pushq  %rbp
  pushq  %rbp
  pushq  %rbp
  pushq  %rbp
  movq  %rsp, %rbp
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  call	__tls_get_addr
  pushq  %rbp
  pushq  %rbp
  movl  %eax, %fs:0x1
  movq  %rsp, %rbp
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  call	*__tls_get_addr@GOTPCREL(%rip)
  movl  %esi, -12(%rbp)
