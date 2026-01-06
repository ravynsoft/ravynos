extern int foo1();
extern int foo2();
extern int bar;
int main(){
        int i;
	bar = 14;
	i = foo1() + foo2() + bar;
	if (i == 42)
	  return 0;
	else
	  return 1;

}
