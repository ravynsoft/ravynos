#as: -march=ez80
#objdump: -d
#name: PR 28791: Do not complain about overlarge bit manipulated constants

.*:.*

Disassembly of section .text:

0+ <.text>:
\s+0:\s+1e 19\s+ld e,0x19
\s+2:\s+1e 1a\s+ld e,0x1a
\s+4:\s+1e e6\s+ld e,0xe6
\s+6:\s+1e ff\s+ld e,0xff
\s+8:\s+1e 00\s+ld e,0x00
\s+a:\s+1e f9\s+ld e,0xf9
\s+c:\s+1e cb\s+ld e,0xcb
