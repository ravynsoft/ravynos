#import <Foundation/NSSet.h>
#import "ObjectTesting.h"

int main()
{
  START_SET("mutable set general")

  id    val1, val2, val3, val4, obj;
  NSMutableSet *vals1, *vals2, *vals3, *vals4;

  val1 = @"Hello";
  val2 = @"A Goodbye";
  val3 = @"Testing all strings";
  val4 = @"Another";
  
  vals1 = [NSMutableSet setWithObject: val1];
  vals2 = [vals1 setByAddingObject: val2];
  vals3 = [vals1 setByAddingObject: val3];
  vals4 = [NSMutableSet setWithObject: val4];
  
  obj = [NSMutableSet set];
  PASS([obj isKindOfClass: [NSMutableSet class]] && [obj count] == 0,
    "-count returns zero for an empty set");
  PASS([obj hash] == 0, "-hash returns zero for an empty set");
  PASS(YES == [vals2 containsObject: val2], "-containsObject: works");
  PASS(nil != [vals2 member: @"A Goodbye"], "-member: finds present object");
  PASS(nil == [vals2 member: @"not there"], "-member: doesn't find missing");
  [obj unionSet: vals1];
  PASS_EQUAL(obj, vals1, "union of empty set with non-empty equals non-empty");
  PASS(1 == [obj count], "union contains one object");
  [obj unionSet: vals2];
  PASS(2 == [obj count], "union adds another object to set");

  PASS(NO == [obj intersectsSet: [NSSet set]], "no intersection with empty");
  PASS(YES == [obj intersectsSet: vals3], "test for intersection");
  PASS(NO == [obj intersectsSet: vals4], "test for non-intersection");

  PASS(NO == [obj isSubsetOfSet: [NSSet set]], "not subset of empty");
  PASS(NO == [obj isSubsetOfSet: vals1], "test non subset");
  PASS(YES == [vals1 isSubsetOfSet: obj], "test subset");

  [obj intersectSet: vals3];
  PASS_EQUAL([obj anyObject], val1, "intersection removes an object");

  [obj intersectSet: [NSSet set]];
  PASS(0 == [obj count], "intersect with empty set empties receiver");

  [obj addObject: val1];
  [obj intersectSet: nil];
  PASS(0 == [obj count], "intersect with nil empties receiver");

  END_SET("mutable set general")

  return 0;
}
