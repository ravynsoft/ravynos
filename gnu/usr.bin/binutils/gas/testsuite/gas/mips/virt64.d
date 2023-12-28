#objdump: -dr --prefix-addresses  --show-raw-insn -Mvirt,cp0-names=mips64r2
#name: virt64 instructions
#as: -64 -mvirt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4063e900 	dmfgc0	v1,c0_taghi
[0-9a-f]+ <[^>]*> 406ba105 	dmfgc0	a7,\$20,5
[0-9a-f]+ <[^>]*> 40771300 	dmtgc0	s7,c0_entrylo0
[0-9a-f]+ <[^>]*> 40677302 	dmtgc0	a3,\$14,2
	...
