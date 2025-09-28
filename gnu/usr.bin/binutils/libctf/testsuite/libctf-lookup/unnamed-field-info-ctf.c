struct A
{
  int a;
  char *b;
  struct
  {
    struct
    {
      char *one;
      int two;
    };
    union
    {
      char *three;
    };
  };
  struct
  {
    int four;
  };
  union
  {
    struct
    {
      double x;
      long y;
    };
    struct
    {
      struct { char *foo; } z;
      float aleph;
    };
  };
};

struct A used;
