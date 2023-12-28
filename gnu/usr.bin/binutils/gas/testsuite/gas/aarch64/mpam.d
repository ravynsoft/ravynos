#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
.*:	d538a520 	mrs	x0, mpam0_el1
.*:	d518a520 	msr	mpam0_el1, x0
.*:	d538a500 	mrs	x0, mpam1_el1
.*:	d518a500 	msr	mpam1_el1, x0
.*:	d53da500 	mrs	x0, mpam1_el12
.*:	d51da500 	msr	mpam1_el12, x0
.*:	d53ca500 	mrs	x0, mpam2_el2
.*:	d51ca500 	msr	mpam2_el2, x0
.*:	d53ea500 	mrs	x0, mpam3_el3
.*:	d51ea500 	msr	mpam3_el3, x0
.*:	d53ca400 	mrs	x0, mpamhcr_el2
.*:	d51ca400 	msr	mpamhcr_el2, x0
.*:	d538a480 	mrs	x0, mpamidr_el1
.*:	d53ca600 	mrs	x0, mpamvpm0_el2
.*:	d51ca600 	msr	mpamvpm0_el2, x0
.*:	d53ca620 	mrs	x0, mpamvpm1_el2
.*:	d51ca620 	msr	mpamvpm1_el2, x0
.*:	d53ca640 	mrs	x0, mpamvpm2_el2
.*:	d51ca640 	msr	mpamvpm2_el2, x0
.*:	d53ca660 	mrs	x0, mpamvpm3_el2
.*:	d51ca660 	msr	mpamvpm3_el2, x0
.*:	d53ca680 	mrs	x0, mpamvpm4_el2
.*:	d51ca680 	msr	mpamvpm4_el2, x0
.*:	d53ca6a0 	mrs	x0, mpamvpm5_el2
.*:	d51ca6a0 	msr	mpamvpm5_el2, x0
.*:	d53ca6c0 	mrs	x0, mpamvpm6_el2
.*:	d51ca6c0 	msr	mpamvpm6_el2, x0
.*:	d53ca6e0 	mrs	x0, mpamvpm7_el2
.*:	d51ca6e0 	msr	mpamvpm7_el2, x0
.*:	d53ca420 	mrs	x0, mpamvpmv_el2
.*:	d51ca420 	msr	mpamvpmv_el2, x0
