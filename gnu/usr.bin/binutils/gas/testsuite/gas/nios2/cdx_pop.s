# Source file used to test the pop.n instruction
	
foo:
	pop.n	{ra}
	pop.n	{fp,ra}
	pop.n	{r16,ra}
	pop.n	{r16,fp,ra}
	pop.n	{r16,r17,r18,r19,r20,r21,r22,r23,fp,ra}
	pop.n	{ra},0x0
	pop.n	{ra},0x3c
	pop.n	{r16,r17,r18,r19,r20,r21,r22,r23,fp,ra},0x3c
