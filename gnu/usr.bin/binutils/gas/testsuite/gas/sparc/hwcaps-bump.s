# Tests to check for the proper bumping of hardware capabilities
# associated to opcode architectures.
        .text
        wr	%g1, %g2, %mcdper
        wr	%g2, 0x3, %mwait
