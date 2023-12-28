#objdump: -pdr --prefix-addresses --show-raw-insn
#name: MIPS DSP ASE Rev3 for MIPS32
#as: -mdspr3 -32

# Check MIPS DSP ASE Rev3 for MIPS32 Instruction Assembly

.*: +file format .*mips.*
#...
ASEs:
#...
	DSP ASE
	DSP R2 ASE
	DSP R3 ASE
#...
FLAGS 1: .*
FLAGS 2: .*

Disassembly of section .text:
0+0000 <[^>]*> 0418ffff 	bposge32c	00000000 <text_label>
.*0: R_MIPS_PC16	text_label

	\.\.\.
