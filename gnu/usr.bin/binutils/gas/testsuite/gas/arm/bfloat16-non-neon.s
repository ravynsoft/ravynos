.syntax unified
vcvtb.bf16.f32 s20, s11
it ne
vcvtbne.bf16.f32 s11, s20
vcvtbal.bf16.f32 s0, s0
vcvtt.bf16.f32 s20, s11
it ne
vcvttne.bf16.f32 s11, s20
vcvttal.bf16.f32 s0, s0
