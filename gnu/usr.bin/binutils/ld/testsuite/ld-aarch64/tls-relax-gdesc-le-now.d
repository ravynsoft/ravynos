#source: tls-relax-gdesc-le.s
#target: [check_shared_lib_support]
#ld: -shared -z now
#readelf: -dr
#...
 0x.+ \(STRTAB\)   \s+0x.+
 0x.+ \(SYMTAB\)   \s+0x.+
 0x.+ \(STRSZ\)    \s+.+ \(bytes\)
 0x.+ \(SYMENT\)   \s+.+ \(bytes\)
 0x.+ \(PLTGOT\)   \s+0x.+
 0x.+ \(PLTRELSZ\) \s+.+ \(bytes\)
 0x.+ \(PLTREL\)   \s+RELA
 0x.+ \(JMPREL\)   \s+0x.+
 0x.+ (\(BIND_NOW\) \s+|\(FLAGS\)  \s+   BIND_NOW)
 0x.+ \(FLAGS_1\)  \s+   Flags: NOW
 0x.+ \(NULL\)     \s+   0x0

Relocation section '\.rela\.plt' at offset .+ contains 1 entry:
  Offset          Info           Type           Sym\. Value    Sym\. Name \+ Addend
.+  .+ R_AARCH64_TLSDESC                    0
