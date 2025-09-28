#as: -mx86-used-note=no --generate-missing-build-notes=no
#source: ../x86-64-unwind.s
#readelf: -S
#name: x86-64 (ILP32) unwind

There are 6 section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] .text             PROGBITS        00000000 000034 000000 00  AX  0   0  1
  \[ 2\] .data             PROGBITS        00000000 000034 000000 00  WA  0   0  1
  \[ 3\] .bss              NOBITS          00000000 000034 000000 00  WA  0   0  1
  \[ 4\] .eh_frame         X86_64_UNWIND   00000000 000034 000008 00   A  0   0  1
  \[ 5\] .shstrtab         STRTAB          00000000 [0-9a-f]+ 000026 00   .  0   0  1
Key to Flags:
#pass
