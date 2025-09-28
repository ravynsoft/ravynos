	.text
	.global _start
	.global _mainCRTStartup
_start:
_mainCRTStartup:
	.byte 1
	.global data
	.data
data:
	.byte 2

	.section .text.very.long.section.name,"rx"
vls:
	.byte 3

	.section .data$1,"wd"
	.byte 4
	.section .rodata$1,"rd"
	.byte 5

	.section .data$123,"wd"
	.byte 4
	.section .rodata$123,"rd"
	.byte 5
	.section .data$123456789,"wd"
	.byte 4
	.section .rodata$123456789,"rd"
	.byte 5

	.section .data.very.long.section,"wd"
	.byte 6
	.section .rodata.very.long.section,"rd"
	.byte 7

	.section .data.very.long.section$1,"wd"
	.byte 6
	.section .rodata.very.long.section$1,"rd"
	.byte 7
	.section .data.very.long.section$1234,"wd"
	.byte 6
	.section .rodata.very.long.section$1234,"rd"
	.byte 7

	.end
