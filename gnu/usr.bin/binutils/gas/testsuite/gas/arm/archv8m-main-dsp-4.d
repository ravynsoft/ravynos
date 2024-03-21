#name: ARMv8-M Mainline with DSP instructions (Security Extensions 3)
#source: archv8m-cmse-msr.s
#as: -march=armv8-m.main+dsp
#objdump: -dr --prefix-addresses --show-raw-insn -M force-thumb

.*: +file format .*arm.*

Disassembly of section .text:
0+.* <[^>]*> f3ef 8008 	mrs	r0, MSP
0+.* <[^>]*> f3ef 8088 	mrs	r0, MSP_NS
0+.* <[^>]*> f3ef 8008 	mrs	r0, MSP
0+.* <[^>]*> f3ef 8088 	mrs	r0, MSP_NS
0+.* <[^>]*> f3ef 8109 	mrs	r1, PSP
0+.* <[^>]*> f3ef 8189 	mrs	r1, PSP_NS
0+.* <[^>]*> f3ef 8109 	mrs	r1, PSP
0+.* <[^>]*> f3ef 8189 	mrs	r1, PSP_NS
0+.* <[^>]*> f3ef 820a 	mrs	r2, MSPLIM
0+.* <[^>]*> f3ef 828a 	mrs	r2, MSPLIM_NS
0+.* <[^>]*> f3ef 820a 	mrs	r2, MSPLIM
0+.* <[^>]*> f3ef 828a 	mrs	r2, MSPLIM_NS
0+.* <[^>]*> f3ef 830b 	mrs	r3, PSPLIM
0+.* <[^>]*> f3ef 838b 	mrs	r3, PSPLIM_NS
0+.* <[^>]*> f3ef 830b 	mrs	r3, PSPLIM
0+.* <[^>]*> f3ef 838b 	mrs	r3, PSPLIM_NS
0+.* <[^>]*> f3ef 8410 	mrs	r4, PRIMASK
0+.* <[^>]*> f3ef 8490 	mrs	r4, PRIMASK_NS
0+.* <[^>]*> f3ef 8410 	mrs	r4, PRIMASK
0+.* <[^>]*> f3ef 8490 	mrs	r4, PRIMASK_NS
0+.* <[^>]*> f3ef 8511 	mrs	r5, BASEPRI
0+.* <[^>]*> f3ef 8591 	mrs	r5, BASEPRI_NS
0+.* <[^>]*> f3ef 8511 	mrs	r5, BASEPRI
0+.* <[^>]*> f3ef 8591 	mrs	r5, BASEPRI_NS
0+.* <[^>]*> f3ef 8613 	mrs	r6, FAULTMASK
0+.* <[^>]*> f3ef 8693 	mrs	r6, FAULTMASK_NS
0+.* <[^>]*> f3ef 8613 	mrs	r6, FAULTMASK
0+.* <[^>]*> f3ef 8693 	mrs	r6, FAULTMASK_NS
0+.* <[^>]*> f3ef 8714 	mrs	r7, CONTROL
0+.* <[^>]*> f3ef 8794 	mrs	r7, CONTROL_NS
0+.* <[^>]*> f3ef 8714 	mrs	r7, CONTROL
0+.* <[^>]*> f3ef 8794 	mrs	r7, CONTROL_NS
0+.* <[^>]*> f3ef 8898 	mrs	r8, SP_NS
0+.* <[^>]*> f3ef 8898 	mrs	r8, SP_NS
0+.* <[^>]*> f380 8808 	msr	MSP, r0
0+.* <[^>]*> f380 8888 	msr	MSP_NS, r0
0+.* <[^>]*> f380 8808 	msr	MSP, r0
0+.* <[^>]*> f380 8888 	msr	MSP_NS, r0
0+.* <[^>]*> f381 8809 	msr	PSP, r1
0+.* <[^>]*> f381 8889 	msr	PSP_NS, r1
0+.* <[^>]*> f381 8809 	msr	PSP, r1
0+.* <[^>]*> f381 8889 	msr	PSP_NS, r1
0+.* <[^>]*> f382 880a 	msr	MSPLIM, r2
0+.* <[^>]*> f382 888a 	msr	MSPLIM_NS, r2
0+.* <[^>]*> f382 880a 	msr	MSPLIM, r2
0+.* <[^>]*> f382 888a 	msr	MSPLIM_NS, r2
0+.* <[^>]*> f383 880b 	msr	PSPLIM, r3
0+.* <[^>]*> f383 888b 	msr	PSPLIM_NS, r3
0+.* <[^>]*> f383 880b 	msr	PSPLIM, r3
0+.* <[^>]*> f383 888b 	msr	PSPLIM_NS, r3
0+.* <[^>]*> f384 8810 	msr	PRIMASK, r4
0+.* <[^>]*> f384 8890 	msr	PRIMASK_NS, r4
0+.* <[^>]*> f384 8810 	msr	PRIMASK, r4
0+.* <[^>]*> f384 8890 	msr	PRIMASK_NS, r4
0+.* <[^>]*> f385 8811 	msr	BASEPRI, r5
0+.* <[^>]*> f385 8891 	msr	BASEPRI_NS, r5
0+.* <[^>]*> f385 8811 	msr	BASEPRI, r5
0+.* <[^>]*> f385 8891 	msr	BASEPRI_NS, r5
0+.* <[^>]*> f386 8813 	msr	FAULTMASK, r6
0+.* <[^>]*> f386 8893 	msr	FAULTMASK_NS, r6
0+.* <[^>]*> f386 8813 	msr	FAULTMASK, r6
0+.* <[^>]*> f386 8893 	msr	FAULTMASK_NS, r6
0+.* <[^>]*> f387 8814 	msr	CONTROL, r7
0+.* <[^>]*> f387 8894 	msr	CONTROL_NS, r7
0+.* <[^>]*> f387 8814 	msr	CONTROL, r7
0+.* <[^>]*> f387 8894 	msr	CONTROL_NS, r7
0+.* <[^>]*> f388 8898 	msr	SP_NS, r8
0+.* <[^>]*> f388 8898 	msr	SP_NS, r8
