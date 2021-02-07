#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id val1,val2,val3,obj;
  NSArray *arr,*vals1,*vals2,*vals3;

  val1 = @"Hello";
  val2 = @"A Goodbye";
  val3 = @"Testing all strings";
  vals1 = [[[NSArray arrayWithObject:val1] arrayByAddingObject:val2] retain];
  vals2 = [[vals1 arrayByAddingObject:val2] retain];
  vals3 = [[vals1 arrayByAddingObject:val3] retain];

  obj = [NSArray new];
  arr = obj;
  PASS(obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 0,
       "-count returns zero for an empty array");
  PASS([arr hash] == 0, "-hash returns zero for an empty array");
  PASS(vals3 != nil && [vals3 containsObject:val2], "-containsObject works");
  PASS(vals3 != nil && [vals3 indexOfObject:@"A Goodbye"] == 1,
       "-indexOfObject: finds object");
  PASS(vals3 != nil && [vals3 indexOfObjectIdenticalTo:val2],
       "-indexOfObjectIdenticalTo: finds identical object");
  {
    NSEnumerator *e;
    id v1, v2, v3;
    e = [arr objectEnumerator];
    v1 = [e nextObject];
    v2 = [e nextObject];
    PASS(e != nil && v1 == nil && v2 == nil,
         "-objectEnumerator: is ok for empty array");
    e = [vals1 objectEnumerator];
    v1 = [e nextObject];
    v2 = [e nextObject];
    v3 = [e nextObject];
    PASS(v1 != nil && v2 != nil && v3 == nil && [vals1 containsObject:v1] &&
         [vals1 containsObject:v2] && [v1 isEqual:val1] && [v2 isEqual: val2],
	 "-objectEnumerator: enumerates the array");
  }
  {
    obj = [arr description];
    obj = [obj propertyList];
    PASS(obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 0,
         "-description gives us a text property-list (empty array)");
    obj = [arr description];
    obj = [obj propertyList];
    PASS(obj != nil && [obj isKindOfClass:[NSArray class]] && [obj isEqual:arr],
         "-description gives us a text property-list");
  }
  PASS(vals1 != nil && [vals1 isKindOfClass: [NSArray class]] &&
      [vals1 count] == 2, "-count returns two for an array with two objects");
  PASS([vals1 hash] == 2, "-hash returns two for an array with two objects");
  PASS([vals1 indexOfObject:nil] == NSNotFound,
       "-indexOfObject: gives NSNotFound for a nil object");
  PASS([vals1 indexOfObject:val3] == NSNotFound,
       "-indexOfObject: gives NSNotFound for a object not in the array");
  PASS([vals1 isEqualToArray:vals1],
       "Array is equal to itself using -isEqualToArray:");
  PASS(![vals1 isEqualToArray:vals2],"Similar arrays are not equal using -isEqualToArray:");

  {
    NSArray *a;
    NSRange r = NSMakeRange(0,2);
    a = [vals2 subarrayWithRange:r];
    PASS(a != nil && [a isKindOfClass:[NSArray class]] && [a count] == 2 &&
         [a objectAtIndex:0] == val1 && [a objectAtIndex:1] == val2,
	 "-subarrayWithRange: seems ok");
    r = NSMakeRange(1,2);
    PASS_EXCEPTION([arr subarrayWithRange:r];,@"NSRangeException","-subarrayWithRange with invalid range");
  }

  {
    NSString *c = @"/";
    NSString *s = @"Hello/A Goodbye";
    NSString *a = [vals1 componentsJoinedByString: c];
    PASS(a != nil && [a isKindOfClass:[NSString class]] && [a isEqual:s],
         "-componentsJoinedByString: seems ok");
  }
  {
    NSArray *a = [vals1 sortedArrayUsingSelector:@selector(compare:)];
    PASS(a != nil && [a isKindOfClass:[NSArray class]] && [a count] == 2 &&
         [a objectAtIndex:0] == val2 && [a objectAtIndex:1] == val1,
	 "-sortedArrayUsingSelector: seems ok");

  }

  {
    NSMutableArray *unsorted = [NSMutableArray arrayWithContentsOfFile: @"random.plist"];
    NSArray *expected = [NSArray arrayWithContentsOfFile: @"sorted.plist"];
    [unsorted sortUsingSelector: @selector(compare:)];
    PASS_EQUAL(unsorted, expected, "Sorting larger arrays works.");

  }

  [arp release]; arp = nil;
  return 0;
}
