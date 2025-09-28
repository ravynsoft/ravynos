#readelf: -Sg -T
#as: -x
#name: ia64 unwind group

There are 12 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  \[ 0\]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  \[ 1\] \.group            GROUP            0000000000000000  00000040
       0000000000000014  0000000000000004           9     5     4
  \[ 2\] \.text             PROGBITS         0000000000000000  00000060
       0000000000000000  0000000000000000  AX       0     0     16
  \[ 3\] \.data             PROGBITS         0000000000000000  00000060
       0000000000000000  0000000000000000  WA       0     0     1
  \[ 4\] \.bss              NOBITS           0000000000000000  00000060
       0000000000000000  0000000000000000  WA       0     0     1
  \[ 5\] \.gnu\.linkonce\.t\.f PROGBITS         0000000000000000  00000060
       0000000000000000  0000000000000000 AXG       0     0     16
  \[ 6\] \.gnu\.linkonce\.ia6 PROGBITS         0000000000000000  00000060
       0000000000000010  0000000000000000  AG       0     0     8
  \[ 7\] \.gnu\.linkonce\.ia6 IA_64_UNWIND     0000000000000000  00000070
       0000000000000018  0000000000000000 ALG       5     5     8
  \[ 8\] \.rela\.gnu\.linkonc RELA             0000000000000000  .*
       0000000000000048  0000000000000018  IG       9     7     8
  \[ 9\] \.symtab           SYMTAB           0000000000000000  .*
       00000000000000d8  0000000000000018          10     9     8
  \[10\] \.strtab           STRTAB           0000000000000000  .*
       0000000000000005  0000000000000000           0     0     1
  \[11\] \.shstrtab         STRTAB           0000000000000000  [0-9a-f]+
       0000000000000081  0000000000000000           0     0     1
Key to Flags:
#...

COMDAT group section \[    1\] `\.group' \[foo\] contains 4 sections:
   \[Index\]    Name
   \[    5\]   \.gnu\.linkonce\.t\.foo
   \[    6\]   \.gnu\.linkonce\.ia64unwi\.foo
   \[    7\]   \.gnu\.linkonce\.ia64unw\.foo
   \[    8\]   \.rela\.gnu\.linkonce\.ia64unw\.foo
