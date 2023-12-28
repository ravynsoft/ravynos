#PROG: strip
#source: compressed-1.s
#as: --64 -mx86-used-note=no --gdwarf-3
#strip:
#readelf: -S --wide
#name: strip on uncompressed debug sections

There are 5 section headers, starting at offset 0x[0-9a-f]+:

Section Headers:
  \[Nr\] Name              Type            Address          Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            0000000000000000 000000 000000 00      0   0  0
  \[ 1\] .text             PROGBITS        0000000000000000 000040 000015 00  AX  0   0 16
  \[ 2\] .data             PROGBITS        0000000000000000 000055 000000 00  WA  0   0  1
  \[ 3\] .bss              NOBITS          0000000000000000 000055 000000 00  WA  0   0  1
  \[ 4\] .shstrtab         STRTAB          0000000000000000 [0-9a-f]+ 00001c 00 .* 0   0  1
Key to Flags:
#...
