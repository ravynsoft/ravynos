# Source file used to test the doubleword memory access macros
# (ld and friends).

# By default test ld.
	.set	r4, $4

# If defined, test sd instead.
	.ifdef	tsd
	.macro	ld ops:vararg
	sd	\ops
	.endm
	.endif
# If defined, test l.d instead.
	.ifdef	tl_d
	.set	r4, $f4
	.macro	ld ops:vararg
	l.d	\ops
	.endm
	.endif
# If defined, test s.d instead.
	.ifdef	ts_d
	.set	r4, $f4
	.macro	ld ops:vararg
	s.d	\ops
	.endm
	.endif
# If defined, test ldc1 instead.
	.ifdef	tldc1
	.set	r4, $f4
	.macro	ld ops:vararg
	ldc1	\ops
	.endm
	.endif
# If defined, test sdc1 instead.
	.ifdef	tsdc1
	.set	r4, $f4
	.macro	ld ops:vararg
	sdc1	\ops
	.endm
	.endif

	.macro	data
	.bss
	.align	12
	.sbss
	.align	12
	.data
	.align	12
data_label:
	.extern big_external_data_label,1000
	.extern small_external_data_label,1
	.comm big_external_common,1000
	.comm small_external_common,1
	.lcomm big_local_common,1000
	.lcomm small_local_common,1
	.endm

	.ifndef	forward
	data
	.endif

	.text
	.align	12
text_label:
	ld	r4,0
	ld	r4,1
	ld	r4,0x8000
	ld	r4,-0x8000
	ld	r4,0x10000
	ld	r4,0x1a5a5
	ld	r4,0($5)
	ld	r4,1($5)
	ld	r4,0x8000($5)
	ld	r4,-0x8000($5)
	ld	r4,0x10000($5)
	ld	r4,0x1a5a5($5)
	ld	r4,data_label
	ld	r4,big_external_data_label
	ld	r4,small_external_data_label
	ld	r4,big_external_common
	ld	r4,small_external_common
	ld	r4,big_local_common
	ld	r4,small_local_common
	ld	r4,data_label+1
	ld	r4,big_external_data_label+1
	ld	r4,small_external_data_label+1
	ld	r4,big_external_common+1
	ld	r4,small_external_common+1
	ld	r4,big_local_common+1
	ld	r4,small_local_common+1
	ld	r4,data_label+0x8000
	ld	r4,big_external_data_label+0x8000
	ld	r4,small_external_data_label+0x8000
	ld	r4,big_external_common+0x8000
	ld	r4,small_external_common+0x8000
	ld	r4,big_local_common+0x8000
	ld	r4,small_local_common+0x8000
	ld	r4,data_label-0x8000
	ld	r4,big_external_data_label-0x8000
	ld	r4,small_external_data_label-0x8000
	ld	r4,big_external_common-0x8000
	ld	r4,small_external_common-0x8000
	ld	r4,big_local_common-0x8000
	ld	r4,small_local_common-0x8000
	ld	r4,data_label+0x10000
	ld	r4,big_external_data_label+0x10000
	ld	r4,small_external_data_label+0x10000
	ld	r4,big_external_common+0x10000
	ld	r4,small_external_common+0x10000
	ld	r4,big_local_common+0x10000
	ld	r4,small_local_common+0x10000
	ld	r4,data_label+0x1a5a5
	ld	r4,big_external_data_label+0x1a5a5
	ld	r4,small_external_data_label+0x1a5a5
	ld	r4,big_external_common+0x1a5a5
	ld	r4,small_external_common+0x1a5a5
	ld	r4,big_local_common+0x1a5a5
	ld	r4,small_local_common+0x1a5a5
	ld	r4,data_label($5)
	ld	r4,big_external_data_label($5)
	ld	r4,small_external_data_label($5)
	ld	r4,big_external_common($5)
	ld	r4,small_external_common($5)
	ld	r4,big_local_common($5)
	ld	r4,small_local_common($5)
	ld	r4,data_label+1($5)
	ld	r4,big_external_data_label+1($5)
	ld	r4,small_external_data_label+1($5)
	ld	r4,big_external_common+1($5)
	ld	r4,small_external_common+1($5)
	ld	r4,big_local_common+1($5)
	ld	r4,small_local_common+1($5)
	ld	r4,data_label+0x8000($5)
	ld	r4,big_external_data_label+0x8000($5)
	ld	r4,small_external_data_label+0x8000($5)
	ld	r4,big_external_common+0x8000($5)
	ld	r4,small_external_common+0x8000($5)
	ld	r4,big_local_common+0x8000($5)
	ld	r4,small_local_common+0x8000($5)
	ld	r4,data_label-0x8000($5)
	ld	r4,big_external_data_label-0x8000($5)
	ld	r4,small_external_data_label-0x8000($5)
	ld	r4,big_external_common-0x8000($5)
	ld	r4,small_external_common-0x8000($5)
	ld	r4,big_local_common-0x8000($5)
	ld	r4,small_local_common-0x8000($5)
	ld	r4,data_label+0x10000($5)
	ld	r4,big_external_data_label+0x10000($5)
	ld	r4,small_external_data_label+0x10000($5)
	ld	r4,big_external_common+0x10000($5)
	ld	r4,small_external_common+0x10000($5)
	ld	r4,big_local_common+0x10000($5)
	ld	r4,small_local_common+0x10000($5)
	ld	r4,data_label+0x1a5a5($5)
	ld	r4,big_external_data_label+0x1a5a5($5)
	ld	r4,small_external_data_label+0x1a5a5($5)
	ld	r4,big_external_common+0x1a5a5($5)
	ld	r4,small_external_common+0x1a5a5($5)
	ld	r4,big_local_common+0x1a5a5($5)
	ld	r4,small_local_common+0x1a5a5($5)

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align	2
	.space	8

	.ifdef	forward
	data
	.endif
