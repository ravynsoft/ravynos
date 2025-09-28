#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 ctl

# Test the ctl instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 001137fa 	rdctl	r8,ctl31
0+0004 <[^>]*> 001137ba 	rdctl	r8,ctl30
0+0008 <[^>]*> 0011377a 	rdctl	r8,ctl29
0+000c <[^>]*> 0011303a 	rdctl	r8,status
0+0010 <[^>]*> 001130ba 	rdctl	r8,bstatus
0+0014 <[^>]*> 0011307a 	rdctl	r8,estatus
0+0018 <[^>]*> 400177fa 	wrctl	ctl31,r8
0+001c <[^>]*> 400177ba 	wrctl	ctl30,r8
0+0020 <[^>]*> 4001777a 	wrctl	ctl29,r8
0+0024 <[^>]*> 4001703a 	wrctl	status,r8
0+0028 <[^>]*> 400170ba 	wrctl	bstatus,r8
0+002c <[^>]*> 4001707a 	wrctl	estatus,r8
