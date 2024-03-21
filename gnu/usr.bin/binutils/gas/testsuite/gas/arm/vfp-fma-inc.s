	.syntax unified
	.arch armv7-a
	.fpu neon-vfpv4

	.include "itblock.s"

func:
	.macro dyadic op cond="" f32=".f32" f64=".f64"
	itblock 2 \cond
	\op\cond\f32 s0,s1,s2
	\op\cond\f64 d0,d1,d2
	.endm

	.macro dyadic_c op
	dyadic \op
	dyadic \op eq
	.endm

	dyadic_c vfma
	dyadic_c vfms
	dyadic_c vfnma
	dyadic_c vfnms
