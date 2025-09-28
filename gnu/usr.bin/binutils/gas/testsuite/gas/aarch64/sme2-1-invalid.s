	mov	0, za.b[w8, 0]
	mov	{ z0.b - z1.b }, 0

	mov	{ z0.d - z1.d }, za.q[w8, 0]
	mov	{ z1.d - z2.d }, za.d[w8, 0]
	mov	{ z0.d - z1.d }, za.d[w7, 0]
	mov	{ z0.d - z1.d }, za.d[w12, 0]
	mov	{ z0.d - z1.d }, za.d[w8, -1]
	mov	{ z0.d - z1.d }, za.d[w8, 8]

	mov	{ z0.d - z3.d }, za.q[w8, 0]
	mov	{ z1.d - z4.d }, za.d[w8, 0]
	mov	{ z2.d - z5.d }, za.d[w8, 0]
	mov	{ z3.d - z6.d }, za.d[w8, 0]
	mov	{ z0.d - z3.d }, za.d[w7, 0]
	mov	{ z0.d - z3.d }, za.d[w12, 0]
	mov	{ z0.d - z3.d }, za.d[w8, -1]
	mov	{ z0.d - z3.d }, za.d[w8, 8]

	mov	{ z1.b - z2.b }, za0h.b[w8, 0:1]
	mov	{ z0.b - z1.b }, za1h.b[w12, 0:1]
	mov	{ z0.b - z1.b }, za1v.b[w12, 0:1]
	mov	{ z0.b - z1.b }, za0h.b[w11, 0:1]
	mov	{ z0.b - z1.b }, za0h.b[w16, 0:1]
	mov	{ z0.b - z1.b }, za0h.b[w12, -2:-1]
	mov	{ z0.b - z1.b }, za0h.b[w12, 1:2]
	mov	{ z0.b - z1.b }, za0h.b[w12, 15:16]
	mov	{ z0.b - z1.b }, za0h.b[w12, 16:17]
	mov	{ z0.b - z1.b }, za0h.b[w12, 0]
	mov	{ z0.b - z1.b }, za0h.b[w12, 0:2]
	mov	{ z0.b - z1.b }, za0h.b[w12, 0:3]
	mov	{ z0.b - z1.b }, za0h.b[w12, 0:1, vgx2]
	mov	{ z0.b - z1.b }, za0h.b[w12, 0:1, vgx4]
	mov	{ z0.b - z1.b }, za0.b[w12, 0:1]

	mov	{ z1.h - z2.h }, za0h.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za2h.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za2v.h[w12, 0:1]
	mov	{ z0.h - z1.h }, za0h.h[w11, 0:1]
	mov	{ z0.h - z1.h }, za0h.h[w16, 0:1]
	mov	{ z0.h - z1.h }, za0h.h[w12, -2:-1]
	mov	{ z0.h - z1.h }, za0h.h[w12, 1:2]
	mov	{ z0.h - z1.h }, za0h.h[w12, 7:8]
	mov	{ z0.h - z1.h }, za0h.h[w12, 8:9]
	mov	{ z0.h - z1.h }, za0h.h[w12, 0]
	mov	{ z0.h - z1.h }, za0h.h[w12, 0:2]
	mov	{ z0.h - z1.h }, za0h.h[w12, 0:3]
	mov	{ z0.h - z1.h }, za0h.h[w12, 0:1, vgx2]
	mov	{ z0.h - z1.h }, za0h.h[w12, 0:1, vgx4]
	mov	{ z0.h - z1.h }, za0.h[w12, 0:1]

	mov	{ z1.s - z2.s }, za0h.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za4h.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za4v.s[w12, 0:1]
	mov	{ z0.s - z1.s }, za0h.s[w11, 0:1]
	mov	{ z0.s - z1.s }, za0h.s[w16, 0:1]
	mov	{ z0.s - z1.s }, za0h.s[w12, -2:-1]
	mov	{ z0.s - z1.s }, za0h.s[w12, 1:2]
	mov	{ z0.s - z1.s }, za0h.s[w12, 3:4]
	mov	{ z0.s - z1.s }, za0h.s[w12, 4:5]
	mov	{ z0.s - z1.s }, za0h.s[w12, 0]
	mov	{ z0.s - z1.s }, za0h.s[w12, 0:2]
	mov	{ z0.s - z1.s }, za0h.s[w12, 0:3]
	mov	{ z0.s - z1.s }, za0h.s[w12, 0:1, vgx2]
	mov	{ z0.s - z1.s }, za0h.s[w12, 0:1, vgx4]
	mov	{ z0.s - z1.s }, za0.s[w12, 0:1]

	mov	{ z1.d - z2.d }, za0h.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za8h.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za8v.d[w12, 0:1]
	mov	{ z0.d - z1.d }, za0h.d[w11, 0:1]
	mov	{ z0.d - z1.d }, za0h.d[w16, 0:1]
	mov	{ z0.d - z1.d }, za0h.d[w12, -2:-1]
	mov	{ z0.d - z1.d }, za0h.d[w12, 1:2]
	mov	{ z0.d - z1.d }, za0h.d[w12, 2:3]
	mov	{ z0.d - z1.d }, za0h.d[w12, 0]
	mov	{ z0.d - z1.d }, za0h.d[w12, 0:2]
	mov	{ z0.d - z1.d }, za0h.d[w12, 0:3]
	mov	{ z0.d - z1.d }, za0h.d[w12, 0:1, vgx2]
	mov	{ z0.d - z1.d }, za0h.d[w12, 0:1, vgx4]
	mov	{ z0.d - z1.d }, za0.d[w12, 0:1]

	mov	{ z1.b - z4.b }, za0h.b[w12, 0:3]
	mov	{ z2.b - z5.b }, za0h.b[w12, 0:3]
	mov	{ z3.b - z6.b }, za0h.b[w12, 0:3]
	mov	{ z0.b - z3.b }, za1h.b[w12, 0:3]
	mov	{ z0.b - z3.b }, za1v.b[w12, 0:3]
	mov	{ z0.b - z3.b }, za0h.b[w11, 0:3]
	mov	{ z0.b - z3.b }, za0h.b[w16, 0:3]
	mov	{ z0.b - z3.b }, za0h.b[w12, -4:-1]
	mov	{ z0.b - z3.b }, za0h.b[w12, 1:4]
	mov	{ z0.b - z3.b }, za0h.b[w12, 2:5]
	mov	{ z0.b - z3.b }, za0h.b[w12, 3:6]
	mov	{ z0.b - z3.b }, za0h.b[w12, 13:16]
	mov	{ z0.b - z3.b }, za0h.b[w12, 14:17]
	mov	{ z0.b - z3.b }, za0h.b[w12, 15:18]
	mov	{ z0.b - z3.b }, za0h.b[w12, 16:19]
	mov	{ z0.b - z3.b }, za0h.b[w12, 0]
	mov	{ z0.b - z3.b }, za0h.b[w12, 0:1]
	mov	{ z0.b - z3.b }, za0h.b[w12, 0:2]
	mov	{ z0.b - z3.b }, za0h.b[w12, 0:3, vgx2]
	mov	{ z0.b - z3.b }, za0h.b[w12, 0:3, vgx4]
	mov	{ z0.b - z3.b }, za0.b[w12, 0:3]

	mov	{ z1.h - z2.h }, za0h.h[w12, 0:3]
	mov	{ z2.h - z5.h }, za0h.h[w12, 0:3]
	mov	{ z3.h - z6.h }, za0h.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za2h.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za2v.h[w12, 0:3]
	mov	{ z0.h - z3.h }, za0h.h[w11, 0:3]
	mov	{ z0.h - z3.h }, za0h.h[w16, 0:3]
	mov	{ z0.h - z3.h }, za0h.h[w12, -4:-1]
	mov	{ z0.h - z3.h }, za0h.h[w12, 1:2]
	mov	{ z0.h - z3.h }, za0h.h[w12, 5:8]
	mov	{ z0.h - z3.h }, za0h.h[w12, 6:9]
	mov	{ z0.h - z3.h }, za0h.h[w12, 7:10]
	mov	{ z0.h - z3.h }, za0h.h[w12, 8:11]
	mov	{ z0.h - z3.h }, za0h.h[w12, 0]
	mov	{ z0.h - z3.h }, za0h.h[w12, 0:1]
	mov	{ z0.h - z3.h }, za0h.h[w12, 0:2]
	mov	{ z0.h - z3.h }, za0h.h[w12, 0:3, vgx2]
	mov	{ z0.h - z3.h }, za0h.h[w12, 0:3, vgx4]
	mov	{ z0.h - z3.h }, za0.h[w12, 0:3]

	mov	{ z1.s - z2.s }, za0h.s[w12, 0:3]
	mov	{ z2.s - z5.s }, za0h.s[w12, 0:3]
	mov	{ z3.s - z6.s }, za0h.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za4h.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za4v.s[w12, 0:3]
	mov	{ z0.s - z3.s }, za0h.s[w11, 0:3]
	mov	{ z0.s - z3.s }, za0h.s[w16, 0:3]
	mov	{ z0.s - z3.s }, za0h.s[w12, -4:-1]
	mov	{ z0.s - z3.s }, za0h.s[w12, 1:4]
	mov	{ z0.s - z3.s }, za0h.s[w12, 2:5]
	mov	{ z0.s - z3.s }, za0h.s[w12, 3:6]
	mov	{ z0.s - z3.s }, za0h.s[w12, 4:7]
	mov	{ z0.s - z3.s }, za0h.s[w12, 0]
	mov	{ z0.s - z3.s }, za0h.s[w12, 0:1]
	mov	{ z0.s - z3.s }, za0h.s[w12, 0:2]
	mov	{ z0.s - z3.s }, za0h.s[w12, 0:3, vgx2]
	mov	{ z0.s - z3.s }, za0h.s[w12, 0:3, vgx4]
	mov	{ z0.s - z3.s }, za0.s[w12, 0:3]

	mov	{ z1.d - z2.d }, za0h.d[w12, 0:3]
	mov	{ z2.d - z5.d }, za0h.d[w12, 0:3]
	mov	{ z3.d - z6.d }, za0h.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za8h.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za8v.d[w12, 0:3]
	mov	{ z0.d - z3.d }, za0h.d[w11, 0:3]
	mov	{ z0.d - z3.d }, za0h.d[w16, 0:3]
	mov	{ z0.d - z3.d }, za0h.d[w12, -4:-1]
	mov	{ z0.d - z3.d }, za0h.d[w12, 1:4]
	mov	{ z0.d - z3.d }, za0h.d[w12, 2:5]
	mov	{ z0.d - z3.d }, za0h.d[w12, 3:6]
	mov	{ z0.d - z3.d }, za0h.d[w12, 4:7]
	mov	{ z0.d - z3.d }, za0h.d[w12, 0]
	mov	{ z0.d - z3.d }, za0h.d[w12, 0:1]
	mov	{ z0.d - z3.d }, za0h.d[w12, 0:2]
	mov	{ z0.d - z3.d }, za0h.d[w12, 0:3, vgx2]
	mov	{ z0.d - z3.d }, za0h.d[w12, 0:3, vgx4]
	mov	{ z0.d - z3.d }, za0.d[w12, 0:3]

	mova	0, za.b[w8, 0]
	mova	{ z0.b - z1.b }, 0

	mova	za.q[w8, 0], { z0.q - z1.q }
	mova	za.d[w7, 0], { z0.d - z1.d }
	mova	za.d[w12, 0], { z0.d - z1.d }
	mova	za.d[w8, -1], { z0.d - z1.d }
	mova	za.d[w8, 8], { z0.d - z1.d }
	mova	za.d[w8, 0], { z1.d - z2.d }

	mova	za.q[w8, 0], { z0.q - z3.q }
	mova	za.d[w7, 0], { z0.d - z3.d }
	mova	za.d[w12, 0], { z0.d - z3.d }
	mova	za.d[w8, -1], { z0.d - z3.d }
	mova	za.d[w8, 8], { z0.d - z3.d }
	mova	za.d[w8, 0], { z1.d - z4.d }
	mova	za.d[w8, 0], { z2.d - z5.d }
	mova	za.d[w8, 0], { z3.d - z6.d }

	mova	za0h.b[w8, 0:1], { z1.b - z2.b }
	mova	za1h.b[w12, 0:1], { z0.b - z1.b }
	mova	za1v.b[w12, 0:1], { z0.b - z1.b }
	mova	za0h.b[w11, 0:1], { z0.b - z1.b }
	mova	za0h.b[w16, 0:1], { z0.b - z1.b }
	mova	za0h.b[w12, -2:-1], { z0.b - z1.b }
	mova	za0h.b[w12, 1:2], { z0.b - z1.b }
	mova	za0h.b[w12, 15:16], { z0.b - z1.b }
	mova	za0h.b[w12, 16:17], { z0.b - z1.b }
	mova	za0h.b[w12, 0], { z0.b - z1.b }
	mova	za0h.b[w12, 0:2], { z0.b - z1.b }
	mova	za0h.b[w12, 0:3], { z0.b - z1.b }
	mova	za0h.b[w12, 0:1, vgx2], { z0.b - z1.b }
	mova	za0h.b[w12, 0:1, vgx4], { z0.b - z1.b }
	mova	za0.b[w12, 0:1], { z0.b - z1.b }

	mova	za0h.h[w12, 0:1], { z1.h - z2.h }
	mova	za2h.h[w12, 0:1], { z0.h - z1.h }
	mova	za2v.h[w12, 0:1], { z0.h - z1.h }
	mova	za0h.h[w11, 0:1], { z0.h - z1.h }
	mova	za0h.h[w16, 0:1], { z0.h - z1.h }
	mova	za0h.h[w12, -2:-1], { z0.h - z1.h }
	mova	za0h.h[w12, 1:2], { z0.h - z1.h }
	mova	za0h.h[w12, 7:8], { z0.h - z1.h }
	mova	za0h.h[w12, 8:9], { z0.h - z1.h }
	mova	za0h.h[w12, 0], { z0.h - z1.h }
	mova	za0h.h[w12, 0:2], { z0.h - z1.h }
	mova	za0h.h[w12, 0:3], { z0.h - z1.h }
	mova	za0h.h[w12, 0:1, vgx2], { z0.h - z1.h }
	mova	za0h.h[w12, 0:1, vgx4], { z0.h - z1.h }
	mova	za0.h[w12, 0:1], { z0.h - z1.h }

	mova	za0h.s[w12, 0:1], { z1.s - z2.s }
	mova	za4h.s[w12, 0:1], { z0.s - z1.s }
	mova	za4v.s[w12, 0:1], { z0.s - z1.s }
	mova	za0h.s[w11, 0:1], { z0.s - z1.s }
	mova	za0h.s[w16, 0:1], { z0.s - z1.s }
	mova	za0h.s[w12, -2:-1], { z0.s - z1.s }
	mova	za0h.s[w12, 1:2], { z0.s - z1.s }
	mova	za0h.s[w12, 3:4], { z0.s - z1.s }
	mova	za0h.s[w12, 4:5], { z0.s - z1.s }
	mova	za0h.s[w12, 0], { z0.s - z1.s }
	mova	za0h.s[w12, 0:2], { z0.s - z1.s }
	mova	za0h.s[w12, 0:3], { z0.s - z1.s }
	mova	za0h.s[w12, 0:1, vgx2], { z0.s - z1.s }
	mova	za0h.s[w12, 0:1, vgx4], { z0.s - z1.s }
	mova	za0.s[w12, 0:1], { z0.s - z1.s }

	mova	za0h.d[w12, 0:1], { z1.d - z2.d }
	mova	za8h.d[w12, 0:1], { z0.d - z1.d }
	mova	za8v.d[w12, 0:1], { z0.d - z1.d }
	mova	za0h.d[w11, 0:1], { z0.d - z1.d }
	mova	za0h.d[w16, 0:1], { z0.d - z1.d }
	mova	za0h.d[w12, -2:-1], { z0.d - z1.d }
	mova	za0h.d[w12, 1:2], { z0.d - z1.d }
	mova	za0h.d[w12, 2:3], { z0.d - z1.d }
	mova	za0h.d[w12, 0], { z0.d - z1.d }
	mova	za0h.d[w12, 0:2], { z0.d - z1.d }
	mova	za0h.d[w12, 0:3], { z0.d - z1.d }
	mova	za0h.d[w12, 0:1, vgx2], { z0.d - z1.d }
	mova	za0h.d[w12, 0:1, vgx4], { z0.d - z1.d }
	mova	za0.d[w12, 0:1], { z0.d - z1.d }

	mova	za0h.b[w12, 0:3], { z1.b - z4.b }
	mova	za0h.b[w12, 0:3], { z2.b - z5.b }
	mova	za0h.b[w12, 0:3], { z3.b - z6.b }
	mova	za1h.b[w12, 0:3], { z0.b - z3.b }
	mova	za1v.b[w12, 0:3], { z0.b - z3.b }
	mova	za0h.b[w11, 0:3], { z0.b - z3.b }
	mova	za0h.b[w16, 0:3], { z0.b - z3.b }
	mova	za0h.b[w12, -4:-1], { z0.b - z3.b }
	mova	za0h.b[w12, 1:4], { z0.b - z3.b }
	mova	za0h.b[w12, 2:5], { z0.b - z3.b }
	mova	za0h.b[w12, 3:6], { z0.b - z3.b }
	mova	za0h.b[w12, 13:16], { z0.b - z3.b }
	mova	za0h.b[w12, 14:17], { z0.b - z3.b }
	mova	za0h.b[w12, 15:18], { z0.b - z3.b }
	mova	za0h.b[w12, 16:19], { z0.b - z3.b }
	mova	za0h.b[w12, 0], { z0.b - z3.b }
	mova	za0h.b[w12, 0:1], { z0.b - z3.b }
	mova	za0h.b[w12, 0:2], { z0.b - z3.b }
	mova	za0h.b[w12, 0:3, vgx2], { z0.b - z3.b }
	mova	za0h.b[w12, 0:3, vgx4], { z0.b - z3.b }
	mova	za0.b[w12, 0:3], { z0.b - z3.b }

	mova	za0h.h[w12, 0:3], { z1.h - z2.h }
	mova	za0h.h[w12, 0:3], { z2.h - z5.h }
	mova	za0h.h[w12, 0:3], { z3.h - z6.h }
	mova	za2h.h[w12, 0:3], { z0.h - z3.h }
	mova	za2v.h[w12, 0:3], { z0.h - z3.h }
	mova	za0h.h[w11, 0:3], { z0.h - z3.h }
	mova	za0h.h[w16, 0:3], { z0.h - z3.h }
	mova	za0h.h[w12, -4:-1], { z0.h - z3.h }
	mova	za0h.h[w12, 1:2], { z0.h - z3.h }
	mova	za0h.h[w12, 5:8], { z0.h - z3.h }
	mova	za0h.h[w12, 6:9], { z0.h - z3.h }
	mova	za0h.h[w12, 7:10], { z0.h - z3.h }
	mova	za0h.h[w12, 8:11], { z0.h - z3.h }
	mova	za0h.h[w12, 0], { z0.h - z3.h }
	mova	za0h.h[w12, 0:1], { z0.h - z3.h }
	mova	za0h.h[w12, 0:2], { z0.h - z3.h }
	mova	za0h.h[w12, 0:3, vgx2], { z0.h - z3.h }
	mova	za0h.h[w12, 0:3, vgx4], { z0.h - z3.h }
	mova	za0.h[w12, 0:3], { z0.h - z3.h }

	mova	za0h.s[w12, 0:3], { z1.s - z2.s }
	mova	za0h.s[w12, 0:3], { z2.s - z5.s }
	mova	za0h.s[w12, 0:3], { z3.s - z6.s }
	mova	za4h.s[w12, 0:3], { z0.s - z3.s }
	mova	za4v.s[w12, 0:3], { z0.s - z3.s }
	mova	za0h.s[w11, 0:3], { z0.s - z3.s }
	mova	za0h.s[w16, 0:3], { z0.s - z3.s }
	mova	za0h.s[w12, -4:-1], { z0.s - z3.s }
	mova	za0h.s[w12, 1:4], { z0.s - z3.s }
	mova	za0h.s[w12, 2:5], { z0.s - z3.s }
	mova	za0h.s[w12, 3:6], { z0.s - z3.s }
	mova	za0h.s[w12, 4:7], { z0.s - z3.s }
	mova	za0h.s[w12, 0], { z0.s - z3.s }
	mova	za0h.s[w12, 0:1], { z0.s - z3.s }
	mova	za0h.s[w12, 0:2], { z0.s - z3.s }
	mova	za0h.s[w12, 0:3, vgx2], { z0.s - z3.s }
	mova	za0h.s[w12, 0:3, vgx4], { z0.s - z3.s }
	mova	za0.s[w12, 0:3], { z0.s - z3.s }

	mova	za0h.d[w12, 0:3], { z1.d - z2.d }
	mova	za0h.d[w12, 0:3], { z2.d - z5.d }
	mova	za0h.d[w12, 0:3], { z3.d - z6.d }
	mova	za8h.d[w12, 0:3], { z0.d - z3.d }
	mova	za8v.d[w12, 0:3], { z0.d - z3.d }
	mova	za0h.d[w11, 0:3], { z0.d - z3.d }
	mova	za0h.d[w16, 0:3], { z0.d - z3.d }
	mova	za0h.d[w12, -4:-1], { z0.d - z3.d }
	mova	za0h.d[w12, 1:4], { z0.d - z3.d }
	mova	za0h.d[w12, 2:5], { z0.d - z3.d }
	mova	za0h.d[w12, 3:6], { z0.d - z3.d }
	mova	za0h.d[w12, 4:7], { z0.d - z3.d }
	mova	za0h.d[w12, 0], { z0.d - z3.d }
	mova	za0h.d[w12, 0:1], { z0.d - z3.d }
	mova	za0h.d[w12, 0:2], { z0.d - z3.d }
	mova	za0h.d[w12, 0:3, vgx2], { z0.d - z3.d }
	mova	za0h.d[w12, 0:3, vgx4], { z0.d - z3.d }
	mova	za0.d[w12, 0:3], { z0.d - z3.d }
