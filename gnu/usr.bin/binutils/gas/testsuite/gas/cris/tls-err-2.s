; Like tls-err-1.s but for PIC TLS suffixes.

; { dg-do assemble }
; { dg-options "--pic --no-underscore --em=criself" }

 .syntax no_register_prefix
 .text
start:
 move.b extsym:GDGOTREL16,r4	; { dg-error "PIC relocation size does not match" "" { xfail *-*-* } }
 move.b extsym12:GDGOTREL,r5	; { dg-error "PIC relocation size does not match" }
 move.w extsym2:GDGOTREL,r5	; { dg-error "PIC relocation size does not match" }
 move.d extsym3:GDGOTREL16,r6	; { dg-error "PIC relocation size does not match" }
 move extsym4:GDGOTREL16,srp	; { dg-error "PIC relocation size does not match" }
 move.b extsym5:TPOFFGOT16,r4	; { dg-error "PIC relocation size does not match" "" { xfail *-*-* } }
 move.b extsym15:TPOFFGOT,r7	; { dg-error "PIC relocation size does not match" }
 move.w extsym6:DTPREL,r5	; { dg-error "PIC relocation size does not match" }
 move.d extsym7:DTPREL16,r6	; { dg-error "PIC relocation size does not match" }
 move.d extsym7:TPOFFGOT16,r6	; { dg-error "PIC relocation size does not match" }
 move extsym8:TPOFFGOT16,srp	; { dg-error "PIC relocation size does not match" }
