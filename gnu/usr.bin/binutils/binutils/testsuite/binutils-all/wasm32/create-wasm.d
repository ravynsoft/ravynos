#PROG: objcopy
#source: create-wasm.s
#as:
#objcopy: -Ielf32-wasm32 -Owasm
#objdump: -bbinary -s

.*:.*file format binary

Contents of section .data:
 0000 0061736d 01000000 01030100 00030100  .asm............
