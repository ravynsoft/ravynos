void lib2_default (void);
void lib2_hidden (void);
void lib2_internal (void);
void lib2_protected (void);

void __attribute__((visibility ("default")))
lib2_default (void)
{
}

void __attribute__((visibility ("hidden")))
lib2_hidden (void)
{
}

void __attribute__((visibility ("internal")))
lib2_internal (void)
{
}

void __attribute__((visibility ("protected")))
lib2_protected (void)
{
}
