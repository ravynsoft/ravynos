 .text
 .p2align 2
 .global _start
_start:
 brsl lr,f1_a1
 brsl lr,setjmp
 br _start

 .type setjmp,@function
 .global setjmp
setjmp:
 bi lr
 .size setjmp,.-setjmp

 .type longjmp,@function
longjmp:
 bi lr
 .size longjmp,.-longjmp

 .word .L1

 .section .ov_a1,"ax",@progbits
 .p2align 2
 .global f1_a1
 .type f1_a1,@function
f1_a1:
 bi lr
 .size f1_a1,.-f1_a1

.L1:
 .word .L1, .L2, .L3
.L2:

 .section .ov_a2,"ax",@progbits
 .p2align 2
 .type f1_a2,@function
f1_a2:
 br longjmp
 .size f1_a2,.-f1_a2

.L3:
 .word .L2, .L4
.L4:

 .section .nonalloc,"",@progbits
 .word .L1,.L2,.L3,.L4

_SPUEAR_f1_a2 = f1_a2
 .global _SPUEAR_f1_a2

_SPUEAR_version=3
 .global _SPUEAR_version
