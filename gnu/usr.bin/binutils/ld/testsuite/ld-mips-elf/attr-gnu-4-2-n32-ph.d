#source: empty.s -mips3 -msingle-float
#ld: -e 0
#objdump: -p

[^:]*:     file format elf32-n.*mips.*

Program Header:
0x70000003 off    0x000000.. vaddr 0x100000.. paddr 0x100000.. align 2\*\*3
         filesz 0x00000018 memsz 0x00000018 flags r--
#...
    LOAD off    0x00000000 vaddr 0x10000000 paddr 0x10000000 align 2\*\*16
         filesz 0x000000.. memsz 0x000000.. flags r--
private flags = 20000020: \[abi=N32\] \[mips3\] \[not 32bitmode\]

MIPS ABI Flags Version: 0

ISA: MIPS3
GPR size: 64
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float \(single precision\)
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000
