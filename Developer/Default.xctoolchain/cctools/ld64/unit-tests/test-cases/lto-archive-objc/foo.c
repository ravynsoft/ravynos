
extern void neverFindMe();

__attribute__((constructor))
void myinit()
{
    neverFindMe();
}
