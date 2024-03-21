#source: start.s
#source: wrap1a.s
#source: wrap1b.s
#source: wrap1c.s
#ld: -m elf64mmix --wrap deal
#as: -no-expand
#objdump: -d

.*:     file format elf64-mmix

Disassembly of section \.text:

0+ <(_start|Main)>:
   0:	e3fd0001 	setl \$253,0x1
   4:	f2000001 	pushj \$0,8 <__wrap_deal>

0+8 <__wrap_deal>:
   8:	f0000001 	jmp c <deal>

0+c <deal>:
   c:	fd000000 	swym 0,0,0
