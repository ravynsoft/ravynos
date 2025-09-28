/* Ensure we get syscall declared.  */
#define _DEFAULT_SOURCE

#include <unistd.h>
#include <link.h>
#include <syscall.h>

#define STRING_COMMA_LEN(STR) (STR), (sizeof (STR) - 1)

int
main (int argc, char **argv)
{
  char **ev = &argv[argc + 1];
  char **evp = ev;
  ElfW(auxv_t) *av;
  const ElfW(Phdr) *phdr = NULL;
  size_t phnum = 0;
  size_t loadnum = 0;
  int fd = STDOUT_FILENO;
  size_t i;

  while (*evp++ != NULL)
    ;

  av = (ElfW(auxv_t) *) evp;

  for (; av->a_type != AT_NULL; ++av)
    switch (av->a_type)
      {
      case AT_PHDR:
	phdr = (const void *) av->a_un.a_val;
	break;
      case AT_PHNUM:
	phnum = av->a_un.a_val;
	break;
      }

  for (i = 0; i < phnum; i++, phdr++)
    if (phdr->p_type == PT_LOAD)
      loadnum++;

  syscall (SYS_write, fd, STRING_COMMA_LEN ("PASS\n"));

  syscall (SYS_exit, !loadnum);
  return 0;
}
