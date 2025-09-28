// Memory Encryption Contexts, an extension of RME.

// Read from MEC system registers.
mrs x0, mecidr_el2
mrs x0, mecid_p0_el2
mrs x0, mecid_a0_el2
mrs x0, mecid_p1_el2
mrs x0, mecid_a1_el2
mrs x0, vmecid_p_el2
mrs x0, vmecid_a_el2
mrs x0, mecid_rl_a_el3

// Write to MEC system registers.
msr mecid_p0_el2, x0
msr mecid_a0_el2, x0
msr mecid_p1_el2, x0
msr mecid_a1_el2, x0
msr vmecid_p_el2, x0
msr vmecid_a_el2, x0
msr mecid_rl_a_el3, x0
