#include <stdio.h>

void __attribute__((weak)) my_weak()
{

}

int main()
{
	my_weak();
	return 0;
}

