# SHA2
sha512h X0, Q0, V1.2D
sha512h Q0, Q1, V2.16B
sha512h2 X0, Q0, V1.2D
sha512h2 Q0, Q1, V2.16B
sha512su0 V1.2D, v2.16B
sha512su0 V0, V2.2D
sha512su1 X0, X1, X2
sha512su1 V1.2D, V2.16B, V2.2D
eor3 V1.2D, V2.2D, V3.2D, V4.2D
rax1 V0.4S, V2.4S, V3.4S
xar v0.2d, v1.2d, v3.2d, 128
xar v0.2d, v1.2d, v3.2d, -128

