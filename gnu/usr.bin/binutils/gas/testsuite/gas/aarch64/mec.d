#name: MEC System registers
#objdump: -dr

.*:     file format .*

Disassembly of section .text:

0+ <.*>:

[^:]*:	d53ca8e0 	mrs	x0, mecidr_el2
[^:]*:	d53ca800 	mrs	x0, mecid_p0_el2
[^:]*:	d53ca820 	mrs	x0, mecid_a0_el2
[^:]*:	d53ca840 	mrs	x0, mecid_p1_el2
[^:]*:	d53ca860 	mrs	x0, mecid_a1_el2
[^:]*:	d53ca900 	mrs	x0, vmecid_p_el2
[^:]*:	d53ca920 	mrs	x0, vmecid_a_el2
[^:]*:	d53eaa20 	mrs	x0, mecid_rl_a_el3
[^:]*:	d51ca800 	msr	mecid_p0_el2, x0
[^:]*:	d51ca820 	msr	mecid_a0_el2, x0
[^:]*:	d51ca840 	msr	mecid_p1_el2, x0
[^:]*:	d51ca860 	msr	mecid_a1_el2, x0
[^:]*:	d51ca900 	msr	vmecid_p_el2, x0
[^:]*:	d51ca920 	msr	vmecid_a_el2, x0
[^:]*:	d51eaa20 	msr	mecid_rl_a_el3, x0
