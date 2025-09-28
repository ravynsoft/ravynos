		.section .sdata
		.globl exdat1a
		.globl exdat1b
		.globl exdat1c
exdat1a:	.long 6
exdat1b:	.long 7
exdat1c:	.long 8

		.section .sdata2
		.globl exdat2a
		.globl exdat2b
		.globl exdat2c
exdat2a:	 .long 5
exdat2b:	 .long 4
exdat2c:	 .long 3

		.section .PPC.EMB.sdata0
		.globl exdat0a
		.globl exdat0b
		.globl exdat0c
exdat0a:	 .long 1
exdat0b:	 .long 2
exdat0c:	 .long 3

		.section .sbss
		.globl exbss1a
		.globl exbss1b
exbss1a:	.int
exbss1b:	.int
