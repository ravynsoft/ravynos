#as: -Os
#ld: -static --relax -T	$srcdir/$subdir/relax_jmp.ld
#objdump: -d --prefix-addresses -j .text

.*:     file format .*nds32.*


Disassembly of section .text:
0+0000 <[^>]*> j	00000008 <main>
0+0004 <[^>]*> jal	00000008 <main>
0+0008 <[^>]*> nop16

