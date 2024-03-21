#source: reloc-ext24.s
#ld: --no-relax --defsym foobar=0xabcdef
#objdump: -d -r

tmpdir/dump:     file format elf32-s12z


Disassembly of section .text:

00fe0000 <here>:
  fe0000:	ba ab cd ef 	jmp foobar
  fe0004:	01          	nop

00fe0005 <_etext>:
	...
