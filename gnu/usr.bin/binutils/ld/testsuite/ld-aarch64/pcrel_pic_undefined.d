#name: PC-Rel relocation against undefined
#source: pcrel.s
#target: [check_shared_lib_support]
#ld: -shared -e0 -defsym global_b=0x2000
#error: .*: relocation R_AARCH64_ADR_PREL_PG_HI21 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_ADR_PREL_PG_HI21_NC against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_ADR_PREL_LO21 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_LD_PREL_LO19 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_PREL16 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_PREL32 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
#error: .*: relocation R_AARCH64_PREL64 against symbol `global_a.*bind externally.*fPIC.*
#error: .*: dangerous relocation: unsupported relocation
