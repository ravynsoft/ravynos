#pragma clang diagnostic ignored "-Wat-protocol"
@protocol P;

Protocol *getProtocol(void)
{
// Don't try to compile this on known-broken compilers.
#if !defined(__clang__)
	return @protocol(P);
	// Clang versions before 7 are broken, clang versions after 7 regard this
	// as a hard error and will refuse to compile it.
#elif __clang_major__ == 7
	return @protocol(P);
#else
	return 0;
#endif
}
