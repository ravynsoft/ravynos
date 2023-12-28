#as: -march=z80n
#source: z80n_all.s
#objdump: -r
#name: Z80N big-endian relocation

.*:[     ]+file format (coff|elf32)\-z80

RELOCATION RECORDS FOR \[\.text\]:
OFFSET[   ]+TYPE[              ]+VALUE\s*
0*c2a[ ]+r_imm16be[           ]+push_value\s*
