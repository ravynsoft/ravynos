
extern void foo1();
extern void foo2();
extern void bar1();
extern void bar2();

extern int foo_data1;
extern int foo_data2;
extern int bar_data1;
extern int bar_data2;



// make external relocation to foo_data1 and bar_data1
int* pfoo = &foo_data1;
int* pbar = &bar_data1;

void* pfoo1;
void* pbar1;

int main (void)
{
	// make non-lazy reference to foo1 and bar1
	pfoo1 = &foo1;
	pbar1 = &bar1;
	
	// make lazy reference to foo2 and bar2
	foo2();
	bar2();
   
   // make non-lazy reference to foo_data2 and bar_data2
   return *pfoo + *pbar + foo_data2 + bar_data2;
}

