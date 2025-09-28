; Test mismatch between cpu option passed by mcpu option and .cpu
; directive option.
; { dg-do assemble }
; { dg-options "--mcpu=arc700" }
 .cpu EM; { dg-warning "Warning: Command-line value overrides \".cpu\" directive" }
