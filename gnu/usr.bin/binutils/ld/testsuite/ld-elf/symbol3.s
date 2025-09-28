 .ifdef HPUX
badsym	.comm 4
 .else
	.comm badsym,4
 .endif
