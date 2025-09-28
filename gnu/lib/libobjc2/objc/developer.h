#if defined(__clang__) && !defined(__OBJC_RUNTIME_INTERNAL__)
#pragma clang system_header
#endif

enum objc_developer_mode_np
{
	/** User mode - the default. */
	objc_developer_mode_user,
	/** Developer mode - allows replacing classes. */
	objc_developer_mode_developer
};
/*
 * Sets the developer mode.  When in user mode (the default),
 * loading two classes with the same name will cause the program to abort.  In
 * developer mode, the new class will replace the old one.  If the ivar layouts
 * are the same, the new class will be treated as a category.  If they are
 * different, then it will replace the old one in the class table, meaning that
 * message sends to the class will go to the new version, but existing
 * instances will not acquire the new methods.
 *
 * The runtime currently only supports two modes, although more may be added in
 * the future.  The behaviour of the existing modes will be maintained if this
 * is the case.
 */
void objc_setDeveloperMode_np(enum objc_developer_mode_np);
