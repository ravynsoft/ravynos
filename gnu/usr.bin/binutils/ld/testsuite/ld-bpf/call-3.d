#as: --EL
#source: call-3.s
#objdump: -dr
#ld: -EL
#name: CALL check unsigned underflow

.*: +file format .*bpf.*

Disassembly of section .text:

[0-9a-f]+ <bar>:
 *[0-9a-f]+:	b7 00 00 00 00 00 00 00 	mov %r0,0
 *[0-9a-f]+:	95 00 00 00 00 00 00 00 	exit

[0-9a-f]+ <main>:
 *[0-9a-f]+:	b7 00 00 00 03 00 00 00 	mov %r0,3
 *[0-9a-f]+:	b7 01 00 00 01 00 00 00 	mov %r1,1
 *[0-9a-f]+:	85 10 00 00 fb ff ff ff 	call -5
 *[0-9a-f]+:	95 00 00 00 00 00 00 00 	exit
