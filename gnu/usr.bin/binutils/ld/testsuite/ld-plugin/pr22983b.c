int func_attr_used_disabled(int val){
	return val + 1;
}
__attribute__((used))
int func_attr_used_enabled(int val){
	return val + 1;
}
__attribute__((used))
int var_attr_used_enabled = 0b0101010101;
int var_attr_used_disabled = 0b0101010101;
