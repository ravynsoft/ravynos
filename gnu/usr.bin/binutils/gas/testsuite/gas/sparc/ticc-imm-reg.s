! Make ticc aliases operate as per V8 SPARC Architecture Manual
	.text
foo:
	ta	%o0
	ta	%o0 + %o2
	ta	%l0 +  10
	ta	%l0 + -10
	ta	%l0 -  10
	ta	%l0 - -10
	ta	127
	ta	 10 + %i0
	ta	-10 + %i0
