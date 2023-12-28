 .macro err op:vararg
  .ifndef AT
   .ifndef Y
    \op
   .endif
  .endif
 .endm

 .macro errat op:vararg
  .ifndef AT
   \op
  .endif
 .endm

 .macro erry op:vararg
  .ifndef Y
   \op
  .endif
 .endm

 .text
	bc 0,0,.
 errat	bc 1,0,.	# z bit
	bc 2,0,.
 errat	bc 3,0,.	# z bit
	bc 4,0,.
 errat	bc 5,0,.	# at = 01 reserved
 erry	bc 6,0,.	# z bit
 erry	bc 7,0,.	# z bit
	bc 8,0,.
 errat	bc 9,0,.	# z bit
	bc 10,0,.
 errat	bc 11,0,.	# z bit
	bc 12,0,.
 errat	bc 13,0,.	# at = 01 reserved
 erry	bc 14,0,.	# z bit
 erry	bc 15,0,.	# z bit
	bc 16,0,.
 errat	bc 17,0,.	# at = 01 reserved
	bc 18,0,.
 errat	bc 19,0,.	# at = 01 reserved
	bc 20,0,.
 err	bc 21,0,.	# z bit
 err	bc 22,0,.	# z bit
 err	bc 23,0,.	# z bit
 erry	bc 24,0,.	# z bit
 erry	bc 25,0,.	# z bit
 erry	bc 26,0,.	# z bit
 erry	bc 27,0,.	# z bit
 err	bc 28,0,.	# z bit
 err	bc 29,0,.	# z bit
 err	bc 30,0,.	# z bit
 err	bc 31,0,.	# z bit

 err	bcctr 0,0
 err	bcctr 1,0
 err	bcctr 2,0
 err	bcctr 3,0
	bcctr 4,0
 errat	bcctr 5,0
 erry	bcctr 6,0
 erry	bcctr 7,0
 err	bcctr 8,0
 err	bcctr 9,0
 err	bcctr 10,0
 err	bcctr 11,0
	bcctr 12,0
 errat	bcctr 13,0
 erry	bcctr 14,0
 erry	bcctr 15,0
 err	bcctr 16,0
 err	bcctr 17,0
 err	bcctr 18,0
 err	bcctr 19,0
	bcctr 20,0
 err	bcctr 21,0
 err	bcctr 22,0
 err	bcctr 23,0
 err	bcctr 24,0
 err	bcctr 25,0
 err	bcctr 26,0
 err	bcctr 27,0
 err	bcctr 28,0
 err	bcctr 29,0
 err	bcctr 30,0
 err	bcctr 31,0

	bclr 0,0
 errat	bclr 1,0
	bclr 2,0
 errat	bclr 3,0
	bclr 4,0
 errat	bclr 5,0
 erry	bclr 6,0
 erry	bclr 7,0
	bclr 8,0
 errat	bclr 9,0
	bclr 10,0
 errat	bclr 11,0
	bclr 12,0
 errat	bclr 13,0
 erry	bclr 14,0
 erry	bclr 15,0
	bclr 16,0
 errat	bclr 17,0
	bclr 18,0
 errat	bclr 19,0
	bclr 20,0
 err	bclr 21,0
 err	bclr 22,0
 err	bclr 23,0
 erry	bclr 24,0
 erry	bclr 25,0
 erry	bclr 26,0
 erry	bclr 27,0
 err	bclr 28,0
 err	bclr 29,0
 err	bclr 30,0
 err	bclr 31,0

 .ifdef POWER8
	bctar 0,0
 errat	bctar 1,0
	bctar 2,0
 errat	bctar 3,0
	bctar 4,0
 errat	bctar 5,0
	bctar 6,0
	bctar 7,0
	bctar 8,0
 errat	bctar 9,0
	bctar 10,0
 errat	bctar 11,0
	bctar 12,0
 errat	bctar 13,0
	bctar 14,0
	bctar 15,0
	bctar 16,0
 errat	bctar 17,0
	bctar 18,0
 errat	bctar 19,0
	bctar 20,0
 errat	bctar 21,0
 errat	bctar 22,0
 errat	bctar 23,0
	bctar 24,0
	bctar 25,0
	bctar 26,0
	bctar 27,0
 errat	bctar 28,0
 errat	bctar 29,0
 errat	bctar 30,0
 errat	bctar 31,0
 .endif
