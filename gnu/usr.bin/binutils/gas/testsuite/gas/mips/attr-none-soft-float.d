#as: -msoft-float
#DUMPPROG: readelf
#source: empty.s
#readelf: -A
#name: MIPS infer fpabi (soft-precision)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Soft float

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: 0
CPR2 size: 0
FP ABI: Soft float
ISA Extension: .*
ASEs:
#...
FLAGS 1: 0000000.
FLAGS 2: 00000000

