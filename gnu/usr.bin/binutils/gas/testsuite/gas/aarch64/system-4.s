/* Armv8.7-a DSB memory nXS barrier variant.  */
.arch armv8.7-a

    dsb #16
    dsb #20
    dsb #24
    dsb #28

    dsb oshnxs
    dsb nshnxs
    dsb ishnxs
    dsb synxs
