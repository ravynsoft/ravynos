extern const char * getstr3(int);
extern int printf (const char *, ...);

extern const char *addr_of_str;
extern const char *addr_of_str2;

/* "foobar" needs to be a string literal, so that it's put into
   a mergable string section, then merged with the "foobar" from merge4b.s
   and then (when the linker is buggy) doesn't cover the additional
   nul byte after "foobar" in the asm source (which addr_of_str2 is supposed
   to point into.  */
const char * getstr3(int i)
{
  return i ? "blabla" : "foobar";
}

int main(void)
{
  printf ("1: %s\n", addr_of_str);
  printf ("2: %s\n", addr_of_str2);
  printf ("3: %s\n", getstr3(1));
  return 0;
}
