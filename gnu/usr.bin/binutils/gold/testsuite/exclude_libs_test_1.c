void lib1_default (void);
void lib1_hidden (void);
void lib1_internal (void);
void lib1_protected (void);
void lib1_ref (void);
extern void lib2_default (void);

void __attribute__((visibility ("default")))
lib1_default (void)
{
}

void __attribute__((visibility ("hidden")))
lib1_hidden (void)
{
}

void __attribute__((visibility ("internal")))
lib1_internal (void)
{
}

void __attribute__((visibility ("protected")))
lib1_protected (void)
{
}

void
lib1_ref (void)
{
  lib2_default ();
}
