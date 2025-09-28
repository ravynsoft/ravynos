__thread char bar[10] __attribute__ ((visibility("hidden")));
extern char size_of_bar __asm__ ("bar@SIZE");
char *bar_size = &size_of_bar;
