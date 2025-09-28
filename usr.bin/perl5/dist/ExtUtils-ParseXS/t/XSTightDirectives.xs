MODULE = My PACKAGE = My

#ifdef MYDEF123
void
do(dbh)
   SV *dbh
CODE:
{
   int x;
   ++x;
}
#else
void
do(dbh)
   SV *dbh
CODE:
{
   int x;
   ++x;
}
#endif
