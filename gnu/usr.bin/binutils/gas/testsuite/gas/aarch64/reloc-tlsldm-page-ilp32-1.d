#as: -mabi=ilp32
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	0b030041 	add	w1, w2, w3
   4:	90000000 	adrp	x0, 0 <dummy>
			4: R_AARCH64_P32_TLSLD_ADR_PAGE21	dummy

