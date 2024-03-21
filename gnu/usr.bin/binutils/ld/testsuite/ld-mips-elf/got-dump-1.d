#name: GOT dump (readelf -A) test 1
#source: got-dump-1.s
#as: -mips3
#ld: -Tgot-dump-1.ld -shared
#readelf: -A

Primary GOT:
 Canonical gp value: 00068000

 Reserved entries:
   Address     Access  Initial Purpose
  00060010 -32752\(gp\) 00000000 Lazy resolver
  00060014 -32748\(gp\) 80000000 Module pointer \(GNU extension\)

 Local entries:
   Address     Access  Initial
  00060018 -32744\(gp\) 00060000
  0006001c -32740\(gp\) 00060004

 Global entries:
   Address     Access  Initial Sym.Val. Type    Ndx Name
  00060020 -32736\(gp\) 00050020 00050020 FUNC    UND extern
  00060024 -32732\(gp\) 00050000 00050000 FUNC      7 glob
  00060028 -32728\(gp\) 00000000 00000000 NOTYPE  UND undef

