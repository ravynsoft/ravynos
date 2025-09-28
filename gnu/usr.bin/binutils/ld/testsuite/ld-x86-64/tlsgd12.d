#name: TLS GD->IE transition check without PLT
#as: --64
#ld: -melf_x86_64
#error: .*TLS transition from R_X86_64_TLSGD to R_X86_64_GOTTPOFF against `foo'.*failed.*
