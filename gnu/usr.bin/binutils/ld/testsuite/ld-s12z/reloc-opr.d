#source: reloc-opr.s
#ld: --no-relax --defsym bar=0xabcdef
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <_etext-0x5>:
  fe0000:	83 fa ab cd 	sub d5, bar
  fe0004:	ef 

00fe0005 <_etext>:
	...
