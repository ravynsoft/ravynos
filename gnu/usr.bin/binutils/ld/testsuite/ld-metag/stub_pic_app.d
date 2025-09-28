tmpdir/stub_pic_app:     file format elf32-metag
architecture: metag, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x.*

Disassembly of section .plt:

.* <.*>:
.*:	02008105 	          MOVT      D0Re0,#0x1020
.*:	02049480 	          ADD       D0Re0,D0Re0,#0x9290
.*:	b70001e3 	          SETL      \[A0StP\+\+\],D0Re0,D1Re0
.*:	c600012a 	          GETD      PC,\[D0Re0\+#4\]
.*:	a0fffffe 	          NOP
.* <_lib_func@plt>:
.*:	82108105 	          MOVT      A0.2,#0x1020
.*:	82149440 	          ADD       A0.2,A0.2,#0x9288
.*:	c600806a 	          GETD      PC,\[A0.2\]
.*:	03000004 	          MOV       D1Re0,#0
.*:	a0fffee0 	          B         .* <.*>
Disassembly of section .text:
.* <__start-0x10>:
.*:	82188105 	          MOVT      A0.3,#0x1020
.*:	ac1a8f03 	          JUMP      A0.3,#0x51e0
.*:	82188105 	          MOVT      A0.3,#0x1020
.*:	ac1a8ee3 	          JUMP      A0.3,#0x51dc
.* <__start>:
.*:	abffff94 	          CALLR     D1RtP,.* <_lib_func@plt\+0x14>
.*:	abfffed4 	          CALLR     D1RtP,.* <_lib_func@plt>
.*:	abffff94 	          CALLR     D1RtP,.* <_lib_func@plt\+0x1c>
	\.\.\.
.* <_far2>:
.*:	a0fffffe 	          NOP
.* <_far>:
.*:	abfffff4 	          CALLR     D1RtP,.* <_far2>
