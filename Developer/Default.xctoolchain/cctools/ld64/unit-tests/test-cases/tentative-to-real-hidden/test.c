
// tentative definitions
int tent1;
int tent2;
int  __attribute__((visibility("hidden"))) tent3;

// initialized to point to tentative definitions
int* pa = &tent1;
int* pb = &tent2;
int* pc = &tent3;

