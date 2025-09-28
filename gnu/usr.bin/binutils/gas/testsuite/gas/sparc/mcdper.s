# Test reads/writes to the %mcdper asr register
	.text
	rd	%mcdper, %g1
	wr	%g1, 0x3, %mcdper
	wr	%g1, %g2, %mcdper
