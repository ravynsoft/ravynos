
extern "C" int ctr();
extern "C" void dtr();

class Foo
{
public:
	Foo() : field(ctr()) { }
        ~Foo() { dtr(); }
private:
	int field;
};


Foo f1;
Foo f2;


