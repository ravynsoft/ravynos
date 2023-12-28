#objdump: -dr --prefix-addresses  --show-raw-insn -Mvirt,cp0-names=mips32r2
#name: virt instructions
#as: -32 -mvirt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 4063e800 	mfgc0	v1,c0_taghi
[0-9a-f]+ <[^>]*> 406ba005 	mfgc0	t3,\$20,5
[0-9a-f]+ <[^>]*> 40771200 	mtgc0	s7,c0_entrylo0
[0-9a-f]+ <[^>]*> 40677202 	mtgc0	a3,\$14,2
[0-9a-f]+ <[^>]*> 42000028 	hypcall
[0-9a-f]+ <[^>]*> 4212b028 	hypcall	0x256
[0-9a-f]+ <[^>]*> 4200000b 	tlbginv
[0-9a-f]+ <[^>]*> 4200000c 	tlbginvf
[0-9a-f]+ <[^>]*> 42000010 	tlbgp
[0-9a-f]+ <[^>]*> 42000009 	tlbgr
[0-9a-f]+ <[^>]*> 4200000a 	tlbgwi
[0-9a-f]+ <[^>]*> 4200000e 	tlbgwr
	...
