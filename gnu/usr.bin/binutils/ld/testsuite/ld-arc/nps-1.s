        .text
        .global __start
__start:
        xldb r10, [ foo ]
        xldw r10, [ foo ]
        xld r10, [ foo ]
        xstb r10, [ foo ]
        xstw r10, [ foo ]
        xst r10, [ foo ]

