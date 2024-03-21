// Test illegal ARMv8.3 LDRAA and LDRAB instructions
.text

	// Good.
	ldraa x0, [x1,#8]
	ldrab x0, [x1,#8]


	ldraa x0, [x1,#1]
	ldraa x0, [x1,#4]
	ldraa x0, [x1,#-10]
	ldraa x0, [x1,#4096]
	ldraa x0, [x1,#5555]
	ldraa x0, [x1,#-4104]
	ldraa x0, [xz]
	ldraa x0, [sp],
	ldraa x0, [x1,#1]!
	ldraa x0, [x1,#4]!
	ldraa x0, [x1,#-10]!
	ldraa x0, [x1,#4096]!
	ldraa x0, [x1,#5555]!
	ldraa x0, [x1,#-4104]!
	ldraa x0, [xz]
	ldraa x0, [x1], #8


	ldrab x0, [x1,#1]
	ldrab x0, [x1,#4]
	ldrab x0, [x1,#-10]
	ldrab x0, [x1,#4096]
	ldrab x0, [x1,#5555]
	ldrab x0, [x1,#-4104]
	ldrab x0, [xz]
	ldrab x0, [sp],
	ldrab x0, [x1,#1]!
	ldrab x0, [x1,#4]!
	ldrab x0, [x1,#-10]!
	ldrab x0, [x1,#4096]!
	ldrab x0, [x1,#5555]!
	ldrab x0, [x1,#-4104]!
	ldrab x0, [xz]
	ldrab x0, [x1], #8
