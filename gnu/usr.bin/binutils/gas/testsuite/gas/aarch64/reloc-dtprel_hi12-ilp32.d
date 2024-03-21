#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	11000341 	add	w1, w26, #0x0
			0: R_AARCH64_P32_TLSLD_ADD_DTPREL_HI12	x
