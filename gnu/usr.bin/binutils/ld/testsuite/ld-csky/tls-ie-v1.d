#name: TLS Initial Exec link test (C-SKY v1)
#source: tls-ie-v1.s
#as: -mcpu=ck610
#ld: -shared --hash-style=sysv
#readelf: -d -r

Dynamic section at offset 0x[0-9a-f]+ contains 10 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       .*
 0x00000005 \(STRTAB\)                     .*
 0x00000006 \(SYMTAB\)                     .*
 0x0000000a \(STRSZ\)                      .* \(bytes\)
 0x0000000b \(SYMENT\)                     .* \(bytes\)
 0x00000003 \(PLTGOT\)                     0x[0-9a-f]+
 0x00000007 \(RELA\)                       0x[0-9a-f]+
 0x00000008 \(RELASZ\)                     12 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x00000000 \(NULL\)                       0x0

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
[0-9a-f]+  0000003a R_CKCORE_TLS_TPOF            0
