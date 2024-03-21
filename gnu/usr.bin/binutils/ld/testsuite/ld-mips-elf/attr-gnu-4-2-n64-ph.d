#source: empty.s -mips3 -msingle-float
#ld: -e 0
#objdump: -p

[^:]*:     file format elf64-.*mips.*

Program Header:
0x70000003 off    0x00000000000000b0 vaddr 0x00000001200000b0 paddr 0x00000001200000b0 align 2\*\*3
         filesz 0x0000000000000018 memsz 0x0000000000000018 flags r--
    LOAD off    0x0000000000000000 vaddr 0x0000000120000000 paddr 0x0000000120000000 align 2\*\*16
         filesz 0x00000000000000.. memsz 0x00000000000000.. flags r--
private flags = 20000000: \[abi=64\] \[mips3\] \[not 32bitmode\]

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
