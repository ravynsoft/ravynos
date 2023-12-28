#source: undef-tls.s
#ld: -e0 --emit-relocs
#objdump: -dr
#...
#error:.*: in function `get':.*
#error:.*: undefined reference to `tls'.*
#error:.*: TLS relocation R_AARCH64_TLSLE_ADD_TPREL_HI12 against undefined symbol `tls'.*
#error:.*: dangerous relocation: unsupported relocation.*
#error:.*: undefined reference to `tls'.*
#error:.*: TLS relocation R_AARCH64_TLSLE_ADD_TPREL_LO12_NC against undefined symbol `tls'.*
#error:.*: dangerous relocation: unsupported relocation.*
#error:.*: undefined reference to `dtl'.*
#error:.*: TLS relocation R_AARCH64_TLSLD_ADD_DTPREL_HI12 against undefined symbol `dtl'.*
#error:.*: dangerous relocation: unsupported relocation.*
#error:.*: undefined reference to `dtl'.*
#error:.*: TLS relocation R_AARCH64_TLSLD_ADD_DTPREL_LO12 against undefined symbol `dtl'.*
#error:.*: dangerous relocation: unsupported relocation.*
