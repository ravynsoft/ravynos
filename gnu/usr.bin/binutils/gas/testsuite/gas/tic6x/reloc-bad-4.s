# Test bad uses of $DSBT_INDEX.  Use with a bad symbol
.globl a
.text
.nocmp
.globl f
f:
	addaw .D1X b14,$dsbt_index(a),a5
