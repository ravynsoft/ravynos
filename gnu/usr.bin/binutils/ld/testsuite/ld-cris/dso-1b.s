	.include "dso-1.s"
	.symver dsofn,expfn@@TST2
	.include "init.s"
	.symver _init,expobj@@TST2
