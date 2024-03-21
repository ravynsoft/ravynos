#source: reloc-pc-rel-7-15.s
#ld: --no-relax --defsym here=0xfe0000 --defsym foo=here-9
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <here>:
  fe0000:	2f ff f6    	ble \*\-10
  fe0003:	01          	nop$