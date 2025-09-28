	.macro csr val
	csrr a0,\val
	csrw \val, a1
	.endm

	# Supported privileged specs, 1.9.1, 1.10, 1.11 and 1.12.

	# User Counter/Timers
	csr cycle
	csr time
	csr instret
	csr hpmcounter3
	csr hpmcounter4
	csr hpmcounter5
	csr hpmcounter6
	csr hpmcounter7
	csr hpmcounter8
	csr hpmcounter9
	csr hpmcounter10
	csr hpmcounter11
	csr hpmcounter12
	csr hpmcounter13
	csr hpmcounter14
	csr hpmcounter15
	csr hpmcounter16
	csr hpmcounter17
	csr hpmcounter18
	csr hpmcounter19
	csr hpmcounter20
	csr hpmcounter21
	csr hpmcounter22
	csr hpmcounter23
	csr hpmcounter24
	csr hpmcounter25
	csr hpmcounter26
	csr hpmcounter27
	csr hpmcounter28
	csr hpmcounter29
	csr hpmcounter30
	csr hpmcounter31
	csr cycleh
	csr timeh
	csr instreth
	csr hpmcounter3h
	csr hpmcounter4h
	csr hpmcounter5h
	csr hpmcounter6h
	csr hpmcounter7h
	csr hpmcounter8h
	csr hpmcounter9h
	csr hpmcounter10h
	csr hpmcounter11h
	csr hpmcounter12h
	csr hpmcounter13h
	csr hpmcounter14h
	csr hpmcounter15h
	csr hpmcounter16h
	csr hpmcounter17h
	csr hpmcounter18h
	csr hpmcounter19h
	csr hpmcounter20h
	csr hpmcounter21h
	csr hpmcounter22h
	csr hpmcounter23h
	csr hpmcounter24h
	csr hpmcounter25h
	csr hpmcounter26h
	csr hpmcounter27h
	csr hpmcounter28h
	csr hpmcounter29h
	csr hpmcounter30h
	csr hpmcounter31h

	# Supervisor Trap Setup
	csr sstatus
	csr sie
	csr stvec
	csr scounteren		# Added in 1.10

	# Supervisor Configuration
	csr senvcfg		# Added in 1.12

	# Supervisor Trap Handling
	csr sscratch
	csr sepc
	csr scause
	csr stval		# Added in 1.10
	csr sip

	# Supervisor Protection and Translation
	csr satp		# Added in 1.10

	# Machine Information Registers
	csr mvendorid
	csr marchid
	csr mimpid
	csr mhartid
	csr mconfigptr		# Added in 1.12

	# Machine Trap Setup
	csr mstatus
	csr misa
	csr medeleg
	csr mideleg
	csr mie
	csr mtvec
	csr mcounteren		# Added in 1.10
	csr mstatush		# Added in 1.12

	# Machine Trap Handling
	csr mscratch
	csr mepc
	csr mcause
	csr mtval		# Added in 1.10
	csr mip
	csr mtinst		# Added in 1.12
	csr mtval2		# Added in 1.12

	# Machine Configuration
	csr menvcfg		# Added in 1.12
	csr menvcfgh		# Added in 1.12
	csr mseccfg		# Added in 1.12
	csr mseccfgh		# Added in 1.12

	# Machine Memory Protection
	csr pmpcfg0		# Added in 1.10
	csr pmpcfg1		# Added in 1.10
	csr pmpcfg2		# Added in 1.10
	csr pmpcfg3		# Added in 1.10
	csr pmpcfg4		# Added in 1.12
	csr pmpcfg5		# Added in 1.12
	csr pmpcfg6		# Added in 1.12
	csr pmpcfg7		# Added in 1.12
	csr pmpcfg8		# Added in 1.12
	csr pmpcfg9		# Added in 1.12
	csr pmpcfg10		# Added in 1.12
	csr pmpcfg11		# Added in 1.12
	csr pmpcfg12		# Added in 1.12
	csr pmpcfg13		# Added in 1.12
	csr pmpcfg14		# Added in 1.12
	csr pmpcfg15		# Added in 1.12
	csr pmpaddr0		# Added in 1.10
	csr pmpaddr1		# Added in 1.10
	csr pmpaddr2		# Added in 1.10
	csr pmpaddr3		# Added in 1.10
	csr pmpaddr4		# Added in 1.10
	csr pmpaddr5		# Added in 1.10
	csr pmpaddr6		# Added in 1.10
	csr pmpaddr7		# Added in 1.10
	csr pmpaddr8		# Added in 1.10
	csr pmpaddr9		# Added in 1.10
	csr pmpaddr10		# Added in 1.10
	csr pmpaddr11		# Added in 1.10
	csr pmpaddr12		# Added in 1.10
	csr pmpaddr13		# Added in 1.10
	csr pmpaddr14		# Added in 1.10
	csr pmpaddr15		# Added in 1.10
	csr pmpaddr16		# Added in 1.12
	csr pmpaddr17		# Added in 1.12
	csr pmpaddr18		# Added in 1.12
	csr pmpaddr19		# Added in 1.12
	csr pmpaddr20		# Added in 1.12
	csr pmpaddr21		# Added in 1.12
	csr pmpaddr22		# Added in 1.12
	csr pmpaddr23		# Added in 1.12
	csr pmpaddr24		# Added in 1.12
	csr pmpaddr25		# Added in 1.12
	csr pmpaddr26		# Added in 1.12
	csr pmpaddr27		# Added in 1.12
	csr pmpaddr28		# Added in 1.12
	csr pmpaddr29		# Added in 1.12
	csr pmpaddr30		# Added in 1.12
	csr pmpaddr31		# Added in 1.12
	csr pmpaddr32		# Added in 1.12
	csr pmpaddr33		# Added in 1.12
	csr pmpaddr34		# Added in 1.12
	csr pmpaddr35		# Added in 1.12
	csr pmpaddr36		# Added in 1.12
	csr pmpaddr37		# Added in 1.12
	csr pmpaddr38		# Added in 1.12
	csr pmpaddr39		# Added in 1.12
	csr pmpaddr40		# Added in 1.12
	csr pmpaddr41		# Added in 1.12
	csr pmpaddr42		# Added in 1.12
	csr pmpaddr43		# Added in 1.12
	csr pmpaddr44		# Added in 1.12
	csr pmpaddr45		# Added in 1.12
	csr pmpaddr46		# Added in 1.12
	csr pmpaddr47		# Added in 1.12
	csr pmpaddr48		# Added in 1.12
	csr pmpaddr49		# Added in 1.12
	csr pmpaddr50		# Added in 1.12
	csr pmpaddr51		# Added in 1.12
	csr pmpaddr52		# Added in 1.12
	csr pmpaddr53		# Added in 1.12
	csr pmpaddr54		# Added in 1.12
	csr pmpaddr55		# Added in 1.12
	csr pmpaddr56		# Added in 1.12
	csr pmpaddr57		# Added in 1.12
	csr pmpaddr58		# Added in 1.12
	csr pmpaddr59		# Added in 1.12
	csr pmpaddr60		# Added in 1.12
	csr pmpaddr61		# Added in 1.12
	csr pmpaddr62		# Added in 1.12
	csr pmpaddr63		# Added in 1.12

	# Machine Counter/Timer
	csr mcycle
	csr minstret
	csr mhpmcounter3
	csr mhpmcounter4
	csr mhpmcounter5
	csr mhpmcounter6
	csr mhpmcounter7
	csr mhpmcounter8
	csr mhpmcounter9
	csr mhpmcounter10
	csr mhpmcounter11
	csr mhpmcounter12
	csr mhpmcounter13
	csr mhpmcounter14
	csr mhpmcounter15
	csr mhpmcounter16
	csr mhpmcounter17
	csr mhpmcounter18
	csr mhpmcounter19
	csr mhpmcounter20
	csr mhpmcounter21
	csr mhpmcounter22
	csr mhpmcounter23
	csr mhpmcounter24
	csr mhpmcounter25
	csr mhpmcounter26
	csr mhpmcounter27
	csr mhpmcounter28
	csr mhpmcounter29
	csr mhpmcounter30
	csr mhpmcounter31
	csr mcycleh
	csr minstreth
	csr mhpmcounter3h
	csr mhpmcounter4h
	csr mhpmcounter5h
	csr mhpmcounter6h
	csr mhpmcounter7h
	csr mhpmcounter8h
	csr mhpmcounter9h
	csr mhpmcounter10h
	csr mhpmcounter11h
	csr mhpmcounter12h
	csr mhpmcounter13h
	csr mhpmcounter14h
	csr mhpmcounter15h
	csr mhpmcounter16h
	csr mhpmcounter17h
	csr mhpmcounter18h
	csr mhpmcounter19h
	csr mhpmcounter20h
	csr mhpmcounter21h
	csr mhpmcounter22h
	csr mhpmcounter23h
	csr mhpmcounter24h
	csr mhpmcounter25h
	csr mhpmcounter26h
	csr mhpmcounter27h
	csr mhpmcounter28h
	csr mhpmcounter29h
	csr mhpmcounter30h
	csr mhpmcounter31h

	# Machine Counter Setup
	csr mcountinhibit	# Added in 1.11
	csr mhpmevent3
	csr mhpmevent4
	csr mhpmevent5
	csr mhpmevent6
	csr mhpmevent7
	csr mhpmevent8
	csr mhpmevent9
	csr mhpmevent10
	csr mhpmevent11
	csr mhpmevent12
	csr mhpmevent13
	csr mhpmevent14
	csr mhpmevent15
	csr mhpmevent16
	csr mhpmevent17
	csr mhpmevent18
	csr mhpmevent19
	csr mhpmevent20
	csr mhpmevent21
	csr mhpmevent22
	csr mhpmevent23
	csr mhpmevent24
	csr mhpmevent25
	csr mhpmevent26
	csr mhpmevent27
	csr mhpmevent28
	csr mhpmevent29
	csr mhpmevent30
	csr mhpmevent31

	# Hypervisor Trap Setup
	csr hstatus
	csr hedeleg
	csr hideleg
	csr hie
	csr hcounteren
	csr hgeie

	# Hypervisor Trap Handling
	csr htval
	csr hip
	csr hvip
	csr htinst
	csr hgeip

	# Hypervisor Configuration
	csr henvcfg
	csr henvcfgh

	# Hypervisor Protection and Translation
	csr hgatp

	# Hypervisor Counter/Timer Virtualization Registers
	csr htimedelta
	csr htimedeltah

	# Virtual Supervisor Registers
	csr vsstatus
	csr vsie
	csr vstvec
	csr vsscratch
	csr vsepc
	csr vscause
	csr vstval
	csr vsip
	csr vsatp

	# Smaia
	csr miselect
	csr mireg
	csr mtopei
	csr mtopi
	csr mvien
	csr mvip
	csr midelegh
	csr mieh
	csr mvienh
	csr mviph
	csr miph

	# Smstateen/Ssstateen extensions
	csr mstateen0
	csr mstateen1
	csr mstateen2
	csr mstateen3
	csr sstateen0
	csr sstateen1
	csr sstateen2
	csr sstateen3
	csr hstateen0
	csr hstateen1
	csr hstateen2
	csr hstateen3
	csr mstateen0h
	csr mstateen1h
	csr mstateen2h
	csr mstateen3h
	csr hstateen0h
	csr hstateen1h
	csr hstateen2h
	csr hstateen3h

	# Ssaia
	csr siselect
	csr sireg
	csr stopei
	csr stopi
	csr sieh
	csr siph
	csr hvien
	csr hvictl
	csr hviprio1
	csr hviprio2
	csr vsiselect
	csr vsireg
	csr vstopei
	csr vstopi
	csr hidelegh
	csr hvienh
	csr hviph
	csr hviprio1h
	csr hviprio2h
	csr vsieh
	csr vsiph

	# Sscofpmf extension
	csr scountovf
	csr mhpmevent3h
	csr mhpmevent4h
	csr mhpmevent5h
	csr mhpmevent6h
	csr mhpmevent7h
	csr mhpmevent8h
	csr mhpmevent9h
	csr mhpmevent10h
	csr mhpmevent11h
	csr mhpmevent12h
	csr mhpmevent13h
	csr mhpmevent14h
	csr mhpmevent15h
	csr mhpmevent16h
	csr mhpmevent17h
	csr mhpmevent18h
	csr mhpmevent19h
	csr mhpmevent20h
	csr mhpmevent21h
	csr mhpmevent22h
	csr mhpmevent23h
	csr mhpmevent24h
	csr mhpmevent25h
	csr mhpmevent26h
	csr mhpmevent27h
	csr mhpmevent28h
	csr mhpmevent29h
	csr mhpmevent30h
	csr mhpmevent31h

	# Sstc extension
	csr stimecmp
	csr stimecmph
	csr vstimecmp
	csr vstimecmph

	# Supported in previous priv spec, but dropped now

	csr ubadaddr		# 0x043 in 1.9.1, but the value is utval since 1.10
	csr sbadaddr		# 0x143 in 1.9.1, but the value is stval since 1.10
	csr sptbr		# 0x180 in 1.9.1, but the value is satp since 1.10
	csr mbadaddr		# 0x343 in 1.9.1, but the value is mtval since 1.10
	csr mucounteren		# 0x320 in 1.9.1, dropped in 1.10, but the value is mcountinhibit since 1.11
	csr mbase		# 0x380 in 1.9.1, dropped in 1.10
	csr mbound		# 0x381 in 1.9.1, dropped in 1.10
	csr mibase		# 0x382 in 1.9.1, dropped in 1.10
	csr mibound		# 0x383 in 1.9.1, dropped in 1.10
	csr mdbase		# 0x384 in 1.9.1, dropped in 1.10
	csr mdbound		# 0x385 in 1.9.1, dropped in 1.10
	csr mscounteren		# 0x321 in 1.9.1, dropped in 1.10
	csr mhcounteren		# 0x322 in 1.9.1, dropped in 1.10
	csr ustatus		# 0x0   in 1.9.1, dropped in 1.12
	csr uie			# 0x4   in 1.9.1, dropped in 1.12
	csr utvec		# 0x5   in 1.9.1, dropped in 1.12
	csr uscratch		# 0x40  in 1.9.1, dropped in 1.12
	csr uepc		# 0x41  in 1.9.1, dropped in 1.12
	csr ucause		# 0x42  in 1.9.1, dropped in 1.12
	csr utval		# 0x43  in 1.10,  dropped in 1.12
	csr uip			# 0x44  in 1.9.1, dropped in 1.12
	csr sedeleg		# 0x102 in 1.9.1, dropped in 1.12
	csr sideleg		# 0x103 in 1.9.1, dropped in 1.12

	# Unprivileged CSR which are not controlled by privilege spec

	# Float
	csr fflags
	csr frm
	csr fcsr

	# Core debug
	csr dcsr
	csr dpc
	csr dscratch0
	csr dscratch1
	csr dscratch		# 0x7b2, alias to dscratch0

	# Trigger debug
	csr tselect
	csr tdata1
	csr tdata2
	csr tdata3
	csr tinfo
	csr tcontrol
	csr hcontext
	csr scontext
	csr mcontext
	csr mscontext
	csr mcontrol		# 0x7a1, alias to tdata1
	csr mcontrol6		# 0x7a1, alias to tdata1
	csr icount		# 0x7a1, alias to tdata1
	csr itrigger		# 0x7a1, alias to tdata1
	csr etrigger		# 0x7a1, alias to tdata1
	csr tmexttrigger	# 0x7a1, alias to tdata1
	csr textra32		# 0x7a3, alias to tdata3
	csr textra64		# 0x7a3, alias to tdata3

	# Scalar crypto
	csr seed		# 0x015, Entropy Source

	# Vector
	csr vstart
	csr vxsat
	csr vxrm
	csr vcsr
	csr vl
	csr vtype
	csr vlenb
