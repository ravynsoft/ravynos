# Source file used to test the nor instruction
	
foo:
	rdctl	r8,ctl31
	rdctl	r8,ctl30
	rdctl	r8,ctl29
	rdctl	r8,status
	rdctl	r8,bstatus
	rdctl	r8,estatus
	wrctl	ctl31,r8
	wrctl	ctl30,r8
	wrctl	ctl29,r8
	wrctl	status,r8
	wrctl	bstatus,r8
	wrctl	estatus,r8



