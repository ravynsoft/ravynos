.text

test_R_AARCH64_MOVW_UABS_G0:
	movz	x4, :abs_g0:abs_0x1234
	movz	x4, :abs_g0:abs_0x1234 + 4

test_R_AARCH64_MOVW_UABS_G0_NC:
	movz	x4, :abs_g0_nc:abs_0x1234
	movz	x4, :abs_g0_nc:abs_0x1234 + 0x45000

test_R_AARCH64_MOVW_UABS_G1:
	movz	x4, :abs_g1:abs_0x1234 - 4
	movz	x4, :abs_g1:abs_0x11000
	movz	x4, :abs_g1:abs_0x45000 + 0x20010

test_R_AARCH64_MOVW_UABS_G1_NC:
	movz	x4, :abs_g1_nc:abs_0x1234 - 4
	movz	x4, :abs_g1_nc:abs_0x11000
	movz	x4, :abs_g1_nc:abs_0x45000 + 0x100020010

test_R_AARCH64_MOVW_UABS_G2:
	movz	x4, :abs_g2:abs_0x45000 + 0x20010
	movz	x4, :abs_g2:abs_0x3600010000 + 0x100020010

test_R_AARCH64_MOVW_UABS_G2_NC:
	movz	x4, :abs_g2_nc:abs_0x45000 + 0x20010
	movz	x4, :abs_g2_nc:abs_0x3600010000 + 0x3000100020010

test_R_AARCH64_MOVW_UABS_G3:
	movz	x4, :abs_g3:abs_0x3600010000 + 0x100020010
	movz	x4, :abs_g3:abs_0x3600010000 + 0x3000100020010

test_R_AARCH64_MOVW_SABS_G0:
	movz	x4, :abs_g0_s:abs_0x1234 + 4
	movz	x4, :abs_g0_s:abs_0x1234 - 0x2345

test_R_AARCH64_MOVW_SABS_G1:
	movz	x4, :abs_g1_s:abs_0x1234 - 0x2345
	movz	x4, :abs_g1_s:abs_0x45000 + 0x20010
	movz	x4, :abs_g1_s:abs_0x45000 - 0x56000

test_R_AARCH64_MOVW_SABS_G2:
	movz	x4, :abs_g2_s:abs_0x45000 + 0x20010
	movz	x4, :abs_g2_s:abs_0x3600010000 + 0x100020010
	movz	x4, :abs_g2_s:abs_0x3600010000 - 0x4400010000
