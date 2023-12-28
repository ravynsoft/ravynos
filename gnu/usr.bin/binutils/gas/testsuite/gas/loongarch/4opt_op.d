#as-new:
#objdump: -dr

.*:[ 	]+file format .*


Disassembly of section .text:

00000000.* <.text>:
[ 	]+0:[ 	]+08118820 [ 	]+fmadd.s[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+4:[ 	]+08218820 [ 	]+fmadd.d[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+8:[ 	]+08518820 [ 	]+fmsub.s[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+c:[ 	]+08618820 [ 	]+fmsub.d[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+10:[ 	]+08918820 [ 	]+fnmadd.s[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+14:[ 	]+08a18820 [ 	]+fnmadd.d[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+18:[ 	]+08d18820 [ 	]+fnmsub.s[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+1c:[ 	]+08e18820 [ 	]+fnmsub.d[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fa3
[ 	]+20:[ 	]+0c100820 [ 	]+fcmp.caf.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+24:[ 	]+0c108820 [ 	]+fcmp.saf.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+28:[ 	]+0c110820 [ 	]+fcmp.clt.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+2c:[ 	]+0c118820 [ 	]+fcmp.slt.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+30:[ 	]+0c118820 [ 	]+fcmp.slt.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+34:[ 	]+0c120820 [ 	]+fcmp.ceq.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+38:[ 	]+0c128820 [ 	]+fcmp.seq.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+3c:[ 	]+0c130820 [ 	]+fcmp.cle.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+40:[ 	]+0c138820 [ 	]+fcmp.sle.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+44:[ 	]+0c138820 [ 	]+fcmp.sle.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+48:[ 	]+0c140820 [ 	]+fcmp.cun.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+4c:[ 	]+0c148820 [ 	]+fcmp.sun.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+50:[ 	]+0c150820 [ 	]+fcmp.cult.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+54:[ 	]+0c150820 [ 	]+fcmp.cult.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+58:[ 	]+0c158820 [ 	]+fcmp.sult.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+5c:[ 	]+0c160820 [ 	]+fcmp.cueq.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+60:[ 	]+0c168820 [ 	]+fcmp.sueq.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+64:[ 	]+0c170820 [ 	]+fcmp.cule.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+68:[ 	]+0c170820 [ 	]+fcmp.cule.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+6c:[ 	]+0c178820 [ 	]+fcmp.sule.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+70:[ 	]+0c180820 [ 	]+fcmp.cne.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+74:[ 	]+0c188820 [ 	]+fcmp.sne.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+78:[ 	]+0c1a0820 [ 	]+fcmp.cor.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+7c:[ 	]+0c1a8820 [ 	]+fcmp.sor.s[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+80:[ 	]+0c1c0820 [ 	]+fcmp.cune.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+84:[ 	]+0c1c8820 [ 	]+fcmp.sune.s [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+88:[ 	]+0c200820 [ 	]+fcmp.caf.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+8c:[ 	]+0c208820 [ 	]+fcmp.saf.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+90:[ 	]+0c210820 [ 	]+fcmp.clt.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+94:[ 	]+0c218820 [ 	]+fcmp.slt.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+98:[ 	]+0c218820 [ 	]+fcmp.slt.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+9c:[ 	]+0c220820 [ 	]+fcmp.ceq.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+a0:[ 	]+0c228820 [ 	]+fcmp.seq.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+a4:[ 	]+0c230820 [ 	]+fcmp.cle.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+a8:[ 	]+0c238820 [ 	]+fcmp.sle.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+ac:[ 	]+0c238820 [ 	]+fcmp.sle.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+b0:[ 	]+0c240820 [ 	]+fcmp.cun.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+b4:[ 	]+0c248820 [ 	]+fcmp.sun.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+b8:[ 	]+0c250820 [ 	]+fcmp.cult.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+bc:[ 	]+0c250820 [ 	]+fcmp.cult.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+c0:[ 	]+0c258820 [ 	]+fcmp.sult.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+c4:[ 	]+0c260820 [ 	]+fcmp.cueq.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+c8:[ 	]+0c268820 [ 	]+fcmp.sueq.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+cc:[ 	]+0c270820 [ 	]+fcmp.cule.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+d0:[ 	]+0c270820 [ 	]+fcmp.cule.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+d4:[ 	]+0c278820 [ 	]+fcmp.sule.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+d8:[ 	]+0c280820 [ 	]+fcmp.cne.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+dc:[ 	]+0c288820 [ 	]+fcmp.sne.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+e0:[ 	]+0c2a0820 [ 	]+fcmp.cor.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+e4:[ 	]+0c2a8820 [ 	]+fcmp.sor.d[ 	]+[ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+e8:[ 	]+0c2c0820 [ 	]+fcmp.cune.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+ec:[ 	]+0c2c8820 [ 	]+fcmp.sune.d [ 	]+\$fcc0, \$fa1, \$fa2
[ 	]+f0:[ 	]+0d000820 [ 	]+fsel[ 	]+[ 	]+\$fa0, \$fa1, \$fa2, \$fcc0
