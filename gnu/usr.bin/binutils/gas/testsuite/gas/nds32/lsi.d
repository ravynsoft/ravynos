#objdump: -d --prefix-addresses
#name: nds32 load-store immediate instructions
#as:

# Test lsi instructions

.*:     file format .*

Disassembly of section .text:
0+0000 <[^>]*> lwi	\$r0, \[\$r1 \+ #4\]
0+0004 <[^>]*> lhi	\$r0, \[\$r1 \+ #2\]
0+0008 <[^>]*> lhsi	\$r0, \[\$r1 \+ #-2\]
0+000c <[^>]*> lbi	\$r0, \[\$r1 \+ #1\]
0+0010 <[^>]*> lbsi	\$r0, \[\$r1 \+ #-1\]
0+0014 <[^>]*> swi	\$r0, \[\$r1 \+ #4\]
0+0018 <[^>]*> shi	\$r0, \[\$r1 \+ #2\]
0+001c <[^>]*> sbi	\$r0, \[\$r1 \+ #1\]
0+0020 <[^>]*> lwi.bi	\$r0, \[\$r1\], #4
0+0024 <[^>]*> lhi.bi	\$r0, \[\$r1\], #2
0+0028 <[^>]*> lhsi.bi	\$r0, \[\$r1\], #-2
0+002c <[^>]*> lbi.bi	\$r0, \[\$r1\], #1
0+0030 <[^>]*> lbsi.bi	\$r0, \[\$r1\], #-1
0+0034 <[^>]*> swi.bi	\$r0, \[\$r1\], #4
0+0038 <[^>]*> shi.bi	\$r0, \[\$r1\], #2
0+003c <[^>]*> sbi.bi	\$r0, \[\$r1\], #1
