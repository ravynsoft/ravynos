.text

.dword 0
.dword 0

# 2010
.global foo
foo:

# 4-byte literal
.long 0x12345678
.word 0x12345678

# IMAGE_REL_ARM64_ADDR32 (BFD_RELOC_32)
.long foo
.word foo
.long bar
.word bar
.long foo + 1
.word foo + 1
.long bar + 1
.word bar + 1
.long foo - 1
.word foo - 1
.long bar - 1
.word bar - 1

# 8-byte literal
.dword 0x123456789abcdef0
.xword 0x123456789abcdef0

# IMAGE_REL_ARM64_ADDR64 (BFD_RELOC_64)
.dword foo
.xword foo
.dword bar
.xword bar
.dword foo + 1
.xword foo + 1
.dword bar + 1
.xword bar + 1
.dword foo - 1
.xword foo - 1
.dword bar - 1
.xword bar - 1

# IMAGE_REL_ARM64_ADDR32NB (BFD_RELOC_RVA)
.rva foo
.rva bar
.rva foo + 1
.rva bar + 1
.rva foo - 1
.rva bar - 1

# IMAGE_REL_ARM64_BRANCH26 (BFD_RELOC_AARCH64_JUMP26)
b foo
b foo + 4
b foo - 4
b bar
b bar + 4
b bar - 4

# IMAGE_REL_ARM64_BRANCH26 (BFD_RELOC_AARCH64_CALL26)
bl foo
bl foo + 4
bl foo - 4
bl bar
bl bar + 4
bl bar - 4
bl .text - 4

# IMAGE_REL_ARM64_BRANCH19 (BFD_RELOC_AARCH64_BRANCH19)
cbz x0, foo
cbz x0, foo + 4
cbz x0, foo - 4
cbz x0, bar
cbz x0, bar + 4
cbz x0, bar - 4
cbz x0, .text - 4

# IMAGE_REL_ARM64_BRANCH14 (BFD_RELOC_AARCH64_TSTBR14)
tbz x0, 0, foo
tbz x0, 0, foo + 4
tbz x0, 0, foo - 4
tbz x0, 0, bar
tbz x0, 0, bar + 4
tbz x0, 0, bar - 4
tbz x0, 0, .text - 4

# IMAGE_REL_ARM64_PAGEBASE_REL21 (BFD_RELOC_AARCH64_ADR_HI21_PCREL)
adrp x0, foo
adrp x0, foo + 1
adrp x0, foo - 1
adrp x0, bar
adrp x0, bar + 1
adrp x0, bar - 1
adrp x0, .text - 4

# IMAGE_REL_ARM64_REL21 (BFD_RELOC_AARCH64_ADR_LO21_PCREL)
adr x0, foo
adr x0, foo + 1
adr x0, foo - 1
adr x0, bar
adr x0, bar + 1
adr x0, bar - 1
adr x0, .text - 1

# IMAGE_REL_ARM64_PAGEOFFSET_12L (BFD_RELOC_AARCH64_LDST8_LO12)
strb w0, [x0,:lo12:foo]
strb w0, [x0,:lo12:foo + 4]
strb w0, [x0,:lo12:foo - 4]
strb w0, [x0,:lo12:bar]
strb w0, [x0,:lo12:bar + 4]
strb w0, [x0,:lo12:bar - 4]
strb w0, [x0,:lo12:.text - 4]

# IMAGE_REL_ARM64_PAGEOFFSET_12L (BFD_RELOC_AARCH64_LDST16_LO12)
strh w0, [x0,:lo12:foo]
strh w0, [x0,:lo12:foo + 4]
strh w0, [x0,:lo12:foo - 4]
strh w0, [x0,:lo12:bar]
strh w0, [x0,:lo12:bar + 4]
strh w0, [x0,:lo12:bar - 4]
strh w0, [x0,:lo12:.text - 4]

# IMAGE_REL_ARM64_PAGEOFFSET_12L (BFD_RELOC_AARCH64_LDST32_LO12)
str w0, [x0,:lo12:foo]
str w0, [x0,:lo12:foo + 4]
str w0, [x0,:lo12:foo - 4]
str w0, [x0,:lo12:bar]
str w0, [x0,:lo12:bar + 4]
str w0, [x0,:lo12:bar - 4]
str w0, [x0,:lo12:.text - 4]

# IMAGE_REL_ARM64_PAGEOFFSET_12L (BFD_RELOC_AARCH64_LDST64_LO12)
str x0, [x0,:lo12:foo]
str x0, [x0,:lo12:foo + 8]
str x0, [x0,:lo12:foo - 8]
str x0, [x0,:lo12:bar]
str x0, [x0,:lo12:bar + 8]
str x0, [x0,:lo12:bar - 8]
str x0, [x0,:lo12:.text - 8]

# IMAGE_REL_ARM64_PAGEOFFSET_12L (BFD_RELOC_AARCH64_LDST128_LO12)
str q0, [x0,:lo12:foo]
str q0, [x0,:lo12:foo + 16]
str q0, [x0,:lo12:foo - 16]
str q0, [x0,:lo12:bar]
str q0, [x0,:lo12:bar + 16]
str q0, [x0,:lo12:bar - 16]
str q0, [x0,:lo12:.text - 16]

# IMAGE_REL_ARM64_PAGEOFFSET_12A (BFD_RELOC_AARCH64_ADD_LO12)
add x0, x0, #:lo12:foo
add x0, x0, #:lo12:foo + 1
add x0, x0, #:lo12:foo - 1
add x0, x0, #:lo12:bar
add x0, x0, #:lo12:bar + 1
add x0, x0, #:lo12:bar - 1
add x0, x0, #:lo12:.text - 1
