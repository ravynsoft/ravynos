
@interface Foo
@end

@implementation Foo
@end


int main()
{
	return 0;
}


#if __i386__ && __OBJC2__
	int _objc_empty_vtable = 1;
#endif
