	.syntax unified
	.cpu cortex-m4
	.thumb

	.section  .text

	orr r1, #12800	 	/* This is OK.  */
	orr r1, #12801		/* This cannot be encoded in Thumb mode.  */
	/* GAS used to accept it though, and produce a MOV instruction instead.  */
