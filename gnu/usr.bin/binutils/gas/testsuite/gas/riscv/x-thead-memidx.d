#as: -march=rv64gc_xtheadmemidx
#source: x-thead-memidx.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+7805c50b[ 	]+th.ldia[ 	]+a0,\(a1\),0,0
[ 	]+[0-9a-f]+:[ 	]+6af5c50b[ 	]+th.ldib[ 	]+a0,\(a1\),15,1
[ 	]+[0-9a-f]+:[ 	]+5c05c50b[ 	]+th.lwia[ 	]+a0,\(a1\),0,2
[ 	]+[0-9a-f]+:[ 	]+4f05c50b[ 	]+th.lwib[ 	]+a0,\(a1\),-16,3
[ 	]+[0-9a-f]+:[ 	]+d805c50b[ 	]+th.lwuia[ 	]+a0,\(a1\),0,0
[ 	]+[0-9a-f]+:[ 	]+caf5c50b[ 	]+th.lwuib[ 	]+a0,\(a1\),15,1
[ 	]+[0-9a-f]+:[ 	]+3c05c50b[ 	]+th.lhia[ 	]+a0,\(a1\),0,2
[ 	]+[0-9a-f]+:[ 	]+2f05c50b[ 	]+th.lhib[ 	]+a0,\(a1\),-16,3
[ 	]+[0-9a-f]+:[ 	]+b805c50b[ 	]+th.lhuia[ 	]+a0,\(a1\),0,0
[ 	]+[0-9a-f]+:[ 	]+aaf5c50b[ 	]+th.lhuib[ 	]+a0,\(a1\),15,1
[ 	]+[0-9a-f]+:[ 	]+1c05c50b[ 	]+th.lbia[ 	]+a0,\(a1\),0,2
[ 	]+[0-9a-f]+:[ 	]+0f05c50b[ 	]+th.lbib[ 	]+a0,\(a1\),-16,3
[ 	]+[0-9a-f]+:[ 	]+9805c50b[ 	]+th.lbuia[ 	]+a0,\(a1\),0,0
[ 	]+[0-9a-f]+:[ 	]+8af5c50b[ 	]+th.lbuib[ 	]+a0,\(a1\),15,1
[ 	]+[0-9a-f]+:[ 	]+7905d50b[ 	]+th.sdia[ 	]+a0,\(a1\),-16,0
[ 	]+[0-9a-f]+:[ 	]+6bf5d50b[ 	]+th.sdib[ 	]+a0,\(a1\),-1,1
[ 	]+[0-9a-f]+:[ 	]+5c05d50b[ 	]+th.swia[ 	]+a0,\(a1\),0,2
[ 	]+[0-9a-f]+:[ 	]+4e15d50b[ 	]+th.swib[ 	]+a0,\(a1\),1,3
[ 	]+[0-9a-f]+:[ 	]+3845d50b[ 	]+th.shia[ 	]+a0,\(a1\),4,0
[ 	]+[0-9a-f]+:[ 	]+2ad5d50b[ 	]+th.shib[ 	]+a0,\(a1\),13,1
[ 	]+[0-9a-f]+:[ 	]+1ce5d50b[ 	]+th.sbia[ 	]+a0,\(a1\),14,2
[ 	]+[0-9a-f]+:[ 	]+0ef5d50b[ 	]+th.sbib[ 	]+a0,\(a1\),15,3
[ 	]+[0-9a-f]+:[ 	]+60c5c50b[ 	]+th.lrd[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+42c5c50b[ 	]+th.lrw[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+c4c5c50b[ 	]+th.lrwu[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+26c5c50b[ 	]+th.lrh[ 	]+a0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+a0c5c50b[ 	]+th.lrhu[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+02c5c50b[ 	]+th.lrb[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+84c5c50b[ 	]+th.lrbu[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+66c5d50b[ 	]+th.srd[ 	]+a0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+40c5d50b[ 	]+th.srw[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+22c5d50b[ 	]+th.srh[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+04c5d50b[ 	]+th.srb[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+70c5c50b[ 	]+th.lurd[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+52c5c50b[ 	]+th.lurw[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+d4c5c50b[ 	]+th.lurwu[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+36c5c50b[ 	]+th.lurh[ 	]+a0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+b0c5c50b[ 	]+th.lurhu[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+12c5c50b[ 	]+th.lurb[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+94c5c50b[ 	]+th.lurbu[ 	]+a0,a1,a2,2
[ 	]+[0-9a-f]+:[ 	]+76c5d50b[ 	]+th.surd[ 	]+a0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+50c5d50b[ 	]+th.surw[ 	]+a0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+32c5d50b[ 	]+th.surh[ 	]+a0,a1,a2,1
[ 	]+[0-9a-f]+:[ 	]+14c5d50b[ 	]+th.surb[ 	]+a0,a1,a2,2
