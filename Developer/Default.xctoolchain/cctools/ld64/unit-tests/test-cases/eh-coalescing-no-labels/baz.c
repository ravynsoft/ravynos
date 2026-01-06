
extern void other();

void baz() { 
	other();
	other();
}

void __attribute__((weak))  my_weak() { 
	other();
	other();
}

void baz2() { 
	other();
	other();
}

