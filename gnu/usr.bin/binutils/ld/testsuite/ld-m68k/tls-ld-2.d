#as: -mcpu=5206
#source: tls-ld-2.s
#ld: -shared --hash-style=sysv
#readelf: -d -r

Dynamic section at offset .* contains 6 entries:
  Tag        Type                         Name/Value
 0x00000004 \(HASH\)                       0x[0-9a-f]+
 0x00000005 \(STRTAB\)                     0x[0-9a-f]+
 0x00000006 \(SYMTAB\)                     0x[0-9a-f]+
 0x0000000a \(STRSZ\)                      [0-9]+ \(bytes\)
 0x0000000b \(SYMENT\)                     16 \(bytes\)
 0x00000000 \(NULL\)                       0x0

There are no relocations in this file.
