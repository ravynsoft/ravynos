
extern const char** myp;


const char** entry(int i) { 
	if ( i ) {
		*myp = "help";
	}
	return myp;
}

