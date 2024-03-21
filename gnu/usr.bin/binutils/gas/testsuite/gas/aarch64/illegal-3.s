// Test the disassembler's detection of illegal binary sequences.

// PR 21380:

	.inst 0x4dc2d4ec
	.inst 0x4de2d4fc
	.inst 0x4dc2f4ec
	.inst 0x4de2f4fc

// PR 20319:
	# Check FMOV for Unallocated Encodings
        # FMOV (register): type == 0x10
        .inst 0x1ea04000
        # FMOV (scalar, immediate): type == 0x10
        .inst 0x1ea01000
        # FMOV (vector, immediate): Q == 0 && op == 1
        .inst 0x2f00f400
        # FMOV (general):
        # type == 10 && rmode != 01
        .inst 0x1ea60000
        .inst 0x1ea70000
        .inst 0x9ea60000
        .inst 0x9ea70000
        # rmode == 00 && fltsize != 16 && fltsize != intsize
        .inst 0x9e260000
        .inst 0x9e270000
        .inst 0x1e660000
        .inst 0x1e670000
        # rmode == 01 && intsize != 64
        .inst 0x1e2e0000
        .inst 0x1e2f0000
        .inst 0x1e6e0000
        .inst 0x1e6f0000
        .inst 0x1eae0000
        .inst 0x1eaf0000
        .inst 0x1eee0000
        .inst 0x1eef0000
        # rmode == 01 && fltsize != 128
        .inst 0x1e2e0000
        .inst 0x1e2f0000
        .inst 0x1e6e0000
        .inst 0x1e6f0000
        .inst 0x1eee0000
        .inst 0x1eef0000
        .inst 0x9e2e0000
        .inst 0x9e2f0000
        .inst 0x9e6e0000
        .inst 0x9e6f0000
        .inst 0x9eee0000
        .inst 0x9eef0000
        # type == 10 && rmode != 01
        .inst 0x1ea60000
        .inst 0x1ea70000
        .inst 0x9ea60000
        .inst 0x9ea70000

