#name: bad MVE VLDR VSTR wrong error message for addressing mode without [].
#as: -march=armv8.1-m.main+mve.fp -mthumb -mfloat-abi=hard
#error_output: mve-vldr-vstr-bad.l

.*: +file format .*arm.*
