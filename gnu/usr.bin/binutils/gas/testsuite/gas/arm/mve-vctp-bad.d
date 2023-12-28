#name: Invalid MVE vctp instruction
#source: mve-vctp-bad.s
#as: -march=armv8.1-m.main+mve.fp -mfloat-abi=hard
#error_output: mve-vctp-bad.l
