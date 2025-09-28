;;; All these are invalid instructions and should provoke an error
	st d0, #2
	st s,  #4
	mov.b d0, #4
	inc.b #1
	dec.b #12
	com.w  #1
	neg.l  #-1
	ror.b  #3
