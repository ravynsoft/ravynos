#source: relax-local-pic.s
#as:
#ld: -q -A elf32-arclittle -relax
#objdump: -dr

[^:]+:     file format elf.*arc


Disassembly of section \.text:

[0-9a-f]+ <__start>:
\s+[0-9a-f]+:	2700 7f84 0000 [0-9a-f]+\s+add\s+r4,pcl,.*
\s+[0-9a-f]+: R_ARC_PC32	a_in_other_thread
\s+[0-9a-f]+:	1c00 [0-9a-f\s]+	st\s+.*
