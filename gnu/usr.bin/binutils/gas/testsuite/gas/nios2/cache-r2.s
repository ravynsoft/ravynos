# Source file used to test the cache instruction
foo:
	flushd -0x800(r6)
	flushd 0x7ff(r6)
	flushd 0x0(r6)
	flushd -0x001(r6)
	
# use symbol for offset
	flushd foo(r6)
	
# use external symbol
	.global external
	flushd external(r6)
	
# flushi
	flushi r2

#flushp
	flushp
	

