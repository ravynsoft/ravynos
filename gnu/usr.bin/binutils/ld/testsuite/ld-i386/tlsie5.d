#name: TLS IE->LE transition check (R_386_TLS_IE)
#as: --32
#ld: -melf_i386
#error: .*TLS transition from R_386_TLS_IE to R_386_TLS_LE_32 against `foo'.*failed.*
