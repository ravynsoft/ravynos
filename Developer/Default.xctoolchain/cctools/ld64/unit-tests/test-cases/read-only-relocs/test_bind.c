
extern int b;
extern void func();

int test_bind() 
{ 
	func();
	return b; 
}

