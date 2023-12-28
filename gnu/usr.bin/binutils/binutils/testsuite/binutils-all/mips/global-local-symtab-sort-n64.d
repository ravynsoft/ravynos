#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS symbol table sort and section symbol names (relocatable) (n64)
#as: -64 -mno-pdr -mips3
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -s
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s

Symbol table '\.symtab' contains 4 entries:
   Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 0000000000000000     0 SECTION LOCAL  DEFAULT    1 \.data
     2: 0000000000000004     4 OBJECT  LOCAL  DEFAULT    1 bar
     3: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    1 foo
