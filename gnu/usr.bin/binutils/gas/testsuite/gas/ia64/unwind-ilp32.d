#readelf: -ST
#name: ia64 unwind section (ilp32)
#as: -milp32
#source: unwind.s

There are 9 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] .text             PROGBITS        00000000 000040 000000 00  AX  0   0 16
  \[ 2\] .data             PROGBITS        00000000 000040 000000 00  WA  0   0  1
  \[ 3\] .bss              NOBITS          00000000 000040 000000 00  WA  0   0  1
  \[ 4\] .IA_64.unwind_inf PROGBITS        00000000 000040 000008 00   A  0   0  8
  \[ 5\] .IA_64.unwind     IA_64_UNWIND    00000000 000048 000008 00  AL  1   1  8
  \[ 6\] .symtab           SYMTAB          00000000 [0-9a-f]+ 000060 10      7   6  4
  \[ 7\] .strtab           STRTAB          00000000 [0-9a-f]+ 000001 00      0   0  1
  \[ 8\] .shstrtab         STRTAB          00000000 [0-9a-f]+ 00004d 00      0   0  1
Key to Flags:
#...
