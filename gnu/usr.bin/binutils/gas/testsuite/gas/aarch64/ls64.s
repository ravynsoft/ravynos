/* Atomic 64-byte load/store instructions.  */
.arch armv8-a+ls64

/* Single-copy Atomic 64-byte Load.  */
	ld64b x0, [x1]
	ld64b x2, [x1]
	ld64b x4, [x1]
	ld64b x6, [x1]
	ld64b x8, [x1]
	ld64b x10, [x1]
	ld64b x12, [x1]
	ld64b x14, [x1]
	ld64b x16, [x1]
	ld64b x18, [x1]
	ld64b x20, [x1]
	ld64b x22, [x1]

/* Single-copy Atomic 64-byte Store without Return.  */
	st64b x0, [x1]
	st64b x2, [x1]
	st64b x4, [x1]
	st64b x6, [x1]
	st64b x8, [x1]
	st64b x10, [x1]
	st64b x12, [x1]
	st64b x14, [x1]
	st64b x16, [x1]
	st64b x18, [x1]
	st64b x20, [x1]
	st64b x22, [x1]

/* Single-copy Atomic 64-byte Store with Return.  */
	st64bv x1, x0, [x2]
	st64bv x0, x2, [x2]
	st64bv x0, x4, [x2]
	st64bv x0, x6, [x2]
	st64bv x0, x8, [x2]
	st64bv x0, x10, [x2]
	st64bv x0, x12, [x2]
	st64bv x0, x14, [x2]
	st64bv x0, x16, [x2]
	st64bv x0, x18, [x2]
	st64bv x0, x20, [x2]
	st64bv x0, x22, [x2]

/* Single-copy Atomic 64-byte EL0 Store with Return.  */
	st64bv0 x1, x0, [x2]
	st64bv0 x0, x2, [x2]
	st64bv0 x0, x4, [x2]
	st64bv0 x0, x6, [x2]
	st64bv0 x0, x8, [x2]
	st64bv0 x0, x10, [x2]
	st64bv0 x0, x12, [x2]
	st64bv0 x0, x14, [x2]
	st64bv0 x0, x16, [x2]
	st64bv0 x0, x18, [x2]
	st64bv0 x0, x20, [x2]
	st64bv0 x0, x22, [x2]

.arch armv8-a
/* Accelerator Data system register.  */
	mrs x0, accdata_el1
	msr accdata_el1, x0
