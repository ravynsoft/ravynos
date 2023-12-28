#source: attr-gnu-4-2.s
#source: attr-gnu-4-8.s -W
#ld: -r
#readelf: -A
#warning: warning: .* uses -msingle-float \(set by .*\), .* uses unknown floating point ABI 8

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
	None
FLAGS 1: 0000000.
FLAGS 2: 00000000
