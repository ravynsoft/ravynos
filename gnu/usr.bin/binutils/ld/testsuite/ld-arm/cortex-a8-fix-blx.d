
.*:     file format .*


Disassembly of section \.text:

00008f00 <armfn>:
    8f00:	e1a02413 	lsl	r2, r3, r4
    8f04:	e12fff1e 	bx	lr

00008f08 <_start>:
    8f08:	bf00      	nop
    8f0a:	eb01 0002 	add\.w	r0, r1, r2
    8f0e:	f7ff eff8 	blx	8f00 <armfn>
    8f12:	eb01 0002 	add\.w	r0, r1, r2
    8f16:	f7ff eff4 	blx	8f00 <armfn>
    8f1a:	eb01 0002 	add\.w	r0, r1, r2
    8f1e:	f7ff eff0 	blx	8f00 <armfn>
    8f22:	eb01 0002 	add\.w	r0, r1, r2
    8f26:	f7ff efec 	blx	8f00 <armfn>
    8f2a:	eb01 0002 	add\.w	r0, r1, r2
    8f2e:	f7ff efe8 	blx	8f00 <armfn>
    8f32:	eb01 0002 	add\.w	r0, r1, r2
    8f36:	f7ff efe4 	blx	8f00 <armfn>
    8f3a:	eb01 0002 	add\.w	r0, r1, r2
    8f3e:	f7ff efe0 	blx	8f00 <armfn>
    8f42:	eb01 0002 	add\.w	r0, r1, r2
    8f46:	f7ff efdc 	blx	8f00 <armfn>
    8f4a:	eb01 0002 	add\.w	r0, r1, r2
    8f4e:	f7ff efd8 	blx	8f00 <armfn>
    8f52:	eb01 0002 	add\.w	r0, r1, r2
    8f56:	f7ff efd4 	blx	8f00 <armfn>
    8f5a:	eb01 0002 	add\.w	r0, r1, r2
    8f5e:	f7ff efd0 	blx	8f00 <armfn>
    8f62:	eb01 0002 	add\.w	r0, r1, r2
    8f66:	f7ff efcc 	blx	8f00 <armfn>
    8f6a:	eb01 0002 	add\.w	r0, r1, r2
    8f6e:	f7ff efc8 	blx	8f00 <armfn>
    8f72:	eb01 0002 	add\.w	r0, r1, r2
    8f76:	f7ff efc4 	blx	8f00 <armfn>
    8f7a:	eb01 0002 	add\.w	r0, r1, r2
    8f7e:	f7ff efc0 	blx	8f00 <armfn>
    8f82:	eb01 0002 	add\.w	r0, r1, r2
    8f86:	f7ff efbc 	blx	8f00 <armfn>
    8f8a:	eb01 0002 	add\.w	r0, r1, r2
    8f8e:	f7ff efb8 	blx	8f00 <armfn>
    8f92:	eb01 0002 	add\.w	r0, r1, r2
    8f96:	f7ff efb4 	blx	8f00 <armfn>
    8f9a:	eb01 0002 	add\.w	r0, r1, r2
    8f9e:	f7ff efb0 	blx	8f00 <armfn>
    8fa2:	eb01 0002 	add\.w	r0, r1, r2
    8fa6:	f7ff efac 	blx	8f00 <armfn>
    8faa:	eb01 0002 	add\.w	r0, r1, r2
    8fae:	f7ff efa8 	blx	8f00 <armfn>
    8fb2:	eb01 0002 	add\.w	r0, r1, r2
    8fb6:	f7ff efa4 	blx	8f00 <armfn>
    8fba:	eb01 0002 	add\.w	r0, r1, r2
    8fbe:	f7ff efa0 	blx	8f00 <armfn>
    8fc2:	eb01 0002 	add\.w	r0, r1, r2
    8fc6:	f7ff ef9c 	blx	8f00 <armfn>
    8fca:	eb01 0002 	add\.w	r0, r1, r2
    8fce:	f7ff ef98 	blx	8f00 <armfn>
    8fd2:	eb01 0002 	add\.w	r0, r1, r2
    8fd6:	f7ff ef94 	blx	8f00 <armfn>
    8fda:	eb01 0002 	add\.w	r0, r1, r2
    8fde:	f7ff ef90 	blx	8f00 <armfn>
    8fe2:	eb01 0002 	add\.w	r0, r1, r2
    8fe6:	f7ff ef8c 	blx	8f00 <armfn>
    8fea:	eb01 0002 	add\.w	r0, r1, r2
    8fee:	f7ff ef88 	blx	8f00 <armfn>
    8ff2:	eb01 0002 	add\.w	r0, r1, r2
    8ff6:	f7ff ef84 	blx	8f00 <armfn>
    8ffa:	eb01 0002 	add\.w	r0, r1, r2
    8ffe:	f000 e808 	blx	9010 <_start\+0x108>
    9002:	eb01 0002 	add\.w	r0, r1, r2
    9006:	f7ff ef7c 	blx	8f00 <armfn>
    900a:	4770      	bx	lr
#...
    9010:	eaffffba 	b	8f00 <armfn>
