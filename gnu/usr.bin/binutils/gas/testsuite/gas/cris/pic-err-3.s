; Check that --pic isn't recognized for a.out files, with a.out the default.

; { dg-do assemble { target cris-*-*aout* } }
; { dg-options "--pic" }
; { dg-error ".* --pic is invalid" "" { target cris-*-* } 0 }
 nop
