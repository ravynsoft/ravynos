#source: pr19939.s
#as: --64
#ld: -melf_x86_64 -shared -z defs
#error: undefined reference to `foo'
