#as: -a32
#source: xcoff-br16-1.s
#objdump: -P relocs -dr
#name: XCOFF R_RBR/16 reloc test 1

.*
Relocations for \.text .*
vaddr    sgn mod sz type  symndx symbol
00000002  S      16 RBR   [0-9]      c
00000006  S      16 RBR   [0-9]      c



Disassembly of section \.text:

00000000 <\.text>:
   0:	40 82 00 00 	bne     0x0
			2: R_RBR_16	c
   4:	40 82 ff fd 	bnel    0x0
			6: R_RBR_16	c
