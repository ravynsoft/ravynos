#as: -march=z80-full+infc
#objdump: -d
#name: Z80 undocumented instruction IN F,(C)

.*:.*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+ed 70\s+in f,\(c\)
