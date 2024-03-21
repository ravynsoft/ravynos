#DUMPPROG: readelf
#readelf: -s
#name: MIPS global/local symbol table sort and section symbol names (n64)
#as: -64 -mno-pdr -mips3
#source: global-local-symtab.s

Symbol table '\.symtab' contains 9 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 \.text
     2: 0000000000000000     0 SECTION LOCAL  DEFAULT    2 \.data
     3: 0000000000000000     0 SECTION LOCAL  DEFAULT    3 \.bss
     4: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    2 bar
     5: 0000000000000000     0 SECTION LOCAL  DEFAULT    4 \.MIPS\.options
     6: 0000000000000000     0 SECTION LOCAL  DEFAULT    5 \.MIPS\.abiflags
     7: 0000000000000000     0 SECTION LOCAL  DEFAULT    6 \.gnu\.attributes
     8: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    2 foo
