T:
## MRS ##

# MSP
mrs   r0, MSP
mrs   r0, MSP_NS
mrs   r0, msp
mrs   r0, msp_ns

# PSP
mrs   r1, PSP
mrs   r1, PSP_NS
mrs   r1, psp
mrs   r1, psp_ns

# MSPLIM
mrs   r2, MSPLIM
mrs   r2, MSPLIM_NS
mrs   r2, msplim
mrs   r2, msplim_ns

# PSPLIM
mrs   r3, PSPLIM
mrs   r3, PSPLIM_NS
mrs   r3, psplim
mrs   r3, psplim_ns

# PRIMASK
mrs   r4, PRIMASK
mrs   r4, PRIMASK_NS
mrs   r4, primask
mrs   r4, primask_ns

# BASEPRI
mrs   r5, BASEPRI
mrs   r5, BASEPRI_NS
mrs   r5, basepri
mrs   r5, basepri_ns

# FAULTMASK
mrs   r6, FAULTMASK
mrs   r6, FAULTMASK_NS
mrs   r6, faultmask
mrs   r6, faultmask_ns

# CONTROL
mrs   r7, CONTROL
mrs   r7, CONTROL_NS
mrs   r7, control
mrs   r7, control_ns

# SP_NS
mrs   r8, SP_NS
mrs   r8, sp_ns


## MSR ##

# MSP
msr   MSP,	    r0
msr   MSP_NS,	    r0
msr   msp,	    r0
msr   msp_ns,	    r0

# PSP
msr   PSP,	    r1
msr   PSP_NS,	    r1
msr   psp,	    r1
msr   psp_ns,	    r1

# MSPLIM
msr   MSPLIM,	    r2
msr   MSPLIM_NS,    r2
msr   msplim,	    r2
msr   msplim_ns,    r2

# PSPLIM
msr   PSPLIM,	    r3
msr   PSPLIM_NS,    r3
msr   psplim,	    r3
msr   psplim_ns,    r3

# PRIMASK
msr   PRIMASK,	    r4
msr   PRIMASK_NS,   r4
msr   primask,	    r4
msr   primask_ns,   r4

# BASEPRI
msr   BASEPRI,	    r5
msr   BASEPRI_NS,   r5
msr   basepri,	    r5
msr   basepri_ns,   r5

# FAULTMASK
msr   FAULTMASK,    r6
msr   FAULTMASK_NS, r6
msr   faultmask,    r6
msr   faultmask_ns, r6

# CONTROL
msr   CONTROL,	    r7
msr   CONTROL_NS,   r7
msr   control,	    r7
msr   control_ns,   r7

# SP_NS
msr   SP_NS,	    r8
msr   sp_ns,	    r8
