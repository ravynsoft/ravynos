extern int doo();

inline int *goo() {
	static int xyz;
	return &xyz;
}

int *boo();
