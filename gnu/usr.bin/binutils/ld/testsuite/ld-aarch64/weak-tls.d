#source: weak-tls.s
#ld: -e0 --emit-relocs
#objdump: -dr
#...
#error:.*: warning: Weak TLS is implementation defined and may not work as expected.*
#error:.*: warning: Weak TLS is implementation defined and may not work as expected.*
#error:.*: in function `get':.*
#error:.*: relocation truncated to fit: R_AARCH64_TLSLD_ADD_DTPREL_LO12 against undefined symbol `dtl'.*
