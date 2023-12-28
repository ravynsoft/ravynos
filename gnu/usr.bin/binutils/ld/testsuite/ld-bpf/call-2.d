#as: --EL
#source: call-2.s
#source: bar.s
#objdump: -dr
#ld: -EL
#name: CALL with disp32 reloc and addend

.*: +file format .*bpf.*

Disassembly of section .text:

[0-9a-f]+ <.*>:
 *[0-9a-f]+:	b7 01 00 00 01 00 00 00 	mov %r1,1
 *[0-9a-f]+:	85 10 00 00 02 00 00 00 	call 2
 *[0-9a-f]+:	95 00 00 00 00 00 00 00 	exit

[0-9a-f]+ <bar>:
 *[0-9a-f]+:	bf 21 00 00 00 00 00 00 	mov %r1,%r2
 *[0-9a-f]+:	bf 13 00 00 00 00 00 00 	mov %r3,%r1
 *[0-9a-f]+:	95 00 00 00 00 00 00 00 	exit
