#source: reloc-ext18.s
#ld: --no-relax --defsym x=0xabcd
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <here>:
  fe0000:	a0 f8 ab cd 	ld d2, x
