# PowerPC Broadway instruction tests
	.text
start:
	mfiabr 0
	mtiabr 1
	mfdabr 2
	mtdabr 3
	mfgqr 4, 0
	mfgqr 5, 1
	mfgqr 6, 2
	mfgqr 7, 3
	mfgqr 8, 4
	mfgqr 9, 5
	mfgqr 10, 6
	mfgqr 11, 7
	mtgqr 0, 4
	mtgqr 1, 5
	mtgqr 2, 6
	mtgqr 3, 7
	mtgqr 4, 8
	mtgqr 5, 9
	mtgqr 6, 10
	mtgqr 7, 11
	mfwpar 12
	mtwpar 13
	mfdmal 14
	mtdmal 15
	mfdmau 16
	mtdmau 17
	mfhid0 18
	mthid0 19
	mfhid1 20
	mthid1 21
	mfhid2 22
	mthid2 23
	mfhid4 24
	mthid4 25

	mfibatu 0, 0
	mtibatu 0, 1
	mfibatu 2, 1
	mtibatu 1, 3
	mfibatu 4, 2
	mtibatu 2, 5
	mfibatu 6, 3
	mtibatu 3, 7
	mfibatu 8, 4
	mtibatu 4, 9
	mfibatu 10, 5
	mtibatu 5, 11
	mfibatu 12, 6
	mtibatu 6, 13
	mfibatu 14, 7
	mtibatu 7, 15
	mfibatl 16, 0
	mtibatl 0, 17
	mfibatl 18, 1
	mtibatl 1, 19
	mfibatl 20, 2
	mtibatl 2, 21
	mfibatl 22, 3
	mtibatl 3, 23
	mfibatl 24, 4
	mtibatl 4, 25
	mfibatl 26, 5
	mtibatl 5, 27
	mfibatl 28, 6
	mtibatl 6, 29
	mfibatl 30, 7
	mtibatl 7, 31

	mfdbatu 0, 0
	mtdbatu 0, 1
	mfdbatu 2, 1
	mtdbatu 1, 3
	mfdbatu 4, 2
	mtdbatu 2, 5
	mfdbatu 6, 3
	mtdbatu 3, 7
	mfdbatu 8, 4
	mtdbatu 4, 9
	mfdbatu 10, 5
	mtdbatu 5, 11
	mfdbatu 12, 6
	mtdbatu 6, 13
	mfdbatu 14, 7
	mtdbatu 7, 15
	mfdbatl 16, 0
	mtdbatl 0, 17
	mfdbatl 18, 1
	mtdbatl 1, 19
	mfdbatl 20, 2
	mtdbatl 2, 21
	mfdbatl 22, 3
	mtdbatl 3, 23
	mfdbatl 24, 4
	mtdbatl 4, 25
	mfdbatl 26, 5
	mtdbatl 5, 27
	mfdbatl 28, 6
	mtdbatl 6, 29
	mfdbatl 30, 7
	mtdbatl 7, 31
