#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

int main()
{
  START_SET("NSUbiquitousKeyValueStore base");

  NSUbiquitousKeyValueStore *kvStore = [NSUbiquitousKeyValueStore defaultStore];
  [kvStore setObject:@"Hello" forKey:@"World"];
  id obj = [kvStore objectForKey:@"World"];
  PASS([obj isEqualToString:@"Hello"], "Returned proper value");
  
  [kvStore setString:@"Hello" forKey:@"World2"];
  obj = [kvStore objectForKey:@"World2"];
  PASS([obj isEqualToString:@"Hello"], "Returned proper value");
  
  [kvStore setArray: [NSArray arrayWithObject:@"Hello"] forKey:@"World3"];
  obj = [kvStore arrayForKey:@"World3"];
  PASS([obj isEqual:[NSArray arrayWithObject:@"Hello"] ], "Returned proper value");
  
  [kvStore setDictionary:[NSDictionary dictionaryWithObject:@"Hello" forKey:@"World4"] forKey:@"World5"];
  obj = [kvStore dictionaryForKey:@"World5"];
  PASS([obj isEqual:[NSDictionary dictionaryWithObject:@"Hello" forKey:@"World4"]], "Returned proper value");
  
  [kvStore setData:[NSData dataWithBytes:"hello" length:5] forKey:@"World6"];
  obj = [kvStore dataForKey:@"World6"];
  PASS([obj isEqual:[NSData dataWithBytes:"hello" length:5]], "Returned proper value");
  
  [kvStore setDictionary:[NSDictionary dictionaryWithObject:@"Hello" forKey:@"World4"] forKey:@"World5"];
  obj = [kvStore objectForKey:@"World5"];
  PASS([obj isEqual:[NSDictionary dictionaryWithObject:@"Hello" forKey:@"World4"]], "Returned proper value");
 
  END_SET("NSUbiquitousKeyValueStore base");
  return 0;
}
