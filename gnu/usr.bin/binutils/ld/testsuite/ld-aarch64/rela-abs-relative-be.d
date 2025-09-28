#name: rela-abs-relative (big endian)
#source: rela-abs-relative.s
#alltargets: [check_shared_lib_support] aarch64_be-*-*
#ld: -shared -Ttext-segment=0x100000 -Tdata=0x200000 -Trelocs.ld
#objdump: -dR -j .data
#...

Disassembly of section .data:

.* <a>:
  200000:	00 00 00 00 ca fe ca fe 00 00 00 00 00 01 00 ca.*
			200008: R_AARCH64_RELATIVE	\*ABS\*\+0x100ca
  200010:	00 00 00 00 de ad de ad.*
