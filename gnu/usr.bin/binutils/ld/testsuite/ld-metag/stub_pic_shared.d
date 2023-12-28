tmpdir/stub_pic_shared.so:     file format elf32-metag
architecture: metag, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

.* <.*>:
 .*:	82900101 	          ADDT      A0.2,CPC0,#0x20
 .*:	82120660 	          ADD       A0.2,A0.2,#0x40cc
 .*:	a3100c20 	          MOV       D0Re0,A0.2
 .*:	b70001e3 	          SETL      \[A0StP\+\+\],D0Re0,D1Re0
 .*:	c600012a 	          GETD      PC,\[D0Re0\+#4\]
.* <_far2@plt>:
 .*:	82900101 	          ADDT      A0.2,CPC0,#0x20
 .*:	82120580 	          ADD       A0.2,A0.2,#0x40b0
 .*:	c600806a 	          GETD      PC,\[A0.2\]
 .*:	03000004 	          MOV       D1Re0,#0
 .*:	a0fffee0 	          B         .* <.*>
Disassembly of section .text:
.* <__start-0xc>:
.*:	82980101 	          ADDT      A0.3,CPC0,#0x20
.*:	82180100 	          ADD       A0.3,A0.3,#0x20
.*:	a3180ca0 	          MOV       PC,A0.3
.* <__start>:
.*:	abffffb4 	          CALLR     D1RtP,.* <_far2@plt\+0x14>
	\.\.\.
.* <pad_end>:
.*:	829ffef9 	          ADDT      A0.3,CPC0,#0xffdf
.*:	821ffee0 	          ADD       A0.3,A0.3,#0xffdc
.*:	a3180ca0 	          MOV       PC,A0.3
.* <_far2>:
.*:	a0fffffe 	          NOP
.* <_far>:
.*:	abffff94 	          CALLR     D1RtP,.* <pad_end>
