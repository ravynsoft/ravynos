#ld:
#objdump: -d

.*:     file format pei-aarch64-little


Disassembly of section .text:

0000000140001000 <___crt_xc_end__>:
   140001000:	d2800281 	mov	x1, #0x14                  	// #20
   140001004:	14000001 	b	140001008 <foo>

0000000140001008 <foo>:
   140001008:	d65f03c0 	ret
   14000100c:	00000000 	udf	#0

0000000140001010 <__CTOR_LIST__>:
   140001010:	ffffffff 	.inst	0xffffffff ; undefined
   140001014:	ffffffff 	.inst	0xffffffff ; undefined
	...

0000000140001020 <__DTOR_LIST__>:
   140001020:	ffffffff 	.inst	0xffffffff ; undefined
   140001024:	ffffffff 	.inst	0xffffffff ; undefined
	...
