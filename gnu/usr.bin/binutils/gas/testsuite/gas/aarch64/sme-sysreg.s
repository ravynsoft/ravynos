/* Read SME system registers.  */
mrs x0, svcr
mrs x0, id_aa64smfr0_el1
mrs x0, smcr_el1
mrs x0, smcr_el12
mrs x0, smcr_el2
mrs x0, smcr_el3
mrs x0, smpri_el1
mrs x0, smprimap_el2
mrs x0, smidr_el1
mrs x0, tpidr2_el0
mrs x0, mpamsm_el1

/* Write to SME system registers.  */
msr svcr, x0
msr smcr_el1, x0
msr smcr_el12, x0
msr smcr_el2, x0
msr smcr_el3, x0
msr smpri_el1, x0
msr smprimap_el2, x0
msr tpidr2_el0, x0
msr mpamsm_el1, x0
