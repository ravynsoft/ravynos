#objdump: -dr -M notes
#as: -march=armv8-a
#warning_output: sysreg-diagnostic.l

.*:     file format .*

Disassembly of section \.text:

.* <.*>:
.*:	d5130503 	msr	dbgdtrtx_el0, x3
.*:	d5130503 	msr	dbgdtrtx_el0, x3
.*:	d5330503 	mrs	x3, dbgdtrrx_el0
.*:	d5330503 	mrs	x3, dbgdtrrx_el0
.*:	d5180003 	msr	midr_el1, x3  // note: writing to a read-only register
.*:	d5180640 	msr	id_aa64isar2_el1, x0  // note: writing to a read-only register
