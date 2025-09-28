#as: -mabi=ilp32
#source: reloc-tprel_lo12-ldst32.s
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	b980009b 	ldrsw	x27, \[x4\]
			0: R_AARCH64_P32_TLSLE_LDST32_TPREL_LO12	sym
