
extern void bar();


// strong definition of foo overrides weak definition
// in main.c, but this foo needs bar()
void foo() 
{
	bar();
}

void loadme()
{
}

