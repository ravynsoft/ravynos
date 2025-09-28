  .text
  .globl  foo
  .p2align  4
foo:
  movl  %eax, %fs:0x1
  pushq  %rbp
  pushq  %rbp
  pushq  %rbp
  movq  %rsp, %rbp
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  cmp  %rax, %rbp
  je  .L_2
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %edi, -8(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  popq  %rbp
  popq  %rbp
  je  .L_2
  popq  %rbp
  je  .L_2
  movl  %eax, -4(%rbp)
  movl  %esi, -12(%rbp)
  movl  %edi, -8(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  popq  %rbp
  popq  %rbp
  jmp  .L_3
  jmp  .L_3
  jmp  .L_3
  movl  %eax, -4(%rbp)
  movl  %esi, -12(%rbp)
  movl  %edi, -8(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  popq  %rbp
  popq  %rbp
  cmp  %rax, %rbp
  je  .L_2
  jmp  .L_3
.L_2:
  movl  -12(%rbp), %eax
  movl  %eax, -4(%rbp)
.L_3:
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  movl  %esi, -1200(%rbp)
  jmp  .L_3
  popq  %rbp
  retq
