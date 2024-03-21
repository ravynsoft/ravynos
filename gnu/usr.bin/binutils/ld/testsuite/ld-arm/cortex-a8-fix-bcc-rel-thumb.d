
.*:     file format .*


Disassembly of section \.text:

00008f00 <targetfn>:
    8f00:	4770      	bx	lr
#...

00008f08 <_start>:
    8f08:	bf00      	nop
    8f0a:	eb01 0002 	add\.w	r0, r1, r2
    8f0e:	f53f aff7 	bmi\.w	8f00 <targetfn>
    8f12:	eb01 0002 	add\.w	r0, r1, r2
    8f16:	f53f aff3 	bmi\.w	8f00 <targetfn>
    8f1a:	eb01 0002 	add\.w	r0, r1, r2
    8f1e:	f53f afef 	bmi\.w	8f00 <targetfn>
    8f22:	eb01 0002 	add\.w	r0, r1, r2
    8f26:	f53f afeb 	bmi\.w	8f00 <targetfn>
    8f2a:	eb01 0002 	add\.w	r0, r1, r2
    8f2e:	f53f afe7 	bmi\.w	8f00 <targetfn>
    8f32:	eb01 0002 	add\.w	r0, r1, r2
    8f36:	f53f afe3 	bmi\.w	8f00 <targetfn>
    8f3a:	eb01 0002 	add\.w	r0, r1, r2
    8f3e:	f53f afdf 	bmi\.w	8f00 <targetfn>
    8f42:	eb01 0002 	add\.w	r0, r1, r2
    8f46:	f53f afdb 	bmi\.w	8f00 <targetfn>
    8f4a:	eb01 0002 	add\.w	r0, r1, r2
    8f4e:	f53f afd7 	bmi\.w	8f00 <targetfn>
    8f52:	eb01 0002 	add\.w	r0, r1, r2
    8f56:	f53f afd3 	bmi\.w	8f00 <targetfn>
    8f5a:	eb01 0002 	add\.w	r0, r1, r2
    8f5e:	f53f afcf 	bmi\.w	8f00 <targetfn>
    8f62:	eb01 0002 	add\.w	r0, r1, r2
    8f66:	f53f afcb 	bmi\.w	8f00 <targetfn>
    8f6a:	eb01 0002 	add\.w	r0, r1, r2
    8f6e:	f53f afc7 	bmi\.w	8f00 <targetfn>
    8f72:	eb01 0002 	add\.w	r0, r1, r2
    8f76:	f53f afc3 	bmi\.w	8f00 <targetfn>
    8f7a:	eb01 0002 	add\.w	r0, r1, r2
    8f7e:	f53f afbf 	bmi\.w	8f00 <targetfn>
    8f82:	eb01 0002 	add\.w	r0, r1, r2
    8f86:	f53f afbb 	bmi\.w	8f00 <targetfn>
    8f8a:	eb01 0002 	add\.w	r0, r1, r2
    8f8e:	f53f afb7 	bmi\.w	8f00 <targetfn>
    8f92:	eb01 0002 	add\.w	r0, r1, r2
    8f96:	f53f afb3 	bmi\.w	8f00 <targetfn>
    8f9a:	eb01 0002 	add\.w	r0, r1, r2
    8f9e:	f53f afaf 	bmi\.w	8f00 <targetfn>
    8fa2:	eb01 0002 	add\.w	r0, r1, r2
    8fa6:	f53f afab 	bmi\.w	8f00 <targetfn>
    8faa:	eb01 0002 	add\.w	r0, r1, r2
    8fae:	f53f afa7 	bmi\.w	8f00 <targetfn>
    8fb2:	eb01 0002 	add\.w	r0, r1, r2
    8fb6:	f53f afa3 	bmi\.w	8f00 <targetfn>
    8fba:	eb01 0002 	add\.w	r0, r1, r2
    8fbe:	f53f af9f 	bmi\.w	8f00 <targetfn>
    8fc2:	eb01 0002 	add\.w	r0, r1, r2
    8fc6:	f53f af9b 	bmi\.w	8f00 <targetfn>
    8fca:	eb01 0002 	add\.w	r0, r1, r2
    8fce:	f53f af97 	bmi\.w	8f00 <targetfn>
    8fd2:	eb01 0002 	add\.w	r0, r1, r2
    8fd6:	f53f af93 	bmi\.w	8f00 <targetfn>
    8fda:	eb01 0002 	add\.w	r0, r1, r2
    8fde:	f53f af8f 	bmi\.w	8f00 <targetfn>
    8fe2:	eb01 0002 	add\.w	r0, r1, r2
    8fe6:	f53f af8b 	bmi\.w	8f00 <targetfn>
    8fea:	eb01 0002 	add\.w	r0, r1, r2
    8fee:	f53f af87 	bmi\.w	8f00 <targetfn>
    8ff2:	eb01 0002 	add\.w	r0, r1, r2
    8ff6:	f53f af83 	bmi\.w	8f00 <targetfn>
    8ffa:	eb01 0002 	add\.w	r0, r1, r2
    8ffe:	f000 b807 	b\.w	9010 <_start\+0x108>
    9002:	eb01 0002 	add\.w	r0, r1, r2
    9006:	f53f af7b 	bmi\.w	8f00 <targetfn>
    900a:	4770      	bx	lr
#...
    9010:	d401      	bmi\.n	9016 <_start\+0x10e>
    9012:	f7ff bff6 	b\.w	9002 <_start\+0xfa>
    9016:	f7ff bf73 	b\.w	8f00 <targetfn>
