int f(void) __attribute__((weak));
int main(void){return f?f():0;}
