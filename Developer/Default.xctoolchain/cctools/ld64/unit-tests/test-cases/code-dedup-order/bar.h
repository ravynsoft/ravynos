struct Bar {

  int sum;

  Bar(int a, int b) { sum = a + b; }

  template<typename T>
  T getSum() { return sum; }
};

template<typename T>
T bar_impl(T a, T b) {
  return Bar(a, b).getSum<int>();
}
