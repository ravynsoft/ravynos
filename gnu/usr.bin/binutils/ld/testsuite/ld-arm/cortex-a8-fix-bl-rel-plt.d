
.*:     file format .*


Disassembly of section \.plt:

00008e00 <.plt>:
    8e00:	e52de004 	push	{lr}		@ \(str lr, \[sp, #-4\]!\)
    8e04:	e59fe004 	ldr	lr, \[pc, #4\]	@ 8e10 <.*>
    8e08:	e08fe00e 	add	lr, pc, lr
    8e0c:	e5bef008 	ldr	pc, \[lr, #8\]!
    8e10:	0001027c 	\.word	0x0001027c
00008e14 <targetfn@plt>:
    8e14:	e28fc600 	add	ip, pc, #0, 12
    8e18:	e28cca10 	add	ip, ip, #16, 20	@ 0x10000
    8e1c:	e5bcf27c 	ldr	pc, \[ip, #636\]!	@ 0x27c

Disassembly of section \.text:

00008f00 <targetfn>:
    8f00:	4770      	bx	lr
    8f02:	bf00      	nop
#...

00008f08 <_start>:
    8f08:	bf00      	nop
    8f0a:	eb01 0002 	add\.w	r0, r1, r2
    8f0e:	f7ff ef82 	blx	8e14 <targetfn@plt>
    8f12:	eb01 0002 	add\.w	r0, r1, r2
    8f16:	f7ff ef7e 	blx	8e14 <targetfn@plt>
    8f1a:	eb01 0002 	add\.w	r0, r1, r2
    8f1e:	f7ff ef7a 	blx	8e14 <targetfn@plt>
    8f22:	eb01 0002 	add\.w	r0, r1, r2
    8f26:	f7ff ef76 	blx	8e14 <targetfn@plt>
    8f2a:	eb01 0002 	add\.w	r0, r1, r2
    8f2e:	f7ff ef72 	blx	8e14 <targetfn@plt>
    8f32:	eb01 0002 	add\.w	r0, r1, r2
    8f36:	f7ff ef6e 	blx	8e14 <targetfn@plt>
    8f3a:	eb01 0002 	add\.w	r0, r1, r2
    8f3e:	f7ff ef6a 	blx	8e14 <targetfn@plt>
    8f42:	eb01 0002 	add\.w	r0, r1, r2
    8f46:	f7ff ef66 	blx	8e14 <targetfn@plt>
    8f4a:	eb01 0002 	add\.w	r0, r1, r2
    8f4e:	f7ff ef62 	blx	8e14 <targetfn@plt>
    8f52:	eb01 0002 	add\.w	r0, r1, r2
    8f56:	f7ff ef5e 	blx	8e14 <targetfn@plt>
    8f5a:	eb01 0002 	add\.w	r0, r1, r2
    8f5e:	f7ff ef5a 	blx	8e14 <targetfn@plt>
    8f62:	eb01 0002 	add\.w	r0, r1, r2
    8f66:	f7ff ef56 	blx	8e14 <targetfn@plt>
    8f6a:	eb01 0002 	add\.w	r0, r1, r2
    8f6e:	f7ff ef52 	blx	8e14 <targetfn@plt>
    8f72:	eb01 0002 	add\.w	r0, r1, r2
    8f76:	f7ff ef4e 	blx	8e14 <targetfn@plt>
    8f7a:	eb01 0002 	add\.w	r0, r1, r2
    8f7e:	f7ff ef4a 	blx	8e14 <targetfn@plt>
    8f82:	eb01 0002 	add\.w	r0, r1, r2
    8f86:	f7ff ef46 	blx	8e14 <targetfn@plt>
    8f8a:	eb01 0002 	add\.w	r0, r1, r2
    8f8e:	f7ff ef42 	blx	8e14 <targetfn@plt>
    8f92:	eb01 0002 	add\.w	r0, r1, r2
    8f96:	f7ff ef3e 	blx	8e14 <targetfn@plt>
    8f9a:	eb01 0002 	add\.w	r0, r1, r2
    8f9e:	f7ff ef3a 	blx	8e14 <targetfn@plt>
    8fa2:	eb01 0002 	add\.w	r0, r1, r2
    8fa6:	f7ff ef36 	blx	8e14 <targetfn@plt>
    8faa:	eb01 0002 	add\.w	r0, r1, r2
    8fae:	f7ff ef32 	blx	8e14 <targetfn@plt>
    8fb2:	eb01 0002 	add\.w	r0, r1, r2
    8fb6:	f7ff ef2e 	blx	8e14 <targetfn@plt>
    8fba:	eb01 0002 	add\.w	r0, r1, r2
    8fbe:	f7ff ef2a 	blx	8e14 <targetfn@plt>
    8fc2:	eb01 0002 	add\.w	r0, r1, r2
    8fc6:	f7ff ef26 	blx	8e14 <targetfn@plt>
    8fca:	eb01 0002 	add\.w	r0, r1, r2
    8fce:	f7ff ef22 	blx	8e14 <targetfn@plt>
    8fd2:	eb01 0002 	add\.w	r0, r1, r2
    8fd6:	f7ff ef1e 	blx	8e14 <targetfn@plt>
    8fda:	eb01 0002 	add\.w	r0, r1, r2
    8fde:	f7ff ef1a 	blx	8e14 <targetfn@plt>
    8fe2:	eb01 0002 	add\.w	r0, r1, r2
    8fe6:	f7ff ef16 	blx	8e14 <targetfn@plt>
    8fea:	eb01 0002 	add\.w	r0, r1, r2
    8fee:	f7ff ef12 	blx	8e14 <targetfn@plt>
    8ff2:	eb01 0002 	add\.w	r0, r1, r2
    8ff6:	f7ff ef0e 	blx	8e14 <targetfn@plt>
    8ffa:	eb01 0002 	add\.w	r0, r1, r2
    8ffe:	f000 e808 	blx	9010 <_start\+0x108>
    9002:	eb01 0002 	add\.w	r0, r1, r2
    9006:	f7ff ef06 	blx	8e14 <targetfn@plt>
    900a:	4770      	bx	lr
#...
    9010:	eaffff7f 	b	8e14 <targetfn@plt>
