	.data
	.globl protected
	.ifndef XCOFF_TEST
	.type protected,"object"
	.endif
protected:
	.globl hidden
	.ifndef XCOFF_TEST
	.type hidden,"object"
	.endif
hidden:
	.globl internal
	.ifndef XCOFF_TEST
	.type internal,"object"
	.endif
internal:
