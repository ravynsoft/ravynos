#source: csr-insns-pseudo.s
#as: -march=rv32i_zfinx
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <pseudo_csr_insn>:
[ 	]+[0-9a-f]+:[ 	]+000022f3[ 	]+csrr[ 	]+t0,ustatus
[ 	]+[0-9a-f]+:[ 	]+00029073[ 	]+csrw[ 	]+ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+0002a073[ 	]+csrs[ 	]+ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+0002b073[ 	]+csrc[ 	]+ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+000fd073[ 	]+csrw[ 	]+ustatus,31
[ 	]+[0-9a-f]+:[ 	]+000fe073[ 	]+csrs[ 	]+ustatus,31
[ 	]+[0-9a-f]+:[ 	]+000ff073[ 	]+csrc[ 	]+ustatus,31
[ 	]+[0-9a-f]+:[ 	]+c00022f3[ 	]+rdcycle[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+c01022f3[ 	]+rdtime[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+c02022f3[ 	]+rdinstret[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+c80022f3[ 	]+rdcycleh[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+c81022f3[ 	]+rdtimeh[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+c82022f3[ 	]+rdinstreth[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+003022f3[ 	]+frcsr[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+003392f3[ 	]+fscsr[ 	]+t0,t2
[ 	]+[0-9a-f]+:[ 	]+00339073[ 	]+fscsr[ 	]+t2
[ 	]+[0-9a-f]+:[ 	]+002022f3[ 	]+frrm[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+002312f3[ 	]+fsrm[ 	]+t0,t1
[ 	]+[0-9a-f]+:[ 	]+00231073[ 	]+fsrm[ 	]+t1
[ 	]+[0-9a-f]+:[ 	]+002fd2f3[ 	]+fsrmi[ 	]+t0,31
[ 	]+[0-9a-f]+:[ 	]+002fd073[ 	]+fsrmi[ 	]+zero,31
[ 	]+[0-9a-f]+:[ 	]+001022f3[ 	]+frflags[ 	]+t0
[ 	]+[0-9a-f]+:[ 	]+001312f3[ 	]+fsflags[ 	]+t0,t1
[ 	]+[0-9a-f]+:[ 	]+00131073[ 	]+fsflags[ 	]+t1
[ 	]+[0-9a-f]+:[ 	]+001fd2f3[ 	]+fsflagsi[ 	]+t0,31
[ 	]+[0-9a-f]+:[ 	]+001fd073[ 	]+fsflagsi[ 	]+zero,31
