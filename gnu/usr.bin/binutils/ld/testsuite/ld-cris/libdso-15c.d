#source: expdref2.s
#as: --pic --no-underscore --em=criself
#ld: --shared -m crislinux --hash-style=sysv
#ld_after_inputfiles: tmpdir/libdso-15.so
#readelf: -d

Dynamic section at offset 0x1e8 contains 14 entries:
  Tag        Type                         Name/Value
 0x00000001 \(NEEDED\)                     Shared library: \[tmpdir/libdso-15.so\]
 0x00000004 \(HASH\)                       0x94
 0x00000005 \(STRTAB\)                     0x120
 0x00000006 \(SYMTAB\)                     0xc0
 0x0000000a \(STRSZ\)                      45 \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000003 \(PLTGOT\)                     0x2280
 0x00000007 \(RELA\)                       0x17c
 0x00000008 \(RELASZ\)                     24 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x6ffffffe \(VERNEED\)                    0x15c
 0x6fffffff \(VERNEEDNUM\)                 1
 0x6ffffff0 \(VERSYM\)                     0x14e
 0x00000000 \(NULL\)                       0x0
