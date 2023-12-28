#name: PLT PC-relative offset overflow check
#as: --64
#ld: -shared -melf_x86_64 -z max-page-size=0x200000 -z noseparate-code
#error: .*PC-relative offset overflow in PLT entry for `bar'
