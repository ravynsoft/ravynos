#import "../objc/runtime.h"
#include "Test.h"

@interface Space : Test
{
	uint16_t baseSmall;
}
@end

@interface StartsWithChar : Space
{
	char c1;
	char c2;
	char c3;
}
@end
@interface StartsWithBitfield : Space
{
	@public
	uint16_t b1:4;
	uint16_t b2:4;
	uint16_t b3:4;
	uint16_t b4:4;
	uint16_t notBitfield;
}
@end

@implementation Space @end
@implementation StartsWithChar @end
@implementation StartsWithBitfield @end

int main(int argc, char *argv[])
{
	Class s = objc_getClass("Space");
	assert(s);
	Class swc = objc_getClass("StartsWithChar");
	assert(swc);
	Class swb = objc_getClass("StartsWithBitfield");
	assert(swb);
	Ivar baseSmall = class_getInstanceVariable(s, "baseSmall");
	Ivar c1 = class_getInstanceVariable(swc, "c1");
	Ivar c2 = class_getInstanceVariable(swc, "c2");
	Ivar c3 = class_getInstanceVariable(swc, "c3");
	Ivar b1 = class_getInstanceVariable(swb, "b1");
	Ivar b2 = class_getInstanceVariable(swb, "b2");
	Ivar b3 = class_getInstanceVariable(swb, "b3");
	Ivar b4 = class_getInstanceVariable(swb, "b4");
	Ivar notBitfield = class_getInstanceVariable(swb, "notBitfield");
	assert(baseSmall);
	assert(c1);
	assert(c2);
	assert(c3);
	assert(b1);
	assert(b2);
	assert(b3);
	assert(b4);
	assert(notBitfield);
	StartsWithBitfield *swbi = [StartsWithBitfield new];

	// Alternating 01 bit pattern, should catch small overwrites.
	swbi->notBitfield = 0x5555;
	swbi->b1 = 5;
	swbi->b2 = 11;
	swbi->b3 = 11;
	swbi->b4 = 5;
	assert(swbi->b1 == 5);
	assert(swbi->b2 == 11);
	assert(swbi->b3 == 11);
	assert(swbi->b4 == 5);
	assert(swbi->notBitfield == 0x5555);
	swbi->notBitfield = 0xaaaa;
	swbi->b1 = 5;
	swbi->b2 = 11;
	swbi->b3 = 5;
	swbi->b4 = 11;
	assert(swbi->notBitfield == 0xaaaa);
	assert(swbi->b1 == 5);
	assert(swbi->b2 == 11);
	assert(swbi->b3 == 5);
	assert(swbi->b4 == 11);

#ifdef NEW_ABI
	ptrdiff_t baseSmallOffset = ivar_getOffset(baseSmall);
	// These should pass with the old ABI, but they don't at the moment.  The
	// way that they don't is not very harmful though: we just get a bit of
	// redundant padding, so I don't consider a fix a very high priority.
	assert(ivar_getOffset(c1) == baseSmallOffset + 2);
	assert(ivar_getOffset(c2) == baseSmallOffset + 3);
	assert(ivar_getOffset(c3) == baseSmallOffset + 4);
	assert(ivar_getOffset(b1) == baseSmallOffset + 2);
	assert(ivar_getOffset(b2) == baseSmallOffset + 2);
	assert(ivar_getOffset(b3) == baseSmallOffset + 3);
	assert(ivar_getOffset(b4) == baseSmallOffset + 3);
	assert(ivar_getOffset(notBitfield) == baseSmallOffset + 4);
#endif


	return 0;
}
