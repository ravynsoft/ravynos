target:
	th.ldd		a0, a1, (a2), 0
	th.lwd		a0, a1, (a2), 1
	th.lwud		a0, a1, (a2), 2
	th.sdd		a0, a1, (a2), 3
	th.swd		a0, a1, (a2), 0

	th.ldd		a0, a1, (a2), -1, 4
	th.ldd		a0, a1, (a2), 4, 4
	th.ldd		a0, a1, (a2), 0, 3

	th.lwd		a0, a1, (a2), -1, 3
	th.lwd		a0, a1, (a2), 4, 3
	th.lwd		a0, a1, (a2), 0, 4

	th.lwud		a0, a1, (a2), -1, 3
	th.lwud		a0, a1, (a2), 4, 3
	th.lwud		a0, a1, (a2), 0, 4

	th.sdd		a0, a1, (a2), -1, 4
	th.sdd		a0, a1, (a2), 4, 4
	th.sdd		a0, a1, (a2), 0, 3

	th.swd		a0, a1, (a2), -1, 3
	th.swd		a0, a1, (a2), 4, 3
	th.swd		a0, a1, (a2), 0, 4

	th.ldd		a0, a0, (a1), 0
	th.ldd		a0, a1, (a0), 0
	th.ldd		a1, a0, (a0), 0
