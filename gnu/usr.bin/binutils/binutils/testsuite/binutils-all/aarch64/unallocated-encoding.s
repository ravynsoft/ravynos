	.text
func:
	//scale 1, size<0> check for H.
	#st1	{v30.h}[0], [x30]
	.inst 0x0d0043de | (1 << 10)
	#st2	{v29.h, v30.h}[0], [x30]
	.inst 0x0d2043dd | (1 << 10)
	#st3	{v28.h, v29.h, v30.h}[0], [x30]
	.inst 0x0d0063dc | (1 << 10)
	#st4	{v27.h, v28.h, v29.h, v30.h}[0], [x30]
	.inst 0x0d2063db | (1 << 10)

	//scale 2, size<1> check for S.
	#st1	{v30.s}[0], [x30]
	.inst 0x0d0083de | (1 << 11)
	#st2	{v29.s, v30.s}[0], [x30]
	.inst 0x0d2083dd | (1 << 11)
	#st3	{v28.s, v29.s, v30.s}[0], [x30]
	.inst 0x0d00a3dc | (1 << 11)
	#st4	{v27.s, v28.s, v29.s, v30.s}[0], [x30]
	.inst 0x0d20a3db | (1 << 11)

	//scale 2, size<1> check for D.
	#st1	{v30.d}[0], [x30]
	.inst 0x0d0087de | (1 << 11)
	#st2	{v29.d, v30.d}[0], [x30]
	.inst 0x0d2087dd | (1 << 11)
	#st3	{v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d00a7dc | (1 << 11)
	#st4	{v27.d, v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d20a7db | (1 << 11)

	//scale 2, S-bit check for D.
	#st1	{v30.d}[0], [x30]
	.inst 0x0d0087de | (2 << 11)
	#st2	{v29.d, v30.d}[0], [x30]
	.inst 0x0d2087dd | (2 << 11)
	#st3	{v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d00a7dc | (2 << 11)
	#st4	{v27.d, v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d20a7db | (2 << 11)

	//scale 2, size<1> & S-bit check for D.
	#st1	{v30.d}[0], [x30]
	.inst 0x0d0087de | (3 << 11)
	#st2	{v29.d, v30.d}[0], [x30]
	.inst 0x0d2087dd | (3 << 11)
	#st3	{v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d00a7dc | (3 << 11)
	#st4	{v27.d, v28.d, v29.d, v30.d}[0], [x30]
	.inst 0x0d20a7db | (3 << 11)
