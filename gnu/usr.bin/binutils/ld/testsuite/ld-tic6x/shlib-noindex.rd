There are 18 section headers, starting at offset .*:

Section Headers:
  \[Nr\] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
  \[ 0\]                   NULL            00000000 000000 000000 00      0   0  0
  \[ 1\] \.hash             HASH            00008000 001000 000044 04   A  2   0  4
  \[ 2\] \.dynsym           DYNSYM          00008044 001044 0000c0 10   A  3   6  4
  \[ 3\] \.dynstr           STRTAB          00008104 001104 000012 00   A  0   0  1
  \[ 4\] \.rela\.text        RELA            00008118 001118 00000c 0c  AI  2  10  4
  \[ 5\] \.rela\.got         RELA            00008124 001124 000024 0c  AI  2  11  4
  \[ 6\] \.rela\.neardata    RELA            00008148 001148 000018 0c  AI  2  12  4
  \[ 7\] \.dynamic          DYNAMIC         00008160 001160 0000b0 08  WA  3   0  4
  \[ 8\] \.rela\.plt         RELA            10000000 002000 000018 0c  AI  2  11  4
  \[ 9\] \.plt              PROGBITS        10000020 002020 000060 18  AX  0   0 32
  \[10\] \.text             PROGBITS        10000080 002080 000080 00  AX  0   0 32
  \[11\] \.got              PROGBITS        10000100 002100 000028 00  WA  0   0  4
  \[12\] \.neardata         PROGBITS        10000128 002128 000008 00  WA  0   0  4
  \[13\] \.bss              NOBITS          10000130 002130 000004 00  WA  0   0  4
  \[14\] \.c6xabi\.attributes C6000_ATTRIBUTES 00000000 002130 000019 00      0   0  1
  \[15\] \.symtab           SYMTAB          00000000 00214c 0001c0 10     16  22  4
  \[16\] \.strtab           STRTAB .*
  \[17\] \.shstrtab         STRTAB .*
Key to Flags:
#...

Elf file type is DYN \(Shared object file\)
Entry point 0x[0-9a-f]+
There are 4 program headers, starting at offset 52

Program Headers:
  Type           Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg Align
  LOAD           0x001000 0x00008000 0x00008000 0x00210 0x00210 RW  0x1000
  LOAD           0x002000 0x10000000 0x10000000 0x00130 0x00134 RWE 0x1000
  DYNAMIC        0x001160 0x00008160 0x00008160 0x000b0 0x000b0 RW  0x4
  GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x20000 RW  0x8

 Section to Segment mapping:
  Segment Sections\.\.\.
   00     \.hash \.dynsym \.dynstr \.rela\.text \.rela\.got \.rela\.neardata \.dynamic 
   01     \.rela\.plt \.plt \.text \.got \.neardata \.bss 
   02     \.dynamic 
   03     

Dynamic section at offset 0x1160 contains 17 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       0x8000
 0x00000005 \(STRTAB\)                     0x8104
 0x00000006 \(SYMTAB\)                     0x8044
 0x0000000a \(STRSZ\)                      18 \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000003 \(PLTGOT\)                     0x1000010c
 0x00000002 \(PLTRELSZ\)                   24 \(bytes\)
 0x00000014 \(PLTREL\)                     RELA
 0x00000017 \(JMPREL\)                     0x10000000
 0x00000007 \(RELA\)                       0x8118
 0x00000008 \(RELASZ\)                     96 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x00000016 \(TEXTREL\)                    0x0
 0x70000000 \(C6000_DSBT_BASE\)            0x10000100
 0x70000001 \(C6000_DSBT_SIZE\)            0x3
 0x70000003 \(C6000_DSBT_INDEX\)           0x0
 0x00000000 \(NULL\)                       0x0

Relocation section '\.rela\.text' at offset 0x1118 contains 1 entry:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
10000094  00000318 R_C6000_DSBT_INDEX     10000100   \.got - 10000100

Relocation section '\.rela\.got' at offset 0x1124 contains 3 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
10000120  00000501 R_C6000_ABS32          10000130   \.bss \+ 0
1000011c  00000601 R_C6000_ABS32          00000000   b \+ 0
10000124  00000a01 R_C6000_ABS32          10000128   a \+ 0

Relocation section '\.rela\.neardata' at offset 0x1148 contains 2 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
10000128  00000901 R_C6000_ABS32          10000088   sub0 \+ 0
1000012c  00000701 R_C6000_ABS32          00000000   g1 \+ 0

Relocation section '\.rela\.plt' at offset 0x2000 contains 2 entries:
 Offset     Info    Type                Sym\. Value  Symbol's Name \+ Addend
10000114  0000091b R_C6000_JUMP_SLOT      10000088   sub0 \+ 0
10000118  00000b1b R_C6000_JUMP_SLOT      100000c0   sub \+ 0

Symbol table '\.dynsym' contains 12 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 10000020     0 SECTION LOCAL  DEFAULT    9.*
     2: 10000080     0 SECTION LOCAL  DEFAULT   10.*
     3: 10000100     0 SECTION LOCAL  DEFAULT   11.*
     4: 10000128     0 SECTION LOCAL  DEFAULT   12.*
     5: 10000130     0 SECTION LOCAL  DEFAULT   13.*
     6: 00000000     0 NOTYPE  WEAK   DEFAULT  UND b
     7: 00000000     0 NOTYPE  WEAK   DEFAULT  UND g1
     8: 1000012c     4 OBJECT  GLOBAL DEFAULT   12 g2
     9: 10000088    52 FUNC    GLOBAL DEFAULT   10 sub0
    10: 10000128     4 OBJECT  GLOBAL DEFAULT   12 a
    11: 100000c0    52 FUNC    GLOBAL DEFAULT   10 sub

Symbol table '\.symtab' contains 28 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00008000     0 SECTION LOCAL  DEFAULT    1.*
     2: 00008044     0 SECTION LOCAL  DEFAULT    2.*
     3: 00008104     0 SECTION LOCAL  DEFAULT    3.*
     4: 00008118     0 SECTION LOCAL  DEFAULT    4.*
     5: 00008124     0 SECTION LOCAL  DEFAULT    5.*
     6: 00008148     0 SECTION LOCAL  DEFAULT    6.*
     7: 00008160     0 SECTION LOCAL  DEFAULT    7.*
     8: 10000000     0 SECTION LOCAL  DEFAULT    8.*
     9: 10000020     0 SECTION LOCAL  DEFAULT    9.*
    10: 10000080     0 SECTION LOCAL  DEFAULT   10.*
    11: 10000100     0 SECTION LOCAL  DEFAULT   11.*
    12: 10000128     0 SECTION LOCAL  DEFAULT   12.*
    13: 10000130     0 SECTION LOCAL  DEFAULT   13.*
    14: 00000000     0 SECTION LOCAL  DEFAULT   14.*
    15: 00000000     0 FILE    LOCAL  DEFAULT  ABS .*shlib-1\.o
    16: 10000080     0 FUNC    LOCAL  HIDDEN    10 sub1
    17: 00000000     0 FILE    LOCAL  DEFAULT  ABS 
    18: 00008160     0 OBJECT  LOCAL  DEFAULT  ABS _DYNAMIC
    19: 10000130     4 OBJECT  LOCAL  DEFAULT   13 c
    20: 1000010c     0 OBJECT  LOCAL  DEFAULT  ABS _GLOBAL_OFFSET_TABLE_
    21: 10000100     0 NOTYPE  LOCAL  DEFAULT   11 __c6xabi_DSBT_BASE
    22: 00000000     0 NOTYPE  WEAK   DEFAULT  UND b
    23: 00000000     0 NOTYPE  WEAK   DEFAULT  UND g1
    24: 1000012c     4 OBJECT  GLOBAL DEFAULT   12 g2
    25: 10000088    52 FUNC    GLOBAL DEFAULT   10 sub0
    26: 10000128     4 OBJECT  GLOBAL DEFAULT   12 a
    27: 100000c0    52 FUNC    GLOBAL DEFAULT   10 sub
