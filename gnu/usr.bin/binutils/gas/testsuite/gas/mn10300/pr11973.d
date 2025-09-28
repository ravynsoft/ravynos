#objdump: -dr --prefix-address --show-raw-insn
#name: PR11973

.*: +file format.*elf32-[am3|mn10300].*

Disassembly of section .text:
#...
0+0100.*
0+0103 <SomeProc_40000100\+0x3> dd 00 00 00 00 80 04[ 	]+call.*
#...
0+0200.*
0+0203 <SomeProc_40000200\+0x3> dd 00 00 00 00 80 04[ 	]+call.*
#pass
