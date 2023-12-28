#source: greg-4.s
#source: greg-4.s
#source: local2.s
#source: local1.s
#source: regext1.s
#source: start.s
#ld: -m elf64mmix
#readelf: -Ssx1 -T -x2

# Like local1, but with two checks for a local register.

There are 6 section headers, starting at offset .*:

Section Headers:
 +\[Nr\] Name +Type +Address +Offset
 +Size +EntSize +Flags +Link +Info +Align
 +\[ 0\] +NULL +0+ +0+
 +0+ +0+ +0 +0 +0
 +\[ 1\] \.text +PROGBITS +0+ +0+78
 +0+c +0+ +AX +0 +0 +4
 +\[ 2\] \.MMIX\.reg_content PROGBITS +0+7e8 +0+84
 +0+10 +0+ +W +0 +0 +1
 +\[ 3\] \.symtab +SYMTAB +0+ +[0-9a-f]+
 +[0-9a-f]+ +0+18 +4 +[0-9] +8
 +\[ 4\] \.strtab +STRTAB +0+ +[0-9a-f]+
 +[0-9a-f]+ +0+ +0 +0 +1
 +\[ 5\] \.shstrtab +STRTAB +0+ +[0-9a-f]+
 +0+34 +0+ +0 +0 +1
Key to Flags:
#...

Symbol table '\.symtab' contains [0-9]+ entries:
 +Num: +Value +Size +Type +Bind +Vis +Ndx +Name
.* 0+ +0 +NOTYPE +LOCAL +DEFAULT +UND 
.* 0+ +0 +SECTION +LOCAL +DEFAULT +1.*
.* 0+7e8 +0 +SECTION +LOCAL +DEFAULT +2.*
.* 0+ +0 +FILE +LOCAL +DEFAULT +ABS .*
.* 0+fd +0 +NOTYPE +LOCAL +DEFAULT +PRC\[0xff00\] lsym
.* 0+ +0 +FILE +LOCAL +DEFAULT +ABS .*
.* 0+fe +0 +NOTYPE +LOCAL +DEFAULT +PRC\[0xff00\] lsym
.* 0+fc +0 +NOTYPE +GLOBAL +DEFAULT +PRC\[0xff00\] ext1
.* 0+8 +0 +NOTYPE +GLOBAL +DEFAULT +1 _start
#...

Hex dump of section '\.text':
  0x0+ fd020202 fd030201 e3fd0001          .*

Hex dump of section '\.MMIX\.reg_contents':
  0x0+7e8 00000000 0000004e 00000000 0000004e .*
