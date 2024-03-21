struct blah
{
  int boring;
  int __attribute__((vector_size(8))) foo;
  const int __attribute__((vector_size(8))) bar;
  int this_is_printed;
} wibble __attribute__((__used__));
