#name: RAS 1.1 System registers
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d5385540 	mrs	x0, erxmisc2_el1
.*:	d5385560 	mrs	x0, erxmisc3_el1
.*:	d53854c0 	mrs	x0, erxpfgcdn_el1
.*:	d53854a0 	mrs	x0, erxpfgctl_el1
.*:	d5185540 	msr	erxmisc2_el1, x0
.*:	d5185560 	msr	erxmisc3_el1, x0
.*:	d51854c0 	msr	erxpfgcdn_el1, x0
.*:	d51854a0 	msr	erxpfgctl_el1, x0
.*:	d5385480 	mrs	x0, erxpfgf_el1
.*:	d5385540 	mrs	x0, erxmisc2_el1
.*:	d5385560 	mrs	x0, erxmisc3_el1
.*:	d53854c0 	mrs	x0, erxpfgcdn_el1
.*:	d53854a0 	mrs	x0, erxpfgctl_el1
.*:	d5185540 	msr	erxmisc2_el1, x0
.*:	d5185560 	msr	erxmisc3_el1, x0
.*:	d51854c0 	msr	erxpfgcdn_el1, x0
.*:	d51854a0 	msr	erxpfgctl_el1, x0
.*:	d5385480 	mrs	x0, erxpfgf_el1
