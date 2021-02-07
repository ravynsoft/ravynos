/* -*- objc -*- 
 *
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 */

#import <Foundation/Foundation.h>

/* the initial (reset) state of any flag set */
#define NORESULTS     0

/**
 *  A class representing some test case and implementing
 *  the protocol TestProgress allows descendants to have two sets
 *  of flags: the reference one and the actual one.
 *  The reference flag set is what the class's instance's
 *  state supposed to be if the test was successfull.
 *  The actual flag set is the current state of the class's
 *  instance during execution of the test.
 *  Both can be used in conjuction to decide about test's
 *  success or fail by checking at the end of the test
 *  whether the sets are identical.
 */ 
@protocol TestProgress

/**
 * Resets the actual flags to the pristine state (NORESULTS).
 */
- (void)resetFlags;

/**
 * Sets the actual flags defined by the supplied mask.
 */
- (void)setFlags:(NSUInteger)mask;

/**
 * Unset the actual flags defined by the supplied mask.
 */
- (void)unsetFlags:(NSUInteger)mask;

/**
 * Returns YES if the actual flags defined by the supplied mask are
 * set.
 */
- (BOOL)isFlagSet:(NSUInteger)mask;

/**
 * Resets the reference flags to the pristine state (NORESULTS).
 */
- (void)resetReferenceFlags;

/**
 * Stores the supplied mask as the reference flags state.
 * The actual flags state equal to the reference one signifies
 * a success.
 */
- (void)setReferenceFlags:(NSUInteger)mask;

/**
 * Unset the reference flags defined by the supplied mask.
 */
- (void)unsetReferenceFlags:(NSUInteger)mask;

/**
 * Returns YES if the reference flags defined by the supplied mask are
 * set.
 */
- (BOOL)isReferenceFlagSet:(NSUInteger)mask;

/**
 * Returns YES if all conditions are met (by default if the actual flag set
 * is equal to the reference flag set).
 */
- (BOOL)isSuccess;

@end /* TestProgress */

/**
 *  The abstract class representing a test case and implementing
 *  the protocol TestProgress.
 */
@interface TestCase : NSObject <TestProgress>
{
  /* the flags indicating test progress (the actual one) */
  NSUInteger         _flags;
  /* the reference flag set signifying the test's successful state */
  NSUInteger      _refFlags;
  /* the particular flag indicating a failure not related to the test */
  BOOL             _failed;
  /* the particular flag signifying the test has been done */
  BOOL _done;
  /* keeps the extra which can likely be of NSDictionary class in which case
   * it's keys hold parameters of the test */
  id _extra;
  /* the debug mode flag */
  BOOL _debug;
}

- (id)init;

/**
 *  A descendant class can place in the own implementation of the method
 *  some preliminary code needed to conduct the test case. The argument
 *  'extra' is for future enhancements. The extra is only retained within
 *  the method for descendants (the abstarct class doesn't do anything
 *  with this argument).
 *  So descendant's implementation MUST call the super's one.
 */
- (void)setUpTest:(id)extra;

/**
 *  A descendant class should place the code conducting the test in
 *  the own implementation of the method. The argument 'extra' is for
 *  future enhancements.
 */
- (void)startTest:(id)extra;

/**
 *  A descendant class can place in the own implementation of the method
 *  some cleaning code. The argument 'extra' is for future enhancements.
 *  Descendant's implementation MUST call the super's one.
 */
- (void)tearDownTest:(id)extra;

/**
 *  Raises the verbosity level if supplied with YES.
 */
- (void)setDebug:(BOOL)flag;

@end /* TestCase */
