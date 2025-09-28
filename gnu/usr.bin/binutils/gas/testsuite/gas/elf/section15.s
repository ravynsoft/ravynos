	.section .bar,"a",unique,0
	.byte 0
 .pushsection .bar,2,"a",unique,1
	.byte 2
 .popsection
	.byte 0
 .pushsection .bar,3,"a",unique,2
	.byte 2
 .popsection
	.byte 0
 .pushsection .bar,2,"a", %progbits,unique,3
	.byte 3
 .popsection
	.byte 0
 .pushsection .bar,"",unique,4
	.byte 4
 .popsection
	.byte 0
 .pushsection .text,1,"axG",%progbits,foo,comdat,unique,0xffffffff
	.byte -1
 .popsection
	.byte 0
 .pushsection .text,"axG",%progbits,foo,comdat,unique,0xffffffff
	.byte -2
 .popsection
	.byte 0
 .pushsection .bar,"a",unique,1
	.byte 1
 .popsection
	.byte 0
 .pushsection .bar,"a", %progbits,unique,3
	.byte 1
 .popsection
	.byte 0
 .pushsection .bar,"a",unique,2
	.byte 1
 .popsection
	.byte 0
