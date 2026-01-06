
int main()
{
	return 0;
}



void dead_door_knob() {  }


extern "C" int ctr();
extern "C" void dtr();


int ctr() { return 10; }
void dtr() { }


#if __STATIC__
extern "C" void __cxa_atexit();
void __cxa_atexit() {}
#endif

