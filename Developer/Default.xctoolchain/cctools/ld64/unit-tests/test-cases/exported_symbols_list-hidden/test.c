


static int data_static1 = 1;
static int data_static2 = 2;

void func_global1() { ++data_static1; }
void func_global2() { ++data_static2; }

void __attribute__((visibility("hidden"))) func_hidden1() {}
void __attribute__((visibility("hidden"))) func_hidden2() {}

int common_global1;
int common_global2;

int __attribute__((visibility("hidden"))) common_hidden1;
int __attribute__((visibility("hidden"))) common_hidden2;

