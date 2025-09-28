/*
 * Testcase to verify that foo@BAR and foo@@BAR are correctly detected
 * as a multiply defined symbol.
 */
#include "vers.h"

const char * bar1 = "asdf";
const char * bar2 = "asdf";

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
old_foo1()
{
	return 100+bar();

}

int
new_foo()
{
	return 1000+bar();

}

FUNC_SYMVER(original_foo, foo@);
FUNC_SYMVER(old_foo, foo@VERS_1.1);
FUNC_SYMVER(old_foo1, foo@VERS_1.2);
FUNC_SYMVER(new_foo, foo@@VERS_1.2);

int
main ()
{
  return 0;
}
