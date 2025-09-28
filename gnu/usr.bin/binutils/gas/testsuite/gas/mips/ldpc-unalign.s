	.text
        ldpc     $4, 1f+4
        ldpc     $4, 1f+4
        ldpc     $4, 2f
        ldpc     $4, 2f
        ldpc     $4, 2f+4
        ldpc     $4, 2f+4
	.align 3
1:
        nop
2:
	nop
	nop
	nop

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.align  2
	.space  8
