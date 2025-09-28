.text
.globl bar
.globl _start
.option rvc
.align 2
_start:
.L0:
        .rept 6
        call bar
        .endr
.align 2
.L1:
        .uleb128 .L1 - .L0
        .uleb128 .L2 - .L0
.L2:
.align 2
bar:
        nop
