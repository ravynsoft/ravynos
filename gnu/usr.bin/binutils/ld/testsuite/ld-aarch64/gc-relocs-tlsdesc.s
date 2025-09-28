        .global tlsdescvar
        .section        .tbss,"awT",%nobits
        .align  2
        .type   tlsdescvar, %object
        .size   tlsdescvar, 4
tlsdescvar:
        .zero   4

        .align  2
        .type   l_tlsdescvar, %object
        .size   l_tlsdescvar, 4
l_tlsdescvar:
        .zero   4
.text
_test_tls_desc:

        // R_AARCH64_TLSDESC_ADR_PAGE  tlsdescvar
        adrp  x0, :tlsdesc:tlsdescvar
        // R_AARCH64_TLSDESC_LD64_LO12 tlsdescvar
        ldr   x1, [x0, :tlsdesc_lo12:tlsdescvar]
        // R_AARCH64_TLSDESC_ADD_LO12  tlsdescvar
        add   x0, x0, :tlsdesc_lo12:tlsdescvar
        // R_AARCH64_TLSDESC_CALL      tlsdescvar
        .tlsdesccall tlsdescvar
        blr   x1

_test_tls_desc_local:

        // R_AARCH64_TLSDESC_ADR_PAGE  l_tlsdescvar
        adrp  x0, :tlsdesc:l_tlsdescvar
        // R_AARCH64_TLSDESC_LD64_LO12 l_tlsdescvar
        ldr   x1, [x0, :tlsdesc_lo12:l_tlsdescvar]
        // R_AARCH64_TLSDESC_ADD_LO12  l_tlsdescvar
        add   x0, x0, :tlsdesc_lo12:l_tlsdescvar
        // R_AARCH64_TLSDESC_CALL      l_tlsdescvar
        .tlsdesccall l_tlsdescvar
        blr   x1

