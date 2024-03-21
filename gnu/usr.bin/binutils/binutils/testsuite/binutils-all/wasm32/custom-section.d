#PROG: objcopy
#source: custom-section.s
#as:
#objcopy: -Ielf32-wasm32 -Owasm
#objdump: -bbinary -s

.*:.*file format binary

Contents of section .data:
 0000 0061736d 01000000 0008046e 616d6502  .asm.......name.
 0010 0100                                 .. *$
