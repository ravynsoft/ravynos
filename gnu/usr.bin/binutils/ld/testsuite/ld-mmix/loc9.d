#ld: -m elf64mmix
#objdump: -str

# Setting file start through the LOC pseudo, see PR 6607.

.*:     file format elf64-mmix

SYMBOL TABLE:
#...
8000000000000000 g       \*ABS\*	0+ __\.MMIX\.start\.\.text
#...
Contents of section \.text:
 8000000000000000 f4000000                             .*
