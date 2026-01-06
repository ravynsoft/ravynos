extern void* otherget();
extern int main();
extern const char* version();
extern void* mainget();

extern int def;
extern int ghi;
extern int com;

double getpi() { return 3.1415926535; }

void bar() 
{  
}



extern void* __dso_handle;
void* x = &__dso_handle;

int abc = 10;


int com3;
int com4;
int com5;

extern void* foo();

void* all[] = { &main, &version, &mainget, &getpi, &otherget, 
                &bar, &foo, &x, &abc, &def, &ghi, &com, &com3, &com4, &com5 };
                
                
void* foo() 
{
  return all;
}

