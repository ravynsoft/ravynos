
extern void other();

void bar() { 
	other();
	other();
}

void __attribute__((weak))  my_weak() { 
	other();
	other();
}

void bar2() { 
	other();
	other();
}

