#source: opr-linking.s
#ld: --no-relax --defsym here=0xfe0000 --defsym=foo=0xfe0050  --defsym=bar=0xfe0010 --defsym=wiz=0xfe0040
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:


00fe0000 .*:
  fe0000:	01          	nop
  fe0001:	01          	nop
  fe0002:	01          	nop
  fe0003:	01          	nop
  fe0004:	1b 37 f6 fa 	divs\.lw d7, bar, wiz
  fe0008:	fe 00 10 fa 
  fe000c:	fe 00 40 
  fe000f:	bc fa fe 00 	clr\.b foo
  fe0013:	50 
