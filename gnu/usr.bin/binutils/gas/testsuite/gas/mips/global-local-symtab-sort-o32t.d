#DUMPPROG: readelf
#readelf: -s
#name: MIPS global/local symbol table sort and section symbol names (o32)
#as: -32 -mno-pdr
#source: global-local-symtab.s

Symbol table '\.symtab' contains 9 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 SECTION LOCAL  DEFAULT    1.*
     2: 00000000     0 SECTION LOCAL  DEFAULT    2.*
     3: 00000000     0 SECTION LOCAL  DEFAULT    3.*
     4: 00000004     4 OBJECT  LOCAL  DEFAULT    2 bar
     5: 00000000     0 SECTION LOCAL  DEFAULT    4.*
     6: 00000000     0 SECTION LOCAL  DEFAULT    5.*
     7: 00000000     0 SECTION LOCAL  DEFAULT    6.*
     8: 00000000     4 OBJECT  GLOBAL DEFAULT    2 foo
