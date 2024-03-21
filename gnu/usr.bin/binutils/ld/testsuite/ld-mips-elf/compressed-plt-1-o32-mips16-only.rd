
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
10200008  [^ ]+ R_MIPS_JUMP_SLOT  10100020   f_lo_ic
1020000c  [^ ]+ R_MIPS_JUMP_SLOT  10100041   f_lo_dc
10200010  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_dc
10200014  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_ic_dc
10200018  [^ ]+ R_MIPS_JUMP_SLOT  10100071   f_lo_ic_dc
1020001c  [^ ]+ R_MIPS_JUMP_SLOT  10100030   f_lo

Symbol table '\.dynsym' .*
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
# _lo symbols have their address taken, so their PLT symbols need to have
# a nonzero value.  They must also have STO_MIPS_PLT in order to distinguish
# them from old-style lazy-binding stubs.  Non-_lo symbols are only called,
# so their PLT symbols should have a zero value and no STO_MIPS_PLT annotation.
#
# A MIPS16 PLT should only be used as the symbol value if the function has
# a direct MIPS16 caller (dc) and no direct MIPS caller (du).
#...
    .*: 10100020     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic
    .*: 10100041     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_ic_dc
#...
    .*: 10100071     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic_dc
    .*: 10100030     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo
# The start of the GOT-mapped area.  This should only contain functions that
# are accessed purely via the traditional psABI scheme.  The symbol value
# is the address of the lazy-binding stub.
     9: 10101000     0 FUNC    GLOBAL DEFAULT  UND f_ic

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
  10201008 -32744\(gp\) 10100061
  1020100c -32740\(gp\) 10100020
  10201010 -32736\(gp\) 10100071

 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  10201014 -32732\(gp\) 10101000 10101000 FUNC    UND f_ic


PLT GOT:

 Reserved entries:
   Address  Initial Purpose
  10200000 00000000 PLT lazy resolver
  10200004 00000000 Module pointer

 Entries:
   Address  Initial Sym\.Val\. Type    Ndx Name
  10200008 10100000 10100020 FUNC    UND f_lo_ic
  1020000c 10100000 10100041 FUNC    UND f_lo_dc
  10200010 10100000 00000000 FUNC    UND f_dc
  10200014 10100000 00000000 FUNC    UND f_ic_dc
  10200018 10100000 10100071 FUNC    UND f_lo_ic_dc
  1020001c 10100000 10100030 FUNC    UND f_lo


