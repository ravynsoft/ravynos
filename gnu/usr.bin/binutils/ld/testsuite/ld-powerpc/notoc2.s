 .text
 .weak puts
 pld 12,-1(0),1
 pld 12,0(0),1
 paddi 12,0,-1,1
 paddi 12,0,0,1
0:
 paddi 3,0,hello-.,1
 bl puts@notoc
 nop
 b 0b
 .section .rodata
hello:	.asciz "Hello!"
