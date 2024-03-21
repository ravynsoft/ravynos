#name: MIPS symbol table sort and section symbol names (relocatable) (n32)
#source: ../../../gas/testsuite/gas/mips/global-local-symtab.s
#as: -mno-pdr
#ld: -r -T global-local-symtab.ld
#readelf: -sW

Symbol table '\.symtab' contains 5 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 SECTION LOCAL  DEFAULT    1.*
     2: 00000000     0 FILE    LOCAL  DEFAULT  ABS .*global-local-symtab\.o
     3: 00000004     4 OBJECT  LOCAL  DEFAULT    1 bar
     4: 00000000     4 OBJECT  GLOBAL DEFAULT    1 foo
