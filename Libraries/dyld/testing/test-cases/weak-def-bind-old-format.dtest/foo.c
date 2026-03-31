
__attribute__((weak))
int weakTestValue = 1;

int foo() {
	return weakTestValue;
}