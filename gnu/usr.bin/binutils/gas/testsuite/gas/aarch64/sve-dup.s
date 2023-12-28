	dup	z0.b, #-255
	dup	z0.b, #-129
	dup	z0.b, #-128
	dup	z0.b, #-127
	dup	z0.b, #-1
	dup	z0.b, #0
	dup	z0.b, #1
	dup	z0.b, #127
	dup	z0.b, #128
	dup	z0.b, #255

	dup	z0.h, #-65535
	dup	z0.h, #-65536 + 127
	dup	z0.h, #-65536 + 256
	dup	z0.h, #-32768
	dup	z0.h, #-32768 + 256
	dup	z0.h, #-128
	dup	z0.h, #-127
	dup	z0.h, #-1
	dup	z0.h, #0
	dup	z0.h, #1
	dup	z0.h, #127
	dup	z0.h, #256
	dup	z0.h, #32768 - 256
	dup	z0.h, #32768
	dup	z0.h, #65536 - 256
	dup	z0.h, #65536 - 128
	dup	z0.h, #65536 - 127
	dup	z0.h, #65535
	dup	z0.h, #-255, lsl #8
	dup	z0.h, #-129, lsl #8
	dup	z0.h, #-128, lsl #8
	dup	z0.h, #-127, lsl #8
	dup	z0.h, #-1, lsl #8
	dup	z0.h, #0, lsl #8
	dup	z0.h, #1, lsl #8
	dup	z0.h, #127, lsl #8
	dup	z0.h, #128, lsl #8
	dup	z0.h, #255, lsl #8

	dup	z0.s, #-32768
	dup	z0.s, #-32768 + 256
	dup	z0.s, #-128
	dup	z0.s, #-127
	dup	z0.s, #-1
	dup	z0.s, #0
	dup	z0.s, #1
	dup	z0.s, #127
	dup	z0.s, #256
	dup	z0.s, #32768 - 256
	dup	z0.s, #0xffffff80
	dup	z0.s, #0xffffff81
	dup	z0.s, #0xffffffff
	dup	z0.s, #-128, lsl #8
	dup	z0.s, #-127, lsl #8
	dup	z0.s, #-1, lsl #8
	dup	z0.s, #0, lsl #8
	dup	z0.s, #1, lsl #8
	dup	z0.s, #127, lsl #8

	dup	z0.d, #-32768
	dup	z0.d, #-32768 + 256
	dup	z0.d, #-128
	dup	z0.d, #-127
	dup	z0.d, #-1
	dup	z0.d, #0
	dup	z0.d, #1
	dup	z0.d, #127
	dup	z0.d, #256
	dup	z0.d, #32768 - 256
	dup	z0.d, #0xffffffffffffff80
	dup	z0.d, #0xffffffffffffff81
	dup	z0.d, #0xffffffffffffffff
	dup	z0.d, #-128, lsl #8
	dup	z0.d, #-127, lsl #8
	dup	z0.d, #-1, lsl #8
	dup	z0.d, #0, lsl #8
	dup	z0.d, #1, lsl #8
	dup	z0.d, #127, lsl #8

	mov	z0.b, #-255
	mov	z0.b, #-129
	mov	z0.b, #-128
	mov	z0.b, #-127
	mov	z0.b, #-1
	mov	z0.b, #0
	mov	z0.b, #1
	mov	z0.b, #127
	mov	z0.b, #128
	mov	z0.b, #255

	mov	z0.h, #-65535
	mov	z0.h, #-65536 + 127
	mov	z0.h, #-65536 + 256
	mov	z0.h, #-32768
	mov	z0.h, #-32768 + 256
	mov	z0.h, #-128
	mov	z0.h, #-127
	mov	z0.h, #-1
	mov	z0.h, #0
	mov	z0.h, #1
	mov	z0.h, #127
	mov	z0.h, #256
	mov	z0.h, #32768 - 256
	mov	z0.h, #32768
	mov	z0.h, #65536 - 256
	mov	z0.h, #65536 - 128
	mov	z0.h, #65536 - 127
	mov	z0.h, #65535
	mov	z0.h, #-255, lsl #8
	mov	z0.h, #-129, lsl #8
	mov	z0.h, #-128, lsl #8
	mov	z0.h, #-127, lsl #8
	mov	z0.h, #-1, lsl #8
	mov	z0.h, #0, lsl #8
	mov	z0.h, #1, lsl #8
	mov	z0.h, #127, lsl #8
	mov	z0.h, #128, lsl #8
	mov	z0.h, #255, lsl #8

	mov	z0.s, #-32768
	mov	z0.s, #-32768 + 256
	mov	z0.s, #-128
	mov	z0.s, #-127
	mov	z0.s, #-1
	mov	z0.s, #0
	mov	z0.s, #1
	mov	z0.s, #127
	mov	z0.s, #256
	mov	z0.s, #32768 - 256
	mov	z0.s, #0xffffff80
	mov	z0.s, #0xffffff81
	mov	z0.s, #0xffffffff
	mov	z0.s, #-128, lsl #8
	mov	z0.s, #-127, lsl #8
	mov	z0.s, #-1, lsl #8
	mov	z0.s, #0, lsl #8
	mov	z0.s, #1, lsl #8
	mov	z0.s, #127, lsl #8

	mov	z0.d, #-32768
	mov	z0.d, #-32768 + 256
	mov	z0.d, #-128
	mov	z0.d, #-127
	mov	z0.d, #-1
	mov	z0.d, #0
	mov	z0.d, #1
	mov	z0.d, #127
	mov	z0.d, #256
	mov	z0.d, #32768 - 256
	mov	z0.d, #0xffffffffffffff80
	mov	z0.d, #0xffffffffffffff81
	mov	z0.d, #0xffffffffffffffff
	mov	z0.d, #-128, lsl #8
	mov	z0.d, #-127, lsl #8
	mov	z0.d, #-1, lsl #8
	mov	z0.d, #0, lsl #8
	mov	z0.d, #1, lsl #8
	mov	z0.d, #127, lsl #8
