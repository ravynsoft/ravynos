# Test VIS EDGE instructions
	.text
	edge8cc		%g1, %g2, %g3
	edge8		%g1, %g2, %g3
	edge8n		%g1, %g2, %g3
	edge8lcc	%g2, %g3, %g4
	edge8l		%g2, %g3, %g4
	edge8ln 	%g2, %g3, %g4
	edge8cc		%l4, %g2, %g1
	edge8		%l4, %g2, %g1
	edge8n		%l4, %g2, %g1
	edge8lcc	%g2, %g1, %l4
	edge8l		%g2, %g1, %l4
	edge8ln 	%g2, %g1, %l4
	edge32cc	%o5, %o4, %o2
	edge32		%o5, %o4, %o2
	edge32n		%o5, %o4, %o2
	edge32lcc	%o2, %g5, %l1
	edge32l		%o2, %g5, %l1
	edge32ln 	%o2, %g5, %l1
