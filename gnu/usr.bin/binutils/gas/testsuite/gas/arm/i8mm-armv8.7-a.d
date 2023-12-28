#name: Int8 Matrix Multiply extension (Armv8.7-A)
#source: i8mm.s
#as: -mno-warn-deprecated -march=armv8.7-a+i8mm+simd -I$srcdir/$subdir
#objdump: -dr --show-raw-insn
#...
