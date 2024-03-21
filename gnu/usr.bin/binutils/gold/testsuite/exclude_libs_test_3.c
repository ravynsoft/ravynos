void lib3_default (void);
void lib3_hidden (void);
void lib3_internal (void);
void lib3_protected (void);

void __attribute__((visibility ("default")))
lib3_default (void)
{
}

void __attribute__((visibility ("hidden")))
lib3_hidden (void)
{
}

void __attribute__((visibility ("internal")))
lib3_internal (void)
{
}

void __attribute__((visibility ("protected")))
lib3_protected (void)
{
}
