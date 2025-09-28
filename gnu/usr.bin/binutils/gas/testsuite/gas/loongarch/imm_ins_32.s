.equ a, 0x123
.equ d, 2
.equ e,0x100

li.w $r12, a

alsl.w $r11,$r12,$r13,d
alsl.wu $r11,$r12,$r13,d
bytepick.w $r11,$r12,$r13,d

break d
dbcl d
syscall d

slli.w $r11,$r12,d
srli.w $r11,$r12,d
srai.w $r12,$r13,d

bstrins.w $r12,$r13,d,d

slti $r12,$r13,a
sltui $r12,$r13,a
addi.w $r12,$r13,a
andi $r12,$r13,d
ori  $r12,$r13,d
xori $r12,$r13,d
lu12i.w $r12,a
pcaddi $r12,a
pcalau12i $r12,a
pcaddu12i $r12,a
pcaddu18i $r12,a

csrrd $r12,a
csrwr $r12,a
csrxchg $r12,$r13,d
cacop d,$r13,d
lddir $r12,$r13,d
ldpte $r12,d

invtlb d,$r13,$r14

ll.w $r12,$r13,e
sc.w $r12,$r13,e
ll.d $r12,$r13,e
sc.d $r12,$r13,e
ldptr.w $r12,$r13,e
stptr.w $r12,$r13,e
ld.h $r12,$r13,e
ld.w $r12,$r13,e
st.b $r12,$r13,e
st.h $r12,$r13,e
st.w $r12,$r13,e
ld.bu $r12,$r13,e
ld.hu $r12,$r13,e
ld.wu $r12,$r13,e
preld d,$r13,e
preldx d,$r13,$r14

fld.s $f10,$r12,a
fst.s $f10,$r12,a
