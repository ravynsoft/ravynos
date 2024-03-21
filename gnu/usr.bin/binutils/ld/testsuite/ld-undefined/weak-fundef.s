	.text
	.weak undef_weak_fun
	.type undef_weak_fun %function
 .ifdef BL
	bl undef_weak_fun
	nop
 .endif
 .ifdef BLPLT
	bl undef_weak_fun@plt
 .endif
 .ifdef CALLPLT
	call undef_weak_fun@plt
 .endif
 .ifdef HPPA
	bl undef_weak_fun,%r2
	nop
 .endif
