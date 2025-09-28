#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format elf32-.*aarch64


Disassembly of section \.text:

00000000 <.*>:
   0:	90000004 	adrp	x4, c <.*>
			0: R_AARCH64_P32_ADR_PREL_PG_HI21	ptrs
   4:	91000083 	add	x3, x4, #0x0
			4: R_AARCH64_P32_ADD_ABS_LO12_NC	ptrs
   8:	b9000080 	str	w0, \[x4\]
			8: R_AARCH64_P32_LDST32_ABS_LO12_NC	ptrs
   c:	b9000461 	str	w1, \[x3, #4\]
  10:	b9000862 	str	w2, \[x3, #8\]
  14:	90000004 	adrp	x4, c <.*>
			14: R_AARCH64_P32_ADR_GOT_PAGE	ptrs
  18:	f9400083 	ldr	x3, \[x4\]
			18: R_AARCH64_P32_LD32_GOT_LO12_NC	ptrs
  1c:	2a0403e0 	mov	w0, w4
  20:	d65f03c0 	ret
  24:	f9400083 	ldr	x3, \[x4\]
			24: R_AARCH64_P32_LD32_GOTPAGE_LO14	ptrs
