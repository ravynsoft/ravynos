static int var_static = 3;

__attribute__((visibility("hidden")))
int var_hidden = 4;

int var_global = 5;


__attribute__((visibility("hidden"), weak))
int var_weak_hidden = 4;

__attribute__((weak))
int var_weak_global = 5;




static int* foo_static() { return &var_static; }

__attribute__((visibility("hidden")))
int* foo_hidden() { return &var_hidden; }


int* foo_global() { return &var_global; }


__attribute__((visibility("hidden"),weak))
int* foo_weak_hidden() { return &var_weak_hidden; }


__attribute__((weak))
int* foo_weak_global() { return &var_weak_global; }


__attribute__((visibility("hidden")))
void* keep[] = { &foo_static };


