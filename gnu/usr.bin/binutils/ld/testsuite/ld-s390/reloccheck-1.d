#as: -m64 -mzarch -march=z900
#ld: -m elf64_s390 -e start -static
#error: .*misaligned symbol.*
