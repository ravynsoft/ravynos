#source: erratum843419-far.s
#as:
#ld: -Ttext=0x400000 --fix-cortex-a53-843419=adr
#error: .*: erratum 843419 immediate 0x7ffff004 out of range for ADR \(input file too large\) and \-\-fix\-cortex\-a53\-843419=adr used\.  Run the linker with \-\-fix\-cortex\-a53\-843419=full instead.*
#...
