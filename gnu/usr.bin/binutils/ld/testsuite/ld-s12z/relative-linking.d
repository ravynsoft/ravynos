#source: relative-linking.s
#ld: --no-relax --defsym here=0xfe0020 --defsym=foo=0xfe0008  --defsym=bar=0xfe0010 --defsym=wiz=0xfe0040
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <here>:
  fe0000:	20 80 08    	bra foo
  fe0003:	02 b0 bc 80 	brclr.b d0, #3, bar
  fe0007:	0d 
  fe0008:	0b 85 80 38 	dbne d1, wiz
