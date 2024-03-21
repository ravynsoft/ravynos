	.gnu_attribute 4, 1
	.abicalls
        .hidden DW.ref.__gnu_compact_pr2
        .weak   DW.ref.__gnu_compact_pr2
        .section        .data.DW.ref.__gnu_compact_pr2,"awG",@progbits,DW.ref.__gnu_compact_pr2,comdat
        .align  2
        .type   DW.ref.__gnu_compact_pr2, @object
        .size   DW.ref.__gnu_compact_pr2, 4
DW.ref.__gnu_compact_pr2:
        .word   __gnu_compact_pr2
	.text
	.align	2
	.globl	_Z3fooi
	.cfi_sections .eh_frame_entry
$LFB0 = .
	.cfi_startproc
	.cfi_personality 0x1b, DW.ref.__gnu_compact_pr2
	.cfi_lsda 0x1b,$LLSDA0
	.set	nomips16
	.set	nomicromips
	.ent	_Z3fooi
	.type	_Z3fooi, @function
_Z3fooi:
$LEHB0 = .
	nop
$LEHE0 = .
	nop
$LEHB1 = .
	nop
$LEHE1 = .
	nop
$LEHB2 = .
$L3:
	nop
$LEHE2 = .
	.end	_Z3fooi
	.size	_Z3fooi, .-_Z3fooi
	.cfi_fde_data 0x4,0x40
	.cfi_endproc
	.globl	__gnu_compact_pr2
	.cfi_inline_lsda 2
$LLSDA0:
	.byte	0x2
	.uleb128 $LLSDACSE0-$LLSDACSB0
$LLSDACSB0:
	.uleb128 ($LEHB0-$LFB0)|1
	.uleb128 ($LEHE0-$LEHB0)
	.sleb128 -1
	.uleb128 ($LEHB1-$LEHE0)|1
	.uleb128 ($LEHE1-$LEHB1)
	.sleb128 ($L3-($LEHE1))
	.sleb128 (0<<2)|0
	.uleb128 ($LEHB2-$LEHE1)|1
	.uleb128 ($LEHE2-$LEHB2)
	.sleb128 -1
$LLSDACSE0:
