
extern const char* mystring;

const char** myp = &mystring;

int com;

const char* inc() {
  ++com;
  return "";
}


