; Check that --pic isn't recognized for a.out files, specified by emulation.

; { dg-do assemble { target cris-*-* } }
; { dg-options "--pic --em=crisaout" }
; { dg-error ".* --pic is invalid" "" { target cris-*-* } 0 }
 nop
