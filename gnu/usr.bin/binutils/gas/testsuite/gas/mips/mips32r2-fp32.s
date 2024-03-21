# source file to test assembly of mips32r2 FP instructions

	.text
text_label:

      # FPU (cp1) instructions
      #
      # Even registers are supported w/ 32-bit FPU, odd
      # registers supported only for 64-bit FPU.
      # Only the 32-bit FPU instructions are tested here.
     
	mfhc1	$17, $f0
	mthc1	$17, $f0
