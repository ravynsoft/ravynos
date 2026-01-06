
int a = 0;
int func_a() { return a; }

// add code that will cause stack canary 
extern void fill(char*);
void test()
{
	char buf[100];
	fill(buf);
}

