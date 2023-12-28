
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
10200008  [^ ]+ R_MIPS_JUMP_SLOT  10100020   f_lo_iu
1020000c  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_du
10200010  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_du
10200014  [^ ]+ R_MIPS_JUMP_SLOT  10100050   f_lo_iu_du
10200018  [^ ]+ R_MIPS_JUMP_SLOT  10100060   f_lo_du
1020001c  [^ ]+ R_MIPS_JUMP_SLOT  10100070   f_lo

Symbol table '\.dynsym' .*
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
# _lo symbols have their address taken, so their PLT symbols need to have
# a nonzero value.  They must also have STO_MIPS_PLT in order to distinguish
# them from old-style lazy-binding stubs.  Non-_lo symbols are only called,
# so their PLT symbols should have a zero value and no STO_MIPS_PLT annotation.
    .*: 10100020     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu
#...
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_du
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_du
    .*: 10100050     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du
    .*: 10100060     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du
#...
    .*: 10100070     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo
# The start of the GOT-mapped area.  This should only contain functions that
# are accessed purely via the traditional psABI scheme.  The symbol value
# is the address of the lazy-binding stub.
     9: 10101000     0 FUNC    GLOBAL DEFAULT  UND f_iu

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
  10201008 -32744\(gp\) 10100040
  1020100c -32740\(gp\) 10100020
  10201010 -32736\(gp\) 10100050

 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  10201014 -32732\(gp\) 10101000 10101000 FUNC    UND f_iu


PLT GOT:

 Reserved entries:
   Address  Initial Purpose
  10200000 00000000 PLT lazy resolver
  10200004 00000000 Module pointer

 Entries:
   Address  Initial Sym\.Val\. Type    Ndx Name
  10200008 10100000 10100020 FUNC    UND f_lo_iu
  1020000c 10100000 00000000 FUNC    UND f_du
  10200010 10100000 00000000 FUNC    UND f_iu_du
  10200014 10100000 10100050 FUNC    UND f_lo_iu_du
  10200018 10100000 10100060 FUNC    UND f_lo_du
  1020001c 10100000 10100070 FUNC    UND f_lo


