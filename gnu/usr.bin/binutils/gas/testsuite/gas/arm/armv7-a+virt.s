	.text
	.syntax unified
	.arm
foo:
	hvc 0x0000
	hvc 0xffff
	eret
	mrs r1, R8_usr
	mrs r1, R9_usr
	mrs r1, R10_usr
	mrs r1, R11_usr
	mrs r1, R12_usr
	mrs r1, SP_usr
	mrs r1, LR_usr
	mrs r1, R8_fiq
	mrs r1, R9_fiq
	mrs r1, R10_fiq
	mrs r1, R11_fiq
	mrs r1, R12_fiq
	mrs r1, SP_fiq
	mrs r1, LR_fiq
	mrs r1, SPSR_fiq
	mrs r1, SP_irq
	mrs r1, LR_irq
	mrs r1, SPSR_irq
	mrs r1, SP_svc
	mrs r1, LR_svc
	mrs r1, SPSR_svc
	mrs r1, SP_abt
	mrs r1, LR_abt
	mrs r1, SPSR_abt
	mrs r1, SP_und
	mrs r1, LR_und
	mrs r1, SPSR_und
	mrs r1, SP_mon
	mrs r1, LR_mon
	mrs r1, SPSR_mon
	mrs r1, SP_hyp
	mrs r1, ELR_hyp
	mrs r1, SPSR_hyp
	msr R8_usr, r1
	msr R9_usr, r1
	msr R10_usr, r1
	msr R11_usr, r1
	msr R12_usr, r1
	msr SP_usr, r1
	msr LR_usr, r1
	msr R8_fiq, r1
	msr R9_fiq, r1
	msr R10_fiq, r1
	msr R11_fiq, r1
	msr R12_fiq, r1
	msr SP_fiq, r1
	msr LR_fiq, r1
	msr SPSR_fiq, r1
	msr SP_irq, r1
	msr LR_irq, r1
	msr SPSR_irq, r1
	msr SP_svc, r1
	msr LR_svc, r1
	msr SPSR_svc, r1
	msr SP_abt, r1
	msr LR_abt, r1
	msr SPSR_abt, r1
	msr SP_und, r1
	msr LR_und, r1
	msr SPSR_und, r1
	msr SP_mon, r1
	msr LR_mon, r1
	msr SPSR_mon, r1
	msr SP_hyp, r1
	msr ELR_hyp, r1
	msr SPSR_hyp, r1

	.thumb
bar:
	hvc 0x0000
	hvc 0xffff
	eret
	mrs r1, R8_usr
	mrs r1, R9_usr
	mrs r1, R10_usr
	mrs r1, R11_usr
	mrs r1, R12_usr
	mrs r1, SP_usr
	mrs r1, LR_usr
	mrs r1, R8_fiq
	mrs r1, R9_fiq
	mrs r1, R10_fiq
	mrs r1, R11_fiq
	mrs r1, R12_fiq
	mrs r1, SP_fiq
	mrs r1, LR_fiq
	mrs r1, SPSR_fiq
	mrs r1, SP_irq
	mrs r1, LR_irq
	mrs r1, SPSR_irq
	mrs r1, SP_svc
	mrs r1, LR_svc
	mrs r1, SPSR_svc
	mrs r1, SP_abt
	mrs r1, LR_abt
	mrs r1, SPSR_abt
	mrs r1, SP_und
	mrs r1, LR_und
	mrs r1, SPSR_und
	mrs r1, SP_mon
	mrs r1, LR_mon
	mrs r1, SPSR_mon
	mrs r1, SP_hyp
	mrs r1, ELR_hyp
	mrs r1, SPSR_hyp
	msr R8_usr, r1
	msr R9_usr, r1
	msr R10_usr, r1
	msr R11_usr, r1
	msr R12_usr, r1
	msr SP_usr, r1
	msr LR_usr, r1
	msr R8_fiq, r1
	msr R9_fiq, r1
	msr R10_fiq, r1
	msr R11_fiq, r1
	msr R12_fiq, r1
	msr SP_fiq, r1
	msr LR_fiq, r1
	msr SPSR_fiq, r1
	msr SP_irq, r1
	msr LR_irq, r1
	msr SPSR_irq, r1
	msr SP_svc, r1
	msr LR_svc, r1
	msr SPSR_svc, r1
	msr SP_abt, r1
	msr LR_abt, r1
	msr SPSR_abt, r1
	msr SP_und, r1
	msr LR_und, r1
	msr SPSR_und, r1
	msr SP_mon, r1
	msr LR_mon, r1
	msr SPSR_mon, r1
	msr SP_hyp, r1
	msr ELR_hyp, r1
	msr SPSR_hyp, r1

