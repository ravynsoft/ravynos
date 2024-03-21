	.section ".tdata", "awT", @progbits
	.global sg1, sg2, sg3, sg4, sg5, sg6, sg7, sg8
	.global sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8
	.hidden sh1, sh2, sh3, sh4, sh5, sh6, sh7, sh8
	.align	4
sg1:	.long	17
sg2:	.long	18
sg3:	.long	19
sg4:	.long	20
sg5:	.long	21
sg6:	.long	22
sg7:	.long	23
sg8:	.long	24
sl1:	.long	65
sl2:	.long	66
sl3:	.long	67
sl4:	.long	68
sl5:	.long	69
sl6:	.long	70
sl7:	.long	71
sl8:	.long	72
sh1:	.long	157
sh2:	.long	158
sh3:	.long	159
sh4:	.long	160
sh5:	.long	161
sh6:	.long	162
sh7:	.long	163
sh8:	.long	164

	.text
	.global	_start
	.type	_start, @function
_start:
	entry	sp, 32

	/* GD -> IE because variable is not defined in executable */
	movi	a8, sG1@tlsfunc
	movi	a10, sG1@tlsarg
	callx8.tls a8, sG1@tlscall
	
	/* GD -> IE because variable is not defined in executable where
	   the variable is referenced through IE too */
	movi	a8, sG2@tlsfunc
	movi	a10, sG2@tlsarg
	callx8.tls a8, sG2@tlscall

	/* GD -> LE with global variable defined in executable */
	movi	a8, sg1@tlsfunc
	movi	a10, sg1@tlsarg
	callx8.tls a8, sg1@tlscall

	/* GD -> LE with local variable defined in executable */
	movi	a8, sl1@tlsfunc
	movi	a10, sl1@tlsarg
	callx8.tls a8, sl1@tlscall

	/* GD -> LE with hidden variable defined in executable */
	movi	a8, sh1@tlsfunc
	movi	a10, sh1@tlsarg
	callx8.tls a8, sh1@tlscall

	/* LD -> LE */
	movi   a8, _TLS_MODULE_BASE_@tlsfunc
	movi   a10, _TLS_MODULE_BASE_@tlsarg
	callx8.tls a8, _TLS_MODULE_BASE_@tlscall
	movi   a12, 1+sl1@dtpoff
	add    a12, a12, a10
	movi   a13, sl2@dtpoff+2
	add    a13, a13, a10
	
	/* LD -> LE against hidden variables */
	movi   a12, sh1@dtpoff
	add    a12, a12, a10
	movi   a13, 3+sh2@dtpoff
	add    a13, a13, a10

	/* IE against global var */
	rur	a2, THREADPTR
	movi	a3, sG2@tpoff
	add	a3, a3, a2

	/* IE -> LE against global var defined in exec */
	rur	a4, THREADPTR
	movi	a5, sg1@tpoff
	add	a5, a5, a4

	/* IE -> LE against local var */
	rur	a6, THREADPTR
	movi	a7, sl1@tpoff
	add	a7, a7, a6

	/* IE -> LE against hidden var */
	rur	a8, THREADPTR
	movi	a9, sh1@tpoff
	add	a9, a9, a8

	retw
