# Source file used to test the BMX instruction
	
foo:
	extract	r31,r0,31,0
	extract	r0,r31,24,7
	extract	r4,r10,31,0
	extract	r23,r11,20,15
	insert	r31,r0,31,0
	insert	r0,r31,24,7
	insert	r4,r10,31,0
	insert	r23,r11,20,15
	merge	r31,r0,31,0
	merge	r0,r31,24,7
	merge	r4,r10,31,0
	merge	r23,r11,20,15
