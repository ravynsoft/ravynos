#DUMPPROG: readelf
#source: empty.s
#readelf: -A
#name: MIPS infer fpabi (double-precision)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Hard float \(double precision\)

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: .*
CPR2 size: 0
FP ABI: Hard float \(double precision\)
ISA Extension: .*
ASEs:
#...
FLAGS 1: 0000000.
FLAGS 2: 00000000

