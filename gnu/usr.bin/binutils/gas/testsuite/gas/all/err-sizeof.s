;# .sizeof. and .startof. operator diagnostics
;# { dg-do assemble }
	.long	.sizeof.(a b)		;# { dg-error "Error: syntax error" }
	.long	.startof.(x y)		;# { dg-error "Error: syntax error" }
	.long	.sizeof.(a+1)		;# { dg-error "Error: syntax error" }
	.long	.startof.(x-1)		;# { dg-error "Error: syntax error" }
	.long	.sizeof.("a+b")
	.long	.startof.("x-y")
	.long	.sizeof.()		;# { dg-error "Error: expected symbol name" }
	.long	.startof.()		;# { dg-error "Error: expected symbol name" }
;# We don't really care about these, but I didn't find a way to discard
;# them, and I also don't want to use dg-excess-errors here.
;# { dg-error "junk at end" "" { target *-*-* } 3 }
;# { dg-error "junk at end" "junk" { target *-*-* } 4 }
;# { dg-error "junk at end" "junk" { target *-*-* } 5 }
;# { dg-error "junk at end" "junk" { target *-*-* } 6 }
;# { dg-warning "zero assumed" "missing" { target *-*-* } 9 }
;# { dg-warning "zero assumed" "missing" { target *-*-* } 10 }
