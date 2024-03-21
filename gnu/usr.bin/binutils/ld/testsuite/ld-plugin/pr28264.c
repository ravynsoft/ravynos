void
__attribute__ ((symver ("foo@VERSION.1")))
foo (void)
{
}

void
__attribute__ ((symver ("bar@@VERSION.1")))
bar1 (void)
{
}
