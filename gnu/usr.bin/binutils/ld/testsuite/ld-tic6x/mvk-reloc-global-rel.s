.globl s0
.globl s7fff
.globl s80000000
.globl sffff8000
.globl sffffffff
.text
.nocmp
	mvkl .S1 s80000000,a1
	mvkl .S1 sffffffff,a1
	mvkl .S1 s0,a1
	mvkl .S1 sffffffff+0xffffffff,a1
	mvk .S1 sffff8000,a1
	mvk .S1 s0-0x8000,a1
	mvk .S1 s7fff,a1
