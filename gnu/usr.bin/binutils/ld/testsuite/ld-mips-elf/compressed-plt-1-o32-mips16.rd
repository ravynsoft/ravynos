
Dynamic section .*
#...
 0x00000003 \(PLTGOT\)                     0x10201000
#...
 0x70000013 \(MIPS_GOTSYM\)                0x1f
 0x00000014 \(PLTREL\)                     REL
 0x00000017 \(JMPREL\)                     0x10004000
 0x00000002 \(PLTRELSZ\)                   224 \(bytes\)
 0x70000032 \(MIPS_PLTGOT\)                0x10200000
#...
Relocation section '\.rel\.plt' .*
 Offset     Info    Type            Sym\.Value  Sym\. Name
10200008  [^ ]+ R_MIPS_JUMP_SLOT  10100020   f_lo_iu
1020000c  [^ ]+ R_MIPS_JUMP_SLOT  10100030   f_lo_iu_du_dc
10200010  [^ ]+ R_MIPS_JUMP_SLOT  10100040   f_lo_du_ic_dc
10200014  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_du_dc
10200018  [^ ]+ R_MIPS_JUMP_SLOT  10100191   f_lo_iu_dc
1020001c  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_du_ic
10200020  [^ ]+ R_MIPS_JUMP_SLOT  10100070   f_lo_du_ic
10200024  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_dc
10200028  [^ ]+ R_MIPS_JUMP_SLOT  10100080   f_lo_iu_ic
1020002c  [^ ]+ R_MIPS_JUMP_SLOT  10100090   f_lo_ic
10200030  [^ ]+ R_MIPS_JUMP_SLOT  101000a0   f_lo_du_dc
10200034  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_du
10200038  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_du_ic_dc
1020003c  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_du_ic
10200040  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_du_dc
10200044  [^ ]+ R_MIPS_JUMP_SLOT  101001e1   f_lo_dc
10200048  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_du
1020004c  [^ ]+ R_MIPS_JUMP_SLOT  10100100   f_lo_iu_du
10200050  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_dc
10200054  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_ic_dc
10200058  [^ ]+ R_MIPS_JUMP_SLOT  10100110   f_lo_du
1020005c  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_ic_dc
10200060  [^ ]+ R_MIPS_JUMP_SLOT  00000000   f_iu_du_ic_dc
10200064  [^ ]+ R_MIPS_JUMP_SLOT  10100231   f_lo_iu_ic_dc
10200068  [^ ]+ R_MIPS_JUMP_SLOT  10100130   f_lo_iu_du_ic
1020006c  [^ ]+ R_MIPS_JUMP_SLOT  10100140   f_lo_iu_du_ic_dc
10200070  [^ ]+ R_MIPS_JUMP_SLOT  10100251   f_lo_ic_dc
10200074  [^ ]+ R_MIPS_JUMP_SLOT  10100150   f_lo

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
    .*: 10100020     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu
    .*: 10100030     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_dc
#...
    .*: 10100040     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_ic_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_du_dc
    .*: 10100191     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_du_ic
    .*: 10100070     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_ic
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_dc
    .*: 10100080     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_ic
    .*: 10100090     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic
    .*: 101000a0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_du
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_du_ic_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_du_ic
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_du_dc
    .*: 101001e1     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_du
    .*: 10100100     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_ic_dc
    .*: 10100110     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_ic_dc
    .*: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_du_ic_dc
#...
    .*: 10100231     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_ic_dc
    .*: 10100130     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_ic
    .*: 10100140     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_ic_dc
    .*: 10100251     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic_dc
    .*: 10100150     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo
# The start of the GOT-mapped area.  This should only contain functions that
# are accessed purely via the traditional psABI scheme.  The symbol value
# is the address of the lazy-binding stub.
    31: 10101020     0 FUNC    GLOBAL DEFAULT  UND f_iu_ic
    32: 10101010     0 FUNC    GLOBAL DEFAULT  UND f_ic
    33: 10101000     0 FUNC    GLOBAL DEFAULT  UND f_iu

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
  10201008 -32744\(gp\) 10100201
  1020100c -32740\(gp\) 101000d0
  10201010 -32736\(gp\) 101000c0
  10201014 -32732\(gp\) 10100211
  10201018 -32728\(gp\) 10100060
  1020101c -32724\(gp\) 10100120
  10201020 -32720\(gp\) 10100090
  10201024 -32716\(gp\) 10100251
  10201028 -32712\(gp\) 10100070
  1020102c -32708\(gp\) 10100040
  10201030 -32704\(gp\) 10100080
  10201034 -32700\(gp\) 10100231
  10201038 -32696\(gp\) 10100130
  1020103c -32692\(gp\) 10100140
  10201040 -32688\(gp\) 101001a1
  10201044 -32684\(gp\) 101000f0
  10201048 -32680\(gp\) 101000e0
  1020104c -32676\(gp\) 10100020
  10201050 -32672\(gp\) 10100191
  10201054 -32668\(gp\) 10100100
  10201058 -32664\(gp\) 10100030

 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  1020105c -32660\(gp\) 10101020 10101020 FUNC    UND f_iu_ic
  10201060 -32656\(gp\) 10101010 10101010 FUNC    UND f_ic
  10201064 -32652\(gp\) 10101000 10101000 FUNC    UND f_iu


PLT GOT:

 Reserved entries:
   Address  Initial Purpose
  10200000 00000000 PLT lazy resolver
  10200004 00000000 Module pointer

 Entries:
   Address  Initial Sym\.Val\. Type    Ndx Name
  10200008 10100000 10100020 FUNC    UND f_lo_iu
  1020000c 10100000 10100030 FUNC    UND f_lo_iu_du_dc
  10200010 10100000 10100040 FUNC    UND f_lo_du_ic_dc
  10200014 10100000 00000000 FUNC    UND f_du_dc
  10200018 10100000 10100191 FUNC    UND f_lo_iu_dc
  1020001c 10100000 00000000 FUNC    UND f_iu_du_ic
  10200020 10100000 10100070 FUNC    UND f_lo_du_ic
  10200024 10100000 00000000 FUNC    UND f_iu_dc
  10200028 10100000 10100080 FUNC    UND f_lo_iu_ic
  1020002c 10100000 10100090 FUNC    UND f_lo_ic
  10200030 10100000 101000a0 FUNC    UND f_lo_du_dc
  10200034 10100000 00000000 FUNC    UND f_du
  10200038 10100000 00000000 FUNC    UND f_du_ic_dc
  1020003c 10100000 00000000 FUNC    UND f_du_ic
  10200040 10100000 00000000 FUNC    UND f_iu_du_dc
  10200044 10100000 101001e1 FUNC    UND f_lo_dc
  10200048 10100000 00000000 FUNC    UND f_iu_du
  1020004c 10100000 10100100 FUNC    UND f_lo_iu_du
  10200050 10100000 00000000 FUNC    UND f_dc
  10200054 10100000 00000000 FUNC    UND f_ic_dc
  10200058 10100000 10100110 FUNC    UND f_lo_du
  1020005c 10100000 00000000 FUNC    UND f_iu_ic_dc
  10200060 10100000 00000000 FUNC    UND f_iu_du_ic_dc
  10200064 10100000 10100231 FUNC    UND f_lo_iu_ic_dc
  10200068 10100000 10100130 FUNC    UND f_lo_iu_du_ic
  1020006c 10100000 10100140 FUNC    UND f_lo_iu_du_ic_dc
  10200070 10100000 10100251 FUNC    UND f_lo_ic_dc
  10200074 10100000 10100150 FUNC    UND f_lo


