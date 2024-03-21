#skip: *-*-pe *-*-wince *-*-vxworks
#objdump: -dr --prefix-addresses --show-raw-insn
#name: PR9722: Generation of Thumb NOP instruction

.*: +file format .*arm.*

Disassembly of section .text:
0+0 <.*> 46c0[ 	]+nop.*
0+2 <.*> 46c0[ 	]+nop.*
0+4 <.*> bf00[ 	]+nop
