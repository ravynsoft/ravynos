
.*:     file format .*


Disassembly of section \.text:

00008f00 <_start>:
    8f00:	bf00      	nop
    8f02:	eb01 0002 	add\.w	r0, r1, r2
    8f06:	f4ff affc 	bcc\.w	8f02 <_start\+0x2>
    8f0a:	eb01 0002 	add\.w	r0, r1, r2
    8f0e:	f4ff aff8 	bcc\.w	8f02 <_start\+0x2>
    8f12:	eb01 0002 	add\.w	r0, r1, r2
    8f16:	f4ff aff4 	bcc\.w	8f02 <_start\+0x2>
    8f1a:	eb01 0002 	add\.w	r0, r1, r2
    8f1e:	f4ff aff0 	bcc\.w	8f02 <_start\+0x2>
    8f22:	eb01 0002 	add\.w	r0, r1, r2
    8f26:	f4ff affc 	bcc\.w	8f22 <_start\+0x22>
    8f2a:	eb01 0002 	add\.w	r0, r1, r2
    8f2e:	f4ff aff8 	bcc\.w	8f22 <_start\+0x22>
    8f32:	eb01 0002 	add\.w	r0, r1, r2
    8f36:	f4ff aff4 	bcc\.w	8f22 <_start\+0x22>
    8f3a:	eb01 0002 	add\.w	r0, r1, r2
    8f3e:	f4ff aff0 	bcc\.w	8f22 <_start\+0x22>
    8f42:	eb01 0002 	add\.w	r0, r1, r2
    8f46:	f4ff affc 	bcc\.w	8f42 <_start\+0x42>
    8f4a:	eb01 0002 	add\.w	r0, r1, r2
    8f4e:	f4ff aff8 	bcc\.w	8f42 <_start\+0x42>
    8f52:	eb01 0002 	add\.w	r0, r1, r2
    8f56:	f4ff aff4 	bcc\.w	8f42 <_start\+0x42>
    8f5a:	eb01 0002 	add\.w	r0, r1, r2
    8f5e:	f4ff aff0 	bcc\.w	8f42 <_start\+0x42>
    8f62:	eb01 0002 	add\.w	r0, r1, r2
    8f66:	f4ff affc 	bcc\.w	8f62 <_start\+0x62>
    8f6a:	eb01 0002 	add\.w	r0, r1, r2
    8f6e:	f4ff aff8 	bcc\.w	8f62 <_start\+0x62>
    8f72:	eb01 0002 	add\.w	r0, r1, r2
    8f76:	f4ff aff4 	bcc\.w	8f62 <_start\+0x62>
    8f7a:	eb01 0002 	add\.w	r0, r1, r2
    8f7e:	f4ff aff0 	bcc\.w	8f62 <_start\+0x62>
    8f82:	eb01 0002 	add\.w	r0, r1, r2
    8f86:	f4ff affc 	bcc\.w	8f82 <_start\+0x82>
    8f8a:	eb01 0002 	add\.w	r0, r1, r2
    8f8e:	f4ff aff8 	bcc\.w	8f82 <_start\+0x82>
    8f92:	eb01 0002 	add\.w	r0, r1, r2
    8f96:	f4ff aff4 	bcc\.w	8f82 <_start\+0x82>
    8f9a:	eb01 0002 	add\.w	r0, r1, r2
    8f9e:	f4ff aff0 	bcc\.w	8f82 <_start\+0x82>
    8fa2:	eb01 0002 	add\.w	r0, r1, r2
    8fa6:	f4ff affc 	bcc\.w	8fa2 <_start\+0xa2>
    8faa:	eb01 0002 	add\.w	r0, r1, r2
    8fae:	f4ff aff8 	bcc\.w	8fa2 <_start\+0xa2>
    8fb2:	eb01 0002 	add\.w	r0, r1, r2
    8fb6:	f4ff aff4 	bcc\.w	8fa2 <_start\+0xa2>
    8fba:	eb01 0002 	add\.w	r0, r1, r2
    8fbe:	f4ff aff0 	bcc\.w	8fa2 <_start\+0xa2>
    8fc2:	eb01 0002 	add\.w	r0, r1, r2
    8fc6:	f4ff affc 	bcc\.w	8fc2 <_start\+0xc2>
    8fca:	eb01 0002 	add\.w	r0, r1, r2
    8fce:	f4ff aff8 	bcc\.w	8fc2 <_start\+0xc2>
    8fd2:	eb01 0002 	add\.w	r0, r1, r2
    8fd6:	f4ff aff4 	bcc\.w	8fc2 <_start\+0xc2>
    8fda:	eb01 0002 	add\.w	r0, r1, r2
    8fde:	f4ff aff0 	bcc\.w	8fc2 <_start\+0xc2>
    8fe2:	eb01 0002 	add\.w	r0, r1, r2
    8fe6:	f4ff affc 	bcc\.w	8fe2 <_start\+0xe2>
    8fea:	eb01 0002 	add\.w	r0, r1, r2
    8fee:	f4ff aff8 	bcc\.w	8fe2 <_start\+0xe2>
    8ff2:	eb01 0002 	add\.w	r0, r1, r2
    8ff6:	f4ff aff4 	bcc\.w	8fe2 <_start\+0xe2>
    8ffa:	eb01 0002 	add\.w	r0, r1, r2
    8ffe:	f000 b803 	b\.w	9008 <_start\+0x108>
    9002:	4770      	bx	lr
#...
    9008:	d301      	bcc\.n	900e <_start\+0x10e>
    900a:	f7ff bffa 	b\.w	9002 <_start\+0x102>
    900e:	f7ff bfe8 	b\.w	8fe2 <_start\+0xe2>
