
Dynamic section .*
#...
 0x00000003 \(PLTGOT\)                     0x10201000
#...
 0x70000013 \(MIPS_GOTSYM\)                0x9
 0x00000014 \(PLTREL\)                     REL
 0x00000017 \(JMPREL\)                     0x10004000
 0x00000002 \(PLTRELSZ\)                   48 \(bytes\)
 0x70000032 \(MIPS_PLTGOT\)                0x10200000
#...
Relocation section '\.rel\.plt' .*
 Offset     Info    Type            Sym\.Value  Sym\. Name
10200008  [^ ]+ R_MIPS_JUMP_SLOT  10100019   f_lo_ic
1020000c  [^ ]+ R_MIPS_JUMP_SLOT  10100025   f_lo_dc
10200010  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_dc
10200014  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_ic_dc
10200018  [^ ]+ R_MIPS_JUMP_SLOT  10100049   f_lo_ic_dc
1020001c  [^ ]+ R_MIPS_JUMP_SLOT  10100055   f_lo

Symbol table '\.dynsym' .*
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
# _lo symbols have their address taken, so their PLT symbols need to have
# a nonzero value.  They must also have STO_MIPS_PLT in order to distinguish
# them from old-style lazy-binding stubs.  Non-_lo symbols are only called,
# so their PLT symbols should have a zero value and no STO_MIPS_PLT annotation.
#
# All PLTs should be microMIPS.
#...
    .*: 10100019     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic
    .*: 10100025     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_ic_dc
#...
    .*: 10100049     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic_dc
    .*: 10100055     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo
# The start of the GOT-mapped area.  This should only contain functions that
# are accessed purely via the traditional psABI scheme.  The symbol value
# is the address of the lazy-binding stub.
     9: 10101001     0 FUNC    GLOBAL DEFAULT  UND f_ic

Symbol table '\.symtab' .*
#...
Primary GOT:
 Canonical gp value: 10208ff0

 Reserved entries:
   Address     Access  Initial Purpose
  10201000 -32752\(gp\) 00000000 Lazy resolver
  10201004 -32748\(gp\) 80000000 Module pointer \(GNU extension\)

# See the disassembly output for the meaning of each entry.
 Local entries:
   Address     Access  Initial
  10201008 -32744\(gp\) 1010003d
  1020100c -32740\(gp\) 10100019
  10201010 -32736\(gp\) 10100049

 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  10201014 -32732\(gp\) 10101001 10101001 FUNC    UND f_ic


PLT GOT:

 Reserved entries:
   Address  Initial Purpose
  10200000 00000000 PLT lazy resolver
  10200004 00000000 Module pointer

 Entries:
   Address  Initial Sym\.Val\. Type    Ndx Name
  10200008 10100001 10100019 FUNC    UND f_lo_ic
  1020000c 10100001 10100025 FUNC    UND f_lo_dc
  10200010 10100001 00000000 FUNC    UND f_dc
  10200014 10100001 00000000 FUNC    UND f_ic_dc
  10200018 10100001 10100049 FUNC    UND f_lo_ic_dc
  1020001c 10100001 10100055 FUNC    UND f_lo


