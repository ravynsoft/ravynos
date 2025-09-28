#objdump: -dr --prefix-addresses --show-raw-insn
#name: NIOS2 R2 ctl
#as: -march=r2
#source: ctl.s

# Test the ctl instructions

.*: +file format elf32-littlenios2

Disassembly of section .text:
0+0000 <[^>]*> 9be80020 	rdctl	r8,ctl31
0+0004 <[^>]*> 9bc80020 	rdctl	r8,ctl30
0+0008 <[^>]*> 9ba80020 	rdctl	r8,ctl29
0+000c <[^>]*> 98080020 	rdctl	r8,status
0+0010 <[^>]*> 98480020 	rdctl	r8,bstatus
0+0014 <[^>]*> 98280020 	rdctl	r8,estatus
0+0018 <[^>]*> bbe00220 	wrctl	ctl31,r8
0+001c <[^>]*> bbc00220 	wrctl	ctl30,r8
0+0020 <[^>]*> bba00220 	wrctl	ctl29,r8
0+0024 <[^>]*> b8000220 	wrctl	status,r8
0+0028 <[^>]*> b8400220 	wrctl	bstatus,r8
0+002c <[^>]*> b8200220 	wrctl	estatus,r8
