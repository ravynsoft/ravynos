#source: pr18801.s
#as: --64
#ld: -melf_x86_64 -shared
#warning: GNU indirect functions with DT_TEXTREL may result in a segfault at runtime; recompile with -fPIC
