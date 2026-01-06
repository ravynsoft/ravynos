template<typename T>
struct foo_impl {

  foo_impl() {}

  operator T() const { return 0; }
};
