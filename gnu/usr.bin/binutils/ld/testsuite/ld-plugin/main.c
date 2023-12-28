
extern int printf (const char *fmt, ...);

extern const char *text;
extern int func (void);

int retval = 0;

int main (int argc, const char **argv)
{
  printf ("%s\n", text);
  return func ();
}
