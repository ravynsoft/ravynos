extern int foo();
extern int bar();
extern int bar_weak();

int main()
{
	foo();
	bar();
	bar_weak();
	return 0;
}
