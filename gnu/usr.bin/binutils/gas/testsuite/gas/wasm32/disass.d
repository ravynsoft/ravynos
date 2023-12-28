#as:
#objdump: -d -Mregisters,globals
#name: disass.d

.*:     file format elf32-wasm32

Disassembly of section \.text:
00000000 <\.text>:$
   0:	20 00       		get_local 0 <\$dpc>
   2:	23 00       		get_global 0 <\$got>
