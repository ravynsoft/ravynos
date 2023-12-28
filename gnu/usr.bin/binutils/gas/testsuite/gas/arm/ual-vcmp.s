.text
.arm
.syntax unified

vcmp.f32	s0, #0.0
vcmp.f32	s1, #0
vcmpe.f32	s3, #0.0
vcmpe.f32	s4, #0
vcmp.f32	s5, #0.0e2
vcmp.f32	s6, #0e-3
vcmpe.f32	s7, #0.0000
vcmpe.f32	s8, #.0
vcmp.f32        s9, #0x0
vcmpe.f32       s10, #0x0

vcmp.f64	d0, #0.0
vcmp.f64	d1, #0
vcmpe.f64	d2, #0.0
vcmpe.f64	d3, #0
vcmp.f64	d4, #0.0e5
vcmp.f64	d5, #0e-2
vcmpe.f64	d6, #0.00
vcmpe.f64	d7, #.0
vcmp.f64        d8, #0x0
vcmpe.f64       d9, #0x0
