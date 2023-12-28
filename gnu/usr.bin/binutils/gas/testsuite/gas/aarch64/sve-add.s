	add	z0.b, z0.b, #-255
	add	z0.b, z0.b, #-129
	add	z0.b, z0.b, #-128
	add	z0.b, z0.b, #-127
	add	z0.b, z0.b, #-1
	add	z0.b, z0.b, #0
	add	z0.b, z0.b, #1
	add	z0.b, z0.b, #127
	add	z0.b, z0.b, #128
	add	z0.b, z0.b, #255

	add	z0.h, z0.h, #-65536
	add	z0.h, z0.h, #-65535
	add	z0.h, z0.h, #-65536 + 127
	add	z0.h, z0.h, #-65536 + 128
	add	z0.h, z0.h, #-65536 + 129
	add	z0.h, z0.h, #-65536 + 255
	add	z0.h, z0.h, #-65536 + 256
	add	z0.h, z0.h, #-32768 - 256
	add	z0.h, z0.h, #-32768
	add	z0.h, z0.h, #-32768 + 256
	add	z0.h, z0.h, #0
	add	z0.h, z0.h, #1
	add	z0.h, z0.h, #127
	add	z0.h, z0.h, #128
	add	z0.h, z0.h, #129
	add	z0.h, z0.h, #255
	add	z0.h, z0.h, #256
	add	z0.h, z0.h, #32768 - 256
	add	z0.h, z0.h, #32768
	add	z0.h, z0.h, #32768 + 256
	add	z0.h, z0.h, #65536 - 256
	add	z0.h, z0.h, #-255, lsl #8
	add	z0.h, z0.h, #-129, lsl #8
	add	z0.h, z0.h, #-128, lsl #8
	add	z0.h, z0.h, #-127, lsl #8
	add	z0.h, z0.h, #-1, lsl #8
	add	z0.h, z0.h, #0, lsl #8
	add	z0.h, z0.h, #1, lsl #8
	add	z0.h, z0.h, #127, lsl #8
	add	z0.h, z0.h, #128, lsl #8
	add	z0.h, z0.h, #255, lsl #8

	add	z0.s, z0.s, #0
	add	z0.s, z0.s, #1
	add	z0.s, z0.s, #127
	add	z0.s, z0.s, #128
	add	z0.s, z0.s, #129
	add	z0.s, z0.s, #255
	add	z0.s, z0.s, #256
	add	z0.s, z0.s, #0x7f00
	add	z0.s, z0.s, #0x8000
	add	z0.s, z0.s, #0xff00
	add	z0.s, z0.s, #0, lsl #8
	add	z0.s, z0.s, #1, lsl #8
	add	z0.s, z0.s, #127, lsl #8
	add	z0.s, z0.s, #128, lsl #8
	add	z0.s, z0.s, #255, lsl #8

	add	z0.d, z0.d, #0
	add	z0.d, z0.d, #1
	add	z0.d, z0.d, #127
	add	z0.d, z0.d, #128
	add	z0.d, z0.d, #129
	add	z0.d, z0.d, #255
	add	z0.d, z0.d, #256
	add	z0.d, z0.d, #0x7f00
	add	z0.d, z0.d, #0x8000
	add	z0.d, z0.d, #0xff00
	add	z0.d, z0.d, #0, lsl #8
	add	z0.d, z0.d, #1, lsl #8
	add	z0.d, z0.d, #127, lsl #8
	add	z0.d, z0.d, #128, lsl #8
	add	z0.d, z0.d, #255, lsl #8

	sub	z0.b, z0.b, #-255
	sub	z0.b, z0.b, #-129
	sub	z0.b, z0.b, #-128
	sub	z0.b, z0.b, #-127
	sub	z0.b, z0.b, #-1
	sub	z0.b, z0.b, #0
	sub	z0.b, z0.b, #1
	sub	z0.b, z0.b, #127
	sub	z0.b, z0.b, #128
	sub	z0.b, z0.b, #255

	sub	z0.h, z0.h, #-65536
	sub	z0.h, z0.h, #-65535
	sub	z0.h, z0.h, #-65536 + 127
	sub	z0.h, z0.h, #-65536 + 128
	sub	z0.h, z0.h, #-65536 + 129
	sub	z0.h, z0.h, #-65536 + 255
	sub	z0.h, z0.h, #-65536 + 256
	sub	z0.h, z0.h, #-32768 - 256
	sub	z0.h, z0.h, #-32768
	sub	z0.h, z0.h, #-32768 + 256
	sub	z0.h, z0.h, #0
	sub	z0.h, z0.h, #1
	sub	z0.h, z0.h, #127
	sub	z0.h, z0.h, #128
	sub	z0.h, z0.h, #129
	sub	z0.h, z0.h, #255
	sub	z0.h, z0.h, #256
	sub	z0.h, z0.h, #32768 - 256
	sub	z0.h, z0.h, #32768
	sub	z0.h, z0.h, #32768 + 256
	sub	z0.h, z0.h, #65536 - 256
	sub	z0.h, z0.h, #-255, lsl #8
	sub	z0.h, z0.h, #-129, lsl #8
	sub	z0.h, z0.h, #-128, lsl #8
	sub	z0.h, z0.h, #-127, lsl #8
	sub	z0.h, z0.h, #-1, lsl #8
	sub	z0.h, z0.h, #0, lsl #8
	sub	z0.h, z0.h, #1, lsl #8
	sub	z0.h, z0.h, #127, lsl #8
	sub	z0.h, z0.h, #128, lsl #8
	sub	z0.h, z0.h, #255, lsl #8

	sub	z0.s, z0.s, #0
	sub	z0.s, z0.s, #1
	sub	z0.s, z0.s, #127
	sub	z0.s, z0.s, #128
	sub	z0.s, z0.s, #129
	sub	z0.s, z0.s, #255
	sub	z0.s, z0.s, #256
	sub	z0.s, z0.s, #0x7f00
	sub	z0.s, z0.s, #0x8000
	sub	z0.s, z0.s, #0xff00
	sub	z0.s, z0.s, #0, lsl #8
	sub	z0.s, z0.s, #1, lsl #8
	sub	z0.s, z0.s, #127, lsl #8
	sub	z0.s, z0.s, #128, lsl #8
	sub	z0.s, z0.s, #255, lsl #8

	sub	z0.d, z0.d, #0
	sub	z0.d, z0.d, #1
	sub	z0.d, z0.d, #127
	sub	z0.d, z0.d, #128
	sub	z0.d, z0.d, #129
	sub	z0.d, z0.d, #255
	sub	z0.d, z0.d, #256
	sub	z0.d, z0.d, #0x7f00
	sub	z0.d, z0.d, #0x8000
	sub	z0.d, z0.d, #0xff00
	sub	z0.d, z0.d, #0, lsl #8
	sub	z0.d, z0.d, #1, lsl #8
	sub	z0.d, z0.d, #127, lsl #8
	sub	z0.d, z0.d, #128, lsl #8
	sub	z0.d, z0.d, #255, lsl #8
