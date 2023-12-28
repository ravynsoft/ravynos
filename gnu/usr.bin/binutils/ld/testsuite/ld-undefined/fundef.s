	.text
	.type undef_fun_typed %function
 .ifdef BL
	bl undef_fun_typed
	nop
	bl undef_fun_notype
	nop
 .endif
 .ifdef BLPLT
	bl undef_fun_typed@plt
	nop
	bl undef_fun_notype@plt
	nop
 .endif
 .ifdef CALLPLT
	call undef_fun_typed@plt
	call undef_fun_notype@plt
 .endif
 .ifdef HPPA
	bl undef_fun_typed,%r2
	nop
	bl undef_fun_notype,%r2
	nop
 .endif

	.data
	.type undef_data %object
	.dc.a undef_data
	.type undef_pfun %function
	.dc.a undef_pfun
	.dc.a undef_notype
