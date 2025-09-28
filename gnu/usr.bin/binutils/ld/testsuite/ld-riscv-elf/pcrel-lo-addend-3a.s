	.text
	.globl _start
	.align 3
_start:
.LA0:	auipc	a5, %pcrel_hi (ll)
	ld	a0, %pcrel_lo (.LA0)(a5)
	ld	a0, %pcrel_lo (.LA0 + 0x4)(a5)

.LA1:	auipc	a5, %pcrel_hi (ll + 0x4)
	ld	a0, %pcrel_lo (.LA1)(a5)
	ld	a0, %pcrel_lo (.LA1 + 0x4)(a5)

.LA2:	auipc	a5, %got_pcrel_hi (ll)
	ld	a0, %pcrel_lo (.LA2)(a5)

	.globl ll
	.data
ll:
	.dword	0
	.dword	0
	.dword	0
