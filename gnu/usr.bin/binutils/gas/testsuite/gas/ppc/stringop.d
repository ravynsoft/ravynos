#as: -mpower10
#objdump: -dr -Mpower10
#name: string operations

.*


Disassembly of section \.text:

0+0 <_start>:
.*:	(10 01 08 0d|0d 08 01 10) 	vstribr v0,v1
.*:	(10 41 1c 0d|0d 1c 41 10) 	vstribr. v2,v3
.*:	(10 80 28 0d|0d 28 80 10) 	vstribl v4,v5
.*:	(10 c0 3c 0d|0d 3c c0 10) 	vstribl. v6,v7
.*:	(11 03 48 0d|0d 48 03 11) 	vstrihr v8,v9
.*:	(11 43 5c 0d|0d 5c 43 11) 	vstrihr. v10,v11
.*:	(11 82 68 0d|0d 68 82 11) 	vstrihl v12,v13
.*:	(11 c2 7c 0d|0d 7c c2 11) 	vstrihl. v14,v15
.*:	(12 11 91 8d|8d 91 11 12) 	vclrlb  v16,v17,r18
.*:	(12 74 a9 cd|cd a9 74 12) 	vclrrb  v19,v20,r21
