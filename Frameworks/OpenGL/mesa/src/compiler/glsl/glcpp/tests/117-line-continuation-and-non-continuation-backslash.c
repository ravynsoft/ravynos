/* This test case is the minimal case to replicate the bug reported here:
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=65112
 *
 * To trigger the bug, there must be a line-continuation sequence
 * (backslash newline), then an additional newline character, and
 * finally another backslash that is not part of a line-continuation
 * sequence.
 */
\

/* \ */
