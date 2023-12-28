#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:

.*:	d538a460 	mrs	x0, lorc_el1
.*:	d538a420 	mrs	x0, lorea_el1
.*:	d538a440 	mrs	x0, lorn_el1
.*:	d538a400 	mrs	x0, lorsa_el1
.*:	d53ecc80 	mrs	x0, icc_ctlr_el3
.*:	d538cca0 	mrs	x0, icc_sre_el1
.*:	d53cc9a0 	mrs	x0, icc_sre_el2
.*:	d53ecca0 	mrs	x0, icc_sre_el3
.*:	d53ccb20 	mrs	x0, ich_vtr_el2
.*:	d518a460 	msr	lorc_el1, x0
.*:	d518a420 	msr	lorea_el1, x0
.*:	d518a440 	msr	lorn_el1, x0
.*:	d518a400 	msr	lorsa_el1, x0
.*:	d51ecc80 	msr	icc_ctlr_el3, x0
.*:	d518cca0 	msr	icc_sre_el1, x0
.*:	d51cc9a0 	msr	icc_sre_el2, x0
.*:	d51ecca0 	msr	icc_sre_el3, x0
