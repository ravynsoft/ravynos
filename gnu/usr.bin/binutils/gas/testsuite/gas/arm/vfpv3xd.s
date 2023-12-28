# Check double precision load/store are allowed on single precision
# implementation

	fldd	d0, [r0]
	fstd	d0, [r0]

	fldmiad	r0, {d0}
	fldmfdd	r0, {d0}
	fldmiad	r0!, {d0}
	fldmfdd	r0!, {d0}
	fldmdbd	r0!, {d0}
	fldmead	r0!, {d0}

	fstmiad	r0, {d0}
	fstmead	r0, {d0}
	fstmiad	r0!, {d0}
	fstmead	r0!, {d0}
	fstmdbd	r0!, {d0}
	fstmfdd	r0!, {d0}
