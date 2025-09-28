; Test command line option compatibility checking.
; { dg-do assemble }
; { dg-options "--mcpu=archs -mdpfp" }
; { dg-error ".* invalid double-precision FPX option for archs cpu" "" { target arc*-*-* } 0 }
	nop
