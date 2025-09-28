#target: [check_shared_lib_support]
#ld: -pie -defsym foo=0x1 -defsym bar=0x2 -z notext
#readelf: -r

Relocation section '\.rela\.dyn' at offset .* contains 2 entries:
  Offset          Info           Type           Sym\. Value    Sym\. Name \+ Addend
000000000000  000000000000 R_AARCH64_NONE                       0
000000000000  000000000000 R_AARCH64_NONE                       0
