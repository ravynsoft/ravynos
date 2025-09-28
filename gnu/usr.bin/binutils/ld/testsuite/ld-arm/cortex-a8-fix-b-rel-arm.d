
.*:     file format .*


Disassembly of section \.text:

00008f00 <targetfn>:
    8f00:	e12fff1e 	bx	lr
#...

00008f08 <_start>:
    8f08:	bf00      	nop
    8f0a:	eb01 0002 	add\.w	r0, r1, r2
    8f0e:	f000 b87f 	b\.w	9010 <__targetfn_from_thumb>
    8f12:	eb01 0002 	add\.w	r0, r1, r2
    8f16:	f000 b87b 	b\.w	9010 <__targetfn_from_thumb>
    8f1a:	eb01 0002 	add\.w	r0, r1, r2
    8f1e:	f000 b877 	b\.w	9010 <__targetfn_from_thumb>
    8f22:	eb01 0002 	add\.w	r0, r1, r2
    8f26:	f000 b873 	b\.w	9010 <__targetfn_from_thumb>
    8f2a:	eb01 0002 	add\.w	r0, r1, r2
    8f2e:	f000 b86f 	b\.w	9010 <__targetfn_from_thumb>
    8f32:	eb01 0002 	add\.w	r0, r1, r2
    8f36:	f000 b86b 	b\.w	9010 <__targetfn_from_thumb>
    8f3a:	eb01 0002 	add\.w	r0, r1, r2
    8f3e:	f000 b867 	b\.w	9010 <__targetfn_from_thumb>
    8f42:	eb01 0002 	add\.w	r0, r1, r2
    8f46:	f000 b863 	b\.w	9010 <__targetfn_from_thumb>
    8f4a:	eb01 0002 	add\.w	r0, r1, r2
    8f4e:	f000 b85f 	b\.w	9010 <__targetfn_from_thumb>
    8f52:	eb01 0002 	add\.w	r0, r1, r2
    8f56:	f000 b85b 	b\.w	9010 <__targetfn_from_thumb>
    8f5a:	eb01 0002 	add\.w	r0, r1, r2
    8f5e:	f000 b857 	b\.w	9010 <__targetfn_from_thumb>
    8f62:	eb01 0002 	add\.w	r0, r1, r2
    8f66:	f000 b853 	b\.w	9010 <__targetfn_from_thumb>
    8f6a:	eb01 0002 	add\.w	r0, r1, r2
    8f6e:	f000 b84f 	b\.w	9010 <__targetfn_from_thumb>
    8f72:	eb01 0002 	add\.w	r0, r1, r2
    8f76:	f000 b84b 	b\.w	9010 <__targetfn_from_thumb>
    8f7a:	eb01 0002 	add\.w	r0, r1, r2
    8f7e:	f000 b847 	b\.w	9010 <__targetfn_from_thumb>
    8f82:	eb01 0002 	add\.w	r0, r1, r2
    8f86:	f000 b843 	b\.w	9010 <__targetfn_from_thumb>
    8f8a:	eb01 0002 	add\.w	r0, r1, r2
    8f8e:	f000 b83f 	b\.w	9010 <__targetfn_from_thumb>
    8f92:	eb01 0002 	add\.w	r0, r1, r2
    8f96:	f000 b83b 	b\.w	9010 <__targetfn_from_thumb>
    8f9a:	eb01 0002 	add\.w	r0, r1, r2
    8f9e:	f000 b837 	b\.w	9010 <__targetfn_from_thumb>
    8fa2:	eb01 0002 	add\.w	r0, r1, r2
    8fa6:	f000 b833 	b\.w	9010 <__targetfn_from_thumb>
    8faa:	eb01 0002 	add\.w	r0, r1, r2
    8fae:	f000 b82f 	b\.w	9010 <__targetfn_from_thumb>
    8fb2:	eb01 0002 	add\.w	r0, r1, r2
    8fb6:	f000 b82b 	b\.w	9010 <__targetfn_from_thumb>
    8fba:	eb01 0002 	add\.w	r0, r1, r2
    8fbe:	f000 b827 	b\.w	9010 <__targetfn_from_thumb>
    8fc2:	eb01 0002 	add\.w	r0, r1, r2
    8fc6:	f000 b823 	b\.w	9010 <__targetfn_from_thumb>
    8fca:	eb01 0002 	add\.w	r0, r1, r2
    8fce:	f000 b81f 	b\.w	9010 <__targetfn_from_thumb>
    8fd2:	eb01 0002 	add\.w	r0, r1, r2
    8fd6:	f000 b81b 	b\.w	9010 <__targetfn_from_thumb>
    8fda:	eb01 0002 	add\.w	r0, r1, r2
    8fde:	f000 b817 	b\.w	9010 <__targetfn_from_thumb>
    8fe2:	eb01 0002 	add\.w	r0, r1, r2
    8fe6:	f000 b813 	b\.w	9010 <__targetfn_from_thumb>
    8fea:	eb01 0002 	add\.w	r0, r1, r2
    8fee:	f000 b80f 	b\.w	9010 <__targetfn_from_thumb>
    8ff2:	eb01 0002 	add\.w	r0, r1, r2
    8ff6:	f000 b80b 	b\.w	9010 <__targetfn_from_thumb>
    8ffa:	eb01 0002 	add\.w	r0, r1, r2
    8ffe:	f000 b807 	b\.w	9010 <__targetfn_from_thumb>
    9002:	eb01 0002 	add\.w	r0, r1, r2
    9006:	f000 b803 	b\.w	9010 <__targetfn_from_thumb>
    900a:	4770      	bx	lr
#...

00009010 <__targetfn_from_thumb>:
    9010:	4778      	bx	pc
    9012:	e7fd      	b.n	.+ <.+>
    9014:	eaffffb9 	b	8f00 <targetfn>
