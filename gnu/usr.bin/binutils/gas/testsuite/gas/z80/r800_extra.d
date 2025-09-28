#as: -march=r800
#objdump: -d
#name: R800 specific instructions

.*:.*

Disassembly of section .text:

0+ <.text>:
\s+[0-9a-f]+:[	]ed 70[       	]+in f,\(c\)
\s+[0-9a-f]+:[	]ed c5[       	]+mulub a,b
\s+[0-9a-f]+:[	]ed cd[       	]+mulub a,c
\s+[0-9a-f]+:[	]ed d5[       	]+mulub a,d
\s+[0-9a-f]+:[	]ed dd[       	]+mulub a,e
\s+[0-9a-f]+:[	]ed c3[       	]+muluw hl,bc
\s+[0-9a-f]+:[	]ed f3[       	]+muluw hl,sp
