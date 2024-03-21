	.text
_start:
	slbiag	10
	slbiag	10,0
	slbiag	10,1
	paste.  10,11
	paste.  10,11,1
	paste.  10,11,0
	dcbfps	0,3
	dcbf	0,3,4
	dcbstps	0,3
	dcbf	0,3,6
	hwsync
	sync
	sync 0
	sync 0,0
	lwsync
	sync 1
	sync 1,0
	ptesync
	sync 2
	sync 2,0
	phwsync
	sync 4
	sync 4,0
	plwsync
	sync 5
	sync 5,0
	stncisync
	sync 1,1
	stcisync
	sync 0,2
	stsync
	sync 0,3
	wait
	wait 0
	wait 0,0
	waitrsv
	wait 1
	wait 1,0
	pause_short
	wait 2
	wait 2,0
