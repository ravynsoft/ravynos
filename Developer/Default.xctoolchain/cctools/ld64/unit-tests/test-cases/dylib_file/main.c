
extern void foo();
extern void bar();
extern void bar_extra();

int main()
{
	foo();
	bar();
#if BAR_EXTRA
	bar_extra();
#endif
	return 0;
}

