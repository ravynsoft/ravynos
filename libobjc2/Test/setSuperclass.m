#include "../objc/runtime.h"
#include <assert.h>
#include <stdio.h>

#pragma clang diagnostic ignored "-Wdeprecated-declarations"

__attribute__((objc_root_class))
@interface Root
{
	id isa;
}
@end
@interface DefaultSuperclass: Root @end

// test: new superclass when not initialized at the time of class_setSuperclass
@interface NotInitializedSuperclass1: Root @end
@interface Subclass1: DefaultSuperclass @end

// test: new superclass when already initialized at the time of class_setSuperclass
@interface NotInitializedSuperclass2: Root @end
@interface Subclass2: DefaultSuperclass @end
@interface Subclass2Subclass: Subclass2 @end

@interface ChangesDuringInitialize: DefaultSuperclass @end

// test: class gets reparented under its parent's parent.
@interface RemovedFromHierarchy: DefaultSuperclass @end
@interface MovesUpwardsInHierarchy: RemovedFromHierarchy @end

// test: one class initializes anotherwhile initializing during class_setSuperclass
@interface OtherInitializedClass: Root @end
@interface InitializesOneClassWhileBeingInitialized: NotInitializedSuperclass2 @end
@interface Subclass3: DefaultSuperclass @end

// test: transitioning an initialized class to an initialized superclass
// Test4Subclass only inherits access to "onlyExistsOnFinalSuperclass" from
// its new superclass's superclass (Test4FinalSuperclass)
@interface Test4Subclass: Root @end
@interface Test4FinalSuperclass: DefaultSuperclass @end

@implementation Root
+ (Class)class { return self; }
+ (BOOL)respondsToSelector:(SEL)selector {
	return class_respondsToSelector(object_getClass(self), selector);
}
+ (BOOL)instancesRespondToSelector:(SEL)selector {
	return class_respondsToSelector(self, selector);
}
@end

@implementation NotInitializedSuperclass1
static BOOL _notInitializedSuperclass1Initialized = NO;
+ (void)initialize {
	_notInitializedSuperclass1Initialized = YES;
}
+ (void)existsOnNotInitializedSuperclassMeta { };
+ (int)sameNameMeta { return 12; }
+ (int)overriddenMeta { return 12; }

- (BOOL)existsOnNotInitializedSuperclass { return YES; }
- (int)sameName { return 2; }
- (int)overridden { return 2; }
@end

@implementation NotInitializedSuperclass2
static BOOL _notInitializedSuperclass2Initialized = NO;
+ (void)initialize {
	_notInitializedSuperclass2Initialized = YES;
}
+ (void)existsOnNotInitializedSuperclassMeta { };
+ (int)sameNameMeta { return 13; }
+ (int)overriddenMeta { return 13; }
- (BOOL)existsOnNotInitializedSuperclass { return YES; }
- (int)sameName { return 3; }
- (int)overridden { return 3; }
@end

@implementation DefaultSuperclass
static BOOL _alreadyInitializedSuperclassInitialized = NO;
+ (void)initialize {
	_alreadyInitializedSuperclassInitialized = YES;
}
+ (void)existsOnDefaultSuperclassMeta { };
+ (int)sameNameMeta { return 14; }
+ (int)overriddenMeta { return 14; }
- (BOOL)existsOnDefaultSuperclass { return YES; }
- (int)sameName { return 4; }
- (int)overridden { return 4; }
@end

@implementation Subclass1
static BOOL _subclass1Initialized = NO;
+ (void)initialize {
	_subclass1Initialized = YES;
}
+ (int)overriddenMeta { return 15; } // shadows 14
- (BOOL)existsOnSubclass1 { return YES; }
- (int)overridden { return 5; } // shadows 4
@end

@implementation Subclass2
static BOOL _subclass2Initialized = NO;
+ (void)initialize {
	_subclass2Initialized = YES;
}
+ (int)overriddenMeta { return 16; } // shadows 14
- (BOOL)existsOnSubclass2 { return YES; }
- (int)overridden { return 6; } // shadows 4
- (int)intermediateOverride { return 100; }
@end

@implementation Subclass2Subclass
- (int)intermediateOverride { return 200; }
@end

@implementation ChangesDuringInitialize
+ (void)initialize {
	class_setSuperclass(self, objc_getClass("NotInitializedSuperclass1"));
}
+ (int)overriddenMeta { return 18; }
@end

@implementation RemovedFromHierarchy
+ (int)overriddenMeta { return 19; } // shadows 14 on DefaultSuperClass
+ (int)sameNameMeta { return 19; } // shadows 14 on DefaultSuperClass
+ (void)onlyExistsOnRemovedClassMeta { }
- (void)onlyExistsOnRemovedClass { }
@end

@implementation MovesUpwardsInHierarchy
+ (int)overriddenMeta { return 20; } // shadows 19 on RemovedFromHierarchy or 14 on DefaultSuperClass
@end

@implementation OtherInitializedClass
static BOOL _otherInitializedClassInitialized = NO;
+ (void)initialize {
	_otherInitializedClassInitialized = YES;
}
@end

@implementation InitializesOneClassWhileBeingInitialized
+ (void)initialize {
	[OtherInitializedClass class];
}
@end

@implementation Subclass3
@end

@implementation Test4Subclass
@end
@implementation Test4FinalSuperclass
+ (int)onlyExistsOnFinalSuperclassMeta { return 501; }
- (int)onlyExistsOnFinalSuperclass { return 500; }
@end

static int failures = 0;

#define expect(x) do \
{ \
	if (!(x)) \
	{ \
		fprintf(stderr, "expectation FAILED: %s\n", #x); \
		++failures; \
	} \
} while(0)

int main(int argc, char **argv) {
	/* Transitioning to a new superclass before +initialize has been called */
	{
		Class subclass1 = objc_getClass("Subclass1");
		Class secondSuperclass = objc_getClass("NotInitializedSuperclass1");

		assert(!_notInitializedSuperclass1Initialized);
		assert(!_subclass1Initialized);

		class_setSuperclass(subclass1, secondSuperclass);

		// assert: dtable has not been installed; new superclass is still not initialized
		assert(!_notInitializedSuperclass1Initialized);

		[Subclass1 class];
		// initialization and dtable installation has taken place
		assert(_notInitializedSuperclass1Initialized);

		Subclass1 *subclass1instance1 = class_createInstance(subclass1, 0);

		// CLASS
		// can call method on subclass
		expect([subclass1instance1 existsOnSubclass1]);
		// can call method on _new_ superclass
		expect([(id)subclass1instance1 existsOnNotInitializedSuperclass]);
		// does not respond to selector from original superclass
		expect(![subclass1 instancesRespondToSelector:@selector(existsOnDefaultSuperclass)]);
		// *does* respond to selector from new superclass
		expect([subclass1 instancesRespondToSelector:@selector(existsOnNotInitializedSuperclass)]);
		// method existing on both old and new superclass kept, IMP updated
		expect(2 == [subclass1instance1 sameName]);
		// method existing on subclass, old and new superclass kept, IMP kept
		expect(5 == [subclass1instance1 overridden]);

	
		// METACLASS
		// metaclass does not respond to selector from original meta superclass
		expect(![subclass1 respondsToSelector:@selector(existsOnDefaultSuperclassMeta)]);
		// metaclass *does* respond to selector from new meta superclass
		expect([subclass1 respondsToSelector:@selector(existsOnNotInitializedSuperclassMeta)]);
		// method existing on both old and new superclass kept, IMP updated
		expect(12 == [subclass1 sameNameMeta]);
		// method existing on subclass, old and new superclass kept, IMP kept
		expect(15 == [subclass1 overriddenMeta]);
	}

	/* Transitioning to a new superclass when +initialize has already been called */
	{
		Class subclass2 = objc_getClass("Subclass2");
		Class secondSuperclass = objc_getClass("NotInitializedSuperclass2");
		assert(!_notInitializedSuperclass2Initialized);
		assert(!_subclass2Initialized);

		[Subclass2 class];
		[Subclass2Subclass class]; // Make sure the subclass is initialized too.
		assert(_alreadyInitializedSuperclassInitialized);
		assert(_subclass2Initialized);

		Subclass2 *subclass2instance1 = class_createInstance(subclass2, 0);
		assert([subclass2instance1 existsOnSubclass2]);

		class_setSuperclass(subclass2, secondSuperclass);
		assert(_notInitializedSuperclass2Initialized);

		// CLASS
		// can call method on subclass
		expect([subclass2instance1 existsOnSubclass2]);
		// can call method on _new_ superclass
		expect([(id)subclass2instance1 existsOnNotInitializedSuperclass]);
		// does not respond to selector from original superclass
		expect(![subclass2 instancesRespondToSelector:@selector(existsOnDefaultSuperclass)]);
		// *does* respond to selector from new superclass
		expect([subclass2 instancesRespondToSelector:@selector(existsOnNotInitializedSuperclass)]);

		// method existing on both old and new superclass kept, IMP updated
		expect(3 == [subclass2instance1 sameName]);
		// method existing on subclass, old and new superclass kept, IMP kept
		expect(6 == [subclass2instance1 overridden]);
		// method existing only on subclass preserved
		expect(100 == [subclass2instance1 intermediateOverride]);

		// METACLASS
		// metaclass does not respond to selector from original meta superclass
		expect(![subclass2 respondsToSelector:@selector(existsOnDefaultSuperclassMeta)]);
		// metaclass *does* respond to selector from new meta superclass
		expect([subclass2 respondsToSelector:@selector(existsOnNotInitializedSuperclassMeta)]);
		// method existing on both old and new superclass kept, IMP updated
		expect(13 == [subclass2 sameNameMeta]);
		// method existing on subclass, old and new superclass kept, IMP kept
		expect(16 == [subclass2 overriddenMeta]);

		// SUBCLASS
		Subclass2 *subclass2subclassInstance = class_createInstance([Subclass2Subclass class], 0);
		expect(![Subclass2Subclass instancesRespondToSelector:@selector(existsOnDefaultSuperclass)]);
		expect(![Subclass2Subclass respondsToSelector:@selector(existsOnDefaultSuperclassMeta)]);
		expect(3 == [subclass2subclassInstance sameName]);
		expect(6 == [subclass2subclassInstance overridden]);
		expect(200 == [subclass2subclassInstance intermediateOverride]);
		expect(13 == [Subclass2Subclass sameNameMeta]);
		expect(16 == [Subclass2Subclass overriddenMeta]);
	}

	/* Transitioning ourselves to a new superclass while +initialize is running */
	{
		expect(12 == [ChangesDuringInitialize sameNameMeta]);
		expect(18 == [ChangesDuringInitialize overriddenMeta]);
	}

	/* Transitioning to a superclass that's in our inheritance hierarchy already */
	{
		assert(20 == [MovesUpwardsInHierarchy overriddenMeta]);
		assert(19 == [MovesUpwardsInHierarchy sameNameMeta]);
		assert([MovesUpwardsInHierarchy respondsToSelector:@selector(onlyExistsOnRemovedClassMeta)]);
		assert([MovesUpwardsInHierarchy instancesRespondToSelector:@selector(onlyExistsOnRemovedClass)]);

		class_setSuperclass([MovesUpwardsInHierarchy class], [DefaultSuperclass class]);

		expect(20 == [MovesUpwardsInHierarchy overriddenMeta]); // still overridden
		expect(14 == [MovesUpwardsInHierarchy sameNameMeta]); // falls back to DefaultSuperclass
		expect(![MovesUpwardsInHierarchy respondsToSelector:@selector(onlyExistsOnRemovedClassMeta)]);
		expect(![MovesUpwardsInHierarchy instancesRespondToSelector:@selector(onlyExistsOnRemovedClass)]);
	}

	/* Transitioning to a superclass that may cause initialize lock contention */
	{
		assert(!_otherInitializedClassInitialized);
		expect(14 == [Subclass3 sameNameMeta]);
		expect(14 == [Subclass3 overriddenMeta]);

		class_setSuperclass([Subclass3 class], objc_getClass("InitializesOneClassWhileBeingInitialized"));

		expect(_otherInitializedClassInitialized);
		expect(13 == [Subclass3 sameNameMeta]);
		expect(13 == [Subclass3 overriddenMeta]);
	}

	/* Transitioning an initialized class to an initialized superclass. */
	{
		Class test4subclass = objc_getClass("Test4Subclass");
		Class newSuperclass = objc_getClass("Test4FinalSuperclass");

		// Make sure every class in the hierarchy is initialized.
		[Test4Subclass class];
		[Test4FinalSuperclass class];

		expect(![test4subclass respondsToSelector:@selector(onlyExistsOnFinalSuperclassMeta)]);
		expect(![test4subclass instancesRespondToSelector:@selector(onlyExistsOnFinalSuperclass)]);

		class_setSuperclass(test4subclass, newSuperclass);

		Test4Subclass *test4instance = class_createInstance(test4subclass, 0);

		expect([test4subclass respondsToSelector:@selector(onlyExistsOnFinalSuperclassMeta)]);
		expect([test4subclass instancesRespondToSelector:@selector(onlyExistsOnFinalSuperclass)]);

		expect(501 == [(id)test4subclass onlyExistsOnFinalSuperclassMeta]);
		expect(500 == [(id)test4instance onlyExistsOnFinalSuperclass]);
	}

	return failures;
}
