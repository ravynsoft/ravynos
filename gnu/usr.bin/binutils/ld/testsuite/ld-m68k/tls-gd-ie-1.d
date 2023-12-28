#as: -mcpu=5206
#source: tls-gd-ie-1.s
#ld: -shared --hash-style=sysv
#readelf: -d -r

Dynamic section at offset .* contains 10 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       0x[0-9a-f]+
 0x00000005 \(STRTAB\)                     0x[0-9a-f]+
 0x00000006 \(SYMTAB\)                     0x[0-9a-f]+
 0x0000000a \(STRSZ\)                      [0-9]+ \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000007 \(RELA\)                       0x[0-9a-f]+
 0x00000008 \(RELASZ\)                     60 \(bytes\)
 0x00000009 \(RELAENT\)                    12 \(bytes\)
 0x00000016 \(TEXTREL\)                    0x0
 0x00000000 \(NULL\)                       0x0

Relocation section '.rela.dyn' at offset 0x[0-9a-f]+ contains 5 entries:
 Offset     Info    Type            Sym.Value  Sym. Name \+ Addend
[0-9a-f]+  [0-9a-f]+ R_68K_32          00000000   __tls_get_addr \+ 0
[0-9a-f]+  [0-9a-f]+ R_68K_32          00000000   __m68k_read_tp \+ 0
[0-9a-f]+  [0-9a-f]+ R_68K_TLS_.?TP.....? 00000000   x \+ 0
[0-9a-f]+  [0-9a-f]+ R_68K_TLS_.?TP.....? 00000000   x \+ 0
[0-9a-f]+  [0-9a-f]+ R_68K_TLS_.?TP.....? 00000000   x \+ 0
