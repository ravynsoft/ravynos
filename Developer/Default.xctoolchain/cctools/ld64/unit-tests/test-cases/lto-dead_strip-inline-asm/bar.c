

void qux() {}

asm("\t.text\n"
    "\t.globl _foo\n"
    "_foo:\n"
    "\tnop\n");

extern void foo();

void (*bar())() {
  return foo;
}
