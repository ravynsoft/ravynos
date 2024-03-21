        .global tlsgdvar
        .section        .tbss,"awT",%nobits
        .align  2
        .type   tlsgdvar, %object
        .size   tlsgdvar, 4
tlsgdvar:
        .zero   4
        .align  2
        .type   l_tlsgdvar, %object
        .size   l_tlsgdvar, 4
l_tlsgdvar:
        .zero   4

.text
_test_tls_gd:
        // R_AARCH64_TLSGD_ADR_PAGE21         tldgdvar
        adrp x0, :tlsgd:tlsgdvar
        // R_AARCH64_TLSGD_ADD_LO12_NC        tlsgdvar
        add  x0, x0, :tlsgd_lo12:tlsgdvar
        // R_AARCH64_CALL26
        bl   __tls_get_addr

_test_tls_gd_local:
       // R_AARCH64_TLSGD_ADR_PAGE21         l_tldgdvar
        adrp x0, :tlsgd:l_tlsgdvar
        // R_AARCH64_TLSGD_ADD_LO12_NC        l_tlsgdvar
        add  x0, x0, :tlsgd_lo12:l_tlsgdvar
        // R_AARCH64_CALL26
        bl   __tls_get_addr

