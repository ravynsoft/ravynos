# Source file used to test the BMX instruction
	
foo:
	ldsex	r0,(r0)
	ldsex	r0,(r31)
	ldsex	r31,(r0)
	stsex	r0,r0,(r0)
	stsex	r31,r31,(r31)
	stsex	r4,r31,(r16)
