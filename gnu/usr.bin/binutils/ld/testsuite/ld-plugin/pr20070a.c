extern int printf (const char *fmt, ...);

extern const char *text;

int main (int argc, const char **argv)
{
  printf ("%s\n", text);
  return 0;
}
