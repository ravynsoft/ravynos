
.global _start
_start:
	lui   a0, %hi(gdata)
	addi  a0, a0, %lo(gdata)
	call func
	j .
	.size _start, . - _start

.global func
.align 7
func:
	ret
	.size func, . - func

.data
padding:
	.long 0
	.long 0
	.long 0
	.long 0
	.size padding, . - padding

.global gdata
.type gdata, object
gdata:
	.zero 4
	.size gdata, . - gdata
