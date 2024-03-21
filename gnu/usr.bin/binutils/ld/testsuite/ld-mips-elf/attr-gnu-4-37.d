#source: attr-gnu-4-3.s -mips32r2
#source: attr-gnu-4-7.s
#ld: -r
#readelf: -A
#warning: warning: .* uses -msoft-float \(set by .*\), .* uses -mhard-float

Attribute Section: gnu
File Attributes
  Tag_GNU_MIPS_ABI_FP: Soft float

MIPS ABI Flags Version: 0

ISA: MIPS.*
GPR size: .*
CPR1 size: .*
CPR2 size: 0
FP ABI: Soft float
ISA Extension: None
ASEs:
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000
