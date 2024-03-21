#objdump: -dr --prefix-addresses  --show-raw-insn -Mvirt,cp0-names=mips32r2
#name: virt instructions
#source: virt.s
#as: -32 -mvirt

.*: +file format .*mips.*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 007d 04fc 	mfgc0	v1,c0_taghi
[0-9a-f]+ <[^>]*> 0174 2cfc 	mfgc0	t3,\$20,5
[0-9a-f]+ <[^>]*> 02e2 06fc 	mtgc0	s7,c0_entrylo0
[0-9a-f]+ <[^>]*> 00ee 16fc 	mtgc0	a3,\$14,2
[0-9a-f]+ <[^>]*> 0000 c37c 	hypcall
[0-9a-f]+ <[^>]*> 0256 c37c 	hypcall	0x256
[0-9a-f]+ <[^>]*> 0000 417c 	tlbginv
[0-9a-f]+ <[^>]*> 0000 517c 	tlbginvf
[0-9a-f]+ <[^>]*> 0000 017c 	tlbgp
[0-9a-f]+ <[^>]*> 0000 117c 	tlbgr
[0-9a-f]+ <[^>]*> 0000 217c 	tlbgwi
[0-9a-f]+ <[^>]*> 0000 317c 	tlbgwr
	\.\.\.
