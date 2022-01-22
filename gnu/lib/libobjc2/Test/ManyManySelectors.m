#include "Test.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>


static BOOL methodCalled = NO;

static char selBuffer[] = "XXXXXXXselectorXXXXXXXX";

static id x(id self, SEL _cmd)
{
	methodCalled = YES;
	if (strcmp(selBuffer, sel_getName(_cmd)) != 0)
	{
		fprintf(stderr, "'%s' != '%s'\n", selBuffer, sel_getName(_cmd));
	}
	assert(strcmp(selBuffer, sel_getName(_cmd)) == 0);
	return self;
}

int main(void)
{
	SEL nextSel;
	Class cls = [Test class];
	assert(cls != Nil);
	int sel_size = 0;
	for (uint32_t i=0 ; i<0xf0000 ; i++)
	{
		snprintf(selBuffer, sizeof(selBuffer), "%" PRId32 "selector%" PRIx32, i, i);
		nextSel = sel_registerName(selBuffer);
		if (strcmp(selBuffer, sel_getName(nextSel)) != 0)
		{
			fprintf(stderr, "'%s' != '%s'\n", selBuffer, sel_getName(nextSel));
		}
		assert(strcmp(selBuffer, sel_getName(nextSel)) == 0);
		sel_size += strlen(selBuffer);
	}
	assert(class_addMethod(object_getClass([Test class]), nextSel, (IMP)x, "@@:"));
	assert(cls == [Test class]);
	// Test both the C and assembly code paths.
	objc_msg_lookup(cls, nextSel)(cls, nextSel);
	assert(methodCalled == YES);
	methodCalled = NO;
	objc_msgSend([Test class], nextSel);
	assert(methodCalled == YES);
	return 0;
}

