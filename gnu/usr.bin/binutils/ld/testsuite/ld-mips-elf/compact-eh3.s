	.section	.text.startup,"ax",@progbits
	.align	2
	.cfi_sections .eh_frame_entry
.LFB3 = .
	.cfi_startproc
	.cfi_personality_id 0x2
	.cfi_lsda 0x1b,.LLSDA3
	.global main
	.type	main, @function
main:
.LEHB0 = .
	move	$4,$2

.LEHE0 = .
.L11:
	nop

	lw	$31,28($sp)
	nop

	jal	compact3a
	move	$4,$2
	.cfi_fde_data 0x3,0x42
	.cfi_endproc
	.globl	__gnu_compact_pr2
	.cfi_inline_lsda 2
.LLSDA3:
	.byte	0x2	 
	.uleb128 .LLSDACSE3-.LLSDACSB3	 
.LLSDACSB3:
	 # Region 0 -- NoThrow
	.uleb128 (.LEHB0-.LFB3)|1	 # Length
	 # Region 1 -- Action Chain
	.uleb128 (.LEHE0-.LEHB0)	 # Length
	.sleb128 (.L11-(.LEHE0))	 # Landing Pad Offset
	.sleb128 (0<<2)|0x1	 # Action/Chain Pair
.LLSDACSE3:
