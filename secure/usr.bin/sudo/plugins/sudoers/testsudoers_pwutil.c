/*
 * Build pwutil_impl.c with a function prefix of "testsudoers_" instead
 * of "sudo_" and call our custom getpwnam/getpwuid/getgrnam/getgrgid.
 */

#define PWUTIL_PREFIX		testsudoers

#include <testsudoers_pwutil.h>
#include <tsgetgrpw.h>
#include "pwutil_impl.c"
