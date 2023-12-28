	.text
	.byte 1
	.section .foo,"a?","progbits"
	.byte 1

	.section .text,"axG",%progbits,some_group,comdat
	.byte 1
	.pushsection .note.bar,"?","note"
	.4byte 1f-0f, 3f-2f, 123
0:	.asciz "somevendor"
1:	.balign 4
2:	.byte 1
	.uleb128 5f-4f
3:	.balign 4
	.popsection
4:	.byte 2
5:	.byte 3
