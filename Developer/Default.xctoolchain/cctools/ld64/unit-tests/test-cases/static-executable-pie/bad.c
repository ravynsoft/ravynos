static int my;

int getmy() 
{ 
#if __x86_64__
	__asm(" .quad _my");
#endif
	return my; 
}
