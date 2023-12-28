#as: -mfp64 -mno-odd-spreg -32
#source: empty.s
#DUMPPROG: readelf
#readelf: -A
#name: MIPS infer fpabi (O32 fp64 nooddspreg)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Hard float compat \(32-bit CPU, 64-bit FPU\)

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: 32
CPR1 size: 64
CPR2 size: 0
FP ABI: Hard float compat \(32-bit CPU, 64-bit FPU\)
ISA Extension: .*
ASEs:
#...
FLAGS 1: 00000000
FLAGS 2: 00000000

