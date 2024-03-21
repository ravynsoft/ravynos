.syntax unified

.arm

@ warnings
mrrc  p0,#1,r1,r1,c4		@ unpredictable
mrrc2 p0,#1,r1,r1,c4		@ ditto

@ normal
mrrc  p0,#1,r1,r2,c4		@ predictable
mrrc2 p0,#1,r1,r2,c4		@ ditto
mcrr  p0,#1,r1,r2,c4		@ ditto
mcrr2 p0,#1,r1,r2,c4		@ ditto
mcrr  p0,#1,r1,r1,c4		@ ditto
mcrr2 p0,#1,r1,r1,c4		@ ditto

.thumb

@ warnings
mrrc  p0,#1,r1,r1,c4		@ unpredictable
mrrc2 p0,#1,r1,r1,c4		@ ditto

@ normal
mrrc  p0,#1,r1,r2,c4		@ predictable
mrrc2 p0,#1,r1,r2,c4		@ ditto
mcrr  p0,#1,r1,r2,c4		@ ditto
mcrr2 p0,#1,r1,r2,c4		@ ditto
mcrr  p0,#1,r1,r1,c4		@ ditto
mcrr2 p0,#1,r1,r1,c4		@ ditto
