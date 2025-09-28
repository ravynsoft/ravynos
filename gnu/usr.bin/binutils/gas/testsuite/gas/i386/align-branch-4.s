  .text
  .globl  foo
  .p2align  4
foo:
  movl  %eax, %fs:0x1
  pushl  %ebp
  pushl  %ebp
  pushl  %ebp
  pushl  %ebp
  pushl  %ebp
  movl  %esp, %ebp
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  ret
  pushl  %ebp
  pushl  %ebp
  movl  %eax, %fs:0x1
  movl  %esp, %ebp
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  movl  %esi, -12(%ebp)
  ret	$30
  movl  %esi, -12(%ebp)
