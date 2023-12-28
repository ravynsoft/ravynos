#as: -mabi=lp64
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

0000000000000000 <.*>:
   0:	f940009b 	ldr	x27, \[x4\]
			0: R_AARCH64_TLSLE_LDST64_TPREL_LO12	sym
