
.equiv  SRC,     1800h
.equiv  DST,     1880h

	mov     &SRC(R13), &DST(R13)
#	mov     &SRC, &DST
#	mov     &1800h(R13), &DST(R13)
#	mov     &1800h(R13), &1800h(R13)

