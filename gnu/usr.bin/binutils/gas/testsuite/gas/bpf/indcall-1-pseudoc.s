
    .text
    .align 4
main:
	r0 = 1
	r1 = 1
	r2 = 2
	r6 = bar ll
	callx r6
	exit
bar:
	r0 = 0
	exit
