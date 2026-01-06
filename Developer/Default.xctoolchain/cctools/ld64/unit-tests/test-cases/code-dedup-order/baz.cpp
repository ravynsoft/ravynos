struct Baz {

  int sum;

  Baz(int a, int b) { sum = a + b; }
};

int baz(int a, int b)  {
  return Baz(a, b).sum;
}
