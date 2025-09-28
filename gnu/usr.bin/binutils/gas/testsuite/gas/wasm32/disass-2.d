#as:
#objdump: -d -Mglobals
#name: disass-2.d

.*:     file format elf32-wasm32

Disassembly of section \.text:$
00000000 <\.text>:$
   0:	20 00       		get_local 0
   2:	23 00       		get_global 0 <\$got>
