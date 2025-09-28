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
  jo  label2
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
  je  label2
  popl  %ebp
  je  label2
  movl  %eax, -4(%esp)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  popl  %ebp
  jmp  label3
  jmp  label3
  jmp  label3
  movl  %eax, -4(%ebp)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  popl  %ebp
  popl  %ebp
  inc  %eax
  jc  label2
  movl  %eax, -4(%ebp)
  movl  %esi, -12(%ebp)
  movl  %edi, -8(%ebp)
  movl  %esi, -12(%ebp)
  and  %eax, %ebx
  jl  label3
label2:
  movl  -12(%ebp), %eax
  movl  %eax, -4(%ebp)
label3:
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
  je label3
  je label3
