tmpdir/stub:     file format elf32-metag
architecture: metag, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x.*

Disassembly of section .text:
.* <__start-0x8>:
.*:	82188105 	          MOVT      A0.3,#0x1020
.*:	ac1a8303 	          JUMP      A0.3,#0x5060
.* <__start>:
.*:	abffffd4 	          CALLR     D1RtP,.* <__start-0x8>
	\.\.\.
.* <_far>:
.*:	a0fffffe 	          NOP
