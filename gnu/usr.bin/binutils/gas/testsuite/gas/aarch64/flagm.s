/* FLAGM (Condition flag manipulation) feature from Armv8.4-A.  */
.arch armv8.4-a

	cfinv
	rmif    x30, #8, #15
	setf8   w0
	setf16  w0


/* FLAGM feature enabled with +flagm.  */
.arch armv8-a+flagm

	cfinv
	rmif    x30, #8, #15
	setf8   w0
	setf16  w0
