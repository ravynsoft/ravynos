#readelf: -S
#name: ia64 section name

There are 8 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  \[ 0\]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  \[ 1\] \.text             PROGBITS         0000000000000000  00000040
       0000000000000000  0000000000000000  AX       0     0     16
  \[ 2\] \.data             PROGBITS         0000000000000000  00000040
       0000000000000000  0000000000000000  WA       0     0     1
  \[ 3\] \.bss              NOBITS           0000000000000000  00000040
       0000000000000000  0000000000000000  WA       0     0     1
  \[ 4\] \.foo              PROGBITS         0000000000000000  00000040
       0000000000000008  0000000000000000  WA       0     0     8
  \[ 5\] \.symtab           SYMTAB           0000000000000000  .*
       0000000000000078  0000000000000018           6     5     8
  \[ 6\] \.strtab           STRTAB           0000000000000000  .*
       0000000000000001  0000000000000000           0     0     1
  \[ 7\] \.shstrtab         STRTAB           0000000000000000  [0-9a-f]+
       0000000000000031  0000000000000000           0     0     1
Key to Flags:
#...
