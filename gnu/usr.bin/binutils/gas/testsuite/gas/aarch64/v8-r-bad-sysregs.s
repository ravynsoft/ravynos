// Invalid system registers for Armv8-R AArch64
.arch armv8-r

// No EL3 for Armv8-R
mrs x0, sctlr_el3
msr ttbr0_el3, x0
mrs x0, TCR_EL3

msr mpuir_el1, x0 // write to read-only register
msr mpuir_el2, x0 // write to read-only register

// Four sysregs are not in R-profile:
mrs x0, ttbr0_el2
msr ttbr0_el2, x0

mrs x0, ttbr1_el2
msr ttbr1_el2, x0

mrs x0, vsttbr_el2
msr vsttbr_el2, x0

mrs x0, vttbr_el2
msr vttbr_el2, x0
