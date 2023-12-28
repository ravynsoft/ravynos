extern int FLAGS_verbose;
extern void bar (void);
int
a(void) {
return FLAGS_verbose;
}
void unused (void) { bar(); }
int main() { return a (); }
