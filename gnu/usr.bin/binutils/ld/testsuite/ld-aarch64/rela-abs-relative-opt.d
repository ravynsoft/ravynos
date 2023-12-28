#name: rela-abs-relative --no-apply-dynamic-relocs
#source: rela-abs-relative.s
#target: [check_shared_lib_support]
#ld: -shared -Ttext-segment=0x100000 -Tdata=0x200000 -Trelocs.ld --no-apply-dynamic-relocs
#notarget: aarch64_be-*-*
#objdump: -dR -j .data
#...

Disassembly of section .data:

.* <a>:
  200000:	fe ca fe ca 00 00 00 00 00 00 00 00 00 00 00 00.*
			200008: R_AARCH64_RELATIVE	\*ABS\*\+0x100ca
  200010:	ad de ad de 00 00 00 00.*
