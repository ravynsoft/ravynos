/* Missing +lor.  */
mrs x0, lorc_el1
mrs x0, lorea_el1
mrs x0, lorn_el1
mrs x0, lorsa_el1

/* Write to R/O system registers.  */
msr ich_vtr_el2, x0
