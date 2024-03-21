	.syntax unified
	.arch armv8-a
	.arch_extension pan

	.arm
A1:
        setpan #0
        setpan #1

        .thumb
T1:
        setpan #0
        setpan #1

