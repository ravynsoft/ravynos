	.text
func:
	# Scalar base register out of bound
	vfmal.f16	d2, s0, s16[1]
	vfmsl.f16	q13, d1, d8[3]
	# Scalar index out of bound
	vfmal.f16	d16, s2, s15[2]
	vfmsl.f16	q13, d1, d7[4]
