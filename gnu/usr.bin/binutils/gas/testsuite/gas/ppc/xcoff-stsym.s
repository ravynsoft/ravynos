	.file	"main.c"
	.csect _main.rw_[RW],4

	.csect .text[PR]
	.bs	_main.rw_[RW]
	.stabx	"x:V6",x.2,133,0
	.es
	.bs	_main.rw_[RW]
	.stabx	"y:V6",y.1,133,0
	.es

	.csect _main.rw_[RW],4
x.2:
	.long	100
y.1:
	.long	110
