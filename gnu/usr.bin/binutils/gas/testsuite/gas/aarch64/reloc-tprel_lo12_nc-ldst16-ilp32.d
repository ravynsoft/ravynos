#as: -mabi=ilp32
#source: reloc-tprel_lo12_nc-ldst16.s
#objdump: -dr
# This test is only valid on ELF based ports.
#notarget: *-*-*coff *-*-pe *-*-wince *-*-*aout* *-*-netbsd

.*:     file format .*

Disassembly of section \.text:

00000000 <.*>:
   0:	7940009b 	ldrh	w27, \[x4\]
			0: R_AARCH64_P32_TLSLE_LDST16_TPREL_LO12_NC	sym
