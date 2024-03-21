#source: attr-gnu-4-2.s
#as: -msingle-float
#DUMPPROG: readelf
#readelf: -A
#name: MIPS gnu_attribute 4,2 (single precision)

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Hard float \(single precision\)

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: .*
CPR2 size: 0
FP ABI: Hard float \(single precision\)
ISA Extension: .*
ASEs:
#...
FLAGS 1: 0000000.
FLAGS 2: 00000000

