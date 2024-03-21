#as: -mabi=lp64
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	d2a00000 	movz	x0, #0x0, lsl #16
			0: R_AARCH64_TLSDESC_OFF_G1	var
   4:	f2800000 	movk	x0, #0x0
			4: R_AARCH64_TLSDESC_OFF_G0_NC	var
   8:	f8606a41 	ldr	x1, \[x18, x0\]
			8: R_AARCH64_TLSDESC_LDR	var
   c:	8b000240 	add	x0, x18, x0
			c: R_AARCH64_TLSDESC_ADD	var
  10:	d63f0020 	blr	x1
			10: R_AARCH64_TLSDESC_CALL	var
