.comm   gempy,4,4
.text
	and	x0,x0,x0
	and	x0,x0,#1
	adrp    x2,tempy
	adrp	x7,tempy2
	adrp	x17,tempy3
        adrp	x3,:got:gempy
