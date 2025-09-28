# Test for correct generation of XGATE insns when using the %hi and %lo modifiers.
	
	.sect .text
;Test Constants
hiTestLo:
	ldl R2, %hi(0x8844)
hiTestHi:
	ldh R2, %hi(0x8844)
loTestLo:
	ldl R3, %lo(0x8844)
loTestHi:
	ldh R3, %lo(0x8844)
;Test Fixups
hiTestLoF:
	ldl R2, %hi(test)
hiTestHiF:
	ldh R2, %hi(test)
loTestLoF:
	ldl R3, %lo(test)
loTestHiF:
	ldh R3, %lo(test)
;Test Relocs
hiTestLoR:
	ldl R2, %hi(symValue)
hiTestHiR:
	ldh R2, %hi(symValue)
loTestLoR:
	ldl R3, %lo(symValue)
loTestHiR:
	ldh R3, %lo(symValue)

symValue:
test = 0xff88
