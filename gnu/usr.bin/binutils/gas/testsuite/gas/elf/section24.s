	.section	.text,"ax",%progbits
  .word 0
	.section	.data,"aw"
  .word 0
	.section	.bss,"aw",%nobits
  .word 0
	.section	.rodata,"a"
  .word 0

/* Test that we can set the 'R' flag on an existing section.  */
	.section	.text,"axR",%progbits
  .word 0
	.section	.data,"awR"
  .word 0
	.section	.bss,"awR",%nobits
  .word 0
	.section	.rodata,"aR"
  .word 0

/* Test that the 'R' flag does not get clobbered when the section is switched
   back to.  */
	.section	.text,"ax",%progbits
  .word 0
	.section	.data,"aw"
  .word 0
	.section	.bss,"aw",%nobits
  .word 0
	.section	.rodata,"a"
  .word 0

	.section	.text
  .word 0
	.section	.data
  .word 0
	.section	.bss
  .word 0
	.section	.rodata
  .word 0
