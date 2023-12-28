        .global tlslevar
        .section        .tbss,"awT",%nobits
        .align  2
        .type   tlslevar, %object
        .size   tlslevar, 4
tlslevar:
        .zero   4

        .align  2
        .type   l_tlslevar, %object
        .size   l_tlslevar, 4
l_tlslevar:
        .zero   4

.text
l_test_tls_le:

        // R_AARCH64_TLSLE_ADD_TPREL_LO12      tlslevar
        add  x0, x1, :tprel_lo12:tlslevar
        // R_AARCH64_TLSLE_ADD_TPREL_HI12      tlslevar
        add  x0, x1, :tprel_hi12:tlslevar
        // R_AARCH64_TLSLE_ADD_TPREL_HI12      tlslevar
        add  x0, x1, :tprel_hi12:tlslevar, lsl #12
        // R_AARCH64_TLSLE_ADD_TPREL_LO12_NC   tlslevar
        add  x0, x1, :tprel_lo12_nc:tlslevar

l_test_tls_le_local:

        // R_AARCH64_TLSLE_ADD_TPREL_LO12      l_tlslevar
        add  x0, x1, :tprel_lo12:l_tlslevar
        // R_AARCH64_TLSLE_ADD_TPREL_HI12      l_tlslevar
        add  x0, x1, :tprel_hi12:l_tlslevar
        // R_AARCH64_TLSLE_ADD_TPREL_HI12      l_tlslevar
        add  x0, x1, :tprel_hi12:l_tlslevar, lsl #12
        // R_AARCH64_TLSLE_ADD_TPREL_LO12_NC   l_tlslevar
        add  x0, x1, :tprel_lo12_nc:l_tlslevar
