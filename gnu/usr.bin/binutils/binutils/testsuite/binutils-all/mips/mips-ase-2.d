#PROG: objcopy
#objdump: -dp --prefix-addresses --show-raw-insn
#name: MIPS ELF file ASE information interpretation for disassembly 2

# Verify that in the presence of its ASE flag MDMX code is disassembled
# with MIPS64r2, where MDMX presence is not implied.

.*: +file format .*mips.*
private flags = .[8-f]......: .*mdmx.*

MIPS ABI Flags Version: 0

ISA: MIPS64r2
GPR size: 32
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float \(32-bit CPU, 64-bit FPU\)
ISA Extension: None
ASEs:
	MDMX ASE
FLAGS 1: .*
FLAGS 2: .*

Disassembly of section \.text:
[0-9a-f]+ <[^>]*> 7aa2080b 	add\.qh	\$v0,\$v1,\$v2
[0-9a-f]+ <[^>]*> 46c520c0 	add\.ps	\$f3,\$f4,\$f5
[0-9a-f]+ <[^>]*> 46c83998 	addr\.ps	\$f6,\$f7,\$f8
	\.\.\.
