#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSEnumerator.h>

#if     defined(GNUSTEP_BASE_LIBRARY)
#import <Foundation/NSSerialization.h>
#endif

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *key1, *key2, *key3, *val1, *val2, *val3;
  NSArray *keys1, *keys2, *keys3, *keys4, *vals1, *vals2, *vals3, *vals4;
  id obj;
  NSDictionary *dict;
  id vals1Array[2] = { nil, nil };
  id keys1Array[2] = { nil, nil };
  key1 = @"Key1";
  key2 = @"Key2";
  key3 = @"Key3";
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  keys1 = [NSArray arrayWithObjects:key1,key2,nil];
  keys2 = [NSArray arrayWithObjects:key1,key2,key3,nil];
  keys3 = [NSArray arrayWithObjects:key1,key2,key2,nil]; /* duplicate keys */
  keys4 = [NSArray arrayWithObjects:key1,key2,nil];
  vals1 = [NSArray arrayWithObjects:val1,val2,nil];
  vals2 = [NSArray arrayWithObjects:val1,val2,val2,nil]; /* duplicate vals */
  vals3 = [NSArray arrayWithObjects:val1,val2,val3,nil];
  vals4 = [NSArray arrayWithObjects:val1, val2, val3, [NSDate date],
                                    [NSNumber numberWithInt:2],
				    [NSData data], nil];
  keys4 = [NSArray arrayWithObjects:key1, key2, key2, @"date", @"number",
                                    @"data",nil];


  dict = [NSDictionary new];
  PASS(dict != nil &&
       [dict isKindOfClass:[NSDictionary class]]
       && [dict count] == 0,
       "-count returns zero for an empty dictionary");

  PASS([dict hash] == 0, "-hash returns zero for an empty dictionary");
  obj = [dict allKeys];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSArray class]] &&
       [obj count] == 0,
       "-allKeys gives an empty array for an empty dictionary");

  obj = [dict allKeysForObject:nil];
  PASS(obj == nil, "-allKeysForObject: gives nil for an empty dictionary");

  obj = [dict allValues];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSArray class]] &&
       [obj count] == 0,
       "-allValues gives an empty array for an empty dictionary");
  {
    id o1;
    id o2;

    o1 = [dict objectForKey:nil];
    o2 = [dict objectForKey:key1];
    PASS(o1 == nil && o2 == nil,
         "-objectForKey: gives nil for an empty dictionary");
  }

  {
    NSEnumerator *e = [dict keyEnumerator];
    id k1,k2;

    k1 = [e nextObject];
    k2 = [e nextObject];
    PASS(e != nil && k1 == nil && k2 == nil,
         "-keyEnumerator: is ok for empty dictionary");
  }

  {
    NSEnumerator *e = [dict objectEnumerator];
    id v1,v2;

    v1 = [e nextObject];
    v2 = [e nextObject];
    PASS(e != nil && v1 == nil && v2 == nil,
         "-objectEnumerator: is ok for empty dictionary");
  }

  {
    NSString *notFound = @"notFound";
    NSArray *a = [dict objectsForKeys:keys1 notFoundMarker:notFound];
    PASS(a != nil &&
         [a isKindOfClass:[NSArray class]] &&
	 [a count] == 2 &&
	 [a objectAtIndex:0] == notFound &&
	 [a objectAtIndex:1] == notFound,
	 "-objectsForKeys:notFoundMarker: is ok for empty dictionary");
  }

  obj = [dict description];
  obj = [obj propertyList];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSDictionary class]] &&
       [obj count] == 0,
       "-description gives us a text property-list");

  dict = [[NSDictionary dictionaryWithObjects:vals1 forKeys:keys1] retain];
  PASS(dict != nil &&
       [dict isKindOfClass:[NSDictionary class]] &&
       [dict count] == 2,
       "-count returns two for an dictionary with two keys");

  PASS([dict hash] == 2, "-hash returns two for a dictionary with two keys");

  obj = [dict allKeys];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSArray class]] &&
       [obj count] == 2 &&
       [obj containsObject:key1] &&
       [obj containsObject:key1],
       "-allKeys gives the keys we put in the dictionary");

  {
    NSArray *o1,*o2;
    o1 = [dict allKeysForObject:val1];
    o2 = [dict allKeysForObject:val2];
    PASS(o1 != nil &&
         [o1 isKindOfClass:[NSArray class]] &&
	 [o1 count] == 1 &&
	 [o1 containsObject:key1] &&
         o2 != nil &&
         [o2 isKindOfClass:[NSArray class]] &&
	 [o2 count] == 1 &&
	 [o2 containsObject:key2],
	 "-allKeysForObject: gives the key we expect");
  }
  obj = [dict allValues];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSArray class]] &&
       [obj count] == 2 &&
       [obj containsObject:val1] &&
       [obj containsObject:val2],
       "-allValues gives the values we put in the dictionary");

  PASS([dict objectForKey:nil] == nil,"-objectForKey: gives nil for a nil key");
  PASS([dict objectForKey:key3] == nil,
       "-objectForKey: gives nil for a key not in the dictionary");

  {
    id o1 = [dict objectForKey: key1];
    id o2 = [dict objectForKey: key2];
    PASS(o1 == val1 && o2 == val2,
         "-objectForKey: gives the objects we added for the keys");
  }

  {
    NSEnumerator *e = [dict keyEnumerator];
    id k1,k2,k3;
    k1 = [e nextObject];
    k2 = [e nextObject];
    k3 = [e nextObject];
    PASS(k1 != nil &&
         k2 != nil &&
	 k3 == nil &&
	 k1 != k2 &&
	 [keys1 containsObject:k1] &&
	 [keys1 containsObject:k2],
         "-keyEnumerator: enumerates the dictionary");
  }

  {
    NSEnumerator *e = [dict objectEnumerator];
    id v1,v2,v3;
    v1 = [e nextObject];
    v2 = [e nextObject];
    v3 = [e nextObject];

    PASS(v1 != nil &&
         v2 != nil &&
	 v3 == nil &&
	 v1 != v2 &&
	 [vals1 containsObject:v1] &&
	 [vals1 containsObject:v2],
         "-objectEnumerator: enumerates the dictionary");
  }

  {
    NSString *notFound = @"notFound";
    NSArray *a = [dict objectsForKeys:keys2 notFoundMarker:notFound];

    PASS(a != nil &&
         [a isKindOfClass:[NSArray class]] &&
	 [a count] == 3 &&
	 [a objectAtIndex:0] == val1 &&
	 [a objectAtIndex:1] == val2 &&
	 [a objectAtIndex:2] == notFound,
	 "-objectsForKeys:notFoundMarker: is ok for dictionary");
  }

  {
    NSArray *a = [dict keysSortedByValueUsingSelector:@selector(compare:)];
    PASS(a != nil &&
         [a isKindOfClass:[NSArray class]] &&
         [a count] == 2 &&
	 [a objectAtIndex:0] == key2 &&
	 [a objectAtIndex:1] == key1,
	 "-keysSortedByValueUsingSelector: seems ok");
  }

  obj = [dict description];
  obj = [obj propertyList];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSDictionary class]] &&
       [obj isEqual:dict],
       "-description gives us a text property-list");

  [dict getObjects: &vals1Array andKeys: &keys1Array];
  uint8_t found = 0;
  if (vals1Array[0] == val1 || vals1Array[1] == val1)
    {
      found |= 1;
    }
  if (vals1Array[0] == val2 || vals1Array[1] == val2)
    {
      found |= 1 << 1;
    }
  if (keys1Array[0] == key1 || keys1Array[1] == key1)
    {
      found |= 1 << 2;
    }
  if (keys1Array[0] == key2 || keys1Array[1] == key2)
    {
      found |= 1 << 3;
    }
  PASS(found == 0b1111, "-getObjects:andKeys: returns correct objects");
  PASS_RUNS([dict getObjects: NULL andKeys: &keys1Array],
    "-getObjects:andKeys: can ignore objects");
  PASS_RUNS([dict getObjects: &vals1Array andKeys: NULL],
    "-getObjects:andKeys: can ignore keys");


  ASSIGN(dict,[NSDictionary dictionaryWithObjects:vals4 forKeys:keys4]);
  PASS(dict != nil, "we can create a dictionary with several keys");

#if     defined(GNUSTEP_BASE_LIBRARY)
  obj = [NSSerializer serializePropertyList:dict];
  obj = [NSDeserializer deserializePropertyListFromData:obj
                                      mutableContainers:YES];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSDictionary class]] &&
       [obj isEqual:dict],
       "data/number/data are ok in serialized property-list");
#endif

  NSArray	*a = [NSArray array];
  NSDictionary	*d = [NSDictionary dictionaryWithObjectsAndKeys:
    @"val", a, @"val2", @"key2", nil];
  PASS_EQUAL([d objectForKey: a], @"val", "array as dictionary key works")

  [arp release]; arp = nil;
  return 0;
}
