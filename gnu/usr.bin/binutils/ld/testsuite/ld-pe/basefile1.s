.globl _start
.globl start
.text
_start:
start:
	.long 0

.globl _d1
.globl _d2
.globl _d3
.data
_d1:
	.long 1
_d2:
	.secrel32 _d3
_d3:
	.long 2

