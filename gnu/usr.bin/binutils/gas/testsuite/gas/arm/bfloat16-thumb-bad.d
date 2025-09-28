#name: Bfloat 16 Thumb failure cases
#source: bfloat16-bad.s
#as: -mno-warn-deprecated -mthumb -march=armv8.6-a+simd
#error_output: bfloat16-thumb-bad.l
