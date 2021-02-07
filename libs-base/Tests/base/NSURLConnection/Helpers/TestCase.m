/*
 *  Author: Sergei Golovin <Golovin.SV@gmail.com>
 */

#import "TestCase.h"

@implementation TestCase

- (id) init
{
  if ((self = [super init]) != nil)
    {
      [self resetFlags];
      [self setReferenceFlags: NORESULTS];
      _failed = NO;
      _done = NO;
    }

  return self;
}

- (void) dealloc
{
  [self tearDownTest: _extra];

  [super dealloc];
}

/* TestProgress */
- (void) resetFlags
{
  _flags = NORESULTS;
}

- (void) setFlags:(NSUInteger)mask
{
  _flags = _flags | mask;
}

- (void) unsetFlags:(NSUInteger)mask
{
  _flags = _flags & ~mask;
}

- (BOOL) isFlagSet:(NSUInteger)mask
{
  return ((_flags & mask) == mask);
}

- (void) resetReferenceFlags
{
  _refFlags = NORESULTS;
}

- (void) setReferenceFlags:(NSUInteger)mask
{
  _refFlags = _refFlags | mask;
}

- (void) unsetReferenceFlags:(NSUInteger)mask
{
  _refFlags = _refFlags & ~mask;
}

- (BOOL) isReferenceFlagSet:(NSUInteger)mask
{
  return ((_refFlags & mask) == mask);
}

- (BOOL) isSuccess
{
  if(!_failed && (_flags == _refFlags))
    {
      return YES;
    }

  return NO;
}
/* end of TestProgress */

- (void)setUpTest:(id)extra
{
  [self resetFlags];
  [self resetReferenceFlags];
  _failed = NO;
  _done = NO;

  ASSIGN(_extra, extra);
}

- (void)startTest:(id)extra
{
  // does nothing
}

- (void)tearDownTest:(id)extra
{
  DESTROY(_extra);
}

- (void)setDebug:(BOOL)flag
{
  _debug = flag;
}

@end /* TestCase */
