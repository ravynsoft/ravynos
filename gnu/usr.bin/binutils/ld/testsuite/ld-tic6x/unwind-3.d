#ld: -T unwind.ld
#objdump: -s

.*:     file format.*

#...
Contents of section .c6xabi.exidx:
 9000 (00f8ff7f 07020083 1cf8ff7f 01000000|7ffff800 83000207 7ffff81c 00000001)  .*
 9010 (38f8ff7f 07040083 54f8ff7f 01000000|7ffff838 82000407 7ffff854 00000001)  .*
Contents of section .far:
#...
