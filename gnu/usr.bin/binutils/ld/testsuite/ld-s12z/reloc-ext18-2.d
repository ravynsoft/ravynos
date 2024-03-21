#source: reloc-ext18.s
#ld: --no-relax --defsym x=0x2abcd
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <here>:
  fe0000:	a0 fc ab cd 	ld d2, x
