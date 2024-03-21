.syntax unified

vcx1 p0, q0, #0
vcx1 p0, q0, #2048
vcx1 p0, q0, #1920
vcx1 p0, q0, #64
vcx1 p0, q0, #63
vcx1 p7, q0, #0
vcx1 p0, q7, #0
vcx1a p0, q0, #0
vcx1a p0, q0, #2048
vcx1a p0, q0, #1920
vcx1a p0, q0, #64
vcx1a p0, q0, #63
vcx1a p7, q0, #0
vcx1a p0, q7, #0

vptt.i8 eq, q0, q0
vcx1t p0, q0, #0
vcx1at p0, q0, #0

vcx2 p0, q0, q0, #0
vcx2 p0, q0, q0, #64
vcx2 p0, q0, q0, #60
vcx2 p0, q0, q0, #2
vcx2 p0, q0, q0, #1
vcx2 p7, q0, q0, #0
vcx2 p0, q7, q0, #0
vcx2 p0, q0, q7, #0
vcx2a p0, q0, q0, #0
vcx2a p0, q0, q0, #64
vcx2a p0, q0, q0, #60
vcx2a p0, q0, q0, #2
vcx2a p0, q0, q0, #1
vcx2a p7, q0, q0, #0
vcx2a p0, q7, q0, #0
vcx2a p0, q0, q7, #0

vptt.i8 eq, q0, q0
vcx2t p0, q0, q0, #0
vcx2at p0, q0, q0, #0

vcx3 p0, q0, q0, q0, #0
vcx3 p0, q0, q0, q0, #8
vcx3 p0, q0, q0, q0, #6
vcx3 p0, q0, q0, q0, #1
vcx3 p7, q0, q0, q0, #0
vcx3 p0, q7, q0, q0, #0
vcx3 p0, q0, q7, q0, #0
vcx3 p0, q0, q0, q7, #0
vcx3a p0, q0, q0, q0, #0
vcx3a p0, q0, q0, q0, #8
vcx3a p0, q0, q0, q0, #6
vcx3a p0, q0, q0, q0, #1
vcx3a p7, q0, q0, q0, #0
vcx3a p0, q7, q0, q0, #0
vcx3a p0, q0, q7, q0, #0
vcx3a p0, q0, q0, q7, #0

vptt.i8 eq, q0, q0
vcx3t p0, q0, q0, q0, #0
vcx3at p0, q0, q0, q0, #0
