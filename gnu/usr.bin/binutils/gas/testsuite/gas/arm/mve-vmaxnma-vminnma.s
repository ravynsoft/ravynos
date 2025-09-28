.syntax unified
.thumb
vmaxnma.f16 q0, q1
vmaxnma.f16 q7, q7
vmaxnma.f32 q0, q1
vmaxnma.f32 q7, q7
vminnma.f16 q0, q1
vminnma.f16 q7, q7
vminnma.f32 q0, q1
vminnma.f32 q7, q7
vpstete
vmaxnmat.f16 q0, q1
vmaxnmae.f32 q7, q7
vminnmat.f32 q0, q1
vminnmae.f16 q7, q7
