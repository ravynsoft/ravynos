# source file to test assembly of mips32r2 cop2 instructions

        .set noreorder
	.set noat

	.text
text_label:
      # cp2 instructions

	mfhc2	$17, 0x5555
	mthc2	$17, 0x5555

