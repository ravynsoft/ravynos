#name: Int8 Matrix Multiply extension (Armv8.8-A)
#source: i8mm.s
#as: -mno-warn-deprecated -march=armv8.8-a+i8mm+simd -I$srcdir/$subdir
#objdump: -dr --show-raw-insn
#...
