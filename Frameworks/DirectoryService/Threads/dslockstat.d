provider dslockstat
{
	probe mutex__acquire(long, const char *, const char *, int);
	probe mutex__release(long, const char *, const char *, int);
};
