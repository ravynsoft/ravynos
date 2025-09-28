#source: ifunc-7.s
#target: [check_shared_lib_support]
#ld: -shared
#objdump: -dr -j .text

# Check if adrp and ldr have been relocated correctly.

.*:     file format elf.+aarch64.*


Disassembly of section \.text:

[0-9a-f]+ <foo>:
 [0-9a-f]+:	d65f03c0 	ret

[0-9a-f]+ <__start>:
 [0-9a-f]+:	[0-9a-f]+ 	bl	[0-9a-f]+ <\*ABS\*\+0x[0-9a-f]+@plt>
 [0-9a-f]+:	[0-9a-f]+ 	adrp	x0, [0-9]+ <__start\+0x[0-9a-f]+>
 [0-9a-f]+:	[0-9a-f]+ 	ldr	x0, \[x0, .+\]
