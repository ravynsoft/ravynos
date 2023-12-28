#as: -msoft-float
#source: attr-gnu-4-3.s
#DUMPPROG: readelf
#readelf: -A
#name: MIPS gnu_attribute 4,3 (-msoft-float)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Soft float

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: .*
CPR2 size: 0
FP ABI: Soft float
ISA Extension: .*
ASEs:
#...
FLAGS 1: 0000000.
FLAGS 2: 00000000

