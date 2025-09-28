	.syntax unified

	.word myfunc(GOTFUNCDESC)
	.word myfunc(GOTOFFFUNCDESC)
	.word myfunc(FUNCDESC)

	.type myfunc, %function
myfunc:
	bx	lr
