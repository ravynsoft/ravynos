#name: TLS GD->LE transition check without PLT
#as: --32 -mrelax-relocations=yes
#ld: -melf_i386
#error: .*TLS transition from R_386_TLS_GD to R_386_TLS_LE_32 against `foo'.*failed.*
