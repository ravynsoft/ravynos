/* Embed a version of the real activation helper that has been altered
 * to be testable. We monkey-patch it like this because we don't want to
 * compile test-only code into the real setuid executable, and Automake
 * versions older than 1.16 can't cope with expanding directory variables
 * in SOURCES when using subdir-objects. */
#define ACTIVATION_LAUNCHER_TEST
#include "bus/activation-helper.c"
#include "bus/activation-helper-bin.c"
