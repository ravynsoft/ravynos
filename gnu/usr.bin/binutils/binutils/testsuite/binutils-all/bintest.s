	.globl text_symbol
	.text
text_symbol:	
static_text_symbol:
	.long	1
	.long	external_symbol
	.globl data_symbol
	.data
data_symbol:
static_data_symbol:
	.long	2
	.comm common_symbol,4
	.text
	.global text_symbol2
text_symbol2:
	.long	2
	.global text_symbol3
text_symbol3:
	.long 3
	
