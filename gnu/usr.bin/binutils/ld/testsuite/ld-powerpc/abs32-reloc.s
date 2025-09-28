 .globl x
 .hidden x

 .data
 .p2align 2
x:
 .long x
 .long a
 .long b
 .long c

 .text
 .p2align 2
 .globl _start
 .type _start,@function
_start:
0:
 lwz 7,x@got(30)
 lwz 8,a@got(30)
 lwz 9,b@got(30)
 lwz 10,c@got(30)
 .size _start,.-_start
