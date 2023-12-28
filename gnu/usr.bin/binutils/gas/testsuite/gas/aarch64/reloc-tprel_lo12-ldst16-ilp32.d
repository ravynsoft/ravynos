#as: -mabi=ilp32
#source: reloc-tprel_lo12-ldst16.s
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	7980009b 	ldrsh	x27, \[x4\]
			0: R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12	sym
