#name: GOT dump (readelf -A) test 2
#source: got-dump-2.s
#as: -mips3
#ld: -Tgot-dump-2.ld -shared
#readelf: -A

Primary GOT:
 Canonical gp value: 0001236000008000

 Reserved entries:
           Address     Access          Initial Purpose
  0001236000000010 -32752\(gp\) 0000000000000000 Lazy resolver
  0001236000000018 -32744\(gp\) 8000000000000000 Module pointer \(GNU extension\)

 Local entries:
           Address     Access          Initial
  0001236000000020 -32736\(gp\) 0001236000000000
  0001236000000028 -32728\(gp\) 0001236000000008

 Global entries:
           Address     Access          Initial         Sym.Val. Type    Ndx Name
  0001236000000030 -32720\(gp\) 0001235000000020 0001235000000020 FUNC    UND extern
  0001236000000038 -32712\(gp\) 0001235000000000 0001235000000000 FUNC      7 glob
  0001236000000040 -32704\(gp\) 0000000000000000 0000000000000000 NOTYPE  UND undef

