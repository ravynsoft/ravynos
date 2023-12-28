#as: -march=armv8-a+sve
#objdump: -dr


.* file format .*

Disassembly of section .*:

0+ <.*>:
.*:	d5380480 	mrs	x0, id_aa64zfr0_el1
.*:	d538049b 	mrs	x27, id_aa64zfr0_el1
.*:	d5381200 	mrs	x0, zcr_el1
.*:	d538121b 	mrs	x27, zcr_el1
.*:	d5181200 	msr	zcr_el1, x0
.*:	d518121a 	msr	zcr_el1, x26
.*:	d53d1200 	mrs	x0, zcr_el12
.*:	d53d121b 	mrs	x27, zcr_el12
.*:	d51d1200 	msr	zcr_el12, x0
.*:	d51d121a 	msr	zcr_el12, x26
.*:	d53c1200 	mrs	x0, zcr_el2
.*:	d53c121b 	mrs	x27, zcr_el2
.*:	d51c1200 	msr	zcr_el2, x0
.*:	d51c121a 	msr	zcr_el2, x26
.*:	d53e1200 	mrs	x0, zcr_el3
.*:	d53e121b 	mrs	x27, zcr_el3
.*:	d51e1200 	msr	zcr_el3, x0
.*:	d51e121a 	msr	zcr_el3, x26
