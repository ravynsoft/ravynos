.syntax unified
.text
.thumb
@ Case 1
it eq
vpsteq
@ Case 2
it eq
vaddt.i32 q0, q1, q2
@ Case 3
it eq
vadd.i32 q0, q1, q2
@ Case 4
vpst
vaddeq.i32 q0, q1, q2
@ Case 5
vpst
vaddt.i32 q0, q1, q2
@ Case 6
vpst
vadd.i32 q0, q1, q2
@ Case 7
vaddeq.i32 q0, q1, q2
@ Case 8
vaddt.i32 q0, q1, q2
@ Case 9
vadd.i32 q0, q1, q2
@ Case 10
it eq
addeq r0, r0, r1
@ Case 11
it eq
addt r0, r0, r1
addeq r0, r0, r1
@ Case 12
it eq
add r0, r0, r1
@ Case 13
vpst
addeq r0, r0, r1
@ Case 14
vpst
addt r0, r0, r1
vaddt.i32 q0, q0, q1
@ Case 15
vpst
add r0, r0, r1
@ Case 16
addeq r0, r0, r1
@ Case 17
addt r0, r0, r1
@ Case 18
add r0, r0, r1
it le
vpstete
vaddt.i32 q0, q1, q2
vadde.i32 q0, q1, q2
vaddt.i32 q0, q1, q2
vadde.i32 q0, q1, q2
vpste
vaddt.i32 q0, q1, q2
vaddt.i32 q0, q1, q2
vpste
vaddt.i32 q0, q1, q2
vaddeq.i32 q0, q1, q2
vpstet
vaddt.i32 q0, q1, q2
vadde.i32 q0, q1, q2
