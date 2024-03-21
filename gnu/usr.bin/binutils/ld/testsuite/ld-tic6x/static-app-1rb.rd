There are 15 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] \.hash             HASH            00008000 001000 000038 04   A  2   0  4
  \[ 2\] \.dynsym           DYNSYM          00008038 001038 000090 10   A  3   6  4
  \[ 3\] \.dynstr           STRTAB          000080c8 0010c8 00000a 00   A  0   0  1
  \[ 4\] \.rela\.got         RELA            000080d4 0010d4 000024 0c  AI  2   8  4
  \[ 5\] \.rela\.neardata    RELA            000080f8 0010f8 000018 0c  AI  2   9  4
  \[ 6\] \.dynamic          DYNAMIC         00008110 001110 000090 08  WA  3   0  4
  \[ 7\] \.text             PROGBITS        10000000 002000 0000c0 00  AX  0   0 32
  \[ 8\] \.got              PROGBITS        100000c0 0020c0 000020 00  WA  0   0  4
  \[ 9\] \.neardata         PROGBITS        100000e0 0020e0 00000c 00  WA  0   0  4
  \[10\] \.bss              NOBITS          100000ec 0020ec 000004 00  WA  0   0  4
  \[11\] \.c6xabi\.attributes C6000_ATTRIBUTES 00000000 0020ec 000019 00      0   0  1
  \[12\] \.symtab           SYMTAB          00000000 002108 0001b0 10     13  21  4
  \[13\] \.strtab           STRTAB .*
  \[14\] \.shstrtab         STRTAB .*
Key to Flags:
#...

Elf file type is EXEC \(Executable file\)
Entry point 0x10000000
There are 4 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00008000 0x00008000 0x001a0 0x001a0 RW  0x1000
  LOAD           0x002000 0x10000000 0x10000000 0x000ec 0x000f0 RWE 0x1000
  DYNAMIC        0x001110 0x00008110 0x00008110 0x00090 0x00090 RW  0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x20000 RW  0x8

 Section to Segment mapping:
  Segment Sections\.\.\.
   00     \.hash \.dynsym \.dynstr \.rela\.got \.rela\.neardata \.dynamic 
   01     \.text \.got \.neardata \.bss 
   02     \.dynamic 
   03     

Dynamic section at offset 0x1110 contains 13 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       0x8000
 0x00000005 \(STRTAB\)                     0x80c8
 0x00000006 \(SYMTAB\)                     0x8038
 0x0000000a \(STRSZ\)                      10 \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000015 \(DEBUG\)                      0x0
 0x00000007 \(RELA\)                       0x80d4
 0x00000008 \(RELASZ\)                     60 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x70000000 \(C6000_DSBT_BASE\)            0x100000c0
 0x70000001 \(C6000_DSBT_SIZE\)            0x3
 0x70000003 \(C6000_DSBT_INDEX\)           0x0
 0x00000000 \(NULL\)                       0x0

Relocation section '\.rela\.got' at offset 0x10d4 contains 3 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
100000d8  00000401 R_C6000_ABS32          100000ec   \.bss \+ 0
100000d4  00000601 R_C6000_ABS32          100000e8   b \+ 0
100000dc  00000801 R_C6000_ABS32          100000e0   a \+ 0

Relocation section '\.rela\.neardata' at offset 0x10f8 contains 2 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
100000e0  00000101 R_C6000_ABS32          10000000   \.text \+ 8
100000e4  00000701 R_C6000_ABS32          00000000   g1 \+ 0

Symbol table '\.dynsym' contains 9 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 10000000     0 SECTION LOCAL  DEFAULT    7.*
     2: 100000c0     0 SECTION LOCAL  DEFAULT    8.*
     3: 100000e0     0 SECTION LOCAL  DEFAULT    9.*
     4: 100000ec     0 SECTION LOCAL  DEFAULT   10.*
     5: 100000ec     4 OBJECT  LOCAL  DEFAULT   10 c
     6: 100000e8     4 OBJECT  GLOBAL DEFAULT    9 b
     7: 00000000     0 NOTYPE  WEAK   DEFAULT  UND g1
     8: 100000e0     4 OBJECT  GLOBAL DEFAULT    9 a

Symbol table '\.symtab' contains 27 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00008000     0 SECTION LOCAL  DEFAULT    1.*
     2: 00008038     0 SECTION LOCAL  DEFAULT    2.*
     3: 000080c8     0 SECTION LOCAL  DEFAULT    3.*
     4: 000080d4     0 SECTION LOCAL  DEFAULT    4.*
     5: 000080f8     0 SECTION LOCAL  DEFAULT    5.*
     6: 00008110     0 SECTION LOCAL  DEFAULT    6.*
     7: 10000000     0 SECTION LOCAL  DEFAULT    7.*
     8: 100000c0     0 SECTION LOCAL  DEFAULT    8.*
     9: 100000e0     0 SECTION LOCAL  DEFAULT    9.*
    10: 100000ec     0 SECTION LOCAL  DEFAULT   10.*
    11: 00000000     0 SECTION LOCAL  DEFAULT   11.*
    12: 00000000     0 FILE    LOCAL  DEFAULT  ABS .*shlib-1\.o
    13: 10000000     0 FUNC    LOCAL  HIDDEN     7 sub1
    14: 00000000     0 FILE    LOCAL  DEFAULT  ABS .*shlib-app-1r\.o
    15: 10000080     0 NOTYPE  LOCAL  DEFAULT    7 fish
    16: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    17: 00008110     0 OBJECT  LOCAL  DEFAULT    6 _DYNAMIC
    18: 100000ec     4 OBJECT  LOCAL  DEFAULT   10 c
    19: 100000cc     0 OBJECT  LOCAL  DEFAULT    8 _GLOBAL_OFFSET_TABLE_
    20: 100000c0     0 NOTYPE  LOCAL  DEFAULT    8 __c6xabi_DSBT_BASE
    21: 100000e8     4 OBJECT  GLOBAL DEFAULT    9 b
    22: 00000000     0 NOTYPE  WEAK   DEFAULT  UND g1
    23: 100000e4     4 OBJECT  GLOBAL DEFAULT    9 g2
    24: 10000008    52 FUNC    GLOBAL DEFAULT    7 sub0
    25: 100000e0     4 OBJECT  GLOBAL DEFAULT    9 a
    26: 10000040    52 FUNC    GLOBAL DEFAULT    7 sub
