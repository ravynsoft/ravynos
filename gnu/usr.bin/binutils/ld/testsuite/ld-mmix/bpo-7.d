#source: start.s
#source: bpo-4.s
#source: greg-1.s
#as: -linker-allocated-gregs
#ld: -m elf64mmix
#error: too many global registers: 224
