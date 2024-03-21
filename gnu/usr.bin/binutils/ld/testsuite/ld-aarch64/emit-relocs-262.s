.comm   gempy,4,4
.text

.word	_GOT_ - .
.xword  _GOT_ - . + 0x12
.hword	_GOT_ - . 
.hword	_GOT_ - . 

	and	x0,x0,x0
	and	x0,x0,#0x1



