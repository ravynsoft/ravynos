	.text
	.global test_dot_req_and_unreq
test_dot_req_and_unreq:

	# Check that builtin register alias 'r0' works.
	add r0, r0, r0

	# Create an alias for r0.
	foo .req r0

	# Check that it works.
	add foo, foo, foo

	# Now remove the alias.
        .unreq foo

	# And make sure that it no longer works.
	add foo, foo, foo

	# Attempt to remove the builtin alias for r0.
        .unreq r0
	
	# That is ignored, so this should still work.
	add r0, r0, r0

	# Now attempt to re-alias foo.  There used to be a bug whereby the
	# first creation of an alias called foo would also create an alias
	# called FOO, but the .unreq of foo would not delete FOO.  Thus a
	# second attempt at aliasing foo (to something different than
	# before) would fail because the assembler would complain that FOO
	# already existed.
	foo .req r1

	add foo, foo, foo

	# Check that the upper case alias was also recreated.
	add FOO, FOO, FOO

	# Check that a second attempt to alias foo, using a mixed case
	# version of the name, will fail.
	Foo .req r2
