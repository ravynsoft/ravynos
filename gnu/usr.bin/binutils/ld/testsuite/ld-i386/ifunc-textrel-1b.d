#source: ../ld-x86-64/ifunc-textrel-1.s
#as: --32
#ld: -m elf_i386 -shared
#warning: GNU indirect functions with DT_TEXTREL may result in a segfault at runtime; recompile with -fPIC
