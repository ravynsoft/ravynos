#ld: -T unwind.ld
#objdump: -s

.*:     file format.*

#...
Contents of section .c6xabi.exidx:
 9000 (00f8ff7f 07020083 1cf8ff7f 7af8ff7f|7ffff800 83000207 7ffff81c 7ffff87a)  .*
 9010 (38f8ff7f 07020083 56f8ff7f 01000000|7ffff838 83000207 7ffff856 00000001)  .*
Contents of section .far:
#...
