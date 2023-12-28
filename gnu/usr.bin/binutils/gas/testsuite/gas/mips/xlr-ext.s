# Source file used to test XLR's assembler instructions

	.set 	noreorder
	.set 	noat

	.globl	text_label	.text
text_label:	

	lui     $0, 0x00
	lw 	    $1, 0x01
	lw 	    $2, 0x02
	
	daddwc	$3, $1, $2
	
	ldaddw	$3, $1
	ldaddwu	$3, $1
	ldaddd	$3, $1

	swapw	$3, $1
	swapwu	$3, $1
	
	msgwait
	msgld	$0
	msgsnd	$0

# Force at least 8 (non-delay-slot) zero bytes, to make 'objdump' print ...
	.space  8
