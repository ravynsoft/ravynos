#source: pr23633.s
#PROG: objcopy
#objcopy: -S --keep-symbols=$srcdir/$subdir/pr23633.list
#objdump: -s

#...
Contents of section .text:
 0000 00000000.*
#pass
