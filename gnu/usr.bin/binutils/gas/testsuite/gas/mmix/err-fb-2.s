# { dg-do assemble { target mmix-*-* } }
# Mostly like fb-2.s, but with LOCs to indeterminable sections
# *different* to the one LOC'd from.  Two cases are tested: an
# undefined symbol and a section just indeterminable at the first pass
# (at the point of the LOC).

1H IS 5
0H LOC #10
1H BYTE 1B
0H LOC 2F+#20+0F 		# { dg-error "indeterminable" "" }
0H IS 4
   .section .text.unknown
2H SWYM 
1H IS 50
1H GREG 1B+1F
 SWYM
1H LDA $30,1B
1H OCTA 1B,1F
   LOC undefd 			# { dg-error "unknown" "" }
1H SWYM

9H IS 42
 WYDE 9B,9F
9H IS 9B+1
 WYDE 9B,9F
9H IS 9B+1
