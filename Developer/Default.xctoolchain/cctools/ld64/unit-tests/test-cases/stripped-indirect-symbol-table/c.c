extern void b();
extern void bb();

extern void func(void*);


void c()
{
	func(&b);
	func(&bb);
}