#name: BTI PLT with only GNU PROP
#source: property-bti-pac1.s
#as: -mabi=lp64 -defsym __property_bti__=1
#ld: -e _start -L./tmpdir -lbti-plt-so
#objdump: -dr -j .plt
#target: *linux*

[^:]*: *file format elf64-.*aarch64

Disassembly of section \.plt:

[0-9a-f]+ <.*>:
.*:	d503245f 	bti	c
.*:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
.*:	90000090 	adrp	x16, 410000 <.*>
.*:	f9...... 	ldr	x17, \[x16, #....\]
.*:	91...... 	add	x16, x16, #0x...
.*:	d61f0220 	br	x17
.*:	d503201f 	nop
.*:	d503201f 	nop

[0-9a-f]+ <.*>:
.*:	d503245f 	bti	c
.*:	90000090 	adrp	x16, 410000 <.*>
.*:	f9...... 	ldr	x17, \[x16, #....\]
.*:	91...... 	add	x16, x16, #0x...
.*:	d61f0220 	br	x17
.*:	d503201f 	nop
