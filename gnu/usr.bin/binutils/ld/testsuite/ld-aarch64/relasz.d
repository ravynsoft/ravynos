#source: relasz.s
#target: [check_shared_lib_support]
#ld: -shared --hash-style=sysv -Taarch64.ld
#readelf: -d
# Check that the RELASZ section has the correct size even if we are
# using a non-default linker script that merges .rela.dyn and .rela.plt
# in the output.

Dynamic section at offset 0x[0-9a-f]+ contains 9 entries:
  Tag        Type                         Name/Value
 0x0000000000000004 \(HASH\)               0x[0-9a-f]+
 0x0000000000000005 \(STRTAB\)             0x[0-9a-f]+
 0x0000000000000006 \(SYMTAB\)             0x[0-9a-f]+
 0x000000000000000a \(STRSZ\)              [0-9]+ \(bytes\)
 0x000000000000000b \(SYMENT\)             [0-9]+ \(bytes\)
 0x0000000000000007 \(RELA\)               0x[0-9a-f]+
 0x0000000000000008 \(RELASZ\)             24 \(bytes\)
 0x0000000000000009 \(RELAENT\)            24 \(bytes\)
 0x0000000000000000 \(NULL\)               0x0
