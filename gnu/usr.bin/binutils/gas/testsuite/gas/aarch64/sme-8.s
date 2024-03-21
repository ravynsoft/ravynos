/* SME mode selection and state access instructions.  */

/* SVCR system register access.  */
mrs x0, svcr
msr svcr, x0

/* MSR SVCR Immediate access.  */
msr svcrsm, #0
msr svcrza, #0
msr svcrsmza, #0

msr svcrsm, #1
msr svcrza, #1
msr svcrsmza, #1

/* SMSTART.  */
smstart
smstart sm
smstart za
smstart SM
smstart ZA

/* SMSTOP.  */
smstop
smstop sm
smstop za
smstop SM
smstop ZA
