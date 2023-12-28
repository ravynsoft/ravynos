# Source file used to test the cache instruction
foo:
	flushd -0x8000(r6)
	flushd 0x7fff(r6)
	flushd 0x0(r6)
	flushd -0x0001(r6)
	
# use symbol for offset
	flushd foo(r6)
	
# use external symbol
	.global external
	flushd external(r6)
	
# flushi
	flushi r2

#flushp
	flushp
	

