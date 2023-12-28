# Test for pc-relative relocations
.set norelax
.text
.global _start
_start:
	br ext_label
	br ext_label + 16

