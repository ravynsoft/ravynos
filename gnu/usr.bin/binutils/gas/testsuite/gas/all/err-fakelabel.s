;# Test that fake labels aren't accepted.
;# { dg-do assemble }
label:			;# { dg-error "Error: |Fatal error:" }
