#name: PR ld/12718
#as: --64
#ld: -melf_x86_64 -z noseparate-code
#readelf: -S --wide

There are 5 section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
 +\[Nr\] Name +Type +Address +Off +Size +ES +Flg +Lk +Inf +Al
 +\[ 0\] +NULL +0+ +0+ +0+ +0+ +0 +0 +0
 +\[ 1\] +.text +PROGBITS +[0-9a-f]+ +[0-9a-f]+ +000006 00 +AX +0 +0 +1
 +\[ 2\] +.symtab +SYMTAB +0+ +[0-9a-f]+ +[0-9a-f]+ 18 +3 +[0-9] +8
 +\[ 3\] +.strtab +STRTAB +0+ +[0-9a-f]+ +[0-9a-f]+ 00 +0 +0 +1
 +\[ 4\] +.shstrtab +STRTAB +0+ +[0-9a-f]+ +[0-9a-f]+ +0+ +0 +0 +1
Key to Flags:
#pass
