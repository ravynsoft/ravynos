# Test onadd/onsub/onmul/ondiv
        .text
        onadd	%f0, %f8, %f16
        onsub	%f8, %f16, %f24
        onmul	%f32, %f24, %f16
        ondiv	%f8, %f0, %f8
