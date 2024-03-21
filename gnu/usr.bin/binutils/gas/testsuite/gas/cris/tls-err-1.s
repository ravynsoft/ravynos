; Like pic-err-1.s but for non-pic TLS suffixes.

; { dg-do assemble }
; { dg-options "--no-underscore --em=criself" }

 .syntax no_register_prefix
 .text
start:
 move.b extsym:TPOFF16,r4	; { dg-error "TLS relocation size does not match" "" { xfail *-*-* } }
 move.b extsym12:TPOFF,r5	; { dg-error "TLS relocation size does not match" }
 move.w extsym2:TPOFF,r5	; { dg-error "TLS relocation size does not match" }
 move.d extsym3:TPOFF16,r6	; { dg-error "TLS relocation size does not match" }
 move extsym4:TPOFF16,srp	; { dg-error "TLS relocation size does not match" }
 move.b extsym15:GD,r7		; { dg-error "TLS relocation size does not match" }
 move.w extsym6:GD,r5		; { dg-error "TLS relocation size does not match" }
 move extsym8:TPOFF16,srp	; { dg-error "TLS relocation size does not match" }
 move.b extsym5:IE,r7		; { dg-error "TLS relocation size does not match" }
 move.w extsym16:IE,r5		; { dg-error "TLS relocation size does not match" }
