#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSData.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *key1, *key2, *key3, *val1, *val2, *val3;
  NSArray *keys1, *keys2, *keys3, *vals1, *vals2, *vals3;
  id obj;
  NSMutableDictionary *dict;

  key1 = @"Key1";
  key2 = @"Key2";
  key3 = @"Key3";
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  keys1 = [NSArray arrayWithObjects:key1,key2,nil];
  keys2 = [NSArray arrayWithObjects:key1,key2,key3,nil];
  keys3 = [NSArray arrayWithObjects:key1,key2,key2,nil]; /* duplicate keys */
  vals1 = [NSArray arrayWithObjects:val1,val2,nil];
  vals2 = [NSArray arrayWithObjects:val1,val2,val2,nil]; /* duplicate vals */
  vals3 = [NSArray arrayWithObjects:val1,val2,val3,nil];
   
  
  dict = [NSMutableDictionary new];
  PASS(dict != nil && 
       [dict isKindOfClass:[NSMutableDictionary class]] 
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
  
  dict = [[NSMutableDictionary dictionaryWithObjects:vals1 forKeys:keys1] retain];
  PASS(dict != nil && 
       [dict isKindOfClass:[NSMutableDictionary class]] && 
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
 
  dict = [NSMutableDictionary new];
  [dict setObject:@"hello" forKey:@"world"];
  PASS(dict != nil &&
       [dict isKindOfClass:[NSMutableDictionary class]] &&
       [[dict objectForKey:@"world"] isEqual:@"hello"],
       "-setObject:forKey: is ok");

  [dict setValue:@"hello" forKey:@"Lücke"];
  PASS([[dict valueForKey:@"Lücke"] isEqualToString:@"hello"],
      "unicode keys work with setValue:forKey:");

  [arp release]; arp = nil;
  return 0;
}
