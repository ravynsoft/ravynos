.attribute arch, "rv32ic"
.option norelax			# FIXME: assembler fill the paddings after parsing everything,
				# so we probably won't fill anything for the norelax region when
				# the riscv_opts.relax is enabled at somewhere.

.section .text.cross.section.A, "ax"
.option push
.global funcA
funcA:
addi	a0, zero, 1		# rv32i
.option arch, +c
j	funcA			# rv32ic
.section .text.corss.section.B, "ax"
.globl funcB
funcB:
addi	a0, zero, 2		# rv32ic, need to be added since start of section
.option arch, -c
j	funcB			# rv32i
.option pop

.section .text.data, "ax"
.option push
.word	0			# $d
.long	1
addi	a0, zero, 1		# rv32ic
.data
.word	2			# don't add mapping symbols for non-text section
.section .text.data
addi	a0, zero, 2		# $x, but same as previous addi, so removed
.byte	2			# $d, dumped as .word
.short	3
.byte	5
.option pop

.section .text.odd.align.start.insn, "ax"
.option push
.option norelax
.option arch, +c
addi	a0, zero, 1		# $xrv32ic
.byte	1			# $d
.option arch, -c
.align	3			# odd alignment, $x replaced by $d + $x
addi	a0, zero, 2		# $xrv32i
.option pop

.section .text.odd.align.start.data, "ax"
.option push
.option norelax
.option arch, +c
.byte	1			# $d
.align	2			# odd alignment, $xrv32ic replaced by $d + $xrv32ic
addi	a0, zero, 1
.option pop

.section .text.zero.fill.first, "ax"
.option push
.option norelax
.fill	1, 0, 0			# $d with zero size, removed in make_mapping_symbol
addi	a0, zero, 1		# $xrv32ic
.option pop

.section .text.zero.fill.last, "ax"
.option push
.option norelax
addi	a0, zero, 1		# $xrv32ic
.fill	1, 0, 0			# $d with zero size, removed in make_mapping_symbol
addi	a0, zero, 2		# $x, FIXME: need find a way to remove?
.option pop

# last overlap next first
.section .text.zero.fill.align.A, "ax"
.option push
.option norelax
.align	2			# $xrv32ic, .align and .fill are in the different frag, so neither be removed
.fill	1, 0, 0			# $d with zero size, removed in make_mapping_symbol when adding $xrv32ic
addi	a0, zero, 1		# $x, should be removed in riscv_check_mapping_symbols
addi	a0, zero, 2
.option pop

# last overlap next first
.section .text.zero.fill.align.B, "ax"
.option push
.option norelax
.align	2			# $xrv32ic, .align and .fill are in the different frag, so neither be removed,
				# but will be removed in riscv_check_mapping_symbols
.fill	1, 0, 0			# $d with zero size, removed in make_mapping_symbol when adding $xrv32ic
.option arch, -c
addi	a0, zero, 1		# $xrv32i
addi	a0, zero, 2
.option pop

.section .text.last.section, "ax"
.option push
.option norelax
.option arch, -c
addi	a0, zero, 1		# $xrv32i
.word	1			# $d
.align	2			# zero section padding, $x at the end of section, removed in riscv_check_mapping_symbols
.option pop

.section .text.section.padding, "ax"
.option push
.option norelax
.align	2
addi	a0, zero, 1		# $rv32ic
.option arch, +a
.align	2			# 2-byte padding, $x, removed
addi	a0, zero, 2		# $xrv32iac
.word	1			# $d
.option pop			# 2-byte padding, $x

.section .text.relax.align, "ax"
.option push
.option relax
.option arch, rv32ic
.balign	4			# $xrv32ic, add at the start of section
addi	a0, zero, 1		# $x, won't added
.option arch, -c
.align	3			# $x, won't added
addi	a0, zero, 2		# $xrv32i
.option pop
