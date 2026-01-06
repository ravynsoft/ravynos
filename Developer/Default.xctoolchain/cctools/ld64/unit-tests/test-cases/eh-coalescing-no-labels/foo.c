
extern void other();

void foo() { 
	other();
	other();
}

void __attribute__((weak))  my_weak() { 
	other();
	other();
}

void foo2() { 
	other();
	other();
}

