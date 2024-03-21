#target: *-*-*eabi* *-*-nacl*

.*:     file format .*


Disassembly of section \.text:

00000000 <two>:
       0:	f000 c802 	blx	800008 <__far_fn_from_thumb>
	...
#...
00800008 <__far_fn_from_thumb>:
  800008:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 80000c <__far_fn_from_thumb\+0x4>
  80000c:	7fff0000 	.word	0x7fff0000

00800010 <three>:
  800010:	f001 e806 	blx	801020 <__far_fn1_from_thumb>
  800014:	f001 e800 	blx	801018 <__far_fn2_from_thumb>
	...
  800ff8:	bf00      	nop

00800ffa <label1>:
  800ffa:	ea81 0002 	eor.w	r0, r1, r2
  800ffe:	f000 b813 	b.w	801028 <__far_fn1_from_thumb\+0x8>
  801002:	ea81 0002 	eor.w	r0, r1, r2
  801006:	ea81 0002 	eor.w	r0, r1, r2
  80100a:	f7ff bff6 	b.w	800ffa <label1>
  80100e:	ea81 0002 	eor.w	r0, r1, r2
  801012:	ea81 0002 	eor.w	r0, r1, r2
	...

00801018 <__far_fn2_from_thumb>:
  801018:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 80101c <__far_fn2_from_thumb\+0x4>
  80101c:	80000004 	.word	0x80000004

00801020 <__far_fn1_from_thumb>:
  801020:	e51ff004 	ldr	pc, \[pc, #-4\]	@ 801024 <__far_fn1_from_thumb\+0x4>
  801024:	80000000 	.word	0x80000000
  801028:	d001      	beq.n	80102e <__far_fn1_from_thumb\+0xe>
  80102a:	f7ff bfea 	b.w	801002 <label1\+0x8>
  80102e:	f7ff bfe4 	b.w	800ffa <label1>
