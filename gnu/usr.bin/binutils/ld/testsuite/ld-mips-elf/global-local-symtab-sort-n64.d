#name: MIPS symbol table sort and section symbol names (relocatable) (n64)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -sW

Symbol table '\.symtab' contains 5 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 \.data
     2: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS .*global-local-symtab\.o
     3: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    1 bar
     4: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    1 foo
