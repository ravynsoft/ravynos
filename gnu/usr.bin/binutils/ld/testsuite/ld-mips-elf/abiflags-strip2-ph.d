#source: jr.s -mips32r2 -mfpxx
#source: jr.s -march=octeon -mfp64 RUN_OBJCOPY
#ld: -e 0
#objcopy_objects: -R .MIPS.abiflags
#objdump: -p

[^:]*:     file format elf32-.*

Program Header:
0x70000003 off    0x0000.... vaddr 0x004000.. paddr 0x004000.. align 2\*\*3
         filesz 0x00000018 memsz 0x00000018 flags r--
#...
private flags = 808b1100: \[abi=O32\] \[mips64r2\] \[32bitmode\]

MIPS ABI Flags Version: 0

ISA: MIPS64r2
GPR size: 32
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float \(32-bit CPU, 64-bit FPU\)
ISA Extension: Cavium Networks Octeon
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000

