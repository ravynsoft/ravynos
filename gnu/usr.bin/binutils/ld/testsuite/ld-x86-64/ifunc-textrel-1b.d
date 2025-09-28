#source: ifunc-textrel-1.s
#as: --64 -defsym __x86_64__=1
#ld: -m elf_x86_64 -shared
#warning: GNU indirect functions with DT_TEXTREL may result in a segfault at runtime; recompile with -fPIC
