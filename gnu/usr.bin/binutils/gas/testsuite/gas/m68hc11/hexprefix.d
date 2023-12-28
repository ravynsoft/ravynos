#objdump: -d -mm9s12x --prefix-addresses --reloc
#as: -mm9s12x
#name: verify hex prefixes present and not duplicated (hexprefix)

.*:     file format elf32-m68hc12


Disassembly of section .text:
0x00000000 ldaa	0x00001234
0x00000003 ldab	#0x12
0x00000005 ldd	\*0x00000023
0x00000007 ldx	#0x00001234
0x0000000a movw	0x00001234, 0x00002345
0x00000010 movb	0x00003456, 0x00004567
0x00000016 orx	0x00008765
0x0000001a call	0x00104007 \{0x00008007, 0x3d\}
			1a: R_M68HC12_RL_JUMP	\*ABS\*
0x0000001e movw	#0x00001234, 0x00002345
0x00000024 movb	#0x23, 0x00003456
