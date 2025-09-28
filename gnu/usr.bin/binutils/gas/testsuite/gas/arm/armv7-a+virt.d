# name: ARMv7-a+virt Instructions
# as: -march=armv7-a+virt
# objdump: -dr --prefix-addresses --show-raw-insn

.*: +file format .*arm.*

Disassembly of section .text:
0[0-9a-f]+ <[^>]+> e1400070 	hvc	0
0[0-9a-f]+ <[^>]+> e14fff7f 	hvc	65535	@ 0xffff
0[0-9a-f]+ <[^>]+> e160006e 	eret
0[0-9a-f]+ <[^>]+> e1001200 	mrs	r1, R8_usr
0[0-9a-f]+ <[^>]+> e1011200 	mrs	r1, R9_usr
0[0-9a-f]+ <[^>]+> e1021200 	mrs	r1, R10_usr
0[0-9a-f]+ <[^>]+> e1031200 	mrs	r1, R11_usr
0[0-9a-f]+ <[^>]+> e1041200 	mrs	r1, R12_usr
0[0-9a-f]+ <[^>]+> e1051200 	mrs	r1, SP_usr
0[0-9a-f]+ <[^>]+> e1061200 	mrs	r1, LR_usr
0[0-9a-f]+ <[^>]+> e1081200 	mrs	r1, R8_fiq
0[0-9a-f]+ <[^>]+> e1091200 	mrs	r1, R9_fiq
0[0-9a-f]+ <[^>]+> e10a1200 	mrs	r1, R10_fiq
0[0-9a-f]+ <[^>]+> e10b1200 	mrs	r1, R11_fiq
0[0-9a-f]+ <[^>]+> e10c1200 	mrs	r1, R12_fiq
0[0-9a-f]+ <[^>]+> e10d1200 	mrs	r1, SP_fiq
0[0-9a-f]+ <[^>]+> e10e1200 	mrs	r1, LR_fiq
0[0-9a-f]+ <[^>]+> e14e1200 	mrs	r1, SPSR_fiq
0[0-9a-f]+ <[^>]+> e1011300 	mrs	r1, SP_irq
0[0-9a-f]+ <[^>]+> e1001300 	mrs	r1, LR_irq
0[0-9a-f]+ <[^>]+> e1401300 	mrs	r1, SPSR_irq
0[0-9a-f]+ <[^>]+> e1031300 	mrs	r1, SP_svc
0[0-9a-f]+ <[^>]+> e1021300 	mrs	r1, LR_svc
0[0-9a-f]+ <[^>]+> e1421300 	mrs	r1, SPSR_svc
0[0-9a-f]+ <[^>]+> e1051300 	mrs	r1, SP_abt
0[0-9a-f]+ <[^>]+> e1041300 	mrs	r1, LR_abt
0[0-9a-f]+ <[^>]+> e1441300 	mrs	r1, SPSR_abt
0[0-9a-f]+ <[^>]+> e1071300 	mrs	r1, SP_und
0[0-9a-f]+ <[^>]+> e1061300 	mrs	r1, LR_und
0[0-9a-f]+ <[^>]+> e1461300 	mrs	r1, SPSR_und
0[0-9a-f]+ <[^>]+> e10d1300 	mrs	r1, SP_mon
0[0-9a-f]+ <[^>]+> e10c1300 	mrs	r1, LR_mon
0[0-9a-f]+ <[^>]+> e14c1300 	mrs	r1, SPSR_mon
0[0-9a-f]+ <[^>]+> e10f1300 	mrs	r1, SP_hyp
0[0-9a-f]+ <[^>]+> e10e1300 	mrs	r1, ELR_hyp
0[0-9a-f]+ <[^>]+> e14e1300 	mrs	r1, SPSR_hyp
0[0-9a-f]+ <[^>]+> e120f201 	msr	R8_usr, r1
0[0-9a-f]+ <[^>]+> e121f201 	msr	R9_usr, r1
0[0-9a-f]+ <[^>]+> e122f201 	msr	R10_usr, r1
0[0-9a-f]+ <[^>]+> e123f201 	msr	R11_usr, r1
0[0-9a-f]+ <[^>]+> e124f201 	msr	R12_usr, r1
0[0-9a-f]+ <[^>]+> e125f201 	msr	SP_usr, r1
0[0-9a-f]+ <[^>]+> e126f201 	msr	LR_usr, r1
0[0-9a-f]+ <[^>]+> e128f201 	msr	R8_fiq, r1
0[0-9a-f]+ <[^>]+> e129f201 	msr	R9_fiq, r1
0[0-9a-f]+ <[^>]+> e12af201 	msr	R10_fiq, r1
0[0-9a-f]+ <[^>]+> e12bf201 	msr	R11_fiq, r1
0[0-9a-f]+ <[^>]+> e12cf201 	msr	R12_fiq, r1
0[0-9a-f]+ <[^>]+> e12df201 	msr	SP_fiq, r1
0[0-9a-f]+ <[^>]+> e12ef201 	msr	LR_fiq, r1
0[0-9a-f]+ <[^>]+> e16ef201 	msr	SPSR_fiq, r1
0[0-9a-f]+ <[^>]+> e121f301 	msr	SP_irq, r1
0[0-9a-f]+ <[^>]+> e120f301 	msr	LR_irq, r1
0[0-9a-f]+ <[^>]+> e160f301 	msr	SPSR_irq, r1
0[0-9a-f]+ <[^>]+> e123f301 	msr	SP_svc, r1
0[0-9a-f]+ <[^>]+> e122f301 	msr	LR_svc, r1
0[0-9a-f]+ <[^>]+> e162f301 	msr	SPSR_svc, r1
0[0-9a-f]+ <[^>]+> e125f301 	msr	SP_abt, r1
0[0-9a-f]+ <[^>]+> e124f301 	msr	LR_abt, r1
0[0-9a-f]+ <[^>]+> e164f301 	msr	SPSR_abt, r1
0[0-9a-f]+ <[^>]+> e127f301 	msr	SP_und, r1
0[0-9a-f]+ <[^>]+> e126f301 	msr	LR_und, r1
0[0-9a-f]+ <[^>]+> e166f301 	msr	SPSR_und, r1
0[0-9a-f]+ <[^>]+> e12df301 	msr	SP_mon, r1
0[0-9a-f]+ <[^>]+> e12cf301 	msr	LR_mon, r1
0[0-9a-f]+ <[^>]+> e16cf301 	msr	SPSR_mon, r1
0[0-9a-f]+ <[^>]+> e12ff301 	msr	SP_hyp, r1
0[0-9a-f]+ <[^>]+> e12ef301 	msr	ELR_hyp, r1
0[0-9a-f]+ <[^>]+> e16ef301 	msr	SPSR_hyp, r1
0[0-9a-f]+ <[^>]+> f7e0 8000 	hvc	#0
0[0-9a-f]+ <[^>]+> f7ef 8fff 	hvc	#65535	@ 0xffff
0[0-9a-f]+ <[^>]+> f3de 8f00 	subs	pc, lr, #0
0[0-9a-f]+ <[^>]+> f3e0 8120 	mrs	r1, R8_usr
0[0-9a-f]+ <[^>]+> f3e1 8120 	mrs	r1, R9_usr
0[0-9a-f]+ <[^>]+> f3e2 8120 	mrs	r1, R10_usr
0[0-9a-f]+ <[^>]+> f3e3 8120 	mrs	r1, R11_usr
0[0-9a-f]+ <[^>]+> f3e4 8120 	mrs	r1, R12_usr
0[0-9a-f]+ <[^>]+> f3e5 8120 	mrs	r1, SP_usr
0[0-9a-f]+ <[^>]+> f3e6 8120 	mrs	r1, LR_usr
0[0-9a-f]+ <[^>]+> f3e8 8120 	mrs	r1, R8_fiq
0[0-9a-f]+ <[^>]+> f3e9 8120 	mrs	r1, R9_fiq
0[0-9a-f]+ <[^>]+> f3ea 8120 	mrs	r1, R10_fiq
0[0-9a-f]+ <[^>]+> f3eb 8120 	mrs	r1, R11_fiq
0[0-9a-f]+ <[^>]+> f3ec 8120 	mrs	r1, R12_fiq
0[0-9a-f]+ <[^>]+> f3ed 8120 	mrs	r1, SP_fiq
0[0-9a-f]+ <[^>]+> f3ee 8120 	mrs	r1, LR_fiq
0[0-9a-f]+ <[^>]+> f3fe 8120 	mrs	r1, SPSR_fiq
0[0-9a-f]+ <[^>]+> f3e1 8130 	mrs	r1, SP_irq
0[0-9a-f]+ <[^>]+> f3e0 8130 	mrs	r1, LR_irq
0[0-9a-f]+ <[^>]+> f3f0 8130 	mrs	r1, SPSR_irq
0[0-9a-f]+ <[^>]+> f3e3 8130 	mrs	r1, SP_svc
0[0-9a-f]+ <[^>]+> f3e2 8130 	mrs	r1, LR_svc
0[0-9a-f]+ <[^>]+> f3f2 8130 	mrs	r1, SPSR_svc
0[0-9a-f]+ <[^>]+> f3e5 8130 	mrs	r1, SP_abt
0[0-9a-f]+ <[^>]+> f3e4 8130 	mrs	r1, LR_abt
0[0-9a-f]+ <[^>]+> f3f4 8130 	mrs	r1, SPSR_abt
0[0-9a-f]+ <[^>]+> f3e7 8130 	mrs	r1, SP_und
0[0-9a-f]+ <[^>]+> f3e6 8130 	mrs	r1, LR_und
0[0-9a-f]+ <[^>]+> f3f6 8130 	mrs	r1, SPSR_und
0[0-9a-f]+ <[^>]+> f3ed 8130 	mrs	r1, SP_mon
0[0-9a-f]+ <[^>]+> f3ec 8130 	mrs	r1, LR_mon
0[0-9a-f]+ <[^>]+> f3fc 8130 	mrs	r1, SPSR_mon
0[0-9a-f]+ <[^>]+> f3ef 8130 	mrs	r1, SP_hyp
0[0-9a-f]+ <[^>]+> f3ee 8130 	mrs	r1, ELR_hyp
0[0-9a-f]+ <[^>]+> f3fe 8130 	mrs	r1, SPSR_hyp
0[0-9a-f]+ <[^>]+> f381 8020 	msr	R8_usr, r1
0[0-9a-f]+ <[^>]+> f381 8120 	msr	R9_usr, r1
0[0-9a-f]+ <[^>]+> f381 8220 	msr	R10_usr, r1
0[0-9a-f]+ <[^>]+> f381 8320 	msr	R11_usr, r1
0[0-9a-f]+ <[^>]+> f381 8420 	msr	R12_usr, r1
0[0-9a-f]+ <[^>]+> f381 8520 	msr	SP_usr, r1
0[0-9a-f]+ <[^>]+> f381 8620 	msr	LR_usr, r1
0[0-9a-f]+ <[^>]+> f381 8820 	msr	R8_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8920 	msr	R9_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8a20 	msr	R10_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8b20 	msr	R11_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8c20 	msr	R12_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8d20 	msr	SP_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8e20 	msr	LR_fiq, r1
0[0-9a-f]+ <[^>]+> f391 8e20 	msr	SPSR_fiq, r1
0[0-9a-f]+ <[^>]+> f381 8130 	msr	SP_irq, r1
0[0-9a-f]+ <[^>]+> f381 8030 	msr	LR_irq, r1
0[0-9a-f]+ <[^>]+> f391 8030 	msr	SPSR_irq, r1
0[0-9a-f]+ <[^>]+> f381 8330 	msr	SP_svc, r1
0[0-9a-f]+ <[^>]+> f381 8230 	msr	LR_svc, r1
0[0-9a-f]+ <[^>]+> f391 8230 	msr	SPSR_svc, r1
0[0-9a-f]+ <[^>]+> f381 8530 	msr	SP_abt, r1
0[0-9a-f]+ <[^>]+> f381 8430 	msr	LR_abt, r1
0[0-9a-f]+ <[^>]+> f391 8430 	msr	SPSR_abt, r1
0[0-9a-f]+ <[^>]+> f381 8730 	msr	SP_und, r1
0[0-9a-f]+ <[^>]+> f381 8630 	msr	LR_und, r1
0[0-9a-f]+ <[^>]+> f391 8630 	msr	SPSR_und, r1
0[0-9a-f]+ <[^>]+> f381 8d30 	msr	SP_mon, r1
0[0-9a-f]+ <[^>]+> f381 8c30 	msr	LR_mon, r1
0[0-9a-f]+ <[^>]+> f391 8c30 	msr	SPSR_mon, r1
0[0-9a-f]+ <[^>]+> f381 8f30 	msr	SP_hyp, r1
0[0-9a-f]+ <[^>]+> f381 8e30 	msr	ELR_hyp, r1
0[0-9a-f]+ <[^>]+> f391 8e30 	msr	SPSR_hyp, r1
