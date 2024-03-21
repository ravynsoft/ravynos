# Test bad uses of $DSBT_INDEX.  Uses with addend.
.text
.nocmp
.globl f
f:
	addaw .D1X b14,$dsbt_index(__c6xabi_DSBT_BASE),a5
	addaw .D1X b14,$dsbt_index(__c6xabi_DSBT_BASE)+0,a5
	addaw .D1X b14,$dsbt_index(__c6xabi_DSBT_BASE)+4,a5
