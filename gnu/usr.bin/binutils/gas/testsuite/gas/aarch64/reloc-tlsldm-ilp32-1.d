#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	8b030041 	add	x1, x2, x3
   4:	10000000 	adr	x0, 0 <dummy>
			4: R_AARCH64_P32_TLSLD_ADR_PREL21	dummy
