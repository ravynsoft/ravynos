.syntax unified
.text
.arch armv8-a         @ SET BASE
.arch_extension fp    @ ADD FP-ARMV8
vfms.f32 s0, s1, s2   @ OK

.arch armv8-a         @ RESET
vfms.f32 s0, s1, s2   @ ERROR

.fpu vfpv2            @ SET VFPV2
vfms.f32 s0, s1, s2   @ ERROR

.arch armv8-a         @ RESET
.fpu fp-armv8         @ ADD FP-ARMV8
vfms.f32 s0, s1, s2   @ OK
.fpu vfpv2            @ RESET to VFPV2
vfms.f32 s0, s1, s2   @ ERROR
