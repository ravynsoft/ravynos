#as: -march=armv8-a+ras
#objdump: -dr

.*:     file format .*

Disassembly of section \.text:

0+ <.*>:
[^:]+:	d503221f 	esb
[^:]+:	d503221f 	esb
[^:]+:	d5385305 	mrs	x5, erridr_el1
[^:]+:	d5185327 	msr	errselr_el1, x7
[^:]+:	d5385327 	mrs	x7, errselr_el1
[^:]+:	d5385405 	mrs	x5, erxfr_el1
[^:]+:	d5185425 	msr	erxctlr_el1, x5
[^:]+:	d5385425 	mrs	x5, erxctlr_el1
[^:]+:	d5185445 	msr	erxstatus_el1, x5
[^:]+:	d5385445 	mrs	x5, erxstatus_el1
[^:]+:	d5185465 	msr	erxaddr_el1, x5
[^:]+:	d5385465 	mrs	x5, erxaddr_el1
[^:]+:	d5185505 	msr	erxmisc0_el1, x5
[^:]+:	d5385505 	mrs	x5, erxmisc0_el1
[^:]+:	d5185525 	msr	erxmisc1_el1, x5
[^:]+:	d5385525 	mrs	x5, erxmisc1_el1
[^:]+:	d53c5265 	mrs	x5, vsesr_el2
[^:]+:	d518c125 	msr	disr_el1, x5
[^:]+:	d538c125 	mrs	x5, disr_el1
[^:]+:	d53cc125 	mrs	x5, vdisr_el2
[^:]+:	d503221f 	esb
[^:]+:	d503221f 	esb
[^:]+:	d5385305 	mrs	x5, erridr_el1
[^:]+:	d5185327 	msr	errselr_el1, x7
[^:]+:	d5385327 	mrs	x7, errselr_el1
[^:]+:	d5385405 	mrs	x5, erxfr_el1
[^:]+:	d5185425 	msr	erxctlr_el1, x5
[^:]+:	d5385425 	mrs	x5, erxctlr_el1
[^:]+:	d5185445 	msr	erxstatus_el1, x5
[^:]+:	d5385445 	mrs	x5, erxstatus_el1
[^:]+:	d5185465 	msr	erxaddr_el1, x5
[^:]+:	d5385465 	mrs	x5, erxaddr_el1
[^:]+:	d5185505 	msr	erxmisc0_el1, x5
[^:]+:	d5385505 	mrs	x5, erxmisc0_el1
[^:]+:	d5185525 	msr	erxmisc1_el1, x5
[^:]+:	d5385525 	mrs	x5, erxmisc1_el1
[^:]+:	d53c5265 	mrs	x5, vsesr_el2
[^:]+:	d518c125 	msr	disr_el1, x5
[^:]+:	d538c125 	mrs	x5, disr_el1
[^:]+:	d53cc125 	mrs	x5, vdisr_el2
[^:]+:	d503221f 	esb
[^:]+:	d503221f 	esb
[^:]+:	d5385305 	mrs	x5, erridr_el1
[^:]+:	d5185327 	msr	errselr_el1, x7
[^:]+:	d5385327 	mrs	x7, errselr_el1
[^:]+:	d5385405 	mrs	x5, erxfr_el1
[^:]+:	d5185425 	msr	erxctlr_el1, x5
[^:]+:	d5385425 	mrs	x5, erxctlr_el1
[^:]+:	d5185445 	msr	erxstatus_el1, x5
[^:]+:	d5385445 	mrs	x5, erxstatus_el1
[^:]+:	d5185465 	msr	erxaddr_el1, x5
[^:]+:	d5385465 	mrs	x5, erxaddr_el1
[^:]+:	d5185505 	msr	erxmisc0_el1, x5
[^:]+:	d5385505 	mrs	x5, erxmisc0_el1
[^:]+:	d5185525 	msr	erxmisc1_el1, x5
[^:]+:	d5385525 	mrs	x5, erxmisc1_el1
[^:]+:	d53c5265 	mrs	x5, vsesr_el2
[^:]+:	d518c125 	msr	disr_el1, x5
[^:]+:	d538c125 	mrs	x5, disr_el1
[^:]+:	d53cc125 	mrs	x5, vdisr_el2
