#include <locale.h>
#include <time.h>
#include <stdio.h>
#include "Test.h"
#include "../objc/hooks.h"
#include "../objc/capabilities.h"


int count;
static const int loops = 50000;
static const int depth = 42;
static const int calls = loops * (depth+1);
static int d = depth;
Class TestCls;
int tracecount;

@implementation Test (Nothing)
+ nothing
{
	count++;
	if (d > 0)
	{
		d--;
		[self nothing];
		d++;
	}
	return 0;
}
@end
__thread IMP real;

static int interposecount;
id interpose(id self, SEL _cmd) {
	interposecount++;
	return real(self, _cmd);
}

//IMP hook(id object, SEL selector, IMP method, int isReturn, void *wordReturn) { tracecount++; return (IMP)logExit; }
IMP hook(id object, SEL selector, IMP method, int isReturn, void *wordReturn) { tracecount++; real = method; return (IMP)0; }
IMP hook0(id object, SEL selector, IMP method, int isReturn, void *wordReturn) { tracecount++; real = method; return (IMP)1; }
IMP hook1(id object, SEL selector, IMP method, int isReturn, void *wordReturn) { tracecount++; real = method; return (IMP)interpose; }

int main(void)
{
	if (!objc_test_capability(OBJC_CAP_TRACING))
	{
		fprintf(stderr, "Tracing support not compiled into runtime\n");
		return 0;
	}
	TestCls = objc_getClass("Test");
	objc_registerTracingHook(@selector(nothing), hook);
	interposecount = 0;
	count = 0;
	tracecount = 0;
	for (int i=0 ; i<loops ; i++)
	{
		[TestCls nothing];
	}
	assert(count == calls);
	assert(tracecount == calls);
	assert(interposecount == 0);
	objc_registerTracingHook(@selector(nothing), hook0);
	interposecount = 0;
	count = 0;
	tracecount = 0;
	for (int i=0 ; i<loops ; i++)
	{
		[TestCls nothing];
	}
	assert(count == calls);
	assert(tracecount == calls * 2);
	assert(interposecount == 0);
	objc_registerTracingHook(@selector(nothing), hook1);
	interposecount = 0;
	count = 0;
	tracecount = 0;
	for (int i=0 ; i<loops ; i++)
	{
		[TestCls nothing];
	}
	assert(count == calls);
	assert(tracecount == calls);
	assert(interposecount == calls);
	return 0;
}
