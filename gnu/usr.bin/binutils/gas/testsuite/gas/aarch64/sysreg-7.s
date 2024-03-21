.arch armv8-a+lor

/* Read from system registers.  */
mrs x0, lorc_el1
mrs x0, lorea_el1
mrs x0, lorn_el1
mrs x0, lorsa_el1
mrs x0, icc_ctlr_el3
mrs x0, icc_sre_el1
mrs x0, icc_sre_el2
mrs x0, icc_sre_el3
mrs x0, ich_vtr_el2

/* Write to system registers.  */
msr lorc_el1, x0
msr lorea_el1, x0
msr lorn_el1, x0
msr lorsa_el1, x0
msr icc_ctlr_el3, x0
msr icc_sre_el1, x0
msr icc_sre_el2, x0
msr icc_sre_el3, x0
