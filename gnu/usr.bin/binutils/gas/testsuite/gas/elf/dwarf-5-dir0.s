	.section	.debug_info,"",%progbits
	.4byte	0x8a
	.2byte  0x2
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1

	.file 0 "../not-the-build-directory/master-source-file.c"
	.line 1
	.text
	.octa 0x12345678901234567890123456789012

	.file 1 "secondary directory/secondary source file"
	.line 2
	.word 2

	.file 2 "/tmp" "foo.c" md5 0x95828e8bc4f7404dbf7526fb7bd0f192
	.line 5
	.word 6
