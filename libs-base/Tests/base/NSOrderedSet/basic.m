#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSOrderedSet.h>

static NSString *stringData = @"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
@"<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">"
@"<plist version=\"1.0\">"
@"<dict>"
@"	<key>$archiver</key>"
@"	<string>NSKeyedArchiver</string>"
@"	<key>$objects</key>"
@"	<array>"
@"		<string>$null</string>"
@"		<dict>"
@"			<key>$class</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>14</integer>"
@"			</dict>"
@"			<key>NS.object.0</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>2</integer>"
@"			</dict>"
@"			<key>NS.object.1</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>3</integer>"
@"			</dict>"
@"			<key>NS.object.10</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>12</integer>"
@"			</dict>"
@"			<key>NS.object.11</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>13</integer>"
@"			</dict>"
@"			<key>NS.object.2</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>4</integer>"
@"			</dict>"
@"			<key>NS.object.3</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>5</integer>"
@"			</dict>"
@"			<key>NS.object.4</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>6</integer>"
@"			</dict>"
@"			<key>NS.object.5</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>7</integer>"
@"			</dict>"
@"			<key>NS.object.6</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>8</integer>"
@"			</dict>"
@"			<key>NS.object.7</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>9</integer>"
@"			</dict>"
@"			<key>NS.object.8</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>10</integer>"
@"			</dict>"
@"			<key>NS.object.9</key>"
@"			<dict>"
@"				<key>CF$UID</key>"
@"				<integer>11</integer>"
@"			</dict>"
@"		</dict>"
@"		<string>Now</string>"
@"		<string>is</string>"
@"		<string>the</string>"
@"		<string>time</string>"
@"		<string>for</string>"
@"		<string>all</string>"
@"		<string>Horrible</string>"
@"		<string>men</string>"
@"		<string>to</string>"
@"		<string>Flee From</string>"
@"		<string>the aid</string>"
@"		<string>of their country</string>"
@"		<dict>"
@"			<key>$classes</key>"
@"			<array>"
@"				<string>NSMutableOrderedSet</string>"
@"				<string>NSOrderedSet</string>"
@"				<string>NSObject</string>"
@"			</array>"
@"			<key>$classname</key>"
@"			<string>NSMutableOrderedSet</string>"
@"		</dict>"
@"	</array>"
@"	<key>$top</key>"
@"	<dict>"
@"		<key>root</key>"
@"		<dict>"
@"			<key>CF$UID</key>"
@"			<integer>1</integer>"
@"		</dict>"
@"	</dict>"
@"	<key>$version</key>"
@"	<integer>100000</integer>"
@"</dict>"
@"</plist>";

int main()
{
  START_SET("NSOrderedSet base")

  NSOrderedSet *testObj, *testObj2;
  NSMutableOrderedSet *mutableTest1, *mutableTest2;
  NSMutableArray *testObjs = [NSMutableArray new];
  NSData *data = [stringData dataUsingEncoding: NSUTF8StringEncoding];
  NSMutableSet *testSet;

  testObj = [NSOrderedSet new];
  [testObjs addObject: testObj];
  PASS(testObj != nil && [testObj count] == 0,
	   "can create an empty ordered set");

  testObj = [NSOrderedSet orderedSetWithObject: @"Hello"];
  [testObjs addObject: testObj];
  PASS(testObj != nil && [testObj count] == 1,
	   "can create an ordered set with one element");

  id objs[] = {@"Hello", @"Hello1"};
  testObj = [NSOrderedSet orderedSetWithObjects: objs count: 2];
  [testObjs addObject: testObj];
  PASS(testObj != nil && [testObj count] == 2,
	   "can create an ordered set with multi element");

  id objs1[] = {@"Hello", @"Hello"};
  testObj = [NSOrderedSet orderedSetWithObjects: objs1 count: 2];
  [testObjs addObject: testObj];
  PASS(testObj != nil && [testObj count] == 1,
	   "cannot create an ordered set with multiple like elements");

  NSMutableArray *arr = [NSMutableArray array];
  [arr addObject: @"Hello"];
  [arr addObject: @"World"];
  testObj = [NSOrderedSet orderedSetWithArray: arr];
  [testObjs addObject: testObj];
  PASS(testObj != nil && [testObj count] == 2,
	   "Is able to initialize with array");

  id objs3[] = {@"Hello"};
  id objc4[] = {@"World"};
  testObj  = [NSOrderedSet orderedSetWithObjects: objs3 count: 1];
  [testObjs addObject: testObj];
  testObj2 = [NSOrderedSet orderedSetWithObjects: objc4 count: 1];
  [testObjs addObject: testObj2];
  BOOL result = [testObj intersectsOrderedSet: testObj2];
  PASS(result == NO,
	   "Sets do not intersect!");

  id objs5[] = {@"Hello"};
  id objc6[] = {@"Hello"};
  testObj  = [NSOrderedSet orderedSetWithObjects: objs5 count: 1];
  [testObjs addObject: testObj];
  testObj2 = [NSOrderedSet orderedSetWithObjects: objc6 count: 1];
  [testObjs addObject: testObj2];
  BOOL result1 = [testObj intersectsOrderedSet: testObj2];
  PASS(result1 == YES,
	   "Sets do intersect!");

  id o1 = @"Hello";
  id o2 = @"World";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o1];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o2];
  [testObjs addObject: mutableTest2];
  [mutableTest1 unionOrderedSet: mutableTest2];
  PASS(mutableTest1 != nil && mutableTest2 != nil && [mutableTest1 count] == 2,
	   "mutableSets union properly");

  id o3 = @"Hello";
  id o4 = @"World";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o3];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o4];
  [testObjs addObject: mutableTest2];
  [mutableTest1 intersectOrderedSet: mutableTest2];
  PASS(mutableTest1 != nil && mutableTest2 != nil && [mutableTest1 count] == 0,
	   "mutableSets do not intersect");

  id o5 = @"Hello";
  id o6 = @"Hello";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o5];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o6];
  [testObjs addObject: mutableTest2];
  [mutableTest1 intersectOrderedSet: mutableTest2];
  PASS(mutableTest1 != nil && mutableTest2 != nil && [mutableTest1 count] == 1,
	   "mutableSets do intersect");

  id o7 = @"Hello";
  id o8 = @"World";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o7];
  [mutableTest1 addObject: o8];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o7];
  [testObjs addObject: mutableTest2];
  BOOL isSubset = [mutableTest2 isSubsetOfOrderedSet: mutableTest1];
  PASS(isSubset,
       "mutableTest2 is subset of mutableTest1");

  testSet = [NSMutableSet set];
  [testSet addObject: o7];
  [testSet addObject: o8];
  isSubset = [mutableTest2 isSubsetOfSet: testSet];
  PASS(isSubset,
       "mutableTest2 is subset of testSet");

  id o9 = @"Hello";
  id o10 = @"World";
  id o11 = @"Ready";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o9];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o10];
  [mutableTest2 addObject: o9];
  [testObjs addObject: mutableTest2];
  isSubset = [mutableTest2 isSubsetOfOrderedSet: mutableTest1];
  PASS(isSubset == NO,
       "mutableTest2 is not subset of mutableTest1");

  testSet = [NSMutableSet set];
  [testSet addObject: o9];
  isSubset = [mutableTest2 isSubsetOfSet: testSet];
  PASS(isSubset == NO,
       "mutableTest2 is not subset of testSet");

  o9 = @"Hello";
  o10 = @"World";
  o11 = @"Ready";
  id o12 = @"ToGo";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o9];
  [mutableTest1 addObject: o10];
  [mutableTest1 addObject: o12];
  [mutableTest1 addObject: o11];
  [testObjs addObject: mutableTest1];
  mutableTest2 = [NSMutableOrderedSet orderedSet];
  [mutableTest2 addObject: o9];
  [mutableTest2 addObject: o10];
  [testObjs addObject: mutableTest2];
  isSubset = [mutableTest2 isSubsetOfOrderedSet: mutableTest1];
  PASS(isSubset,
       "mutableTest2 is subset of mutableTest1");

  testSet = [NSMutableSet set];
  [testSet addObject: o9];
  [testSet addObject: o10];
  [testSet addObject: o12];
  [testSet addObject: o11];
  isSubset = [mutableTest2 isSubsetOfSet: testSet];
  PASS(isSubset,
       "mutableTest2 is subset of testSet");

  o9 = @"Hello";
  o10 = @"World";
  o11 = @"Ready";
  o12 = @"ToGo";
  mutableTest1 = [NSMutableOrderedSet orderedSet];
  [mutableTest1 addObject: o9];
  [mutableTest1 addObject: o10];
  [mutableTest1 addObject: o12];
  [mutableTest1 addObject: o11];
  [testObjs addObject: mutableTest1];
  PASS([mutableTest1 isEqual: mutableTest1],
       "mutableTest1 is equal to itself");

  o9 = @"Hello";
  o10 = @"World";
  o11 = @"Ready";
  o12 = @"ToGo";
  NSMutableOrderedSet *mutableTest3 = [NSMutableOrderedSet orderedSet];
  [mutableTest3 addObject: o9];
  [mutableTest3 addObject: o10];
  [mutableTest3 addObject: o12];
  [mutableTest3 addObject: o11];
  [mutableTest3 insertObject: @"Hello" atIndex: 2];
  [testObjs addObject: mutableTest3];
  PASS([mutableTest3 isEqual: mutableTest1] == YES,
       "Insert at index does not replace existing object");

  NSMutableOrderedSet *mutableTest4 = [NSMutableOrderedSet orderedSet];
  [mutableTest4 addObject: @"Now"];
  [mutableTest4 addObject: @"is"];
  [mutableTest4 addObject: @"the"];
  [mutableTest4 addObject: @"time"];
  [mutableTest4 addObject: @"for"];
  [mutableTest4 addObject: @"all"];
  [mutableTest4 addObject: @"Good"];
  [mutableTest4 addObject: @"men"];
  [mutableTest4 addObject: @"to"];
  [mutableTest4 addObject: @"come"];
  [mutableTest4 addObject: @"to the aid"];
  [mutableTest4 addObject: @"of their country"];
  [mutableTest4 moveObjectsAtIndexes: [NSIndexSet indexSetWithIndex: 3] toIndex: 10];
  [testObjs addObject: mutableTest4];
  PASS([[mutableTest4 objectAtIndex: 10] isEqual: @"time"] == YES,
       "Move to index moves to correct index");

  NSMutableOrderedSet *mutableTest5 = [NSMutableOrderedSet orderedSet];
  [mutableTest5 addObject: @"Now"];
  [mutableTest5 addObject: @"is"];
  [mutableTest5 addObject: @"the"];
  [mutableTest5 exchangeObjectAtIndex: 0 withObjectAtIndex: 2];
  [testObjs addObject: mutableTest5];
  PASS([[mutableTest5 objectAtIndex: 0] isEqual: @"the"] == YES &&
       [[mutableTest5 objectAtIndex: 2] isEqual: @"Now"] == YES,
       "Exchanges indexes properly");
  //NSLog(@"RESULT: %@",mutableTest4);

  mutableTest4 = [NSMutableOrderedSet orderedSet];
  [mutableTest4 addObject: @"Now"];
  [mutableTest4 addObject: @"is"];
  [mutableTest4 addObject: @"the"];
  [mutableTest4 addObject: @"time"];
  [mutableTest4 addObject: @"for"];
  [mutableTest4 addObject: @"all"];
  [mutableTest4 addObject: @"Good"];
  [mutableTest4 addObject: @"men"];
  [mutableTest4 addObject: @"to"];
  [mutableTest4 addObject: @"come to"];
  [mutableTest4 addObject: @"the aid"];
  [mutableTest4 addObject: @"of their country"];
  NSMutableIndexSet *is = [NSMutableIndexSet indexSetWithIndex: 6];
  [is addIndex: 9];
  NSMutableArray *array = [NSMutableArray arrayWithObjects: @"Horrible", @"Flee From", nil];
  [mutableTest4 replaceObjectsAtIndexes: is
			    withObjects: array];
  [testObjs addObject: mutableTest4];
  PASS([[mutableTest4 objectAtIndex: 9] isEqual: @"Flee From"] == YES,
       "replaceObjectsAtIndexes: adds to correct indexes");

  id uobj = [NSKeyedUnarchiver unarchiveObjectWithData: data];
  PASS((uobj != nil &&
	[uobj isKindOfClass: [NSMutableOrderedSet class]] &&
	[uobj containsObject: @"Now"]),
       "Object unarchives correctly from macOS archive")

  test_NSObject(@"NSOrderedSet", testObjs);
  test_NSCoding(testObjs);
  test_NSCopying(@"NSOrderedSet", @"NSMutableOrderedSet", testObjs, YES, NO);
  test_NSMutableCopying(@"NSOrderedSet", @"NSMutableOrderedSet", testObjs);

  END_SET("NSOrderedSet base")
  return 0;
}
