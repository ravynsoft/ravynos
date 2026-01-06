extern int common_var;
int *fn();

int
main(int argc, char **argv)
{
	return 0!=&common_var;
}
