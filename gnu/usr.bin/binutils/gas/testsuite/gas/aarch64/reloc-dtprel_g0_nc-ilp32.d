#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	72800010 	movk	w16, #0x0
			0: R_AARCH64_P32_TLSLD_MOVW_DTPREL_G0_NC	x
