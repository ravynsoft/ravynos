#as: -64 -mfix-cn63xxp1
#objdump: -M reg-names=numeric -dr
#name: MIPS octeon-pref mfix-cn63xxp1

.*:     file format .*

Disassembly of section .text:

[0-9a-f]+ <foo>:
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc050000 	pref	0x5,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc190000 	pref	0x19,0\(\$0\)
.*:	cc1a0000 	pref	0x1a,0\(\$0\)
.*:	cc1b0000 	pref	0x1b,0\(\$0\)
.*:	cc1c0000 	pref	0x1c,0\(\$0\)
.*:	cc1d0000 	pref	0x1d,0\(\$0\)
.*:	cc1e0000 	pref	0x1e,0\(\$0\)
.*:	cc1f0000 	pref	0x1f,0\(\$0\)
#pass
