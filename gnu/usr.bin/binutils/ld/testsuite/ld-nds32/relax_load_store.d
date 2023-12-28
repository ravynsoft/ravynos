#as: -Os
#ld: -static --relax -T	\$srcdir/\$subdir/relax_load_store.ld
#objdump: -d --prefix-addresses -j .text

.*:     file format .*nds32.*


Disassembly of section .text:
0+0000 <[^>]*> lwi.gp	\$r0, \[ \+ #0\]
0+0004 <[^>]*> lhi.gp	\$r0, \[ \+ #4\]
0+0008 <[^>]*> lbi.gp	\$r0, \[ \+ #6\]

