; Check that TLS PIC suffixes aren't accepted when non-PIC.

; { dg-do assemble }
; { dg-options "--no-underscore --em=criself" }

 .syntax no_register_prefix
 .text
start:
 move.d extsym2:GDGOTREL,r5	; { dg-error "operand" }
 move.w extsym2:GDGOTREL16,r5	; { dg-error "operand" }
 move.d extsym1:DTPREL,r4	; { dg-error "operand" }
 move.w extsym3:DTPREL16,r6	; { dg-error "operand" }
 move.w extsym13:TPOFFGOT16,r10	; { dg-error "operand" }
 move extsym4:TPOFFGOT,srp	; { dg-error "operand" }
