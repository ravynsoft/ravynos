
tmpdir/stub_shared.so:     file format elf32-metag
architecture: metag, flags 0x00000150:
HAS_SYMS, DYNAMIC, D_PAGED
start address 0x.*

Disassembly of section .plt:

.* <.*>:
 .*:	82900001 	          ADDT      A0.2,CPC0,#0
 .*:	82120700 	          ADD       A0.2,A0.2,#0x40e0
 .*:	a3100c20 	          MOV       D0Re0,A0.2
 .*:	b70001e3 	          SETL      \[A0StP\+\+\],D0Re0,D1Re0
 .*:	c600012a 	          GETD      PC,\[D0Re0\+#4\]
.* <_far2@plt>:
 .*:	82900001 	          ADDT      A0.2,CPC0,#0
 .*:	82120620 	          ADD       A0.2,A0.2,#0x40c4
 .*:	c600806a 	          GETD      PC,\[A0.2\]
 .*:	03000004 	          MOV       D1Re0,#0
 .*:	a0fffee0 	          B         .* <.*>
Disassembly of section .text:

.* <_lib_func>:
 .*:	00203205 	          MOV       D0FrT,A0FrP
 .*:	86080026 	          ADD       A0FrP,A0StP,#0
 .*:	b72001e3 	          SETL      \[A0StP\+\+\],D0FrT,D1RtP
 .*:	b60802e9 	          SETD      \[A0StP\+#8\+\+\],A1LbP
 .*:	82000040 	          ADD       A0StP,A0StP,#0x8
 .*:	83880001 	          ADDT      A1LbP,CPC1,#0
 .*:	830b0500 	          ADD       A1LbP,A1LbP,#0x60a0
 .*:	abfffe94 	          CALLR     D1RtP,.* <_far2@plt>
 .*:	a70ffe64 	          GETD      A1LbP,\[A0StP\+#-16\]
 .*:	c72041e3 	          GETL      D0FrT,D1RtP,\[A0FrP\+\+\]
 .*:	8e004226 	          SUB       A0StP,A0FrP,#0x8
 .*:	80081805 	          MOV       A0FrP,D0FrT
 .*:	a32008a0 	          MOV       PC,D1RtP
