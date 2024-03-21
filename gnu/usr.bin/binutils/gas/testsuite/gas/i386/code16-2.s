	.code16gcc
	.text
	.section	.text.default_process_op.isra.0,"ax",@progbits
	.type	default_process_op.isra.0, @function
default_process_op.isra.0:
	ret
	.section	.text.mpt_scsi_process_op,"ax",@progbits
	.type	mpt_scsi_process_op, @function
mpt_scsi_process_op:
	jmp	default_process_op.isra.0
