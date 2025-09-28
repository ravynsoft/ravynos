#source: imm.s
#source: imm_symbol.s
#as: -Os
#ld: -static -T	$srcdir/$subdir/imm.ld --relax
#objdump: -d --prefix-addresses -j .text

.*:     file format .*nds32.*


Disassembly of section .text:
0+1000 <[^>]*> sethi	\$r0, #0x11223
0+1004 <[^>]*> ori	\$r0, \$r0, #0x344
0+1008 <[^>]*> movi	\$r0, #0x11223
0+100c <[^>]*> movi55[ 	]+\$r0, #0xf

