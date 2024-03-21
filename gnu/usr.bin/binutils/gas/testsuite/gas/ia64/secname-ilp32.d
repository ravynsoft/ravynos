#readelf: -S
#name: ia64 section name (ilp32)
#as: -milp32
#source: secname.s

There are 8 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] .text             PROGBITS        00000000 000040 000000 00  AX  0   0 16
  \[ 2\] .data             PROGBITS        00000000 000040 000000 00  WA  0   0  1
  \[ 3\] .bss              NOBITS          00000000 000040 000000 00  WA  0   0  1
  \[ 4\] .foo              PROGBITS        00000000 000040 000008 00  WA  0   0  8
  \[ 5\] .symtab           SYMTAB          00000000 [0-9a-f]+ 000050 10      6   5  4
  \[ 6\] .strtab           STRTAB          00000000 [0-9a-f]+ 000001 00      0   0  1
  \[ 7\] .shstrtab         STRTAB          00000000 [0-9a-f]+ 000031 00      0   0  1
Key to Flags:
#...
