
void foo() {}

void mid() {}

static void bar() { foo(); }

int main() { bar(); return 0; }


#if __STATIC__
void myexit() {}
#endif

