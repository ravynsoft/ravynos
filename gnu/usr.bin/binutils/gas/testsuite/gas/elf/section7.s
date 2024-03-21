	.section .bar,"a"
	.byte 0
 .pushsection .bar1,2,"a"
	.byte 2
 .popsection
	.byte 0
 .pushsection .bar2,3,"a"
	.byte 2
 .popsection
	.byte 0
 .pushsection .bar3,2,"a", %progbits
	.byte 3
 .popsection
	.byte 0
 .pushsection .bar4
	.byte 4
 .popsection
	.byte 0
 .pushsection        .text,1,"axG",%progbits,foo,comdat
	.byte -1
 .popsection
	.byte 0
 .pushsection        .text,"axG",%progbits,foo,comdat
	.byte -2
 .popsection
	.byte 0
 .pushsection .bar1,"a"
	.byte 1
 .popsection
	.byte 0
 .pushsection .bar3,"a", %progbits
	.byte 1
 .popsection
	.byte 0
 .pushsection .bar2,"a"
	.byte 1
 .popsection
	.byte 0
