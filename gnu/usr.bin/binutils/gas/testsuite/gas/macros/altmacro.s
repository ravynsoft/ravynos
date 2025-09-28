# Check use of LOCAL directive inside .altmacro.
# Test derived from PR 11507.

        .altmacro
	
        .macro ABC              
                .print "local "
        .endm

	.macro DEF
		LOCAL fred
		.print "fred"
	.endm

# This one is just being perverse, but it should work.
	.macro GHI
		local local
		.print "local"
	.endm

	
        ABC

	DEF

	GHI
