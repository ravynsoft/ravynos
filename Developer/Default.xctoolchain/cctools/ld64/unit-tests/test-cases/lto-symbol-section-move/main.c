extern void* otherget();

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


int def = 20;

int ghi = 30;

int com;

