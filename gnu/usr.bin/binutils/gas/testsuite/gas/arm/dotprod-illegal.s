	.text
func:
	# name and suffix mismatch.
	vudot.s8	d0, d2, d5
	# No .*16 suffix support.
	vudot.u16	d0, d2, d5
	vsdot.s16	d1, d12, d18
	# No .*32 suffix support.
	vudot.u32	d2, d22, d1
	vsdot.s32	d3, d30, d9
	# Scalar base register out of bound
	vudot.u8	d31, d2, d16[0]
	vsdot.s8	q13, q14, d22[1]
	# Scalar index out of bound
	vudot.u8	d1, d8, d15[2]
	vsdot.s8	q14, q7, d15[3]
