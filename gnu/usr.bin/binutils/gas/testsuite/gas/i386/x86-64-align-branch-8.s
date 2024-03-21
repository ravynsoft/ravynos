  .text
  .globl  foo
  .p2align  4
foo:
.L1:
  vpcmpestrm $3, (%rax), %xmm15
  movq  %rsp, %rbp
  movl  %edi, -8(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl  %esi, -12(%rbp)
  movl	%eax, %fs:0x1
  testb $0x4,%al
  jo  .L1
