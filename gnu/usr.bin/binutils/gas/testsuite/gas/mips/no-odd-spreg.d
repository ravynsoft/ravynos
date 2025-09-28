#DUMPPROG: readelf
#source: empty.s
#as: -32 -mno-odd-spreg
#readelf: -A
#name: -mno-odd-spreg test

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: .*

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: .*
CPR2 size: 0
FP ABI: .*
ISA Extension: .*
ASEs:
#...
FLAGS 1: 00000000
FLAGS 2: 00000000
