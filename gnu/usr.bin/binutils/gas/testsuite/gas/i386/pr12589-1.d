#name: i386 jump
#objdump: -drw

.*: +file format .*i386.*

Disassembly of section .text:

0+ <_start>:
[ 	]*[a-f0-9]+:	eb 02                	jmp    4 <scn_pnp>

0+2 <zerob>:
	...

0+4 <scn_pnp>:
[ 	]*[a-f0-9]+:	89 c0                	mov    %eax,%eax
#pass
