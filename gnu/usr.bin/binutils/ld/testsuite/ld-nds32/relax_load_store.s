.data
.global myword
myword:
	.word 0x11
.globl myshort
myshort:
	.short 0x11
.globl mybyte
mybyte:
	.byte 0x11

.text
.global	_start
_start:
	l.w $r0, myword
	l.h $r0, myshort
	l.b $r0, mybyte
