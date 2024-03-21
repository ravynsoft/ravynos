#as: -Os
#ld: -static -T	\$srcdir/\$subdir/gp.ld
#objdump: -d --prefix-addresses -j .text

.*:     file format .*nds32.*


Disassembly of section .text:
0+0000 <[^>]*> addi.gp	\$r0, .*
0+0004 <[^>]*> lbi.gp	\$r0, \[.*\]
0+0008 <[^>]*> lbsi.gp	\$r0, \[.*\]
0+000c <[^>]*> lhi.gp	\$r0, \[.*\]
0+0010 <[^>]*> lhsi.gp	\$r0, \[.*\]
0+0014 <[^>]*> lwi.gp	\$r0, \[.*\]
0+0018 <[^>]*> sbi.gp	\$r0, \[.*\]
0+001c <[^>]*> shi.gp	\$r0, \[.*\]
0+0020 <[^>]*> swi.gp	\$r0, \[.*\]

