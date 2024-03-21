#name: Bfloat 16 bad FPU
#source: bfloat16-neon.s
#as: -mno-warn-deprecated -mfpu=vfpxd -march=armv8.6-a
#error: .*Error: selected FPU does not support instruction.*

