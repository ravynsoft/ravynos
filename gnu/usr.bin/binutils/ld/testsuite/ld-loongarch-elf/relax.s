  .data
  .global a
  .type a, @object
a:
  .word 123

  .text
  .global main
  .type main, @function
main:
  la.local $a0, a
  ld.w $a1, $a0, 0
  la.global $a0, a
  ld.w $a0, $a0, 0
  sub.d $a0, $a0, $a1
  jr $ra
