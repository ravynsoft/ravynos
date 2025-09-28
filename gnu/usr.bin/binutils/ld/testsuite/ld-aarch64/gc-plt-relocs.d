#source: gc-plt1.s
#source: gc-plt-main.s
#source: gc-plt-hidden.s
#source: gc-plt2.s
#target: [check_shared_lib_support]
#ld: --gc-sections -T aarch64.ld --shared --hash-style=sysv
#objdump: -dT

# Shared object with plt related relocs against global symbol
# and local functions gced.  After gc-section removal we are
# checking that the function does not exist.

.*:     file format elf64-(little|big)aarch64

DYNAMIC SYMBOL TABLE:
0+8000 g    DF \.text	0+4 _start
0+0000      D  \*UND\*	0+ foo
0+8008 g    DF \.text	0+ bar

Disassembly of section .text:

0+8000 \<_start\>:
    8000:	9400000c 	bl	8030 \<.*>

0+8004 \<hidfn\>:
    8004:	8a000000 	and	x0, x0, x0

0+8008 \<bar\>:
    8008:	14000001 	b	800c \<foo\>

0+800c \<foo\>:
    800c:	97fffffe 	bl	8004 \<hidfn\>

Disassembly of section .plt:

0+8010 \<\.plt\>:
    8010:	a9bf7bf0 	stp	x16, x30, \[sp, #-16\]!
    8014:	b0000010 	adrp	x16, 9000 .*
    8018:	f9400e11 	ldr	x17, \[x16, #24\]
    801c:	91006210 	add	x16, x16, #0x18
    8020:	d61f0220 	br	x17
    8024:	d503201f 	nop
    8028:	d503201f 	nop
    802c:	d503201f 	nop
    8030:	b0000010 	adrp	x16, 9000 .*
    8034:	f9401211 	ldr	x17, \[x16, #32\]
    8038:	91008210 	add	x16, x16, #0x20
    803c:	d61f0220 	br	x17
