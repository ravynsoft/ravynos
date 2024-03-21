extern void bar(void) __attribute__((__visibility__("hidden")));

void foo (void)
{
  bar ();
}
