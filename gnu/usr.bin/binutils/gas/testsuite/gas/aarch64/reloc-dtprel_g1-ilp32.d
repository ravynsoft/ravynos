#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	52a00009 	movz	w9, #0x0, lsl #16
			0: R_AARCH64_P32_TLSLD_MOVW_DTPREL_G1	x
