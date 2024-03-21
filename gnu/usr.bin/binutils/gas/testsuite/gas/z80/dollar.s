	.data
	.org	0x10
_label1:
	.dw	100$
100$:
	.dw	100$
_label2:
	.dw	110$
100$:
	.dw	100$
110$:
	.dw	110$
.L_label3:
	.dw	100$
	.dw	110$
	.end

