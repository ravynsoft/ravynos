
Dynamic section .*
#...
 0x00000003 \(PLTGOT\)                     0x10202000
#...
 0x70000013 \(MIPS_GOTSYM\)                0x1f
 0x00000014 \(PLTREL\)                     REL
 0x00000017 \(JMPREL\)                     0x10005000
 0x00000002 \(PLTRELSZ\)                   224 \(bytes\)
 0x70000032 \(MIPS_PLTGOT\)                0x10200000
#...
Relocation section '\.rel\.dyn' .*
# All symbols are referenced by a .word in the .data section, so pointer
# equality matters.  If a PLT is needed to satisfy a direct call or %lo
# relocation, the symbol should have a nonzero value and there should be
# no dynamic relocations against it.  The only relocations here are for
# undefined 0-value symbols.  Note that unlike x86, we do not create a PLT
# for the uncalled symbol 'f' in order to maintain backward compatibility
# with pre-PLT ld.sos.
 Offset     Info    Type            Sym\.Value  Sym\. Name
00000000  00000000 R_MIPS_NONE      
10201028  00001f03 R_MIPS_REL32      00000000   f_iu_ic
10201008  00002003 R_MIPS_REL32      00000000   f_ic
10201020  00002103 R_MIPS_REL32      00000000   f_iu
10201000  00002203 R_MIPS_REL32      00000000   f

Relocation section '\.rel\.plt' .*
 Offset     Info    Type            Sym\.Value  Sym\. Name
10200008  [^ ]+ R_MIPS_JUMP_SLOT  10100020   f_lo_iu
1020000c  [^ ]+ R_MIPS_JUMP_SLOT  10100030   f_lo_iu_du_dc
10200010  [^ ]+ R_MIPS_JUMP_SLOT  10100040   f_lo_du_ic_dc
10200014  [^ ]+ R_MIPS_JUMP_SLOT  10100050   f_du_dc
10200018  [^ ]+ R_MIPS_JUMP_SLOT  10100191   f_lo_iu_dc
1020001c  [^ ]+ R_MIPS_JUMP_SLOT  10100060   f_iu_du_ic
10200020  [^ ]+ R_MIPS_JUMP_SLOT  10100070   f_lo_du_ic
10200024  [^ ]+ R_MIPS_JUMP_SLOT  101001a1   f_iu_dc
10200028  [^ ]+ R_MIPS_JUMP_SLOT  10100080   f_lo_iu_ic
1020002c  [^ ]+ R_MIPS_JUMP_SLOT  10100090   f_lo_ic
10200030  [^ ]+ R_MIPS_JUMP_SLOT  101000a0   f_lo_du_dc
10200034  [^ ]+ R_MIPS_JUMP_SLOT  101000b0   f_du
10200038  [^ ]+ R_MIPS_JUMP_SLOT  101000c0   f_du_ic_dc
1020003c  [^ ]+ R_MIPS_JUMP_SLOT  101000d0   f_du_ic
10200040  [^ ]+ R_MIPS_JUMP_SLOT  101000e0   f_iu_du_dc
10200044  [^ ]+ R_MIPS_JUMP_SLOT  101001e1   f_lo_dc
10200048  [^ ]+ R_MIPS_JUMP_SLOT  101000f0   f_iu_du
1020004c  [^ ]+ R_MIPS_JUMP_SLOT  10100100   f_lo_iu_du
10200050  [^ ]+ R_MIPS_JUMP_SLOT  101001f1   f_dc
10200054  [^ ]+ R_MIPS_JUMP_SLOT  10100201   f_ic_dc
10200058  [^ ]+ R_MIPS_JUMP_SLOT  10100110   f_lo_du
1020005c  [^ ]+ R_MIPS_JUMP_SLOT  10100211   f_iu_ic_dc
10200060  [^ ]+ R_MIPS_JUMP_SLOT  10100120   f_iu_du_ic_dc
10200064  [^ ]+ R_MIPS_JUMP_SLOT  10100231   f_lo_iu_ic_dc
10200068  [^ ]+ R_MIPS_JUMP_SLOT  10100130   f_lo_iu_du_ic
1020006c  [^ ]+ R_MIPS_JUMP_SLOT  10100140   f_lo_iu_du_ic_dc
10200070  [^ ]+ R_MIPS_JUMP_SLOT  10100251   f_lo_ic_dc
10200074  [^ ]+ R_MIPS_JUMP_SLOT  10100150   f_lo

Symbol table '\.dynsym' .*
   Num:    Value  Size Type    Bind   Vis      Ndx Name
     0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND 
# All symbols have their address taken, so PLT symbols need to have a nonzero
# value.  They must also have STO_MIPS_PLT in order to distinguish them from
# old-style lazy-binding stubs).
#
# A MIPS16 PLT should only be used as the symbol value if the function has
# a direct MIPS16 caller (dc) and no direct MIPS caller (du).
    .*: 10100020     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu
    .*: 10100030     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_dc
#...
    .*: 10100040     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_ic_dc
    .*: 10100050     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_du_dc
    .*: 10100191     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_dc
    .*: 10100060     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_du_ic
    .*: 10100070     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_ic
    .*: 101001a1     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_dc
    .*: 10100080     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_ic
    .*: 10100090     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic
    .*: 101000a0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du_dc
    .*: 101000b0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_du
    .*: 101000c0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_du_ic_dc
    .*: 101000d0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_du_ic
    .*: 101000e0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_du_dc
    .*: 101001e1     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_dc
    .*: 101000f0     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_du
    .*: 10100100     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du
    .*: 101001f1     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_dc
    .*: 10100201     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_ic_dc
    .*: 10100110     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_du
    .*: 10100211     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_ic_dc
    .*: 10100120     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_iu_du_ic_dc
#...
    .*: 10100231     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_ic_dc
    .*: 10100130     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_ic
    .*: 10100140     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_iu_du_ic_dc
    .*: 10100251     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo_ic_dc
    .*: 10100150     0 FUNC    GLOBAL DEFAULT \[MIPS PLT\]   UND f_lo
# The start of the GOT-mapped area.  This should only contain functions that
# are accessed purely via the traditional psABI scheme.  Since the functions
# have their addresses taken, they cannot use a lazy-binding stub.
# The symbol values are therefore all zero.
    31: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu_ic
    32: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_ic
    33: 00000000     0 FUNC    GLOBAL DEFAULT  UND f_iu
    34: 00000000     0 FUNC    GLOBAL DEFAULT  UND f

Symbol table '\.symtab' .*
#...
Hex dump of section '\.data':
  0x10201000 (00000000|00000000) (101001f1|f1011010) (00000000|00000000) (10100201|01021010) .*
  0x10201010 (101000b0|b0001010) (10100050|50001010) (101000d0|d0001010) (101000c0|c0001010) .*
  0x10201020 (00000000|00000000) (101001a1|a1011010) (00000000|00000000) (10100211|11021010) .*
  0x10201030 (101000f0|f0001010) (101000e0|e0001010) (10100060|60001010) (10100120|20011010) .*
  0x10201040 (10100150|50011010) (101001e1|e1011010) (10100090|90001010) (10100251|51021010) .*
  0x10201050 (10100110|10011010) (101000a0|a0001010) (10100070|70001010) (10100040|40001010) .*
  0x10201060 (10100020|20001010) (10100191|91011010) (10100080|80001010) (10100231|31021010) .*
  0x10201070 (10100100|00011010) (10100030|30001010) (10100130|30011010) (10100140|40011010) .*


Primary GOT:
 Canonical gp value: 10209ff0

 Reserved entries:
   Address     Access  Initial Purpose
  10202000 -32752\(gp\) 00000000 Lazy resolver
  10202004 -32748\(gp\) 80000000 Module pointer \(GNU extension\)

# See the disassembly output for the meaning of each entry.
 Local entries:
   Address     Access  Initial
  10202008 -32744\(gp\) 10100201
  1020200c -32740\(gp\) 101000d0
  10202010 -32736\(gp\) 101000c0
  10202014 -32732\(gp\) 10100211
  10202018 -32728\(gp\) 10100060
  1020201c -32724\(gp\) 10100120
  10202020 -32720\(gp\) 10100090
  10202024 -32716\(gp\) 10100251
  10202028 -32712\(gp\) 10100070
  1020202c -32708\(gp\) 10100040
  10202030 -32704\(gp\) 10100080
  10202034 -32700\(gp\) 10100231
  10202038 -32696\(gp\) 10100130
  1020203c -32692\(gp\) 10100140
  10202040 -32688\(gp\) 101001a1
  10202044 -32684\(gp\) 101000f0
  10202048 -32680\(gp\) 101000e0
  1020204c -32676\(gp\) 10100020
  10202050 -32672\(gp\) 10100191
  10202054 -32668\(gp\) 10100100
  10202058 -32664\(gp\) 10100030

 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  1020205c -32660\(gp\) 00000000 00000000 FUNC    UND f_iu_ic
  10202060 -32656\(gp\) 00000000 00000000 FUNC    UND f_ic
  10202064 -32652\(gp\) 00000000 00000000 FUNC    UND f_iu
  10202068 -32648\(gp\) 00000000 00000000 FUNC    UND f


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
  10200014 10100000 10100050 FUNC    UND f_du_dc
  10200018 10100000 10100191 FUNC    UND f_lo_iu_dc
  1020001c 10100000 10100060 FUNC    UND f_iu_du_ic
  10200020 10100000 10100070 FUNC    UND f_lo_du_ic
  10200024 10100000 101001a1 FUNC    UND f_iu_dc
  10200028 10100000 10100080 FUNC    UND f_lo_iu_ic
  1020002c 10100000 10100090 FUNC    UND f_lo_ic
  10200030 10100000 101000a0 FUNC    UND f_lo_du_dc
  10200034 10100000 101000b0 FUNC    UND f_du
  10200038 10100000 101000c0 FUNC    UND f_du_ic_dc
  1020003c 10100000 101000d0 FUNC    UND f_du_ic
  10200040 10100000 101000e0 FUNC    UND f_iu_du_dc
  10200044 10100000 101001e1 FUNC    UND f_lo_dc
  10200048 10100000 101000f0 FUNC    UND f_iu_du
  1020004c 10100000 10100100 FUNC    UND f_lo_iu_du
  10200050 10100000 101001f1 FUNC    UND f_dc
  10200054 10100000 10100201 FUNC    UND f_ic_dc
  10200058 10100000 10100110 FUNC    UND f_lo_du
  1020005c 10100000 10100211 FUNC    UND f_iu_ic_dc
  10200060 10100000 10100120 FUNC    UND f_iu_du_ic_dc
  10200064 10100000 10100231 FUNC    UND f_lo_iu_ic_dc
  10200068 10100000 10100130 FUNC    UND f_lo_iu_du_ic
  1020006c 10100000 10100140 FUNC    UND f_lo_iu_du_ic_dc
  10200070 10100000 10100251 FUNC    UND f_lo_ic_dc
  10200074 10100000 10100150 FUNC    UND f_lo


