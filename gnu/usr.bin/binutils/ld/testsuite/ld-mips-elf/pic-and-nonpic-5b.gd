
Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Hard float \(double precision\)

Primary GOT:
 Canonical gp value: 000a7ff0

 Reserved entries:
   Address     Access  Initial Purpose
  000a0000 -32752\(gp\) 00000000 Lazy resolver
  000a0004 -32748\(gp\) 80000000 Module pointer \(GNU extension\)

# There must be GOT entries for the R_MIPS_REL32 relocation symbols.
 Global entries:
   Address     Access  Initial Sym\.Val\. Type    Ndx Name
  000a0008 -32744\(gp\) 00000000 00000000 OBJECT  UND obj2
  000a000c -32740\(gp\) 00000000 00000000 FUNC    UND bar


PLT GOT:

 Reserved entries:
   Address  Initial Purpose
  00081000 00000000 PLT lazy resolver
  00081004 00000000 Module pointer

 Entries:
   Address  Initial Sym.Val. Type    Ndx Name
  00081008 00043040 00043060 FUNC    UND foo
