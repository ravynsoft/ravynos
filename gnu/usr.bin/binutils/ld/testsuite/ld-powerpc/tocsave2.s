 .abiversion 2
 .global _start
 .type _start,@function
_start:
 nop
.L1:
# Function prologue tocsave.
 .reloc .,R_PPC64_TOCSAVE,.L1
 nop

 nop
# Call with tocsave on nop
 bl foo
 .reloc .,R_PPC64_TOCSAVE,.L1
 nop

 nop
# A call without tocsave (maybe in a different function, or after alloca
# dynamic stack allocation loses r2 save in same function).
 bl foo
 nop

 blr
 .size _start, .-_start
