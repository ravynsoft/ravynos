/* Read from system register.  */
mrs x0, trbbaser_el1
mrs x0, trbidr_el1
mrs x0, trblimitr_el1
mrs x0, trbmar_el1
mrs x0, trbptr_el1
mrs x0, trbsr_el1
mrs x0, trbtrg_el1

/* Write to system register.  */
msr trbbaser_el1, x0
msr trblimitr_el1, x0
msr trbmar_el1, x0
msr trbptr_el1, x0
msr trbsr_el1, x0
msr trbtrg_el1, x0
