
extern const char** myp;
extern const char* mystring;


__attribute__((section("__DATA,__data_extra")))
int param = 0;


const char** entry(int i) { 
	if ( i ) {
		*myp = "help";
	}
  param = i;
	return myp;
}

int get() { return param; }