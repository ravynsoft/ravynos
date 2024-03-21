.syntax unified

VLD1.8 {d0}, 1f
1:
VLD1.8 {D0}, R0
VLD1.8 {Q1}, R0
VLD1.8 {D0}, [PC]
VLD1.8 {D0}, [PC, #0]
VST1.8 {D0}, R0
VST1.8 {Q1}, R0
VST1.8 {D0}, [PC]
VST1.8 {D0}, [PC, #0]
VST1.8 {D0[]}, [R0]
VST2.8 {D0[], D2[]}, [R0]
VST3.16 {D0[], D1[], D2[]}, [R0]
VST4.32 {D0[], D1[], D2[], D3[]}, [R0]
VLD1.8 {Q0}, [R0, #8]
VLD1.8 {Q0}, [R0, #8]!
VLD1.8 {Q0}, [R0, R1]
VLD1.8 {Q0}, [R0, R1]!
.thumb
VLD1.8 {d0}, 2f
2:
VLD1.8 {D0}, R0
VLD1.8 {Q1}, R0
VLD1.8 {D0}, [PC]
VLD1.8 {D0}, [PC, #0]
VST1.8 {D0}, R0
VST1.8 {Q1}, R0
VST1.8 {D0}, [PC]
VST1.8 {D0}, [PC, #0]

VSHL.I8		d0, d0, #7
VSHL.I8		d0, d0, #8
VSHL.I16	d0, d0, #15
VSHL.I16	d0, d0, #16
VSHL.I32	d0, d0, #31
VSHL.I32	d0, d0, #32
VSHL.I64	d0, d0, #63
VSHL.I64	d0, d0, #64

VQSHL.S8	d0, d0, #7
VQSHL.S8	d0, d0, #8
VQSHL.S16	d0, d0, #15
VQSHL.S16	d0, d0, #16
VQSHL.S32	d0, d0, #31
VQSHL.S32	d0, d0, #32
VQSHL.S64	d0, d0, #63
VQSHL.S64	d0, d0, #64

VQSHLU.S8	d0, d0, #7
VQSHLU.S8	d0, d0, #8
VQSHLU.S16	d0, d0, #15
VQSHLU.S16	d0, d0, #16
VQSHLU.S32	d0, d0, #31
VQSHLU.S32	d0, d0, #32
VQSHLU.S64	d0, d0, #63
VQSHLU.S64	d0, d0, #64
