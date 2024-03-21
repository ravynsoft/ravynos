.syntax unified
.text
.global bar_a
.type bar_a %function
bar_a:
bx lr

.p2align 4
.global bar_o
.type bar_o %object
bar_o:
.word 0

.p2align 4
.thumb
.global bar_t
.type bar_t %function
bar_t:
bx lr

