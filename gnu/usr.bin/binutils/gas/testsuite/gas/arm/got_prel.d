# name: R_ARM_GOT_PREL relocation
# source: got_prel.s
# as: -march=armv5te -meabi=5 --generate-missing-build-notes=no
# readelf: -x 4 -r
# target: *-*-*eabi* *-*-linux-* *-*-elf *-*-nacl*

Relocation section '.rel.text.foo' at offset .* contains 1 entry:
 Offset     Info    Type            Sym.Value  Sym. Name
00000010  00000c60 R_ARM_GOT_PREL    00000000   i

Relocation section '.rel.ARM.exidx.text.foo' at offset .* contains 2 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
00000000  0000042a R_ARM_PREL31      00000000   .text.foo
00000000  00000d00 R_ARM_NONE        00000000   __aeabi_unwind_cpp_pr0

Hex dump of section '.text.foo':
 NOTE: This section has relocations against it, but these have NOT been applied to this dump.
  0x00000000 (034b7b44|4b03447b) (1b681a68|681b681a) (1860101c|60181c10) (7047c046|477046c0) .*
  0x00000010 (0a000000|0000000a)                            ....
