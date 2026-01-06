
extern int a(const char*, const char*);
extern char* b;
extern int a_value;

int entry(const char* param) { 
	if ( a(param, b)  ) {
		return a_value + 10;
	}
	return a_value;
}

