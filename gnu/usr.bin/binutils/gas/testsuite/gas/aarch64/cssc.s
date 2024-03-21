/* Common Short Sequence Compression instructions.  */
abs w0, w0
abs w1, w0
abs w0, w8
abs w30, w5
abs w4, w30
abs x0, x0
abs x1, x0
abs x0, x8
abs x30, x5
abs x4, x30
cnt w0, w0
cnt w1, w0
cnt w0, w8
cnt w30, w5
cnt w4, w30
cnt x0, x0
cnt x1, x0
cnt x0, x8
cnt x30, x5
cnt x4, x30
ctz w0, w0
ctz w1, w0
ctz w0, w8
ctz w30, w5
ctz w4, w30
ctz x0, x0
ctz x1, x0
ctz x0, x8
ctz x30, x5
ctz x4, x30
smax w0, w0, w0
smax w1, w0, w0
smax w0, w1, w0
smax w0, w0, w1
smax w3, w2, w4
smax w30, w0, w0
smax w0, w30, w0
smax w0, w0, w30
smax w14, w7, w28
smax x0, x0, x0
smax x1, x0, x0
smax x0, x1, x0
smax x0, x0, x1
smax x3, x2, x4
smax x30, x0, x0
smax x0, x30, x0
smax x0, x0, x30
smax x14, x7, x28
umax w0, w0, w0
umax w1, w0, w0
umax w0, w1, w0
umax w0, w0, w1
umax w3, w2, w4
umax w30, w0, w0
umax w0, w30, w0
umax w0, w0, w30
umax w14, w7, w28
umax x0, x0, x0
umax x1, x0, x0
umax x0, x1, x0
umax x0, x0, x1
umax x3, x2, x4
umax x30, x0, x0
umax x0, x30, x0
umax x0, x0, x30
umax x14, x7, x28
smin w0, w0, w0
smin w1, w0, w0
smin w0, w1, w0
smin w0, w0, w1
smin w3, w2, w4
smin w30, w0, w0
smin w0, w30, w0
smin w0, w0, w30
smin w14, w7, w28
smin x0, x0, x0
smin x1, x0, x0
smin x0, x1, x0
smin x0, x0, x1
smin x3, x2, x4
smin x30, x0, x0
smin x0, x30, x0
smin x0, x0, x30
smin x14, x7, x28
umin w0, w0, w0
umin w1, w0, w0
umin w0, w1, w0
umin w0, w0, w1
umin w3, w2, w4
umin w30, w0, w0
umin w0, w30, w0
umin w0, w0, w30
umin w14, w7, w28
umin x0, x0, x0
umin x1, x0, x0
umin x0, x1, x0
umin x0, x0, x1
umin x3, x2, x4
umin x30, x0, x0
umin x0, x30, x0
umin x0, x0, x30
umin x14, x7, x28
smax w0, w0, #0
smax w1, w0, #0
smax w0, w1, #0
smax w0, w0, #1
smax w2, w8, #-32
smax w13, w26, #-128
smax w17, w9, #127
smax x0, x0, #0
smax x1, x0, #0
smax x0, x1, #0
smax x0, x0, #1
smax x2, x8, #-32
smax x13, x26, #-128
smax x17, x9, #127
umax w0, w0, #0
umax w1, w0, #0
umax w0, w1, #0
umax w0, w0, #1
umax w2, w8, #34
umax w13, w26, #128
umax w17, w9, #255
umax x0, x0, #0
umax x1, x0, #0
umax x0, x1, #0
umax x0, x0, #1
umax x2, x8, #34
umax x13, x26, #128
umax x17, x9, #255
smin w0, w0, #0
smin w1, w0, #0
smin w0, w1, #0
smin w0, w0, #1
smin w2, w8, #-32
smin w13, w26, #-128
smin w17, w9, #127
smin x0, x0, #0
smin x1, x0, #0
smin x0, x1, #0
smin x0, x0, #1
smin x2, x8, #-32
smin x13, x26, #-128
smin x17, x9, #127
umin w0, w0, #0
umin w1, w0, #0
umin w0, w1, #0
umin w0, w0, #1
umin w2, w8, #34
umin w13, w26, #128
umin w17, w9, #255
umin x0, x0, #0
umin x1, x0, #0
umin x0, x1, #0
umin x0, x0, #1
umin x2, x8, #34
umin x13, x26, #128
umin x17, x9, #255

