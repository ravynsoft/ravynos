foo:
addi $sp, $sp, -512
addi $sp, $sp, 511
lwi $r0, [$sp + 0]
lwi $r7, [$sp + 508]
swi $r0, [$sp + 0]
