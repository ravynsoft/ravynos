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

#endif

void
do(dbh)
   SV *dbh
CODE:
{
   int x;
   ++x;
}
