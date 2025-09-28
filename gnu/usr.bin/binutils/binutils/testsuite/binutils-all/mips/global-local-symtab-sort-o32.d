#PROG: objcopy
#DUMPPROG: readelf
#name: MIPS symbol table sort and section symbol names (relocatable) (o32)
#as: -32 -mno-pdr
#objcopy: -j .data -j .symtab -j .strtab -j .shstrtab
#readelf: -s
#source: ../../../../gas/testsuite/gas/mips/global-local-symtab.s

Symbol table '\.symtab' contains 4 entries:
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
     1: 00000000     0 SECTION LOCAL  DEFAULT    1 \.data
     2: 00000000     4 OBJECT  GLOBAL DEFAULT    1 foo
     3: 00000004     4 OBJECT  LOCAL  DEFAULT    1 bar
