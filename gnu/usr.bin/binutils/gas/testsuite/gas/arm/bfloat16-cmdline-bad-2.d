#name: Bfloat 16 bad processor
#source: bfloat16-non-neon.s
#as: -mno-warn-deprecated -march=armv8.5-a+simd
#error: .*Error: selected processor does not support bf16 instruction.*
