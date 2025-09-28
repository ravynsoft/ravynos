#source: got-1.s
#ld: -shared --hash-style=sysv
#readelf: -d -r

Dynamic section at offset .* contains 9 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       0x[0-9a-f]+
 0x00000005 \(STRTAB\)                     0x[0-9a-f]+
 0x00000006 \(SYMTAB\)                     0x[0-9a-f]+
 0x0000000a \(STRSZ\)                      [0-9]+ \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000007 \(RELA\)                       0x[0-9a-f]+
 0x00000008 \(RELASZ\)                     12 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x00000000 \(NULL\)                       0x0

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
[0-9a-f]+  [0-9a-f]+ R_68K_GLOB_DAT    00000000   a \+ 0
