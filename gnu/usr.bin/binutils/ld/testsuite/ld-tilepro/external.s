        .text
        .global external1
external1: 
	j external1

        .global external2
external2:
	j external1

        .global external_5a
external_5a = 19
        .global external_5b
external_5b = 31

        .global external_8a
external_8a = 17
        .global external_8b
external_8b = 119

        .global external_16a
external_16a = -32134
        .global external_16b
external_16b = 19300

        .global external_32a
external_32a = 0x87654321
        .global external_32b
external_32b = 0xfedcba98

        .data
	.align 0x20
        .global external_data1
external_data1:
