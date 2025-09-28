/*
   Those instructions from the sve2.s file that share mnemonics with
   instructions in SVE.
   Created with the below command
`grep -E '^(ext|ldnt1b|ldnt1d|ldnt1h|ldnt1w|mla|mls|mul|smulh|splice|sqadd|sqsub|stnt1b|stnt1d|stnt1h|stnt1w|tbl|umulh|uqadd|uqsub)\b' sve2.s`

   This test file is here to ensure those instructions with shared mnemonics do
   not work when assembled with only +sve enabled.
*/

ext z17.b, { z21.b, z22.b }, #221
ext z0.b, { z0.b, z1.b }, #0
ext z0.b, { z31.b, z0.b }, #0
ldnt1b { z17.d }, p5/z, [z21.d, x27]
ldnt1b { z0.d }, p0/z, [z0.d, x0]
ldnt1b { z0.d }, p0/z, [z0.d]
ldnt1b { z0.d }, p0/z, [z0.d, xzr]
ldnt1b { z17.s }, p5/z, [z21.s, x27]
ldnt1b { z0.s }, p0/z, [z0.s, x0]
ldnt1b { z0.s }, p0/z, [z0.s]
ldnt1b { z0.s }, p0/z, [z0.s, xzr]
ldnt1d { z17.d }, p5/z, [z21.d, x27]
ldnt1d { z0.d }, p0/z, [z0.d, x0]
ldnt1d { z0.d }, p0/z, [z0.d]
ldnt1d { z0.d }, p0/z, [z0.d, xzr]
ldnt1h { z17.d }, p5/z, [z21.d, x27]
ldnt1h { z0.d }, p0/z, [z0.d, x0]
ldnt1h { z0.d }, p0/z, [z0.d]
ldnt1h { z0.d }, p0/z, [z0.d, xzr]
ldnt1h { z17.s }, p5/z, [z21.s, x27]
ldnt1h { z0.s }, p0/z, [z0.s, x0]
ldnt1h { z0.s }, p0/z, [z0.s]
ldnt1h { z0.s }, p0/z, [z0.s, xzr]
ldnt1w { z17.s }, p5/z, [z21.s, x27]
ldnt1w { z0.s }, p0/z, [z0.s, x0]
ldnt1w { z0.s }, p0/z, [z0.s]
ldnt1w { z0.s }, p0/z, [z0.s, xzr]
ldnt1w { z17.d }, p5/z, [z21.d, x27]
ldnt1w { z0.d }, p0/z, [z0.d, x0]
ldnt1w { z0.d }, p0/z, [z0.d]
ldnt1w { z0.d }, p0/z, [z0.d, xzr]
mla z17.h, z21.h, z3.h[3]
mla z0.h, z0.h, z0.h[4]
mla z0.h, z0.h, z0.h[0]
mla z17.s, z21.s, z3.s[3]
mla z0.s, z0.s, z0.s[0]
mla z17.d, z21.d, z11.d[1]
mla z0.d, z0.d, z0.d[0]
mls z17.h, z21.h, z3.h[3]
mls z0.h, z0.h, z0.h[4]
mls z0.h, z0.h, z0.h[0]
mls z17.s, z21.s, z3.s[3]
mls z0.s, z0.s, z0.s[0]
mls z17.d, z21.d, z11.d[1]
mls z0.d, z0.d, z0.d[0]
mul z17.h, z21.h, z3.h[3]
mul z0.h, z0.h, z0.h[4]
mul z0.h, z0.h, z0.h[0]
mul z17.s, z21.s, z3.s[3]
mul z0.s, z0.s, z0.s[0]
mul z17.d, z21.d, z11.d[1]
mul z0.d, z0.d, z0.d[0]
mul z17.b, z21.b, z27.b
mul z0.b, z0.b, z0.b
mul z0.h, z0.h, z0.h
mul z0.s, z0.s, z0.s
mul z0.d, z0.d, z0.d
smulh z17.b, z21.b, z27.b
smulh z0.b, z0.b, z0.b
smulh z0.h, z0.h, z0.h
smulh z0.s, z0.s, z0.s
smulh z0.d, z0.d, z0.d
splice z17.b, p5, { z21.b, z22.b }
splice z0.b, p0, { z0.b, z1.b }
splice z0.h, p0, { z0.h, z1.h }
splice z0.s, p0, { z0.s, z1.s }
splice z0.d, p0, { z0.d, z1.d }
splice z0.b, p0, { z31.b, z0.b }
sqadd z17.b, p5/m, z17.b, z21.b
sqadd z0.b, p0/m, z0.b, z0.b
sqadd z0.h, p0/m, z0.h, z0.h
sqadd z0.s, p0/m, z0.s, z0.s
sqadd z0.d, p0/m, z0.d, z0.d
sqsub z17.b, p5/m, z17.b, z21.b
sqsub z0.b, p0/m, z0.b, z0.b
sqsub z0.h, p0/m, z0.h, z0.h
sqsub z0.s, p0/m, z0.s, z0.s
sqsub z0.d, p0/m, z0.d, z0.d
stnt1b { z17.s }, p5, [z21.s, x27]
stnt1b { z0.s }, p0, [z0.s, x0]
stnt1b { z0.s }, p0, [z0.s]
stnt1b { z0.s }, p0, [z0.s, xzr]
stnt1b { z17.d }, p5, [z21.d, x27]
stnt1b { z0.d }, p0, [z0.d, x0]
stnt1b { z0.d }, p0, [z0.d]
stnt1b { z0.d }, p0, [z0.d, xzr]
stnt1d { z17.d }, p5, [z21.d, x27]
stnt1d { z0.d }, p0, [z0.d, x0]
stnt1d { z0.d }, p0, [z0.d]
stnt1d { z0.d }, p0, [z0.d, xzr]
stnt1h { z17.s }, p5, [z21.s, x27]
stnt1h { z0.s }, p0, [z0.s, x0]
stnt1h { z0.s }, p0, [z0.s]
stnt1h { z0.s }, p0, [z0.s, xzr]
stnt1h { z17.d }, p5, [z21.d, x27]
stnt1h { z0.d }, p0, [z0.d, x0]
stnt1h { z0.d }, p0, [z0.d]
stnt1h { z0.d }, p0, [z0.d, xzr]
stnt1w { z17.s }, p5, [z21.s, x27]
stnt1w { z0.s }, p0, [z0.s, x0]
stnt1w { z0.s }, p0, [z0.s]
stnt1w { z0.s }, p0, [z0.s, xzr]
stnt1w { z17.d }, p5, [z21.d, x27]
stnt1w { z0.d }, p0, [z0.d, x0]
stnt1w { z0.d }, p0, [z0.d]
stnt1w { z0.d }, p0, [z0.d, xzr]
tbl z17.b, { z21.b, z22.b }, z27.b
tbl z0.b, { z0.b, z1.b }, z0.b
tbl z0.h, { z0.h, z1.h }, z0.h
tbl z0.s, { z0.s, z1.s }, z0.s
tbl z0.d, { z0.d, z1.d }, z0.d
tbl z0.b, { z31.b, z0.b }, z0.b
umulh z17.b, z21.b, z27.b
umulh z0.b, z0.b, z0.b
umulh z0.h, z0.h, z0.h
umulh z0.s, z0.s, z0.s
umulh z0.d, z0.d, z0.d
uqadd z17.b, p5/m, z17.b, z21.b
uqadd z0.b, p0/m, z0.b, z0.b
uqadd z0.h, p0/m, z0.h, z0.h
uqadd z0.s, p0/m, z0.s, z0.s
uqadd z0.d, p0/m, z0.d, z0.d
uqsub z17.b, p5/m, z17.b, z21.b
uqsub z0.b, p0/m, z0.b, z0.b
uqsub z0.h, p0/m, z0.h, z0.h
uqsub z0.s, p0/m, z0.s, z0.s
uqsub z0.d, p0/m, z0.d, z0.d
