#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSExpression.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSPredicate.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

void
testKVC(NSDictionary *dict)
{
  PASS([@"A Title" isEqual: [dict valueForKey: @"title"]], "valueForKeyPath: with string");
  PASS([@"A Title" isEqual: [dict valueForKeyPath: @"title"]], "valueForKeyPath: with string");
PASS([@"John" isEqual: [dict valueForKeyPath: @"Record1.Name"]], "valueForKeyPath: with string");
  PASS(30 == [[dict valueForKeyPath: @"Record2.Age"] intValue], "valueForKeyPath: with int");
}

void
testContains(NSDictionary *dict)
{
  NSPredicate *p;

  p = [NSPredicate predicateWithFormat: @"%@ CONTAINS %@", @"AABBBAA", @"BBB"];
  PASS([p evaluateWithObject: dict], "%%@ CONTAINS %%@");
  p = [NSPredicate predicateWithFormat: @"%@ IN %@", @"BBB", @"AABBBAA"];
  PASS([p evaluateWithObject: dict], "%%@ IN %%@");
}

void
testString(NSDictionary *dict)
{
  NSPredicate *p;

  p = [NSPredicate predicateWithFormat: @"%K == %@", @"Record1.Name", @"John"];
  PASS([p evaluateWithObject: dict], "%%K == %%@");
  p = [NSPredicate predicateWithFormat: @"%K MATCHES[c] %@", @"Record1.Name", @"john"];
  PASS([p evaluateWithObject: dict], "%%K MATCHES[c] %%@");
  p = [NSPredicate predicateWithFormat: @"%K BEGINSWITH %@", @"Record1.Name", @"Jo"];
  PASS([p evaluateWithObject: dict], "%%K BEGINSWITH %%@");
  p = [NSPredicate predicateWithFormat: @"(%K == %@) AND (%K == %@)", @"Record1.Name", @"John", @"Record2.Name", @"Mary"];
  PASS([p evaluateWithObject: dict], "(%%K == %%@) AND (%%K == %%@)");
}

void
testInteger(NSDictionary *dict)
{
  NSPredicate *p;

  p = [NSPredicate predicateWithFormat: @"%K == %d", @"Record1.Age", 34];
  PASS([p evaluateWithObject: dict], "%%K == %%d");
  p = [NSPredicate predicateWithFormat: @"%K = %@", @"Record1.Age", [NSNumber numberWithInt: 34]];
  PASS([p evaluateWithObject: dict], "%%K = %%@");
  p = [NSPredicate predicateWithFormat: @"%K == %@", @"Record1.Age", [NSNumber numberWithInt: 34]];
  PASS([p evaluateWithObject: dict], "%%K == %%@");
  p = [NSPredicate predicateWithFormat: @"%K < %d", @"Record1.Age", 40];
  PASS([p evaluateWithObject: dict], "%%K < %%d");
  p = [NSPredicate predicateWithFormat: @"%K < %@", @"Record1.Age", [NSNumber numberWithInt: 40]];
  PASS([p evaluateWithObject: dict], "%%K < %%@");
  p = [NSPredicate predicateWithFormat: @"%K <= %@", @"Record1.Age", [NSNumber numberWithInt: 40]];
  PASS([p evaluateWithObject: dict], "%%K <= %%@");
  p = [NSPredicate predicateWithFormat: @"%K <= %@", @"Record1.Age", [NSNumber numberWithInt: 34]];
  PASS([p evaluateWithObject: dict], "%%K <= %%@");
  p = [NSPredicate predicateWithFormat: @"%K > %@", @"Record1.Age", [NSNumber numberWithInt: 20]];
  PASS([p evaluateWithObject: dict], "%%K > %%@");
  p = [NSPredicate predicateWithFormat: @"%K >= %@", @"Record1.Age", [NSNumber numberWithInt: 34]];
  PASS([p evaluateWithObject: dict], "%%K >= %%@");
  p = [NSPredicate predicateWithFormat: @"%K >= %@", @"Record1.Age", [NSNumber numberWithInt: 20]];
  PASS([p evaluateWithObject: dict], "%%K >= %%@");
  p = [NSPredicate predicateWithFormat: @"%K != %@", @"Record1.Age", [NSNumber numberWithInt: 20]];
  PASS([p evaluateWithObject: dict], "%%K != %%@");
  p = [NSPredicate predicateWithFormat: @"%K <> %@", @"Record1.Age", [NSNumber numberWithInt: 20]];
  PASS([p evaluateWithObject: dict], "%%K <> %%@");
  p = [NSPredicate predicateWithFormat: @"%K BETWEEN %@", @"Record1.Age", [NSArray arrayWithObjects: [NSNumber numberWithInt: 20], [NSNumber numberWithInt: 40], nil]];
  PASS([p evaluateWithObject: dict], "%%K BETWEEN %%@");
  p = [NSPredicate predicateWithFormat: @"(%K == %d) OR (%K == %d)", @"Record1.Age", 34, @"Record2.Age", 34];
  PASS([p evaluateWithObject: dict], "(%%K == %%d) OR (%%K == %%d)");


}

void
testFloat(NSDictionary *dict)
{
  NSPredicate *p;

  p = [NSPredicate predicateWithFormat: @"%K < %f", @"Record1.Age", 40.5];
  PASS([p evaluateWithObject: dict], "%%K < %%f");
p = [NSPredicate predicateWithFormat: @"%f > %K", 40.5, @"Record1.Age"];
  PASS([p evaluateWithObject: dict], "%%f > %%K");
}

void
testAttregate(NSDictionary *dict)
{
  NSPredicate *p;

  p = [NSPredicate predicateWithFormat: @"%@ IN %K", @"Kid1", @"Record1.Children"];
  PASS([p evaluateWithObject: dict], "%%@ IN %%K");
  p = [NSPredicate predicateWithFormat: @"Any %K == %@", @"Record2.Children", @"Girl1"];
  PASS([p evaluateWithObject: dict], "Any %%K == %%@");
}


void
testBlock(NSDictionary* dict)
{
  START_SET("Block predicates");
# if __has_feature(blocks)
  NSPredicate *p = nil;
  NSPredicate *p2 = nil;
  NSDictionary *v = 
    [NSDictionary dictionaryWithObjectsAndKeys: @"Record2", @"Key", nil];
  p = [NSPredicate predicateWithBlock: ^BOOL(id obj, NSDictionary *bindings)
    {
      NSString *key = [bindings objectForKey: @"Key"];

      if (nil == key)
        {
          key = @"Record1";
        }
      NSString *value = [[obj objectForKey: key] objectForKey: @"Name"];
      return [value isEqualToString: @"John"];
    }];
  PASS([p evaluateWithObject: dict], "BLOCKPREDICATE() without bindings");
  PASS(![p evaluateWithObject: dict 
        substitutionVariables: v], "BLOCKPREDICATE() with bound variables");
  p2 = [p predicateWithSubstitutionVariables: 
    [NSDictionary dictionaryWithObjectsAndKeys: @"Record2", @"Key", nil]];
  PASS(p2 != nil, "BLOCKPREDICATE() instantiated from template");
# ifdef APPLE
  /* The next test is known to be fail on OS X, so mark it as hopeful there. 
   * cf. rdar://25059737
   */
  testHopeful = YES;
# endif 
  PASS(![p2 evaluateWithObject: dict], 
    "BLOCKPREDICATE() with bound variables in separate object");
# ifdef APPLE
  testHopeful = NO;
# endif
#  else
  SKIP("No blocks support in the compiler.");
#  endif
  END_SET("Block predicates");
}
int main()
{
  NSArray *filtered;
  NSArray *pitches;
  NSArray *expect;
  NSArray *a;
  NSMutableDictionary *dict;
  NSPredicate *p;
  NSDictionary *d;

  START_SET("basic")

  dict = [[NSMutableDictionary alloc] init];
  [dict setObject: @"A Title" forKey: @"title"];

  d = [NSDictionary dictionaryWithObjectsAndKeys:
    @"John", @"Name",
    [NSNumber numberWithInt: 34], @"Age",
    [NSArray arrayWithObjects: @"Kid1", @"Kid2", nil], @"Children",
    nil];
  [dict setObject: d forKey: @"Record1"];

  d = [NSDictionary dictionaryWithObjectsAndKeys:
    @"Mary", @"Name",
    [NSNumber numberWithInt: 30], @"Age",
    [NSArray arrayWithObjects: @"Kid1", @"Girl1", nil], @"Children",
    nil];
  [dict setObject: d forKey: @"Record2"];

  testKVC(dict);
  testContains(dict);
  testString(dict);
  testInteger(dict);
  testFloat(dict);
  testAttregate(dict);
  testBlock(dict);
  [dict release];

  pitches = [NSArray arrayWithObjects:
    @"Do", @"Re", @"Mi", @"Fa", @"So", @"La", nil];
  expect = [NSArray arrayWithObjects: @"Do", nil];

  filtered = [pitches filteredArrayUsingPredicate:
    [NSPredicate predicateWithFormat: @"SELF == 'Do'"]];  
  PASS([filtered isEqual: expect], "filter with SELF");

  filtered = [pitches filteredArrayUsingPredicate:
    [NSPredicate predicateWithFormat: @"description == 'Do'"]];
  PASS([filtered isEqual: expect], "filter with description");

  filtered = [pitches filteredArrayUsingPredicate:
    [NSPredicate predicateWithFormat: @"SELF == '%@'", @"Do"]];
  PASS([filtered isEqual: [NSArray array]], "filter with format");

  PASS([NSExpression expressionForEvaluatedObject]
    == [NSExpression expressionForEvaluatedObject],
    "expressionForEvaluatedObject is unique");

  p = [NSPredicate predicateWithFormat: @"SELF == 'aaa'"];
  PASS([p evaluateWithObject: @"aaa"], "SELF equality works");

  d = [NSDictionary dictionaryWithObjectsAndKeys: 
    @"2", @"foo", nil]; 
  p = [NSPredicate predicateWithFormat: @"SELF.foo <= 2"];
  PASS([p evaluateWithObject: d], "SELF.foo <= 2");

  p = [NSPredicate predicateWithFormat:
    @"%K like %@+$b+$c", @"$single", @"b\""];
  PASS_EQUAL([p predicateFormat], @"$single LIKE (\"b\\\"\" + $b) + $c",
    "predicate created with format has the format is preserved");

#if 0
  if ([p respondsToSelector: @selector(subpredicates)])
    NSLog(@"subpredicates=%@", [(NSCompoundPredicate *)p subpredicates]);
  if ([p respondsToSelector: @selector(leftExpression)])
    NSLog(@"left=%@", [(NSComparisonPredicate *)p leftExpression]);
  if ([p respondsToSelector: @selector(rightExpression)])
    NSLog(@"right=%@", [(NSComparisonPredicate *)p rightExpression]);
#endif

  p = [p predicateWithSubstitutionVariables:
    [NSDictionary dictionaryWithObjectsAndKeys:
      @"val_for_single_string", @"single", // why %K does not make a variable
      @"val_for_$b", @"b",
      @"val_for_$c", @"c",
      nil]];
  PASS_EQUAL([p predicateFormat],
    @"$single LIKE (\"b\\\"\" + \"val_for_$b\") + \"val_for_$c\"",
    "Predicate created by substitution has the expected format");

  a = [NSArray arrayWithObjects:
    [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInt: 1], @"a", nil],
    [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInt: 2], @"a", nil],
    nil];
  p = [NSPredicate predicateWithFormat: @"sum(a) == 3"]; 
  PASS([p evaluateWithObject: a], "aggregate sum works");

  END_SET("basic")

  return 0;
}
