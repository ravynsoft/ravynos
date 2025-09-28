#name: PR ld/17057
#as: --32
#ld: -shared -melf_i386
#readelf: -r --wide

Relocation section '.rel.plt' at offset 0x[0-9a-f]+ contains 2 entries:
 Offset     Info    Type                Sym. Value  Symbol's Name
[0-9a-f ]+R_386_JUMP_SLOT +0+ +foo
[0-9a-f ]+R_386_TLS_DESC +0+ +my_tls
