	vsetvl   a0, a1,  a2
	vsetvli  a0, a1,  0
	vsetvli  a0, a1,  0x7ff
	vsetvli  a0, a1,  0x4		# unrecognized vlmul
	vsetvli  a0, a1,  0x20		# unrecognized vsew
	vsetvli  a0, a1,  e8,  m2
	vsetvli  a0, a1,  e16, m4, ta
	vsetvli  a0, a1,  e32, mf4, mu
	vsetvli  a0, a1,  e64, mf8, tu, ma
	vsetivli a0, 0xb, 0
	vsetivli a0, 0xb, 0x3ff
	vsetivli a0, 0xb, 0x4		# unrecognized vlmul
	vsetivli a0, 0xb, 0x20		# unrecognized vsew
	vsetivli a0, 0xb, e8,  m2
	vsetivli a0, 0xb, e16, m4, ta
	vsetivli a0, 0xb, e32, mf4, mu
	vsetivli a0, 0xb, e64, mf8, tu, ma

	vsetvli  a0, a1,  0x700
	vsetvli  a0, a1,  0x400
	vsetvli  a0, a1,  0x300
	vsetvli  a0, a1,  0x100
	vsetivli a0, 0xb, 0x300
	vsetivli a0, 0xb, 0x100

	vlm.v v4, (a0)
	vlm.v v4, 0(a0)
	vle1.v v4, (a0)		# Alias of vlm.v
	vle1.v v4, 0(a0)
	vsm.v v4, (a0)
	vsm.v v4, 0(a0)
	vse1.v v4, (a0)		# Alias of vsm.v
	vse1.v v4, 0(a0)

	vle8.v v4, (a0)
	vle8.v v4, 0(a0)
	vle8.v v4, (a0), v0.t
	vse8.v v4, (a0)
	vse8.v v4, 0(a0)
	vse8.v v4, (a0), v0.t

	vle16.v v4, (a0)
	vle16.v v4, 0(a0)
	vle16.v v4, (a0), v0.t
	vse16.v v4, (a0)
	vse16.v v4, 0(a0)
	vse16.v v4, (a0), v0.t

	vle32.v v4, (a0)
	vle32.v v4, 0(a0)
	vle32.v v4, (a0), v0.t
	vse32.v v4, (a0)
	vse32.v v4, 0(a0)
	vse32.v v4, (a0), v0.t

	vle64.v v4, (a0)
	vle64.v v4, 0(a0)
	vle64.v v4, (a0), v0.t
	vse64.v v4, (a0)
	vse64.v v4, 0(a0)
	vse64.v v4, (a0), v0.t

	vlse8.v v4, (a0), a1
	vlse8.v v4, 0(a0), a1
	vlse8.v v4, (a0), a1, v0.t
	vsse8.v v4, (a0), a1
	vsse8.v v4, 0(a0), a1
	vsse8.v v4, (a0), a1, v0.t

	vlse16.v v4, (a0), a1
	vlse16.v v4, 0(a0), a1
	vlse16.v v4, (a0), a1, v0.t
	vsse16.v v4, (a0), a1
	vsse16.v v4, 0(a0), a1
	vsse16.v v4, (a0), a1, v0.t

	vlse32.v v4, (a0), a1
	vlse32.v v4, 0(a0), a1
	vlse32.v v4, (a0), a1, v0.t
	vsse32.v v4, (a0), a1
	vsse32.v v4, 0(a0), a1
	vsse32.v v4, (a0), a1, v0.t

	vlse64.v v4, (a0), a1
	vlse64.v v4, 0(a0), a1
	vlse64.v v4, (a0), a1, v0.t
	vsse64.v v4, (a0), a1
	vsse64.v v4, 0(a0), a1
	vsse64.v v4, (a0), a1, v0.t

	vloxei8.v v4, (a0), v12
	vloxei8.v v4, 0(a0), v12
	vloxei8.v v4, (a0), v12, v0.t
	vsoxei8.v v4, (a0), v12
	vsoxei8.v v4, 0(a0), v12
	vsoxei8.v v4, (a0), v12, v0.t
	vluxei8.v v4, (a0), v12
	vluxei8.v v4, 0(a0), v12
	vluxei8.v v4, (a0), v12, v0.t
	vsuxei8.v v4, (a0), v12
	vsuxei8.v v4, 0(a0), v12
	vsuxei8.v v4, (a0), v12, v0.t

	vloxei16.v v4, (a0), v12
	vloxei16.v v4, 0(a0), v12
	vloxei16.v v4, (a0), v12, v0.t
	vsoxei16.v v4, (a0), v12
	vsoxei16.v v4, 0(a0), v12
	vsoxei16.v v4, (a0), v12, v0.t
	vluxei16.v v4, (a0), v12
	vluxei16.v v4, 0(a0), v12
	vluxei16.v v4, (a0), v12, v0.t
	vsuxei16.v v4, (a0), v12
	vsuxei16.v v4, 0(a0), v12
	vsuxei16.v v4, (a0), v12, v0.t

	vloxei32.v v4, (a0), v12
	vloxei32.v v4, 0(a0), v12
	vloxei32.v v4, (a0), v12, v0.t
	vsoxei32.v v4, (a0), v12
	vsoxei32.v v4, 0(a0), v12
	vsoxei32.v v4, (a0), v12, v0.t
	vluxei32.v v4, (a0), v12
	vluxei32.v v4, 0(a0), v12
	vluxei32.v v4, (a0), v12, v0.t
	vsuxei32.v v4, (a0), v12
	vsuxei32.v v4, 0(a0), v12
	vsuxei32.v v4, (a0), v12, v0.t

	vloxei64.v v4, (a0), v12
	vloxei64.v v4, 0(a0), v12
	vloxei64.v v4, (a0), v12, v0.t
	vsoxei64.v v4, (a0), v12
	vsoxei64.v v4, 0(a0), v12
	vsoxei64.v v4, (a0), v12, v0.t
	vluxei64.v v4, (a0), v12
	vluxei64.v v4, 0(a0), v12
	vluxei64.v v4, (a0), v12, v0.t
	vsuxei64.v v4, (a0), v12
	vsuxei64.v v4, 0(a0), v12
	vsuxei64.v v4, (a0), v12, v0.t

	vle8ff.v v4, (a0)
	vle8ff.v v4, 0(a0)
	vle8ff.v v4, (a0), v0.t

	vle16ff.v v4, (a0)
	vle16ff.v v4, 0(a0)
	vle16ff.v v4, (a0), v0.t

	vle32ff.v v4, (a0)
	vle32ff.v v4, 0(a0)
	vle32ff.v v4, (a0), v0.t

	vle64ff.v v4, (a0)
	vle64ff.v v4, 0(a0)
	vle64ff.v v4, (a0), v0.t

	vlseg2e8.v v4, (a0)
	vlseg2e8.v v4, 0(a0)
	vlseg2e8.v v4, (a0), v0.t
	vsseg2e8.v v4, (a0)
	vsseg2e8.v v4, 0(a0)
	vsseg2e8.v v4, (a0), v0.t

	vlseg3e8.v v4, (a0)
	vlseg3e8.v v4, 0(a0)
	vlseg3e8.v v4, (a0), v0.t
	vsseg3e8.v v4, (a0)
	vsseg3e8.v v4, 0(a0)
	vsseg3e8.v v4, (a0), v0.t

	vlseg4e8.v v4, (a0)
	vlseg4e8.v v4, 0(a0)
	vlseg4e8.v v4, (a0), v0.t
	vsseg4e8.v v4, (a0)
	vsseg4e8.v v4, 0(a0)
	vsseg4e8.v v4, (a0), v0.t

	vlseg5e8.v v4, (a0)
	vlseg5e8.v v4, 0(a0)
	vlseg5e8.v v4, (a0), v0.t
	vsseg5e8.v v4, (a0)
	vsseg5e8.v v4, 0(a0)
	vsseg5e8.v v4, (a0), v0.t

	vlseg6e8.v v4, (a0)
	vlseg6e8.v v4, 0(a0)
	vlseg6e8.v v4, (a0), v0.t
	vsseg6e8.v v4, (a0)
	vsseg6e8.v v4, 0(a0)
	vsseg6e8.v v4, (a0), v0.t

	vlseg7e8.v v4, (a0)
	vlseg7e8.v v4, 0(a0)
	vlseg7e8.v v4, (a0), v0.t
	vsseg7e8.v v4, (a0)
	vsseg7e8.v v4, 0(a0)
	vsseg7e8.v v4, (a0), v0.t

	vlseg8e8.v v4, (a0)
	vlseg8e8.v v4, 0(a0)
	vlseg8e8.v v4, (a0), v0.t
	vsseg8e8.v v4, (a0)
	vsseg8e8.v v4, 0(a0)
	vsseg8e8.v v4, (a0), v0.t

	vlseg2e16.v v4, (a0)
	vlseg2e16.v v4, 0(a0)
	vlseg2e16.v v4, (a0), v0.t
	vsseg2e16.v v4, (a0)
	vsseg2e16.v v4, 0(a0)
	vsseg2e16.v v4, (a0), v0.t

	vlseg3e16.v v4, (a0)
	vlseg3e16.v v4, 0(a0)
	vlseg3e16.v v4, (a0), v0.t
	vsseg3e16.v v4, (a0)
	vsseg3e16.v v4, 0(a0)
	vsseg3e16.v v4, (a0), v0.t

	vlseg4e16.v v4, (a0)
	vlseg4e16.v v4, 0(a0)
	vlseg4e16.v v4, (a0), v0.t
	vsseg4e16.v v4, (a0)
	vsseg4e16.v v4, 0(a0)
	vsseg4e16.v v4, (a0), v0.t

	vlseg5e16.v v4, (a0)
	vlseg5e16.v v4, 0(a0)
	vlseg5e16.v v4, (a0), v0.t
	vsseg5e16.v v4, (a0)
	vsseg5e16.v v4, 0(a0)
	vsseg5e16.v v4, (a0), v0.t

	vlseg6e16.v v4, (a0)
	vlseg6e16.v v4, 0(a0)
	vlseg6e16.v v4, (a0), v0.t
	vsseg6e16.v v4, (a0)
	vsseg6e16.v v4, 0(a0)
	vsseg6e16.v v4, (a0), v0.t

	vlseg7e16.v v4, (a0)
	vlseg7e16.v v4, 0(a0)
	vlseg7e16.v v4, (a0), v0.t
	vsseg7e16.v v4, (a0)
	vsseg7e16.v v4, 0(a0)
	vsseg7e16.v v4, (a0), v0.t

	vlseg8e16.v v4, (a0)
	vlseg8e16.v v4, 0(a0)
	vlseg8e16.v v4, (a0), v0.t
	vsseg8e16.v v4, (a0)
	vsseg8e16.v v4, 0(a0)
	vsseg8e16.v v4, (a0), v0.t

	vlseg2e32.v v4, (a0)
	vlseg2e32.v v4, 0(a0)
	vlseg2e32.v v4, (a0), v0.t
	vsseg2e32.v v4, (a0)
	vsseg2e32.v v4, 0(a0)
	vsseg2e32.v v4, (a0), v0.t

	vlseg3e32.v v4, (a0)
	vlseg3e32.v v4, 0(a0)
	vlseg3e32.v v4, (a0), v0.t
	vsseg3e32.v v4, (a0)
	vsseg3e32.v v4, 0(a0)
	vsseg3e32.v v4, (a0), v0.t

	vlseg4e32.v v4, (a0)
	vlseg4e32.v v4, 0(a0)
	vlseg4e32.v v4, (a0), v0.t
	vsseg4e32.v v4, (a0)
	vsseg4e32.v v4, 0(a0)
	vsseg4e32.v v4, (a0), v0.t

	vlseg5e32.v v4, (a0)
	vlseg5e32.v v4, 0(a0)
	vlseg5e32.v v4, (a0), v0.t
	vsseg5e32.v v4, (a0)
	vsseg5e32.v v4, 0(a0)
	vsseg5e32.v v4, (a0), v0.t

	vlseg6e32.v v4, (a0)
	vlseg6e32.v v4, 0(a0)
	vlseg6e32.v v4, (a0), v0.t
	vsseg6e32.v v4, (a0)
	vsseg6e32.v v4, 0(a0)
	vsseg6e32.v v4, (a0), v0.t

	vlseg7e32.v v4, (a0)
	vlseg7e32.v v4, 0(a0)
	vlseg7e32.v v4, (a0), v0.t
	vsseg7e32.v v4, (a0)
	vsseg7e32.v v4, 0(a0)
	vsseg7e32.v v4, (a0), v0.t

	vlseg8e32.v v4, (a0)
	vlseg8e32.v v4, 0(a0)
	vlseg8e32.v v4, (a0), v0.t
	vsseg8e32.v v4, (a0)
	vsseg8e32.v v4, 0(a0)
	vsseg8e32.v v4, (a0), v0.t

	vlseg2e64.v v4, (a0)
	vlseg2e64.v v4, 0(a0)
	vlseg2e64.v v4, (a0), v0.t
	vsseg2e64.v v4, (a0)
	vsseg2e64.v v4, 0(a0)
	vsseg2e64.v v4, (a0), v0.t

	vlseg3e64.v v4, (a0)
	vlseg3e64.v v4, 0(a0)
	vlseg3e64.v v4, (a0), v0.t
	vsseg3e64.v v4, (a0)
	vsseg3e64.v v4, 0(a0)
	vsseg3e64.v v4, (a0), v0.t

	vlseg4e64.v v4, (a0)
	vlseg4e64.v v4, 0(a0)
	vlseg4e64.v v4, (a0), v0.t
	vsseg4e64.v v4, (a0)
	vsseg4e64.v v4, 0(a0)
	vsseg4e64.v v4, (a0), v0.t

	vlseg5e64.v v4, (a0)
	vlseg5e64.v v4, 0(a0)
	vlseg5e64.v v4, (a0), v0.t
	vsseg5e64.v v4, (a0)
	vsseg5e64.v v4, 0(a0)
	vsseg5e64.v v4, (a0), v0.t

	vlseg6e64.v v4, (a0)
	vlseg6e64.v v4, 0(a0)
	vlseg6e64.v v4, (a0), v0.t
	vsseg6e64.v v4, (a0)
	vsseg6e64.v v4, 0(a0)
	vsseg6e64.v v4, (a0), v0.t

	vlseg7e64.v v4, (a0)
	vlseg7e64.v v4, 0(a0)
	vlseg7e64.v v4, (a0), v0.t
	vsseg7e64.v v4, (a0)
	vsseg7e64.v v4, 0(a0)
	vsseg7e64.v v4, (a0), v0.t

	vlseg8e64.v v4, (a0)
	vlseg8e64.v v4, 0(a0)
	vlseg8e64.v v4, (a0), v0.t
	vsseg8e64.v v4, (a0)
	vsseg8e64.v v4, 0(a0)
	vsseg8e64.v v4, (a0), v0.t

	vlsseg2e8.v v4, (a0), a1
	vlsseg2e8.v v4, 0(a0), a1
	vlsseg2e8.v v4, (a0), a1, v0.t
	vssseg2e8.v v4, (a0), a1
	vssseg2e8.v v4, 0(a0), a1
	vssseg2e8.v v4, (a0), a1, v0.t

	vlsseg3e8.v v4, (a0), a1
	vlsseg3e8.v v4, 0(a0), a1
	vlsseg3e8.v v4, (a0), a1, v0.t
	vssseg3e8.v v4, (a0), a1
	vssseg3e8.v v4, 0(a0), a1
	vssseg3e8.v v4, (a0), a1, v0.t

	vlsseg4e8.v v4, (a0), a1
	vlsseg4e8.v v4, 0(a0), a1
	vlsseg4e8.v v4, (a0), a1, v0.t
	vssseg4e8.v v4, (a0), a1
	vssseg4e8.v v4, 0(a0), a1
	vssseg4e8.v v4, (a0), a1, v0.t

	vlsseg5e8.v v4, (a0), a1
	vlsseg5e8.v v4, 0(a0), a1
	vlsseg5e8.v v4, (a0), a1, v0.t
	vssseg5e8.v v4, (a0), a1
	vssseg5e8.v v4, 0(a0), a1
	vssseg5e8.v v4, (a0), a1, v0.t

	vlsseg6e8.v v4, (a0), a1
	vlsseg6e8.v v4, 0(a0), a1
	vlsseg6e8.v v4, (a0), a1, v0.t
	vssseg6e8.v v4, (a0), a1
	vssseg6e8.v v4, 0(a0), a1
	vssseg6e8.v v4, (a0), a1, v0.t

	vlsseg7e8.v v4, (a0), a1
	vlsseg7e8.v v4, 0(a0), a1
	vlsseg7e8.v v4, (a0), a1, v0.t
	vssseg7e8.v v4, (a0), a1
	vssseg7e8.v v4, 0(a0), a1
	vssseg7e8.v v4, (a0), a1, v0.t

	vlsseg8e8.v v4, (a0), a1
	vlsseg8e8.v v4, 0(a0), a1
	vlsseg8e8.v v4, (a0), a1, v0.t
	vssseg8e8.v v4, (a0), a1
	vssseg8e8.v v4, 0(a0), a1
	vssseg8e8.v v4, (a0), a1, v0.t

	vlsseg2e16.v v4, (a0), a1
	vlsseg2e16.v v4, 0(a0), a1
	vlsseg2e16.v v4, (a0), a1, v0.t
	vssseg2e16.v v4, (a0), a1
	vssseg2e16.v v4, 0(a0), a1
	vssseg2e16.v v4, (a0), a1, v0.t

	vlsseg3e16.v v4, (a0), a1
	vlsseg3e16.v v4, 0(a0), a1
	vlsseg3e16.v v4, (a0), a1, v0.t
	vssseg3e16.v v4, (a0), a1
	vssseg3e16.v v4, 0(a0), a1
	vssseg3e16.v v4, (a0), a1, v0.t

	vlsseg4e16.v v4, (a0), a1
	vlsseg4e16.v v4, 0(a0), a1
	vlsseg4e16.v v4, (a0), a1, v0.t
	vssseg4e16.v v4, (a0), a1
	vssseg4e16.v v4, 0(a0), a1
	vssseg4e16.v v4, (a0), a1, v0.t

	vlsseg5e16.v v4, (a0), a1
	vlsseg5e16.v v4, 0(a0), a1
	vlsseg5e16.v v4, (a0), a1, v0.t
	vssseg5e16.v v4, (a0), a1
	vssseg5e16.v v4, 0(a0), a1
	vssseg5e16.v v4, (a0), a1, v0.t

	vlsseg6e16.v v4, (a0), a1
	vlsseg6e16.v v4, 0(a0), a1
	vlsseg6e16.v v4, (a0), a1, v0.t
	vssseg6e16.v v4, (a0), a1
	vssseg6e16.v v4, 0(a0), a1
	vssseg6e16.v v4, (a0), a1, v0.t

	vlsseg7e16.v v4, (a0), a1
	vlsseg7e16.v v4, 0(a0), a1
	vlsseg7e16.v v4, (a0), a1, v0.t
	vssseg7e16.v v4, (a0), a1
	vssseg7e16.v v4, 0(a0), a1
	vssseg7e16.v v4, (a0), a1, v0.t

	vlsseg8e16.v v4, (a0), a1
	vlsseg8e16.v v4, 0(a0), a1
	vlsseg8e16.v v4, (a0), a1, v0.t
	vssseg8e16.v v4, (a0), a1
	vssseg8e16.v v4, 0(a0), a1
	vssseg8e16.v v4, (a0), a1, v0.t

	vlsseg2e32.v v4, (a0), a1
	vlsseg2e32.v v4, 0(a0), a1
	vlsseg2e32.v v4, (a0), a1, v0.t
	vssseg2e32.v v4, (a0), a1
	vssseg2e32.v v4, 0(a0), a1
	vssseg2e32.v v4, (a0), a1, v0.t

	vlsseg3e32.v v4, (a0), a1
	vlsseg3e32.v v4, 0(a0), a1
	vlsseg3e32.v v4, (a0), a1, v0.t
	vssseg3e32.v v4, (a0), a1
	vssseg3e32.v v4, 0(a0), a1
	vssseg3e32.v v4, (a0), a1, v0.t

	vlsseg4e32.v v4, (a0), a1
	vlsseg4e32.v v4, 0(a0), a1
	vlsseg4e32.v v4, (a0), a1, v0.t
	vssseg4e32.v v4, (a0), a1
	vssseg4e32.v v4, 0(a0), a1
	vssseg4e32.v v4, (a0), a1, v0.t

	vlsseg5e32.v v4, (a0), a1
	vlsseg5e32.v v4, 0(a0), a1
	vlsseg5e32.v v4, (a0), a1, v0.t
	vssseg5e32.v v4, (a0), a1
	vssseg5e32.v v4, 0(a0), a1
	vssseg5e32.v v4, (a0), a1, v0.t

	vlsseg6e32.v v4, (a0), a1
	vlsseg6e32.v v4, 0(a0), a1
	vlsseg6e32.v v4, (a0), a1, v0.t
	vssseg6e32.v v4, (a0), a1
	vssseg6e32.v v4, 0(a0), a1
	vssseg6e32.v v4, (a0), a1, v0.t

	vlsseg7e32.v v4, (a0), a1
	vlsseg7e32.v v4, 0(a0), a1
	vlsseg7e32.v v4, (a0), a1, v0.t
	vssseg7e32.v v4, (a0), a1
	vssseg7e32.v v4, 0(a0), a1
	vssseg7e32.v v4, (a0), a1, v0.t

	vlsseg8e32.v v4, (a0), a1
	vlsseg8e32.v v4, 0(a0), a1
	vlsseg8e32.v v4, (a0), a1, v0.t
	vssseg8e32.v v4, (a0), a1
	vssseg8e32.v v4, 0(a0), a1
	vssseg8e32.v v4, (a0), a1, v0.t

	vlsseg2e64.v v4, (a0), a1
	vlsseg2e64.v v4, 0(a0), a1
	vlsseg2e64.v v4, (a0), a1, v0.t
	vssseg2e64.v v4, (a0), a1
	vssseg2e64.v v4, 0(a0), a1
	vssseg2e64.v v4, (a0), a1, v0.t

	vlsseg3e64.v v4, (a0), a1
	vlsseg3e64.v v4, 0(a0), a1
	vlsseg3e64.v v4, (a0), a1, v0.t
	vssseg3e64.v v4, (a0), a1
	vssseg3e64.v v4, 0(a0), a1
	vssseg3e64.v v4, (a0), a1, v0.t

	vlsseg4e64.v v4, (a0), a1
	vlsseg4e64.v v4, 0(a0), a1
	vlsseg4e64.v v4, (a0), a1, v0.t
	vssseg4e64.v v4, (a0), a1
	vssseg4e64.v v4, 0(a0), a1
	vssseg4e64.v v4, (a0), a1, v0.t

	vlsseg5e64.v v4, (a0), a1
	vlsseg5e64.v v4, 0(a0), a1
	vlsseg5e64.v v4, (a0), a1, v0.t
	vssseg5e64.v v4, (a0), a1
	vssseg5e64.v v4, 0(a0), a1
	vssseg5e64.v v4, (a0), a1, v0.t

	vlsseg6e64.v v4, (a0), a1
	vlsseg6e64.v v4, 0(a0), a1
	vlsseg6e64.v v4, (a0), a1, v0.t
	vssseg6e64.v v4, (a0), a1
	vssseg6e64.v v4, 0(a0), a1
	vssseg6e64.v v4, (a0), a1, v0.t

	vlsseg7e64.v v4, (a0), a1
	vlsseg7e64.v v4, 0(a0), a1
	vlsseg7e64.v v4, (a0), a1, v0.t
	vssseg7e64.v v4, (a0), a1
	vssseg7e64.v v4, 0(a0), a1
	vssseg7e64.v v4, (a0), a1, v0.t

	vlsseg8e64.v v4, (a0), a1
	vlsseg8e64.v v4, 0(a0), a1
	vlsseg8e64.v v4, (a0), a1, v0.t
	vssseg8e64.v v4, (a0), a1
	vssseg8e64.v v4, 0(a0), a1
	vssseg8e64.v v4, (a0), a1, v0.t

	vloxseg2ei8.v v4, (a0), v12
	vloxseg2ei8.v v4, 0(a0), v12
	vloxseg2ei8.v v4, (a0), v12, v0.t
	vsoxseg2ei8.v v4, (a0), v12
	vsoxseg2ei8.v v4, 0(a0), v12
	vsoxseg2ei8.v v4, (a0), v12, v0.t

	vloxseg3ei8.v v4, (a0), v12
	vloxseg3ei8.v v4, 0(a0), v12
	vloxseg3ei8.v v4, (a0), v12, v0.t
	vsoxseg3ei8.v v4, (a0), v12
	vsoxseg3ei8.v v4, 0(a0), v12
	vsoxseg3ei8.v v4, (a0), v12, v0.t

	vloxseg4ei8.v v4, (a0), v12
	vloxseg4ei8.v v4, 0(a0), v12
	vloxseg4ei8.v v4, (a0), v12, v0.t
	vsoxseg4ei8.v v4, (a0), v12
	vsoxseg4ei8.v v4, 0(a0), v12
	vsoxseg4ei8.v v4, (a0), v12, v0.t

	vloxseg5ei8.v v4, (a0), v12
	vloxseg5ei8.v v4, 0(a0), v12
	vloxseg5ei8.v v4, (a0), v12, v0.t
	vsoxseg5ei8.v v4, (a0), v12
	vsoxseg5ei8.v v4, 0(a0), v12
	vsoxseg5ei8.v v4, (a0), v12, v0.t

	vloxseg6ei8.v v4, (a0), v12
	vloxseg6ei8.v v4, 0(a0), v12
	vloxseg6ei8.v v4, (a0), v12, v0.t
	vsoxseg6ei8.v v4, (a0), v12
	vsoxseg6ei8.v v4, 0(a0), v12
	vsoxseg6ei8.v v4, (a0), v12, v0.t

	vloxseg7ei8.v v4, (a0), v12
	vloxseg7ei8.v v4, 0(a0), v12
	vloxseg7ei8.v v4, (a0), v12, v0.t
	vsoxseg7ei8.v v4, (a0), v12
	vsoxseg7ei8.v v4, 0(a0), v12
	vsoxseg7ei8.v v4, (a0), v12, v0.t

	vloxseg8ei8.v v4, (a0), v12
	vloxseg8ei8.v v4, 0(a0), v12
	vloxseg8ei8.v v4, (a0), v12, v0.t
	vsoxseg8ei8.v v4, (a0), v12
	vsoxseg8ei8.v v4, 0(a0), v12
	vsoxseg8ei8.v v4, (a0), v12, v0.t

	vloxseg2ei16.v v4, (a0), v12
	vloxseg2ei16.v v4, 0(a0), v12
	vloxseg2ei16.v v4, (a0), v12, v0.t
	vsoxseg2ei16.v v4, (a0), v12
	vsoxseg2ei16.v v4, 0(a0), v12
	vsoxseg2ei16.v v4, (a0), v12, v0.t

	vloxseg3ei16.v v4, (a0), v12
	vloxseg3ei16.v v4, 0(a0), v12
	vloxseg3ei16.v v4, (a0), v12, v0.t
	vsoxseg3ei16.v v4, (a0), v12
	vsoxseg3ei16.v v4, 0(a0), v12
	vsoxseg3ei16.v v4, (a0), v12, v0.t

	vloxseg4ei16.v v4, (a0), v12
	vloxseg4ei16.v v4, 0(a0), v12
	vloxseg4ei16.v v4, (a0), v12, v0.t
	vsoxseg4ei16.v v4, (a0), v12
	vsoxseg4ei16.v v4, 0(a0), v12
	vsoxseg4ei16.v v4, (a0), v12, v0.t

	vloxseg5ei16.v v4, (a0), v12
	vloxseg5ei16.v v4, 0(a0), v12
	vloxseg5ei16.v v4, (a0), v12, v0.t
	vsoxseg5ei16.v v4, (a0), v12
	vsoxseg5ei16.v v4, 0(a0), v12
	vsoxseg5ei16.v v4, (a0), v12, v0.t

	vloxseg6ei16.v v4, (a0), v12
	vloxseg6ei16.v v4, 0(a0), v12
	vloxseg6ei16.v v4, (a0), v12, v0.t
	vsoxseg6ei16.v v4, (a0), v12
	vsoxseg6ei16.v v4, 0(a0), v12
	vsoxseg6ei16.v v4, (a0), v12, v0.t

	vloxseg7ei16.v v4, (a0), v12
	vloxseg7ei16.v v4, 0(a0), v12
	vloxseg7ei16.v v4, (a0), v12, v0.t
	vsoxseg7ei16.v v4, (a0), v12
	vsoxseg7ei16.v v4, 0(a0), v12
	vsoxseg7ei16.v v4, (a0), v12, v0.t

	vloxseg8ei16.v v4, (a0), v12
	vloxseg8ei16.v v4, 0(a0), v12
	vloxseg8ei16.v v4, (a0), v12, v0.t
	vsoxseg8ei16.v v4, (a0), v12
	vsoxseg8ei16.v v4, 0(a0), v12
	vsoxseg8ei16.v v4, (a0), v12, v0.t

	vloxseg2ei32.v v4, (a0), v12
	vloxseg2ei32.v v4, 0(a0), v12
	vloxseg2ei32.v v4, (a0), v12, v0.t
	vsoxseg2ei32.v v4, (a0), v12
	vsoxseg2ei32.v v4, 0(a0), v12
	vsoxseg2ei32.v v4, (a0), v12, v0.t

	vloxseg3ei32.v v4, (a0), v12
	vloxseg3ei32.v v4, 0(a0), v12
	vloxseg3ei32.v v4, (a0), v12, v0.t
	vsoxseg3ei32.v v4, (a0), v12
	vsoxseg3ei32.v v4, 0(a0), v12
	vsoxseg3ei32.v v4, (a0), v12, v0.t

	vloxseg4ei32.v v4, (a0), v12
	vloxseg4ei32.v v4, 0(a0), v12
	vloxseg4ei32.v v4, (a0), v12, v0.t
	vsoxseg4ei32.v v4, (a0), v12
	vsoxseg4ei32.v v4, 0(a0), v12
	vsoxseg4ei32.v v4, (a0), v12, v0.t

	vloxseg5ei32.v v4, (a0), v12
	vloxseg5ei32.v v4, 0(a0), v12
	vloxseg5ei32.v v4, (a0), v12, v0.t
	vsoxseg5ei32.v v4, (a0), v12
	vsoxseg5ei32.v v4, 0(a0), v12
	vsoxseg5ei32.v v4, (a0), v12, v0.t

	vloxseg6ei32.v v4, (a0), v12
	vloxseg6ei32.v v4, 0(a0), v12
	vloxseg6ei32.v v4, (a0), v12, v0.t
	vsoxseg6ei32.v v4, (a0), v12
	vsoxseg6ei32.v v4, 0(a0), v12
	vsoxseg6ei32.v v4, (a0), v12, v0.t

	vloxseg7ei32.v v4, (a0), v12
	vloxseg7ei32.v v4, 0(a0), v12
	vloxseg7ei32.v v4, (a0), v12, v0.t
	vsoxseg7ei32.v v4, (a0), v12
	vsoxseg7ei32.v v4, 0(a0), v12
	vsoxseg7ei32.v v4, (a0), v12, v0.t

	vloxseg8ei32.v v4, (a0), v12
	vloxseg8ei32.v v4, 0(a0), v12
	vloxseg8ei32.v v4, (a0), v12, v0.t
	vsoxseg8ei32.v v4, (a0), v12
	vsoxseg8ei32.v v4, 0(a0), v12
	vsoxseg8ei32.v v4, (a0), v12, v0.t

	vloxseg2ei64.v v4, (a0), v12
	vloxseg2ei64.v v4, 0(a0), v12
	vloxseg2ei64.v v4, (a0), v12, v0.t
	vsoxseg2ei64.v v4, (a0), v12
	vsoxseg2ei64.v v4, 0(a0), v12
	vsoxseg2ei64.v v4, (a0), v12, v0.t

	vloxseg3ei64.v v4, (a0), v12
	vloxseg3ei64.v v4, 0(a0), v12
	vloxseg3ei64.v v4, (a0), v12, v0.t
	vsoxseg3ei64.v v4, (a0), v12
	vsoxseg3ei64.v v4, 0(a0), v12
	vsoxseg3ei64.v v4, (a0), v12, v0.t

	vloxseg4ei64.v v4, (a0), v12
	vloxseg4ei64.v v4, 0(a0), v12
	vloxseg4ei64.v v4, (a0), v12, v0.t
	vsoxseg4ei64.v v4, (a0), v12
	vsoxseg4ei64.v v4, 0(a0), v12
	vsoxseg4ei64.v v4, (a0), v12, v0.t

	vloxseg5ei64.v v4, (a0), v12
	vloxseg5ei64.v v4, 0(a0), v12
	vloxseg5ei64.v v4, (a0), v12, v0.t
	vsoxseg5ei64.v v4, (a0), v12
	vsoxseg5ei64.v v4, 0(a0), v12
	vsoxseg5ei64.v v4, (a0), v12, v0.t

	vloxseg6ei64.v v4, (a0), v12
	vloxseg6ei64.v v4, 0(a0), v12
	vloxseg6ei64.v v4, (a0), v12, v0.t
	vsoxseg6ei64.v v4, (a0), v12
	vsoxseg6ei64.v v4, 0(a0), v12
	vsoxseg6ei64.v v4, (a0), v12, v0.t

	vloxseg7ei64.v v4, (a0), v12
	vloxseg7ei64.v v4, 0(a0), v12
	vloxseg7ei64.v v4, (a0), v12, v0.t
	vsoxseg7ei64.v v4, (a0), v12
	vsoxseg7ei64.v v4, 0(a0), v12
	vsoxseg7ei64.v v4, (a0), v12, v0.t

	vloxseg8ei64.v v4, (a0), v12
	vloxseg8ei64.v v4, 0(a0), v12
	vloxseg8ei64.v v4, (a0), v12, v0.t
	vsoxseg8ei64.v v4, (a0), v12
	vsoxseg8ei64.v v4, 0(a0), v12
	vsoxseg8ei64.v v4, (a0), v12, v0.t

	vluxseg2ei8.v v4, (a0), v12
	vluxseg2ei8.v v4, 0(a0), v12
	vluxseg2ei8.v v4, (a0), v12, v0.t
	vsuxseg2ei8.v v4, (a0), v12
	vsuxseg2ei8.v v4, 0(a0), v12
	vsuxseg2ei8.v v4, (a0), v12, v0.t

	vluxseg3ei8.v v4, (a0), v12
	vluxseg3ei8.v v4, 0(a0), v12
	vluxseg3ei8.v v4, (a0), v12, v0.t
	vsuxseg3ei8.v v4, (a0), v12
	vsuxseg3ei8.v v4, 0(a0), v12
	vsuxseg3ei8.v v4, (a0), v12, v0.t

	vluxseg4ei8.v v4, (a0), v12
	vluxseg4ei8.v v4, 0(a0), v12
	vluxseg4ei8.v v4, (a0), v12, v0.t
	vsuxseg4ei8.v v4, (a0), v12
	vsuxseg4ei8.v v4, 0(a0), v12
	vsuxseg4ei8.v v4, (a0), v12, v0.t

	vluxseg5ei8.v v4, (a0), v12
	vluxseg5ei8.v v4, 0(a0), v12
	vluxseg5ei8.v v4, (a0), v12, v0.t
	vsuxseg5ei8.v v4, (a0), v12
	vsuxseg5ei8.v v4, 0(a0), v12
	vsuxseg5ei8.v v4, (a0), v12, v0.t

	vluxseg6ei8.v v4, (a0), v12
	vluxseg6ei8.v v4, 0(a0), v12
	vluxseg6ei8.v v4, (a0), v12, v0.t
	vsuxseg6ei8.v v4, (a0), v12
	vsuxseg6ei8.v v4, 0(a0), v12
	vsuxseg6ei8.v v4, (a0), v12, v0.t

	vluxseg7ei8.v v4, (a0), v12
	vluxseg7ei8.v v4, 0(a0), v12
	vluxseg7ei8.v v4, (a0), v12, v0.t
	vsuxseg7ei8.v v4, (a0), v12
	vsuxseg7ei8.v v4, 0(a0), v12
	vsuxseg7ei8.v v4, (a0), v12, v0.t

	vluxseg8ei8.v v4, (a0), v12
	vluxseg8ei8.v v4, 0(a0), v12
	vluxseg8ei8.v v4, (a0), v12, v0.t
	vsuxseg8ei8.v v4, (a0), v12
	vsuxseg8ei8.v v4, 0(a0), v12
	vsuxseg8ei8.v v4, (a0), v12, v0.t

	vluxseg2ei16.v v4, (a0), v12
	vluxseg2ei16.v v4, 0(a0), v12
	vluxseg2ei16.v v4, (a0), v12, v0.t
	vsuxseg2ei16.v v4, (a0), v12
	vsuxseg2ei16.v v4, 0(a0), v12
	vsuxseg2ei16.v v4, (a0), v12, v0.t

	vluxseg3ei16.v v4, (a0), v12
	vluxseg3ei16.v v4, 0(a0), v12
	vluxseg3ei16.v v4, (a0), v12, v0.t
	vsuxseg3ei16.v v4, (a0), v12
	vsuxseg3ei16.v v4, 0(a0), v12
	vsuxseg3ei16.v v4, (a0), v12, v0.t

	vluxseg4ei16.v v4, (a0), v12
	vluxseg4ei16.v v4, 0(a0), v12
	vluxseg4ei16.v v4, (a0), v12, v0.t
	vsuxseg4ei16.v v4, (a0), v12
	vsuxseg4ei16.v v4, 0(a0), v12
	vsuxseg4ei16.v v4, (a0), v12, v0.t

	vluxseg5ei16.v v4, (a0), v12
	vluxseg5ei16.v v4, 0(a0), v12
	vluxseg5ei16.v v4, (a0), v12, v0.t
	vsuxseg5ei16.v v4, (a0), v12
	vsuxseg5ei16.v v4, 0(a0), v12
	vsuxseg5ei16.v v4, (a0), v12, v0.t

	vluxseg6ei16.v v4, (a0), v12
	vluxseg6ei16.v v4, 0(a0), v12
	vluxseg6ei16.v v4, (a0), v12, v0.t
	vsuxseg6ei16.v v4, (a0), v12
	vsuxseg6ei16.v v4, 0(a0), v12
	vsuxseg6ei16.v v4, (a0), v12, v0.t

	vluxseg7ei16.v v4, (a0), v12
	vluxseg7ei16.v v4, 0(a0), v12
	vluxseg7ei16.v v4, (a0), v12, v0.t
	vsuxseg7ei16.v v4, (a0), v12
	vsuxseg7ei16.v v4, 0(a0), v12
	vsuxseg7ei16.v v4, (a0), v12, v0.t

	vluxseg8ei16.v v4, (a0), v12
	vluxseg8ei16.v v4, 0(a0), v12
	vluxseg8ei16.v v4, (a0), v12, v0.t
	vsuxseg8ei16.v v4, (a0), v12
	vsuxseg8ei16.v v4, 0(a0), v12
	vsuxseg8ei16.v v4, (a0), v12, v0.t

	vluxseg2ei32.v v4, (a0), v12
	vluxseg2ei32.v v4, 0(a0), v12
	vluxseg2ei32.v v4, (a0), v12, v0.t
	vsuxseg2ei32.v v4, (a0), v12
	vsuxseg2ei32.v v4, 0(a0), v12
	vsuxseg2ei32.v v4, (a0), v12, v0.t

	vluxseg3ei32.v v4, (a0), v12
	vluxseg3ei32.v v4, 0(a0), v12
	vluxseg3ei32.v v4, (a0), v12, v0.t
	vsuxseg3ei32.v v4, (a0), v12
	vsuxseg3ei32.v v4, 0(a0), v12
	vsuxseg3ei32.v v4, (a0), v12, v0.t

	vluxseg4ei32.v v4, (a0), v12
	vluxseg4ei32.v v4, 0(a0), v12
	vluxseg4ei32.v v4, (a0), v12, v0.t
	vsuxseg4ei32.v v4, (a0), v12
	vsuxseg4ei32.v v4, 0(a0), v12
	vsuxseg4ei32.v v4, (a0), v12, v0.t

	vluxseg5ei32.v v4, (a0), v12
	vluxseg5ei32.v v4, 0(a0), v12
	vluxseg5ei32.v v4, (a0), v12, v0.t
	vsuxseg5ei32.v v4, (a0), v12
	vsuxseg5ei32.v v4, 0(a0), v12
	vsuxseg5ei32.v v4, (a0), v12, v0.t

	vluxseg6ei32.v v4, (a0), v12
	vluxseg6ei32.v v4, 0(a0), v12
	vluxseg6ei32.v v4, (a0), v12, v0.t
	vsuxseg6ei32.v v4, (a0), v12
	vsuxseg6ei32.v v4, 0(a0), v12
	vsuxseg6ei32.v v4, (a0), v12, v0.t

	vluxseg7ei32.v v4, (a0), v12
	vluxseg7ei32.v v4, 0(a0), v12
	vluxseg7ei32.v v4, (a0), v12, v0.t
	vsuxseg7ei32.v v4, (a0), v12
	vsuxseg7ei32.v v4, 0(a0), v12
	vsuxseg7ei32.v v4, (a0), v12, v0.t

	vluxseg8ei32.v v4, (a0), v12
	vluxseg8ei32.v v4, 0(a0), v12
	vluxseg8ei32.v v4, (a0), v12, v0.t
	vsuxseg8ei32.v v4, (a0), v12
	vsuxseg8ei32.v v4, 0(a0), v12
	vsuxseg8ei32.v v4, (a0), v12, v0.t

	vluxseg2ei64.v v4, (a0), v12
	vluxseg2ei64.v v4, 0(a0), v12
	vluxseg2ei64.v v4, (a0), v12, v0.t
	vsuxseg2ei64.v v4, (a0), v12
	vsuxseg2ei64.v v4, 0(a0), v12
	vsuxseg2ei64.v v4, (a0), v12, v0.t

	vluxseg3ei64.v v4, (a0), v12
	vluxseg3ei64.v v4, 0(a0), v12
	vluxseg3ei64.v v4, (a0), v12, v0.t
	vsuxseg3ei64.v v4, (a0), v12
	vsuxseg3ei64.v v4, 0(a0), v12
	vsuxseg3ei64.v v4, (a0), v12, v0.t

	vluxseg4ei64.v v4, (a0), v12
	vluxseg4ei64.v v4, 0(a0), v12
	vluxseg4ei64.v v4, (a0), v12, v0.t
	vsuxseg4ei64.v v4, (a0), v12
	vsuxseg4ei64.v v4, 0(a0), v12
	vsuxseg4ei64.v v4, (a0), v12, v0.t

	vluxseg5ei64.v v4, (a0), v12
	vluxseg5ei64.v v4, 0(a0), v12
	vluxseg5ei64.v v4, (a0), v12, v0.t
	vsuxseg5ei64.v v4, (a0), v12
	vsuxseg5ei64.v v4, 0(a0), v12
	vsuxseg5ei64.v v4, (a0), v12, v0.t

	vluxseg6ei64.v v4, (a0), v12
	vluxseg6ei64.v v4, 0(a0), v12
	vluxseg6ei64.v v4, (a0), v12, v0.t
	vsuxseg6ei64.v v4, (a0), v12
	vsuxseg6ei64.v v4, 0(a0), v12
	vsuxseg6ei64.v v4, (a0), v12, v0.t

	vluxseg7ei64.v v4, (a0), v12
	vluxseg7ei64.v v4, 0(a0), v12
	vluxseg7ei64.v v4, (a0), v12, v0.t
	vsuxseg7ei64.v v4, (a0), v12
	vsuxseg7ei64.v v4, 0(a0), v12
	vsuxseg7ei64.v v4, (a0), v12, v0.t

	vluxseg8ei64.v v4, (a0), v12
	vluxseg8ei64.v v4, 0(a0), v12
	vluxseg8ei64.v v4, (a0), v12, v0.t
	vsuxseg8ei64.v v4, (a0), v12
	vsuxseg8ei64.v v4, 0(a0), v12
	vsuxseg8ei64.v v4, (a0), v12, v0.t

	vlseg2e8ff.v v4, (a0)
	vlseg2e8ff.v v4, 0(a0)
	vlseg2e8ff.v v4, (a0), v0.t

	vlseg3e8ff.v v4, (a0)
	vlseg3e8ff.v v4, 0(a0)
	vlseg3e8ff.v v4, (a0), v0.t

	vlseg4e8ff.v v4, (a0)
	vlseg4e8ff.v v4, 0(a0)
	vlseg4e8ff.v v4, (a0), v0.t

	vlseg5e8ff.v v4, (a0)
	vlseg5e8ff.v v4, 0(a0)
	vlseg5e8ff.v v4, (a0), v0.t

	vlseg6e8ff.v v4, (a0)
	vlseg6e8ff.v v4, 0(a0)
	vlseg6e8ff.v v4, (a0), v0.t

	vlseg7e8ff.v v4, (a0)
	vlseg7e8ff.v v4, 0(a0)
	vlseg7e8ff.v v4, (a0), v0.t

	vlseg8e8ff.v v4, (a0)
	vlseg8e8ff.v v4, 0(a0)
	vlseg8e8ff.v v4, (a0), v0.t

	vlseg2e16ff.v v4, (a0)
	vlseg2e16ff.v v4, 0(a0)
	vlseg2e16ff.v v4, (a0), v0.t

	vlseg3e16ff.v v4, (a0)
	vlseg3e16ff.v v4, 0(a0)
	vlseg3e16ff.v v4, (a0), v0.t

	vlseg4e16ff.v v4, (a0)
	vlseg4e16ff.v v4, 0(a0)
	vlseg4e16ff.v v4, (a0), v0.t

	vlseg5e16ff.v v4, (a0)
	vlseg5e16ff.v v4, 0(a0)
	vlseg5e16ff.v v4, (a0), v0.t

	vlseg6e16ff.v v4, (a0)
	vlseg6e16ff.v v4, 0(a0)
	vlseg6e16ff.v v4, (a0), v0.t

	vlseg7e16ff.v v4, (a0)
	vlseg7e16ff.v v4, 0(a0)
	vlseg7e16ff.v v4, (a0), v0.t

	vlseg8e16ff.v v4, (a0)
	vlseg8e16ff.v v4, 0(a0)
	vlseg8e16ff.v v4, (a0), v0.t

	vlseg2e32ff.v v4, (a0)
	vlseg2e32ff.v v4, 0(a0)
	vlseg2e32ff.v v4, (a0), v0.t

	vlseg3e32ff.v v4, (a0)
	vlseg3e32ff.v v4, 0(a0)
	vlseg3e32ff.v v4, (a0), v0.t

	vlseg4e32ff.v v4, (a0)
	vlseg4e32ff.v v4, 0(a0)
	vlseg4e32ff.v v4, (a0), v0.t

	vlseg5e32ff.v v4, (a0)
	vlseg5e32ff.v v4, 0(a0)
	vlseg5e32ff.v v4, (a0), v0.t

	vlseg6e32ff.v v4, (a0)
	vlseg6e32ff.v v4, 0(a0)
	vlseg6e32ff.v v4, (a0), v0.t

	vlseg7e32ff.v v4, (a0)
	vlseg7e32ff.v v4, 0(a0)
	vlseg7e32ff.v v4, (a0), v0.t

	vlseg8e32ff.v v4, (a0)
	vlseg8e32ff.v v4, 0(a0)
	vlseg8e32ff.v v4, (a0), v0.t

	vlseg2e64ff.v v4, (a0)
	vlseg2e64ff.v v4, 0(a0)
	vlseg2e64ff.v v4, (a0), v0.t

	vlseg3e64ff.v v4, (a0)
	vlseg3e64ff.v v4, 0(a0)
	vlseg3e64ff.v v4, (a0), v0.t

	vlseg4e64ff.v v4, (a0)
	vlseg4e64ff.v v4, 0(a0)
	vlseg4e64ff.v v4, (a0), v0.t

	vlseg5e64ff.v v4, (a0)
	vlseg5e64ff.v v4, 0(a0)
	vlseg5e64ff.v v4, (a0), v0.t

	vlseg6e64ff.v v4, (a0)
	vlseg6e64ff.v v4, 0(a0)
	vlseg6e64ff.v v4, (a0), v0.t

	vlseg7e64ff.v v4, (a0)
	vlseg7e64ff.v v4, 0(a0)
	vlseg7e64ff.v v4, (a0), v0.t

	vlseg8e64ff.v v4, (a0)
	vlseg8e64ff.v v4, 0(a0)
	vlseg8e64ff.v v4, (a0), v0.t

	vl1r.v v3, (a0)
	vl1r.v v3, 0(a0)
	vl1re8.v v3, (a0)
	vl1re8.v v3, 0(a0)
	vl1re16.v v3, (a0)
	vl1re16.v v3, 0(a0)
	vl1re32.v v3, (a0)
	vl1re32.v v3, 0(a0)
	vl1re64.v v3, (a0)
	vl1re64.v v3, 0(a0)

	vl2r.v v2, (a0)
	vl2r.v v2, 0(a0)
	vl2re8.v v2, (a0)
	vl2re8.v v2, 0(a0)
	vl2re16.v v2, (a0)
	vl2re16.v v2, 0(a0)
	vl2re32.v v2, (a0)
	vl2re32.v v2, 0(a0)
	vl2re64.v v2, (a0)
	vl2re64.v v2, 0(a0)

	vl4r.v v4, (a0)
	vl4r.v v4, 0(a0)
	vl4re8.v v4, (a0)
	vl4re8.v v4, 0(a0)
	vl4re16.v v4, (a0)
	vl4re16.v v4, 0(a0)
	vl4re32.v v4, (a0)
	vl4re32.v v4, 0(a0)
	vl4re64.v v4, (a0)
	vl4re64.v v4, 0(a0)

	vl8r.v v8, (a0)
	vl8r.v v8, 0(a0)
	vl8re8.v v8, (a0)
	vl8re8.v v8, 0(a0)
	vl8re16.v v8, (a0)
	vl8re16.v v8, 0(a0)
	vl8re32.v v8, (a0)
	vl8re32.v v8, 0(a0)
	vl8re64.v v8, (a0)
	vl8re64.v v8, 0(a0)

	vs1r.v v3, (a1)
	vs1r.v v3, 0(a1)
	vs2r.v v2, (a1)
	vs2r.v v2, 0(a1)
	vs4r.v v4, (a1)
	vs4r.v v4, 0(a1)
	vs8r.v v8, (a1)
	vs8r.v v8, 0(a1)

	vneg.v v4, v8
	vneg.v v4, v8, v0.t

	vadd.vv v4, v8, v12
	vadd.vx v4, v8, a1
	vadd.vi v4, v8, 15
	vadd.vi v4, v8, -16
	vadd.vv v4, v8, v12, v0.t
	vadd.vx v4, v8, a1, v0.t
	vadd.vi v4, v8, 15, v0.t
	vadd.vi v4, v8, -16, v0.t
	vsub.vv v4, v8, v12
	vsub.vx v4, v8, a1
	vrsub.vx v4, v8, a1
	vrsub.vi v4, v8, 15
	vrsub.vi v4, v8, -16
	vsub.vv v4, v8, v12, v0.t
	vsub.vx v4, v8, a1, v0.t
	vrsub.vx v4, v8, a1, v0.t
	vrsub.vi v4, v8, 15, v0.t
	vrsub.vi v4, v8, -16, v0.t

	# Aliases
	vwcvt.x.x.v v4, v8
	vwcvtu.x.x.v v4, v8
	vwcvt.x.x.v v4, v8, v0.t
	vwcvtu.x.x.v v4, v8, v0.t

	vwaddu.vv v4, v8, v12
	vwaddu.vx v4, v8, a1
	vwaddu.vv v4, v8, v12, v0.t
	vwaddu.vx v4, v8, a1, v0.t
	vwsubu.vv v4, v8, v12
	vwsubu.vx v4, v8, a1
	vwsubu.vv v4, v8, v12, v0.t
	vwsubu.vx v4, v8, a1, v0.t
	vwadd.vv v4, v8, v12
	vwadd.vx v4, v8, a1
	vwadd.vv v4, v8, v12, v0.t
	vwadd.vx v4, v8, a1, v0.t
	vwsub.vv v4, v8, v12
	vwsub.vx v4, v8, a1
	vwsub.vv v4, v8, v12, v0.t
	vwsub.vx v4, v8, a1, v0.t
	vwaddu.wv v4, v8, v12
	vwaddu.wx v4, v8, a1
	vwaddu.wv v4, v8, v12, v0.t
	vwaddu.wx v4, v8, a1, v0.t
	vwsubu.wv v4, v8, v12
	vwsubu.wx v4, v8, a1
	vwsubu.wv v4, v8, v12, v0.t
	vwsubu.wx v4, v8, a1, v0.t
	vwadd.wv v4, v8, v12
	vwadd.wx v4, v8, a1
	vwadd.wv v4, v8, v12, v0.t
	vwadd.wx v4, v8, a1, v0.t
	vwsub.wv v4, v8, v12
	vwsub.wx v4, v8, a1
	vwsub.wv v4, v8, v12, v0.t
	vwsub.wx v4, v8, a1, v0.t

	vzext.vf2 v4, v8
	vzext.vf2 v4, v8, v0.t
	vsext.vf2 v4, v8
	vsext.vf2 v4, v8, v0.t
	vzext.vf4 v4, v8
	vzext.vf4 v4, v8, v0.t
	vsext.vf4 v4, v8
	vsext.vf4 v4, v8, v0.t
	vzext.vf8 v4, v8
	vzext.vf8 v4, v8, v0.t
	vsext.vf8 v4, v8
	vsext.vf8 v4, v8, v0.t

	vadc.vvm v4, v8, v12, v0
	vadc.vxm v4, v8, a1, v0
	vadc.vim v4, v8, 15, v0
	vadc.vim v4, v8, -16, v0
	vmadc.vvm v4, v8, v12, v0
	vmadc.vxm v4, v8, a1, v0
	vmadc.vim v4, v8, 15, v0
	vmadc.vim v4, v8, -16, v0
	vmadc.vv v4, v8, v12
	vmadc.vx v4, v8, a1
	vmadc.vi v4, v8, 15
	vmadc.vi v4, v8, -16
	vsbc.vvm v4, v8, v12, v0
	vsbc.vxm v4, v8, a1, v0
	vmsbc.vvm v4, v8, v12, v0
	vmsbc.vxm v4, v8, a1, v0
	vmsbc.vv v4, v8, v12
	vmsbc.vx v4, v8, a1

	# Aliases
	vnot.v v4, v8
	vnot.v v4, v8, v0.t

	vand.vv v4, v8, v12
	vand.vx v4, v8, a1
	vand.vi v4, v8, 15
	vand.vi v4, v8, -16
	vand.vv v4, v8, v12, v0.t
	vand.vx v4, v8, a1, v0.t
	vand.vi v4, v8, 15, v0.t
	vand.vi v4, v8, -16, v0.t
	vor.vv v4, v8, v12
	vor.vx v4, v8, a1
	vor.vi v4, v8, 15
	vor.vi v4, v8, -16
	vor.vv v4, v8, v12, v0.t
	vor.vx v4, v8, a1, v0.t
	vor.vi v4, v8, 15, v0.t
	vor.vi v4, v8, -16, v0.t
	vxor.vv v4, v8, v12
	vxor.vx v4, v8, a1
	vxor.vi v4, v8, 15
	vxor.vi v4, v8, -16
	vxor.vv v4, v8, v12, v0.t
	vxor.vx v4, v8, a1, v0.t
	vxor.vi v4, v8, 15, v0.t
	vxor.vi v4, v8, -16, v0.t

	vsll.vv v4, v8, v12
	vsll.vx v4, v8, a1
	vsll.vi v4, v8, 1
	vsll.vi v4, v8, 31
	vsll.vv v4, v8, v12, v0.t
	vsll.vx v4, v8, a1, v0.t
	vsll.vi v4, v8, 1, v0.t
	vsll.vi v4, v8, 31, v0.t
	vsrl.vv v4, v8, v12
	vsrl.vx v4, v8, a1
	vsrl.vi v4, v8, 1
	vsrl.vi v4, v8, 31
	vsrl.vv v4, v8, v12, v0.t
	vsrl.vx v4, v8, a1, v0.t
	vsrl.vi v4, v8, 1, v0.t
	vsrl.vi v4, v8, 31, v0.t
	vsra.vv v4, v8, v12
	vsra.vx v4, v8, a1
	vsra.vi v4, v8, 1
	vsra.vi v4, v8, 31
	vsra.vv v4, v8, v12, v0.t
	vsra.vx v4, v8, a1, v0.t
	vsra.vi v4, v8, 1, v0.t
	vsra.vi v4, v8, 31, v0.t

	# Aliases
	vncvt.x.x.w v4, v8
	vncvt.x.x.w v4, v8, v0.t

	vnsrl.wv v4, v8, v12
	vnsrl.wx v4, v8, a1
	vnsrl.wi v4, v8, 1
	vnsrl.wi v4, v8, 31
	vnsrl.wv v4, v8, v12, v0.t
	vnsrl.wx v4, v8, a1, v0.t
	vnsrl.wi v4, v8, 1, v0.t
	vnsrl.wi v4, v8, 31, v0.t
	vnsra.wv v4, v8, v12
	vnsra.wx v4, v8, a1
	vnsra.wi v4, v8, 1
	vnsra.wi v4, v8, 31
	vnsra.wv v4, v8, v12, v0.t
	vnsra.wx v4, v8, a1, v0.t
	vnsra.wi v4, v8, 1, v0.t
	vnsra.wi v4, v8, 31, v0.t

	# Aliases
	vmsgt.vv v4, v8, v12
	vmsgtu.vv v4, v8, v12
	vmsge.vv v4, v8, v12
	vmsgeu.vv v4, v8, v12
	vmsgt.vv v4, v8, v12, v0.t
	vmsgtu.vv v4, v8, v12, v0.t
	vmsge.vv v4, v8, v12, v0.t
	vmsgeu.vv v4, v8, v12, v0.t
	vmslt.vi v4, v8, 16
	vmslt.vi v4, v8, -15
	vmsltu.vi v4, v8, 16
	vmsltu.vi v4, v8, -15
	vmsge.vi v4, v8, 16
	vmsge.vi v4, v8, -15
	vmsgeu.vi v4, v8, 16
	vmsgeu.vi v4, v8, -15
	vmslt.vi v4, v8, 16, v0.t
	vmslt.vi v4, v8, -15, v0.t
	vmsltu.vi v4, v8, 16, v0.t
	vmsltu.vi v4, v8, -15, v0.t
	vmsge.vi v4, v8, 16, v0.t
	vmsge.vi v4, v8, -15, v0.t
	vmsgeu.vi v4, v8, 16, v0.t
	vmsgeu.vi v4, v8, -15, v0.t

	vmseq.vv v4, v8, v12
	vmseq.vx v4, v8, a1
	vmseq.vi v4, v8, 15
	vmseq.vi v4, v8, -16
	vmseq.vv v4, v8, v12, v0.t
	vmseq.vx v4, v8, a1, v0.t
	vmseq.vi v4, v8, 15, v0.t
	vmseq.vi v4, v8, -16, v0.t
	vmsne.vv v4, v8, v12
	vmsne.vx v4, v8, a1
	vmsne.vi v4, v8, 15
	vmsne.vi v4, v8, -16
	vmsne.vv v4, v8, v12, v0.t
	vmsne.vx v4, v8, a1, v0.t
	vmsne.vi v4, v8, 15, v0.t
	vmsne.vi v4, v8, -16, v0.t
	vmsltu.vv v4, v8, v12
	vmsltu.vx v4, v8, a1
	vmsltu.vv v4, v8, v12, v0.t
	vmsltu.vx v4, v8, a1, v0.t
	vmslt.vv v4, v8, v12
	vmslt.vx v4, v8, a1
	vmslt.vv v4, v8, v12, v0.t
	vmslt.vx v4, v8, a1, v0.t
	vmsleu.vv v4, v8, v12
	vmsleu.vx v4, v8, a1
	vmsleu.vi v4, v8, 15
	vmsleu.vi v4, v8, -16
	vmsleu.vv v4, v8, v12, v0.t
	vmsleu.vx v4, v8, a1, v0.t
	vmsleu.vi v4, v8, 15, v0.t
	vmsleu.vi v4, v8, -16, v0.t
	vmsle.vv v4, v8, v12
	vmsle.vx v4, v8, a1
	vmsle.vi v4, v8, 15
	vmsle.vi v4, v8, -16
	vmsle.vv v4, v8, v12, v0.t
	vmsle.vx v4, v8, a1, v0.t
	vmsle.vi v4, v8, 15, v0.t
	vmsle.vi v4, v8, -16, v0.t
	vmsgtu.vx v4, v8, a1
	vmsgtu.vi v4, v8, 15
	vmsgtu.vi v4, v8, -16
	vmsgtu.vx v4, v8, a1, v0.t
	vmsgtu.vi v4, v8, 15, v0.t
	vmsgtu.vi v4, v8, -16, v0.t
	vmsgt.vx v4, v8, a1
	vmsgt.vi v4, v8, 15
	vmsgt.vi v4, v8, -16
	vmsgt.vx v4, v8, a1, v0.t
	vmsgt.vi v4, v8, 15, v0.t
	vmsgt.vi v4, v8, -16, v0.t

	vminu.vv v4, v8, v12
	vminu.vx v4, v8, a1
	vminu.vv v4, v8, v12, v0.t
	vminu.vx v4, v8, a1, v0.t
	vmin.vv v4, v8, v12
	vmin.vx v4, v8, a1
	vmin.vv v4, v8, v12, v0.t
	vmin.vx v4, v8, a1, v0.t
	vmaxu.vv v4, v8, v12
	vmaxu.vx v4, v8, a1
	vmaxu.vv v4, v8, v12, v0.t
	vmaxu.vx v4, v8, a1, v0.t
	vmax.vv v4, v8, v12
	vmax.vx v4, v8, a1
	vmax.vv v4, v8, v12, v0.t
	vmax.vx v4, v8, a1, v0.t

	vmul.vv v4, v8, v12
	vmul.vx v4, v8, a1
	vmul.vv v4, v8, v12, v0.t
	vmul.vx v4, v8, a1, v0.t
	vmulh.vv v4, v8, v12
	vmulh.vx v4, v8, a1
	vmulh.vv v4, v8, v12, v0.t
	vmulh.vx v4, v8, a1, v0.t
	vmulhu.vv v4, v8, v12
	vmulhu.vx v4, v8, a1
	vmulhu.vv v4, v8, v12, v0.t
	vmulhu.vx v4, v8, a1, v0.t
	vmulhsu.vv v4, v8, v12
	vmulhsu.vx v4, v8, a1
	vmulhsu.vv v4, v8, v12, v0.t
	vmulhsu.vx v4, v8, a1, v0.t

	vwmul.vv v4, v8, v12
	vwmul.vx v4, v8, a1
	vwmul.vv v4, v8, v12, v0.t
	vwmul.vx v4, v8, a1, v0.t
	vwmulu.vv v4, v8, v12
	vwmulu.vx v4, v8, a1
	vwmulu.vv v4, v8, v12, v0.t
	vwmulu.vx v4, v8, a1, v0.t
	vwmulsu.vv v4, v8, v12
	vwmulsu.vx v4, v8, a1
	vwmulsu.vv v4, v8, v12, v0.t
	vwmulsu.vx v4, v8, a1, v0.t

	vmacc.vv v4, v12, v8
	vmacc.vx v4, a1, v8
	vmacc.vv v4, v12, v8, v0.t
	vmacc.vx v4, a1, v8, v0.t
	vnmsac.vv v4, v12, v8
	vnmsac.vx v4, a1, v8
	vnmsac.vv v4, v12, v8, v0.t
	vnmsac.vx v4, a1, v8, v0.t
	vmadd.vv v4, v12, v8
	vmadd.vx v4, a1, v8
	vmadd.vv v4, v12, v8, v0.t
	vmadd.vx v4, a1, v8, v0.t
	vnmsub.vv v4, v12, v8
	vnmsub.vx v4, a1, v8
	vnmsub.vv v4, v12, v8, v0.t
	vnmsub.vx v4, a1, v8, v0.t

	vwmaccu.vv v4, v12, v8
	vwmaccu.vx v4, a1, v8
	vwmaccu.vv v4, v12, v8, v0.t
	vwmaccu.vx v4, a1, v8, v0.t
	vwmacc.vv v4, v12, v8
	vwmacc.vx v4, a1, v8
	vwmacc.vv v4, v12, v8, v0.t
	vwmacc.vx v4, a1, v8, v0.t
	vwmaccsu.vv v4, v12, v8
	vwmaccsu.vx v4, a1, v8
	vwmaccsu.vv v4, v12, v8, v0.t
	vwmaccsu.vx v4, a1, v8, v0.t
	vwmaccus.vx v4, a1, v8
	vwmaccus.vx v4, a1, v8, v0.t

	vdivu.vv v4, v8, v12
	vdivu.vx v4, v8, a1
	vdivu.vv v4, v8, v12, v0.t
	vdivu.vx v4, v8, a1, v0.t
	vdiv.vv v4, v8, v12
	vdiv.vx v4, v8, a1
	vdiv.vv v4, v8, v12, v0.t
	vdiv.vx v4, v8, a1, v0.t
	vremu.vv v4, v8, v12
	vremu.vx v4, v8, a1
	vremu.vv v4, v8, v12, v0.t
	vremu.vx v4, v8, a1, v0.t
	vrem.vv v4, v8, v12
	vrem.vx v4, v8, a1
	vrem.vv v4, v8, v12, v0.t
	vrem.vx v4, v8, a1, v0.t

	vmerge.vvm v4, v8, v12, v0
	vmerge.vxm v4, v8, a1, v0
	vmerge.vim v4, v8, 15, v0
	vmerge.vim v4, v8, -16, v0

	vmv.v.v v8, v12
	vmv.v.x v8, a1
	vmv.v.i v8, 15
	vmv.v.i v8, -16

	vsaddu.vv v4, v8, v12
	vsaddu.vx v4, v8, a1
	vsaddu.vi v4, v8, 15
	vsaddu.vi v4, v8, -16
	vsaddu.vv v4, v8, v12, v0.t
	vsaddu.vx v4, v8, a1, v0.t
	vsaddu.vi v4, v8, 15, v0.t
	vsaddu.vi v4, v8, -16, v0.t
	vsadd.vv v4, v8, v12
	vsadd.vx v4, v8, a1
	vsadd.vi v4, v8, 15
	vsadd.vi v4, v8, -16
	vsadd.vv v4, v8, v12, v0.t
	vsadd.vx v4, v8, a1, v0.t
	vsadd.vi v4, v8, 15, v0.t
	vsadd.vi v4, v8, -16, v0.t
	vssubu.vv v4, v8, v12
	vssubu.vx v4, v8, a1
	vssubu.vv v4, v8, v12, v0.t
	vssubu.vx v4, v8, a1, v0.t
	vssub.vv v4, v8, v12
	vssub.vx v4, v8, a1
	vssub.vv v4, v8, v12, v0.t
	vssub.vx v4, v8, a1, v0.t

	vaaddu.vv v4, v8, v12
	vaaddu.vx v4, v8, a1
	vaaddu.vv v4, v8, v12, v0.t
	vaaddu.vx v4, v8, a1, v0.t
	vaadd.vv v4, v8, v12
	vaadd.vx v4, v8, a1
	vaadd.vv v4, v8, v12, v0.t
	vaadd.vx v4, v8, a1, v0.t
	vasubu.vv v4, v8, v12
	vasubu.vx v4, v8, a1
	vasubu.vv v4, v8, v12, v0.t
	vasubu.vx v4, v8, a1, v0.t
	vasub.vv v4, v8, v12
	vasub.vx v4, v8, a1
	vasub.vv v4, v8, v12, v0.t
	vasub.vx v4, v8, a1, v0.t

	vsmul.vv v4, v8, v12
	vsmul.vx v4, v8, a1
	vsmul.vv v4, v8, v12, v0.t
	vsmul.vx v4, v8, a1, v0.t

	vssrl.vv v4, v8, v12
	vssrl.vx v4, v8, a1
	vssrl.vi v4, v8, 1
	vssrl.vi v4, v8, 31
	vssrl.vv v4, v8, v12, v0.t
	vssrl.vx v4, v8, a1, v0.t
	vssrl.vi v4, v8, 1, v0.t
	vssrl.vi v4, v8, 31, v0.t
	vssra.vv v4, v8, v12
	vssra.vx v4, v8, a1
	vssra.vi v4, v8, 1
	vssra.vi v4, v8, 31
	vssra.vv v4, v8, v12, v0.t
	vssra.vx v4, v8, a1, v0.t
	vssra.vi v4, v8, 1, v0.t
	vssra.vi v4, v8, 31, v0.t

	vnclipu.wv v4, v8, v12
	vnclipu.wx v4, v8, a1
	vnclipu.wi v4, v8, 1
	vnclipu.wi v4, v8, 31
	vnclipu.wv v4, v8, v12, v0.t
	vnclipu.wx v4, v8, a1, v0.t
	vnclipu.wi v4, v8, 1, v0.t
	vnclipu.wi v4, v8, 31, v0.t
	vnclip.wv v4, v8, v12
	vnclip.wx v4, v8, a1
	vnclip.wi v4, v8, 1
	vnclip.wi v4, v8, 31
	vnclip.wv v4, v8, v12, v0.t
	vnclip.wx v4, v8, a1, v0.t
	vnclip.wi v4, v8, 1, v0.t
	vnclip.wi v4, v8, 31, v0.t

	vfadd.vv v4, v8, v12
	vfadd.vf v4, v8, fa2
	vfadd.vv v4, v8, v12, v0.t
	vfadd.vf v4, v8, fa2, v0.t
	vfsub.vv v4, v8, v12
	vfsub.vf v4, v8, fa2
	vfsub.vv v4, v8, v12, v0.t
	vfsub.vf v4, v8, fa2, v0.t
	vfrsub.vf v4, v8, fa2
	vfrsub.vf v4, v8, fa2, v0.t

	vfwadd.vv v4, v8, v12
	vfwadd.vf v4, v8, fa2
	vfwadd.vv v4, v8, v12, v0.t
	vfwadd.vf v4, v8, fa2, v0.t
	vfwsub.vv v4, v8, v12
	vfwsub.vf v4, v8, fa2
	vfwsub.vv v4, v8, v12, v0.t
	vfwsub.vf v4, v8, fa2, v0.t
	vfwadd.wv v4, v8, v12
	vfwadd.wf v4, v8, fa2
	vfwadd.wv v4, v8, v12, v0.t
	vfwadd.wf v4, v8, fa2, v0.t
	vfwsub.wv v4, v8, v12
	vfwsub.wf v4, v8, fa2
	vfwsub.wv v4, v8, v12, v0.t
	vfwsub.wf v4, v8, fa2, v0.t

	vfmul.vv v4, v8, v12
	vfmul.vf v4, v8, fa2
	vfmul.vv v4, v8, v12, v0.t
	vfmul.vf v4, v8, fa2, v0.t
	vfdiv.vv v4, v8, v12
	vfdiv.vf v4, v8, fa2
	vfdiv.vv v4, v8, v12, v0.t
	vfdiv.vf v4, v8, fa2, v0.t
	vfrdiv.vf v4, v8, fa2
	vfrdiv.vf v4, v8, fa2, v0.t

	vfwmul.vv v4, v8, v12
	vfwmul.vf v4, v8, fa2
	vfwmul.vv v4, v8, v12, v0.t
	vfwmul.vf v4, v8, fa2, v0.t

	vfmadd.vv v4, v12, v8
	vfmadd.vf v4, fa2, v8
	vfnmadd.vv v4, v12, v8
	vfnmadd.vf v4, fa2, v8
	vfmsub.vv v4, v12, v8
	vfmsub.vf v4, fa2, v8
	vfnmsub.vv v4, v12, v8
	vfnmsub.vf v4, fa2, v8
	vfmadd.vv v4, v12, v8, v0.t
	vfmadd.vf v4, fa2, v8, v0.t
	vfnmadd.vv v4, v12, v8, v0.t
	vfnmadd.vf v4, fa2, v8, v0.t
	vfmsub.vv v4, v12, v8, v0.t
	vfmsub.vf v4, fa2, v8, v0.t
	vfnmsub.vv v4, v12, v8, v0.t
	vfnmsub.vf v4, fa2, v8, v0.t
	vfmacc.vv v4, v12, v8
	vfmacc.vf v4, fa2, v8
	vfnmacc.vv v4, v12, v8
	vfnmacc.vf v4, fa2, v8
	vfmsac.vv v4, v12, v8
	vfmsac.vf v4, fa2, v8
	vfnmsac.vv v4, v12, v8
	vfnmsac.vf v4, fa2, v8
	vfmacc.vv v4, v12, v8, v0.t
	vfmacc.vf v4, fa2, v8, v0.t
	vfnmacc.vv v4, v12, v8, v0.t
	vfnmacc.vf v4, fa2, v8, v0.t
	vfmsac.vv v4, v12, v8, v0.t
	vfmsac.vf v4, fa2, v8, v0.t
	vfnmsac.vv v4, v12, v8, v0.t
	vfnmsac.vf v4, fa2, v8, v0.t

	vfwmacc.vv v4, v12, v8
	vfwmacc.vf v4, fa2, v8
	vfwnmacc.vv v4, v12, v8
	vfwnmacc.vf v4, fa2, v8
	vfwmsac.vv v4, v12, v8
	vfwmsac.vf v4, fa2, v8
	vfwnmsac.vv v4, v12, v8
	vfwnmsac.vf v4, fa2, v8
	vfwmacc.vv v4, v12, v8, v0.t
	vfwmacc.vf v4, fa2, v8, v0.t
	vfwnmacc.vv v4, v12, v8, v0.t
	vfwnmacc.vf v4, fa2, v8, v0.t
	vfwmsac.vv v4, v12, v8, v0.t
	vfwmsac.vf v4, fa2, v8, v0.t
	vfwnmsac.vv v4, v12, v8, v0.t
	vfwnmsac.vf v4, fa2, v8, v0.t

	vfsqrt.v v4, v8
	vfsqrt.v v4, v8, v0.t
	vfrsqrte7.v v4, v8
	vfrsqrte7.v v4, v8, v0.t
	vfrsqrt7.v v4, v8
	vfrsqrt7.v v4, v8, v0.t
	vfrece7.v v4, v8
	vfrece7.v v4, v8, v0.t
	vfrec7.v v4, v8
	vfrec7.v v4, v8, v0.t
	vfclass.v v4, v8
	vfclass.v v4, v8, v0.t

	vfmin.vv v4, v8, v12
	vfmin.vf v4, v8, fa2
	vfmax.vv v4, v8, v12
	vfmax.vf v4, v8, fa2
	vfmin.vv v4, v8, v12, v0.t
	vfmin.vf v4, v8, fa2, v0.t
	vfmax.vv v4, v8, v12, v0.t
	vfmax.vf v4, v8, fa2, v0.t

	vfneg.v v4, v8
	vfneg.v v4, v8, v0.t
	vfabs.v v4, v8
	vfabs.v v4, v8, v0.t

	vfsgnj.vv v4, v8, v12
	vfsgnj.vf v4, v8, fa2
	vfsgnjn.vv v4, v8, v12
	vfsgnjn.vf v4, v8, fa2
	vfsgnjx.vv v4, v8, v12
	vfsgnjx.vf v4, v8, fa2
	vfsgnj.vv v4, v8, v12, v0.t
	vfsgnj.vf v4, v8, fa2, v0.t
	vfsgnjn.vv v4, v8, v12, v0.t
	vfsgnjn.vf v4, v8, fa2, v0.t
	vfsgnjx.vv v4, v8, v12, v0.t
	vfsgnjx.vf v4, v8, fa2, v0.t

	# Aliases
	vmfgt.vv v4, v8, v12
	vmfge.vv v4, v8, v12
	vmfgt.vv v4, v8, v12, v0.t
	vmfge.vv v4, v8, v12, v0.t

	vmfeq.vv v4, v8, v12
	vmfeq.vf v4, v8, fa2
	vmfne.vv v4, v8, v12
	vmfne.vf v4, v8, fa2
	vmflt.vv v4, v8, v12
	vmflt.vf v4, v8, fa2
	vmfle.vv v4, v8, v12
	vmfle.vf v4, v8, fa2
	vmfgt.vf v4, v8, fa2
	vmfge.vf v4, v8, fa2
	vmfeq.vv v4, v8, v12, v0.t
	vmfeq.vf v4, v8, fa2, v0.t
	vmfne.vv v4, v8, v12, v0.t
	vmfne.vf v4, v8, fa2, v0.t
	vmflt.vv v4, v8, v12, v0.t
	vmflt.vf v4, v8, fa2, v0.t
	vmfle.vv v4, v8, v12, v0.t
	vmfle.vf v4, v8, fa2, v0.t
	vmfgt.vf v4, v8, fa2, v0.t
	vmfge.vf v4, v8, fa2, v0.t

	vfmerge.vfm v4, v8, fa2, v0
	vfmv.v.f v4, fa1

	vfcvt.xu.f.v v4, v8
	vfcvt.x.f.v v4, v8
	vfcvt.rtz.xu.f.v v4, v8
	vfcvt.rtz.x.f.v v4, v8
	vfcvt.f.xu.v v4, v8
	vfcvt.f.x.v v4, v8
	vfcvt.xu.f.v v4, v8, v0.t
	vfcvt.x.f.v v4, v8, v0.t
	vfcvt.rtz.xu.f.v v4, v8, v0.t
	vfcvt.rtz.x.f.v v4, v8, v0.t
	vfcvt.f.xu.v v4, v8, v0.t
	vfcvt.f.x.v v4, v8, v0.t

	vfwcvt.xu.f.v v4, v8
	vfwcvt.x.f.v v4, v8
	vfwcvt.rtz.xu.f.v v4, v8
	vfwcvt.rtz.x.f.v v4, v8
	vfwcvt.f.xu.v v4, v8
	vfwcvt.f.x.v v4, v8
	vfwcvt.f.f.v v4, v8
	vfwcvt.xu.f.v v4, v8, v0.t
	vfwcvt.x.f.v v4, v8, v0.t
	vfwcvt.rtz.xu.f.v v4, v8, v0.t
	vfwcvt.rtz.x.f.v v4, v8, v0.t
	vfwcvt.f.xu.v v4, v8, v0.t
	vfwcvt.f.x.v v4, v8, v0.t
	vfwcvt.f.f.v v4, v8, v0.t

	vfncvt.xu.f.w v4, v8
	vfncvt.x.f.w v4, v8
	vfncvt.rtz.xu.f.w v4, v8
	vfncvt.rtz.x.f.w v4, v8
	vfncvt.f.xu.w v4, v8
	vfncvt.f.x.w v4, v8
	vfncvt.f.f.w v4, v8
	vfncvt.rod.f.f.w v4, v8
	vfncvt.xu.f.w v4, v8, v0.t
	vfncvt.x.f.w v4, v8, v0.t
	vfncvt.rtz.xu.f.w v4, v8, v0.t
	vfncvt.rtz.x.f.w v4, v8, v0.t
	vfncvt.f.xu.w v4, v8, v0.t
	vfncvt.f.x.w v4, v8, v0.t
	vfncvt.f.f.w v4, v8, v0.t
	vfncvt.rod.f.f.w v4, v8, v0.t

	vredsum.vs v4, v8, v12
	vredmaxu.vs v4, v8, v8
	vredmax.vs v4, v8, v8
	vredminu.vs v4, v8, v8
	vredmin.vs v4, v8, v8
	vredand.vs v4, v8, v12
	vredor.vs v4, v8, v12
	vredxor.vs v4, v8, v12
	vredsum.vs v4, v8, v12, v0.t
	vredmaxu.vs v4, v8, v8, v0.t
	vredmax.vs v4, v8, v8, v0.t
	vredminu.vs v4, v8, v8, v0.t
	vredmin.vs v4, v8, v8, v0.t
	vredand.vs v4, v8, v12, v0.t
	vredor.vs v4, v8, v12, v0.t
	vredxor.vs v4, v8, v12, v0.t

	vwredsumu.vs v4, v8, v12
	vwredsum.vs v4, v8, v12
	vwredsumu.vs v4, v8, v12, v0.t
	vwredsum.vs v4, v8, v12, v0.t

	vfredosum.vs v4, v8, v12
	vfredusum.vs v4, v8, v12
	vfredsum.vs v4, v8, v12		# Alias of vfredusum.vs.
	vfredmax.vs v4, v8, v12
	vfredmin.vs v4, v8, v12
	vfredosum.vs v4, v8, v12, v0.t
	vfredusum.vs v4, v8, v12, v0.t
	vfredsum.vs v4, v8, v12, v0.t	# Alias of vfredusum.vs.
	vfredmax.vs v4, v8, v12, v0.t
	vfredmin.vs v4, v8, v12, v0.t

	vfwredosum.vs v4, v8, v12
	vfwredusum.vs v4, v8, v12
	vfwredsum.vs v4, v8, v12	# Alias of vfwredusum.vs.
	vfwredosum.vs v4, v8, v12, v0.t
	vfwredusum.vs v4, v8, v12, v0.t
	vfwredsum.vs v4, v8, v12, v0.t	# Alias of vfwredusum.vs.

	# Aliases
	vmcpy.m v4, v8
	vmmv.m v4, v8
	vmclr.m v4
	vmset.m v4
	vmnot.m v4, v8

	vmand.mm v4, v8, v12
	vmnand.mm v4, v8, v12
	vmandn.mm v4, v8, v12
	vmandnot.mm v4, v8, v12		# Alias of vmandn.mm.
	vmxor.mm v4, v8, v12
	vmor.mm v4, v8, v12
	vmnor.mm v4, v8, v12
	vmorn.mm v4, v8, v12
	vmornot.mm v4, v8, v12		# Alias of vmorn.mm.
	vmxnor.mm v4, v8, v12

	vcpop.m a0, v12
	vfirst.m a0, v12
	vmsbf.m v4, v8
	vmsif.m v4, v8
	vmsof.m v4, v8
	viota.m v4, v8
	vid.v v4
	vcpop.m a0, v12, v0.t
	vfirst.m a0, v12, v0.t
	vmsbf.m v4, v8, v0.t
	vmsif.m v4, v8, v0.t
	vmsof.m v4, v8, v0.t
	viota.m v4, v8, v0.t
	vid.v v4, v0.t

	vmv.x.s a0, v12
	vmv.s.x v4, a0

	vfmv.f.s fa0, v8
	vfmv.s.f v4, fa1

	vslideup.vx v4, v8, a1
	vslideup.vi v4, v8, 0
	vslideup.vi v4, v8, 31
	vslidedown.vx v4, v8, a1
	vslidedown.vi v4, v8, 0
	vslidedown.vi v4, v8, 31
	vslideup.vx v4, v8, a1, v0.t
	vslideup.vi v4, v8, 0, v0.t
	vslideup.vi v4, v8, 31, v0.t
	vslidedown.vx v4, v8, a1, v0.t
	vslidedown.vi v4, v8, 0, v0.t
	vslidedown.vi v4, v8, 31, v0.t

	vslide1up.vx v4, v8, a1
	vslide1down.vx v4, v8, a1
	vslide1up.vx v4, v8, a1, v0.t
	vslide1down.vx v4, v8, a1, v0.t

	vfslide1up.vf v4, v8, fa1
	vfslide1down.vf v4, v8, fa1
	vfslide1up.vf v4, v8, fa1, v0.t
	vfslide1down.vf v4, v8, fa1, v0.t

	vrgather.vv v4, v8, v12
	vrgather.vx v4, v8, a1
	vrgather.vi v4, v8, 0
	vrgather.vi v4, v8, 31
	vrgather.vv v4, v8, v12, v0.t
	vrgather.vx v4, v8, a1, v0.t
	vrgather.vi v4, v8, 0, v0.t
	vrgather.vi v4, v8, 31, v0.t

	vrgatherei16.vv v4, v8, v12
	vrgatherei16.vv v4, v8, v12, v0.t

	vcompress.vm v4, v8, v12

	vmv1r.v v1, v2
	vmv2r.v v2, v4
	vmv4r.v v4, v8
	vmv8r.v v0, v8
