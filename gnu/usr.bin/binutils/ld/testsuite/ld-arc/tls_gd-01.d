#source: tls_gd-01.s
#as: -mcpu=arc700
#ld: -shared --hash-style=sysv
#objdump: -d
#xfail: arc*-*-elf*

[^:]+:     file format elf32-.*arc


Disassembly of section \.text:

[0-9a-f]+ <__start>:
 [0-9a-f]+:	2700 7f80 0000 2080 	add	r0,pcl,0x2080\s+;2224 <baz>
 [0-9a-f]+:	2700 7f80 0000 2080 	add	r0,pcl,0x2080\s+;222c <bar>
