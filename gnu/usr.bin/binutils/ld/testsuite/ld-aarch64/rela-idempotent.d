#name: rela-idempotent
#source: rela-idempotent.s
#target: [check_shared_lib_support]
#ld: -shared -Ttext-segment=0x100000 -Tdata=0x200000 -Trelocs.ld
#notarget: aarch64_be-*-*
#objdump: -dR -j .data
#...

Disassembly of section .data:

.* <l>:
  200000:	00200032.*
			200000: R_AARCH64_RELATIVE	\*ABS\*\+0x200032
  200004:	00000000.*

.* <q>:
  200008:	00200054.*
			200008: R_AARCH64_RELATIVE	\*ABS\*\+0x200054
  20000c:	00000000.*
