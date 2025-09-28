#name: Invalid Armv8.1-M pointer authentication and branch target identification extention
#skip: *-*-pe
#source: armv8_1-m-pacbti-bad.s
#as: -march=armv8.1-m.main
#error_output: armv8_1-m-pacbti-bad.l
