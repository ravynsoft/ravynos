	.global	abs1
abs1 = 0x1000

	.data
	.global data1
data1:
	.text
	.global text1
text1:
	.global text2
text2 = . + 65536
	.end
