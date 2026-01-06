
extern void bar();

int main()
{
#if	CALL_BAR
	bar();
#endif
	return 0;
}
