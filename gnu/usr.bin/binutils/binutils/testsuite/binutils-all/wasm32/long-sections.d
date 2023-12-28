#PROG: objcopy
#source: long-sections.s
#as:
#objcopy: -Ielf32-wasm32 -Owasm
#objdump: -bbinary -s

.*:.*file format binary

Contents of section .data:
 00000 0061736d 01000000 01800200 00000000  .asm............
#...
 00100 00000000 00000000 0000000a 80800400  ................
#pass
