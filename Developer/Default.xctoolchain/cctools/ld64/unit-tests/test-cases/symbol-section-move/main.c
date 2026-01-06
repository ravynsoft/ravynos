
void mm() 
{
}

static void s1() {
  mm();
}

static void s2() {
  mm();
}

int main()
{
  s1();
  s2();
	return 0;
}

const char* version() { return "1.0"; }

static int mylocal()
{
  return 0;
}

void* mainget() { return mylocal; }

double getpi() { return 3.1415926535; }

void foo() 
{
}

void bar() 
{
}

extern void* __dso_handle;
void* x = &__dso_handle;

int abc = 10;

int def = 20;

int ghi = 30;

int com;

int com3;
int com4;
int com5;

