.syntax unified

.thumb
.type entry, %function
.global entry
entry:
  blx label

.type label, %function
label:
  bx  lr

.arm
.type label2, %function
label2:
  blx label3

.type label3, %function
label3:
  bx  lr
