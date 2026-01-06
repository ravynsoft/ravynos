extern int foo2();
extern int foo3();
int main(){
	int i = foo3() + foo2();
	if (i == 42)
	  return 0;
	else
	  return 1;
}
