;;; Check if assembler can discriminate between labels having the same
;;; name as a register

r0:
	mov	r0,@r1-@r0
	add	r0,r0,@gp
	st 	r2,[gp,@r1@sda]
r1:
