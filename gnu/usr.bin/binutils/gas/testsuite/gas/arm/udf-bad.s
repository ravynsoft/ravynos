	.syntax unified

arm:	.arm
	udf	#0x10000

thumb:	.thumb
	udf	#0x10000
	udf.w	#0x10000
	udf.n	#0x100
