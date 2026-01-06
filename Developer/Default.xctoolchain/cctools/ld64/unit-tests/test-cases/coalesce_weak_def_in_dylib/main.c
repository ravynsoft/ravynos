
extern void foo();
extern void wfoo();

void* pfoo = &foo;
void* pwfoo = &wfoo;

int main (void)
{
	if (pfoo != &foo)
		return 1;
	if (pwfoo != &wfoo)
		return 1;

   return 0;
}

