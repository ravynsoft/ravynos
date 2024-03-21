VMOVMI.F32 s27,#11
VMOVMI.F32 s27,#11.0
VMOVMI.F32 s27,#1
VMOVMI.F32 s27,#1.0
VMOVGT.F64 d1,#-20
VMOVGT.F64 d1,#-2
	@ Check that integer-encoded floating-point bit-patterns still work
	vmov s21,#0x40000000
	vmov s15,#0xc0b80000
	vmov  s9,#0xbe280000
