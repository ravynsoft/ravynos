#PROG: objcopy
#source: prepared-section.s
#as:
#objcopy: -Ielf32-wasm32 -Owasm
#objdump: -bbinary -s

.*:.*file format binary

Contents of section .data:
 0000 0061736d 01000000 0006046e 616d6500  .asm.......name.
