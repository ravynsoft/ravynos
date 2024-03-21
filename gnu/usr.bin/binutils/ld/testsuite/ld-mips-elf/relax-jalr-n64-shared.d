#name: MIPS relax-jalr-shared n64
#source: relax-jalr.s
#as: -KPIC
#objdump: --prefix-addresses -d --show-raw-insn
#ld: -shared
#target: [check_shared_lib_support]

.*:     file format elf.*mips.*

Disassembly of section \.text:
	\.\.\.
	\.\.\.
.*	ld	t9,.*
.*	jalr	t9
.*	nop
	\.\.\.
.*	ld	t9,.*
.*	jalr	t9
.*	nop
	\.\.\.
.*	ld	t9,.*
.*	bal	.* <__start>
.*	nop
	\.\.\.
