#ld: -m elf64mmix --defsym __.MMIX.start..text=0x8000000000000000
#objdump: -str

# Setting file start through the special symbol.

.*:     file format elf64-mmix

SYMBOL TABLE:
#...
8000000000000000 g       \*ABS\*	0+ __\.MMIX\.start\.\.text
#...
Contents of section \.text:
 8000000000000000 f4000000                             .*
