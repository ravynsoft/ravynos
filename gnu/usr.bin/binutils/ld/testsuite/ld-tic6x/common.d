#name: C6X common symbols
#as: -mlittle-endian
#ld: -melf32_tic6x_le -Tcommon.ld
#source: common.s
#readelf: -Ss

There are 6 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] \.far              NOBITS          00000080 000080 000008 00  WA  0   0  4
  \[ 2\] \.bss              NOBITS          00000100 000080 000004 00  WA  0   0  4
  \[ 3\] \.symtab           SYMTAB          00000000 [0-9a-f]+ 000050 10      4   3  4
  \[ 4\] \.strtab           STRTAB          00000000 [0-9a-f]+ 000005 00      0   0  1
  \[ 5\] \.shstrtab         STRTAB          00000000 [0-9a-f]+ 000025 00      0   0  1
Key to Flags:
#...

Symbol table '\.symtab' contains 5 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000080     0 SECTION LOCAL  DEFAULT    1.*
     2: 00000100     0 SECTION LOCAL  DEFAULT    2.*
     3: 00000100     4 OBJECT  GLOBAL DEFAULT    2 x
     4: 00000080     8 OBJECT  GLOBAL DEFAULT    1 y
