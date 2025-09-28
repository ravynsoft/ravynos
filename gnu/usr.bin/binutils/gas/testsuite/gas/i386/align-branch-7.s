  .text
  .globl  foo
  .p2align  4
foo:
.L1:
  pcmpestrm $3, (%eax), %xmm0
  movl  %esp, %ebp
  movl  %edi, -228(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl	%eax, %gs:0x1
  testb $0x4,%al
  jo  .L1
