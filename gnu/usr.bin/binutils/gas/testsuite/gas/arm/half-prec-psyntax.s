        .text
        vcvt    d0.f16, q1.f32
        vcvt    q5.f32, d6.f16
        vcvtt   s2.f32, s5.f16
        vcvtb   s2.f32, s5.f16
        vcvtt   s2.f16, s5.f32
        vcvtb   s2.f16, s5.f32
