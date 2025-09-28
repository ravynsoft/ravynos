# Source file used to test the push.n instruction
	
foo:
	push.n	{ra}
	push.n	{ra,fp}
	push.n	{ra,r16}
	push.n	{ra,fp,r16}
	push.n	{ra,fp,r23,r22,r21,r20,r19,r18,r17,r16}
	push.n	{ra},0x0
	push.n	{ra},0x3c
	push.n	{ra,fp,r23,r22,r21,r20,r19,r18,r17,r16},0x3c
