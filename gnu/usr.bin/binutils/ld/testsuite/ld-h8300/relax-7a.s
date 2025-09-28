	.h8300s
# relax expected
	.global _start
	.section	.text.func1,"ax",@progbits
	.align 1
_start:
	sub.l	er0,er0
	sub.l	er2,er2
	mov.l	#var3,er1
	mov.l	@(table+4:32,er2),er2
	jmp	@er2
	.section	.rodata.tab,"a",@progbits
	.align 2
table:
	.long	.L20
	.long	.L21
	.long	.L22
	.long	.L30noRelax
	.long	.L31noRelax
	.long	.L32noRelax
	.long	.L100Relax
	.section	.text.func1
.L20:
	mov.b	@(var1+1:32,er0), r2l
	mov.b	r2l,@(var1+1:32,er0)
	mov.b	@(1:32,er1), r2l
	mov.b	r2l,@(1:32,er1)
	rts
.L21:
	mov.w	@(var2+2:32,er0), r2
	mov.w	r2,@(var2+2:32,er0)
	mov.w	@(2:32,er1), r2
	mov.w	r2,@(2:32,er1)
	rts
.L22:
	mov.l	@(var3+4:32,er0), er2
	mov.l	er2,@(var3+4:32,er0)
	mov.l	@(4:32,er1), er2
	mov.l	er2,@(4:32,er1)
	rts

.L100Relax:
	mov.l	#0x01007800,er0
# part of MOV.L @(d:24,ERs),ERd opcode
	mov.w	@var2+2:32,r1
	rts

# no relax allowed:
.L30noRelax:
	mov.b	@(var4+1:32,er0), r2l
	mov.b	r2l,@(var4+1:32,er0)
	mov.b	@(0x8000:32,er1), r2l
	mov.b	r2l,@(0x8000:32,er1)
	rts
.L31noRelax:
	mov.w	@(var5+2:32,er0), r2
	mov.w	r2,@(var5+2:32,er0)
	mov.w	@(0x8000:32,er1), r2
	mov.w	r2,@(0x8000:32,er1)
	rts
.L32noRelax:
	mov.l	@(var6+4:32,er0), er2
	mov.l	er2,@(var6+4:32,er0)
	mov.l	@(0x8000:32,er1), er2
	mov.l	er2,@(0x8000:32,er1)
	rts
