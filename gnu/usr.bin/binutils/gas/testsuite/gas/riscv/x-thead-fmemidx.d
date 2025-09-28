#as: -march=rv64i_xtheadfmemidx
#source: x-thead-fmemidx.s
#objdump: -dr

.*:[ 	]+file format .*

Disassembly of section .text:

0+000 <target>:
[ 	]+[0-9a-f]+:[ 	]+60c5e50b[ 	]+th.flrd[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+66c5e50b[ 	]+th.flrd[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+40c5e50b[ 	]+th.flrw[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+46c5e50b[ 	]+th.flrw[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+70c5e50b[ 	]+th.flurd[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+76c5e50b[ 	]+th.flurd[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+50c5e50b[ 	]+th.flurw[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+56c5e50b[ 	]+th.flurw[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+60c5f50b[ 	]+th.fsrd[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+66c5f50b[ 	]+th.fsrd[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+40c5f50b[ 	]+th.fsrw[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+46c5f50b[ 	]+th.fsrw[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+70c5f50b[ 	]+th.fsurd[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+76c5f50b[ 	]+th.fsurd[ 	]+fa0,a1,a2,3
[ 	]+[0-9a-f]+:[ 	]+50c5f50b[ 	]+th.fsurw[ 	]+fa0,a1,a2,0
[ 	]+[0-9a-f]+:[ 	]+56c5f50b[ 	]+th.fsurw[ 	]+fa0,a1,a2,3
