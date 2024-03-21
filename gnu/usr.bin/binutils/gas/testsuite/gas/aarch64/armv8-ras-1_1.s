/* Armv8-A RAS 1.1 extension system registers.

Please note that early Armv8-a architectures do not officially support RAS
extension.

Certain use cases require developers to enable only more generic architecture
(e.g. -march=armv8-a) during system development. Users must use RAS extension
registers bearing in mind that system they use must support it.  */

/* Arm8-A.  */
.arch armv8-a

    /* RAS 1.1 Read/Write registers.  */
    mrs x0, erxmisc2_el1
    mrs x0, erxmisc3_el1
    mrs x0, erxpfgcdn_el1
    mrs x0, erxpfgctl_el1
    msr erxmisc2_el1, x0
    msr erxmisc3_el1, x0
    msr erxpfgcdn_el1, x0
    msr erxpfgctl_el1, x0

    /* RAS 1.1 Read-only registers.  */
    mrs x0, erxpfgf_el1

/* Armv8-A + RAS.  */
.arch armv8-a+ras
    /* RAS 1.1 Read/Write registers.  */
    mrs x0, erxmisc2_el1
    mrs x0, erxmisc3_el1
    mrs x0, erxpfgcdn_el1
    mrs x0, erxpfgctl_el1
    msr erxmisc2_el1, x0
    msr erxmisc3_el1, x0
    msr erxpfgcdn_el1, x0
    msr erxpfgctl_el1, x0

    /* RAS 1.1 Read-only registers.  */
    mrs x0, erxpfgf_el1
