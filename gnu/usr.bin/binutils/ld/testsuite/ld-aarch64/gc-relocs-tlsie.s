        .global tlsievar
        .section        .tbss,"awT",%nobits
        .align  2
        .type   tlsievar, %object
        .size   tlsievar, 4
tlsievar:
        .zero   4

       .align  2
        .type   l_tlsievar, %object
        .size   l_tlsievar, 4
l_tlsievar:
        .zero   4

.text
_test_tls_IE:

        // R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21 tlsievar
        adrp x0, :gottprel:tlsievar
        // R_AARCH64_TLSIE_GOTTPREL_LO12_NC    tlsievar
        ldr  x0, [x0, :gottprel_lo12:tlsievar]

_test_tls_IE_local:

        // R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21 l_tlsievar
        adrp x0, :gottprel:l_tlsievar
        // R_AARCH64_TLSIE_GOTTPREL_LO12_NC    l_tlsievar
        ldr  x0, [x0, :gottprel_lo12:l_tlsievar]

