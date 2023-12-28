#source: emit-relocs-local-addend-foo.s
#source: emit-relocs-local-addend-bar.s
#ld: -T relocs.ld -e0 --emit-relocs
#readelf: -r

Relocation section '\.rela\.text' at offset .* contains 4 entries:
  Offset          Info           Type           Sym\. Value    Sym\. Name \+ Addend
000000010000  000200000113 R_AARCH64_ADR_PRE 0000000000010018 \.rodata \+ 0
000000010004  000200000115 R_AARCH64_ADD_ABS 0000000000010018 \.rodata \+ 0
00000001000c  000200000113 R_AARCH64_ADR_PRE 0000000000010018 \.rodata \+ 10
000000010010  000200000115 R_AARCH64_ADD_ABS 0000000000010018 \.rodata \+ 10

Relocation section '\.rela\.rodata' at offset .* contains 2 entries:
  Offset          Info           Type           Sym\. Value    Sym. Name \+ Addend
000000010020  000200000101 R_AARCH64_ABS64   0000000000010018 \.rodata \+ 0
000000010030  000200000101 R_AARCH64_ABS64   0000000000010018 \.rodata \+ 10
