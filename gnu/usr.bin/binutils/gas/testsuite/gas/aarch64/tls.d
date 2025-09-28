#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
   0:	90000000 	adrp	x0, 0 <var>
			0: R_AARCH64_(P32_|)TLSDESC_ADR_PAGE21	var
   4:	f9400001 	ldr	x1, \[x0\]
			4: R_AARCH64_(P32_|)TLSDESC_LD(64|32)_LO12(_NC|)	var
   8:	91000000 	add	x0, x0, #0x0
			8: R_AARCH64_(P32_|)TLSDESC_ADD_LO12	var
   c:	d63f0020 	blr	x1
			c: R_AARCH64_(P32_|)TLSDESC_CALL	var
  10:	90000000 	adrp	x0, 0 <var>
			10: R_AARCH64_(P32_|)TLSGD_ADR_PAGE21	var
  14:	91000000 	add	x0, x0, #0x0
			14: R_AARCH64_(P32_|)TLSGD_ADD_LO12_NC	var
  18:	94000000 	bl	0 <__tls_get_addr>
			18: R_AARCH64_(P32_|)CALL26	__tls_get_addr
  1c:	90000000 	adrp	x0, 0 <var>
			1c: R_AARCH64_(P32_|)TLSIE_ADR_GOTTPREL_PAGE21	var
  20:	f9400000 	ldr	x0, \[x0\]
			20: R_AARCH64_(P32_|)TLSIE_LD(64|32)_GOTTPREL_LO12_NC	var
  24:	91000020 	add	x0, x1, #0x0
			24: R_AARCH64_(P32_|)TLSLE_ADD_TPREL_LO12	var
  28:	91400020 	add	x0, x1, #0x0, lsl #12
			28: R_AARCH64_(P32_|)TLSLE_ADD_TPREL_HI12	var
  2c:	91400020 	add	x0, x1, #0x0, lsl #12
			2c: R_AARCH64_(P32_|)TLSLE_ADD_TPREL_HI12	var
  30:	91000020 	add	x0, x1, #0x0
			30: R_AARCH64_(P32_|)TLSLE_ADD_TPREL_LO12_NC	var
  34:	d2a00000 	movz	x0, #0x0, lsl #16
			34: R_AARCH64_(P32_|)TLSLE_MOVW_TPREL_G1	var
  38:	f2800000 	movk	x0, #0x0
			38: R_AARCH64_(P32_|)TLSLE_MOVW_TPREL_G0_NC	var
