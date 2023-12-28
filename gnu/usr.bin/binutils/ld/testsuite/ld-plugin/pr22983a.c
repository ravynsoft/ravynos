#include <stdio.h>

int func_attr_used_disabled(int val);
int func_attr_used_enabled(int val);
extern int var_attr_used_enabled;
extern int var_attr_used_disabled;

int main(int argc, const char **argv){
	printf("%d\n", var_attr_used_disabled);
	printf("%d\n", var_attr_used_enabled);
	printf("%d\n", func_attr_used_disabled(1));
	printf("%d\n", func_attr_used_enabled(1));
	return 0;
}
