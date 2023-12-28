#as: -32
#source: attr-gnu-4-6.s
#DUMPPROG: readelf
#readelf: -A
#name: MIPS gnu_attribute 4,6 (-mfp64)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Hard float \(32-bit CPU, 64-bit FPU\)

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: 32
CPR1 size: .*
CPR2 size: 0
FP ABI: Hard float \(32-bit CPU, 64-bit FPU\)
ISA Extension: .*
ASEs:
#...
FLAGS 1: 00000001
FLAGS 2: 00000000

