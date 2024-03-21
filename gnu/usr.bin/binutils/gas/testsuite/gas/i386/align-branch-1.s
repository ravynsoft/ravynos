  .text
  .globl  foo
  .p2align  4
foo:
  movl	%eax, %gs:0x1
  pushl  %ebp
  pushl  %ebp
  pushl  %ebp
  pushl  %ebp
  movl  %esp, %ebp
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  cmp  %eax, %ebp
  je  .L_2
  movl  %esi, -12(%ebx)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  popl  %ebp
  popl  %ebp
  popl  %ebp
  je  .L_2
  popl  %ebp
  je  .L_2
  movl  %eax, -4(%esp)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  popl  %ebp
  jmp  .L_3
  jmp  .L_3
  jmp  .L_3
  movl  %eax, -4(%ebp)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  popl  %ebp
  popl  %ebp
  cmp  %eax, %ebp
  je  .L_2
  jmp  .L_3
.L_2:
  movl  -12(%ebp), %eax
  movl  %eax, -4(%ebp)
.L_3:
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, 12(%ebp)
  jmp  bar
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, -1200(%ebp)
  movl  %esi, (%ebp)
  je .L_3
  je .L_3
