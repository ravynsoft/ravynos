/*
 * Testcase to verify that reference to foo@BAR and a definition of foo@@BAR
 * are not treated as a multiple def.
 */
#include "vers.h"

const char * bar1 = "asdf";
const char * bar2 = "asdf";

extern int old_foo1();

int
bar()
{
	return 3;
}

int
original_foo()
{
	return 1+bar();

}

int
old_foo()
{
	return 10+bar();

}

int
new_foo()
{
	return 1000+bar();

}

int
main()
{
  old_foo1();
  return 0;
}

FUNC_SYMVER(original_foo, foo@);
FUNC_SYMVER(old_foo, foo@VERS_1.1);
FUNC_SYMVER(old_foo1, foo@VERS_1.2);
FUNC_SYMVER(new_foo, foo@@VERS_1.2);
