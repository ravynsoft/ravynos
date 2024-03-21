#source: csr-insns-pseudo.s
#as: -march=rv32if
#objdump: -dr -Mno-aliases

.*:[ 	]+file format .*


Disassembly of section .text:

0+000 <pseudo_csr_insn>:
[ 	]+[0-9a-f]+:[ 	]+000022f3[ 	]+csrrs[ 	]+t0,ustatus,zero
[ 	]+[0-9a-f]+:[ 	]+00029073[ 	]+csrrw[ 	]+zero,ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+0002a073[ 	]+csrrs[ 	]+zero,ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+0002b073[ 	]+csrrc[ 	]+zero,ustatus,t0
[ 	]+[0-9a-f]+:[ 	]+000fd073[ 	]+csrrwi[ 	]+zero,ustatus,31
[ 	]+[0-9a-f]+:[ 	]+000fe073[ 	]+csrrsi[ 	]+zero,ustatus,31
[ 	]+[0-9a-f]+:[ 	]+000ff073[ 	]+csrrci[ 	]+zero,ustatus,31
[ 	]+[0-9a-f]+:[ 	]+c00022f3[ 	]+csrrs[ 	]+t0,cycle,zero
[ 	]+[0-9a-f]+:[ 	]+c01022f3[ 	]+csrrs[ 	]+t0,time,zero
[ 	]+[0-9a-f]+:[ 	]+c02022f3[ 	]+csrrs[ 	]+t0,instret,zero
[ 	]+[0-9a-f]+:[ 	]+c80022f3[ 	]+csrrs[ 	]+t0,cycleh,zero
[ 	]+[0-9a-f]+:[ 	]+c81022f3[ 	]+csrrs[ 	]+t0,timeh,zero
[ 	]+[0-9a-f]+:[ 	]+c82022f3[ 	]+csrrs[ 	]+t0,instreth,zero
[ 	]+[0-9a-f]+:[ 	]+003022f3[ 	]+csrrs[ 	]+t0,fcsr,zero
[ 	]+[0-9a-f]+:[ 	]+003392f3[ 	]+csrrw[ 	]+t0,fcsr,t2
[ 	]+[0-9a-f]+:[ 	]+00339073[ 	]+csrrw[ 	]+zero,fcsr,t2
[ 	]+[0-9a-f]+:[ 	]+002022f3[ 	]+csrrs[ 	]+t0,frm,zero
[ 	]+[0-9a-f]+:[ 	]+002312f3[ 	]+csrrw[ 	]+t0,frm,t1
[ 	]+[0-9a-f]+:[ 	]+00231073[ 	]+csrrw[ 	]+zero,frm,t1
[ 	]+[0-9a-f]+:[ 	]+002fd2f3[ 	]+csrrwi[ 	]+t0,frm,31
[ 	]+[0-9a-f]+:[ 	]+002fd073[ 	]+csrrwi[ 	]+zero,frm,31
[ 	]+[0-9a-f]+:[ 	]+001022f3[ 	]+csrrs[ 	]+t0,fflags,zero
[ 	]+[0-9a-f]+:[ 	]+001312f3[ 	]+csrrw[ 	]+t0,fflags,t1
[ 	]+[0-9a-f]+:[ 	]+00131073[ 	]+csrrw[ 	]+zero,fflags,t1
[ 	]+[0-9a-f]+:[ 	]+001fd2f3[ 	]+csrrwi[ 	]+t0,fflags,31
[ 	]+[0-9a-f]+:[ 	]+001fd073[ 	]+csrrwi[ 	]+zero,fflags,31
