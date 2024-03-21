.text
.arm
.syntax unified

vcmp.f32        s0, #0.01
vcmp.f32        s1, #2
vcmpe.f32       s3, 5
vcmpe.f32       s4, #-0.0

vcmp.f64        d0, #-1
vcmpe.f64       d3, #0x35
vcmpe.f64       d4, 0xf
