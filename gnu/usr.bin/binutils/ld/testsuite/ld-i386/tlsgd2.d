#name: TLS GD->LE transition check
#as: --32
#ld: -melf_i386
#error: .*TLS transition from R_386_TLS_GD to R_386_TLS_LE_32 against `foo'.*failed.*
