/* Call Stack Recorder Extension system registers.  */

/* Read from system registers.  */
mrs x0, csrcr_el0
mrs x0, csrptr_el0
mrs x0, csridr_el0
mrs x0, csrptridx_el0
mrs x0, csrcr_el1
mrs x0, csrcr_el12
mrs x0, csrptr_el1
mrs x0, csrptr_el12
mrs x0, csrptridx_el1
mrs x0, csrcr_el2
mrs x0, csrptr_el2
mrs x0, csrptridx_el2

/* Write to system registers.  */
msr csrcr_el0, x0
msr csrptr_el0, x0
msr csrcr_el1, x0
msr csrcr_el12, x0
msr csrptr_el1, x0
msr csrptr_el12, x0
msr csrcr_el2, x0
msr csrptr_el2, x0
