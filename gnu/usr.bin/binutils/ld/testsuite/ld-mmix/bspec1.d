#source: bspec1.s
#source: start.s
#ld: -m elf64mmix
#readelf: -Ssr -x1 -x2

There are 6 section headers, starting at offset .*:

Section Headers:
 +\[Nr\] Name +Type +Address +Offset
 +Size +EntSize +Flags +Link +Info +Align
 +\[ 0\] +NULL +0+ +0+
 +0+ +0+ +0 +0 +0
 +\[ 1\] \.text +PROGBITS +0+ +0+78
 +0+4 +0+ +AX +0 +0 +4
 +\[ 2\] \.MMIX\.spec_data\.2 PROGBITS +0+ +0+7c
 +0+4 +0+ +0 +0 +4
 +\[ 3\] \.symtab +SYMTAB +0+ .*
 +0+d8 +0+18 +4 +3 +8
 +\[ 4\] \.strtab +STRTAB +0+ .*
 +0+26 +0+ +0 +0 +1
 +\[ 5\] \.shstrtab +STRTAB +0+ +[0-9a-f]+
 +0+33 +0+ +0 +0 +1
Key to Flags:
#...

There are no relocations in this file\.

Symbol table '\.symtab' contains 9 entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
 +0: 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
 +1: 0+ +0 +SECTION +LOCAL +DEFAULT +1.*
 +2: 0+ +0 +SECTION +LOCAL +DEFAULT +2.*
 +3: 0+ +0 +FUNC +GLOBAL +DEFAULT +1 Main
 +4: 0+ +0 +NOTYPE +GLOBAL +DEFAULT +1 _start
#...

Hex dump of section '\.text':
  0x0+ e3fd0001                            .*

Hex dump of section '\.MMIX\.spec_data\.2':
  0x0+ 0000002a                            .*
