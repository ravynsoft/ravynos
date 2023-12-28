#source: emit-relocs-local-addend-foo.s
#source: emit-relocs-local-addend-bar.s
#ld: -e0 -r
#readelf: -r

Relocation section '\.rela\.text' at offset .* contains 4 entries:
  Offset          Info           Type           Sym\. Value    Sym\. Name \+ Addend
000000000000  000200000113 R_AARCH64_ADR_PRE 0000000000000000 \.rodata \+ 0
000000000004  000200000115 R_AARCH64_ADD_ABS 0000000000000000 \.rodata \+ 0
00000000000c  000200000113 R_AARCH64_ADR_PRE 0000000000000000 \.rodata \+ 10
000000000010  000200000115 R_AARCH64_ADD_ABS 0000000000000000 \.rodata \+ 10

Relocation section '\.rela\.rodata' at offset .* contains 2 entries:
  Offset          Info           Type           Sym\. Value    Sym. Name \+ Addend
000000000008  000200000101 R_AARCH64_ABS64   0000000000000000 \.rodata \+ 0
000000000018  000200000101 R_AARCH64_ABS64   0000000000000000 \.rodata \+ 10
