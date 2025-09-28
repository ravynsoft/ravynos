gcsrrd	$r0, 1
gcsrwr	$r0, 1
gcsrxchg	$r0, $r1, 1
gtlbflush
hvcl	1
movgr2scr	$scr0, $r1
movscr2gr	$r0, $scr1
jiscr0	100
jiscr1	100
addu12i.w	$r0, $r1, 1
addu12i.d	$r0, $r1, 1
adc.b	$r0, $r1, $r2
adc.h	$r0, $r1, $r2
adc.w	$r0, $r1, $r2
adc.d	$r0, $r1, $r2
sbc.b	$r0, $r1, $r2
sbc.h	$r0, $r1, $r2
sbc.w	$r0, $r1, $r2
sbc.d	$r0, $r1, $r2
rotr.b	$r0, $r1, $r2
rotr.h	$r0, $r1, $r2
rotri.b	$r0, $r1, 1
rotri.h	$r0, $r1, 1
rcr.b	$r0, $r1, $r2
rcr.h	$r0, $r1, $r2
rcr.w	$r0, $r1, $r2
rcr.d	$r0, $r1, $r2
rcri.b	$r0, $r1, 1
rcri.h	$r0, $r1, 1
rcri.w	$r0, $r1, 1
rcri.d	$r0, $r1, 1
fcvt.ud.d	$f0, $f1
fcvt.ld.d	$f0, $f1
fcvt.d.ld	$f0, $f1, $f2
ldl.d	$r0, $r1, 1
ldl.w	$r0, $r1, 1
ldr.w	$r0, $r1, 1
ldr.d	$r0, $r1, 1
stl.w	$r0, $r1, 1
stl.d	$r0, $r1, 1
str.w	$r0, $r1, 1
str.d	$r0, $r1, 1
x86adc.b	$r0, $r1
x86adc.h	$r0, $r1
x86adc.w	$r0, $r1
x86adc.d	$r0, $r1
x86add.b	$r0, $r1
x86add.h	$r0, $r1
x86add.w	$r0, $r1
x86add.d	$r0, $r1
x86add.wu	$r0, $r1
x86add.du	$r0, $r1
x86inc.b	$r0
x86inc.h	$r0
x86inc.w	$r0
x86inc.d	$r0
x86sbc.b	$r0, $r1
x86sbc.h	$r0, $r1
x86sbc.w	$r0, $r1
x86sbc.d	$r0, $r1
x86sub.b	$r0, $r1
x86sub.h	$r0, $r1
x86sub.w	$r0, $r1
x86sub.d	$r0, $r1
x86sub.wu	$r0, $r1
x86sub.du	$r0, $r1
x86dec.b	$r0
x86dec.h	$r0
x86dec.w	$r0
x86dec.d	$r0
x86and.b	$r0, $r1
x86and.h	$r0, $r1
x86and.w	$r0, $r1
x86and.d	$r0, $r1
x86or.b	$r0, $r1
x86or.h	$r0, $r1
x86or.w	$r0, $r1
x86or.d	$r0, $r1
x86xor.b	$r0, $r1
x86xor.h	$r0, $r1
x86xor.w	$r0, $r1
x86xor.d	$r0, $r1
x86mul.b	$r0, $r1
x86mul.h	$r0, $r1
x86mul.w	$r0, $r1
x86mul.d	$r0, $r1
x86mul.bu	$r0, $r1
x86mul.hu	$r0, $r1
x86mul.wu	$r0, $r1
x86mul.du	$r0, $r1
x86rcl.b	$r0, $r1
x86rcl.h	$r0, $r1
x86rcl.w	$r0, $r1
x86rcl.d	$r0, $r1
x86rcli.b	$r0, 1
x86rcli.h	$r0, 1
x86rcli.w	$r0, 1
x86rcli.d	$r0, 1
x86rcr.b	$r0, $r1
x86rcr.h	$r0, $r1
x86rcr.w	$r0, $r1
x86rcr.d	$r0, $r1
x86rcri.b	$r0, 1
x86rcri.h	$r0, 1
x86rcri.w	$r0, 1
x86rcri.d	$r0, 1
x86rotl.b	$r0, $r1
x86rotl.h	$r0, $r1
x86rotl.w	$r0, $r1
x86rotl.d	$r0, $r1
x86rotli.b	$r0, 1
x86rotli.h	$r0, 1
x86rotli.w	$r0, 1
x86rotli.d	$r0, 1
x86rotr.b	$r0, $r1
x86rotr.h	$r0, $r1
x86rotr.d	$r0, $r1
x86rotr.w	$r0, $r1
x86rotri.b	$r0, 1
x86rotri.h	$r0, 1
x86rotri.w	$r0, 1
x86rotri.d	$r0, 1
x86sll.b	$r0, $r1
x86sll.h	$r0, $r1
x86sll.w	$r0, $r1
x86sll.d	$r0, $r1
x86slli.b	$r0, 1
x86slli.h	$r0, 1
x86slli.w	$r0, 1
x86slli.d	$r0, 1
x86srl.b	$r0, $r1
x86srl.h	$r0, $r1
x86srl.w	$r0, $r1
x86srl.d	$r0, $r1
x86srli.b	$r0, 1
x86srli.h	$r0, 1
x86srli.w	$r0, 1
x86srli.d	$r0, 1
x86sra.b	$r0, $r1
x86sra.h	$r0, $r1
x86sra.w	$r0, $r1
x86sra.d	$r0, $r1
x86srai.b	$r0, 1
x86srai.h	$r0, 1
x86srai.w	$r0, 1
x86srai.d	$r0, 1
setx86j	$r0, 1
setx86loope	$r0, $r1
setx86loopne	$r0, $r1
x86mfflag	$r0, 1
x86mtflag	$r0, 1
x86mftop	$r0
x86mttop	1
x86inctop
x86dectop
x86settm
x86clrtm
x86settag	$r0, 1, 1
armadd.w	$r0, $r1, 1
armsub.w	$r0, $r1, 1
armadc.w	$r0, $r1, 1
armsbc.w	$r0, $r1, 1
armand.w	$r0, $r1, 1
armor.w	$r0, $r1, 1
armxor.w	$r0, $r1, 1
armnot.w	$r0, 1
armsll.w	$r0, $r1, 1
armsrl.w	$r0, $r1, 1
armsra.w	$r0, $r1, 1
armrotr.w	$r0, $r1, 1
armslli.w	$r0, 1, 1
armsrli.w	$r0, 1, 1
armsrai.w	$r0, 1, 1
armrotri.w	$r0, 1, 1
armrrx.w	$r0, 1
armmove	$r0, $r1, 1
armmov.w	$r0, 1
armmov.d	$r0, 1
armmfflag	$r0, 1
armmtflag	$r0, 1
setarmj	$r0, 1
