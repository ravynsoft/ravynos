#source: undef-2.s
#source: start.s
#as: -x
#ld: -m elf64mmix
#error: \A[^\n\r]*undefined reference to `undefd'\Z
