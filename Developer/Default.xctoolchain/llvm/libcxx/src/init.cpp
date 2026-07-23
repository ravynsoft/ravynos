extern "C" {
  void __libcxx_memrsc_init(void);
  void __libcxx_exp_memrsc_init(void);
  void __libcxx_ios_init(void) noexcept;
  void __libcxx_init(void);
}

// Initialize global statics. Called from libSystem's init.
void __libcxx_init(void)
{
    __libcxx_memrsc_init();
    __libcxx_exp_memrsc_init();
    __libcxx_ios_init();
}
