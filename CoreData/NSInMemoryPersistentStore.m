#import "NSInMemoryPersistentStore.h"
#import <CoreData/NSPersistentStoreCoordinator.h>

@implementation NSInMemoryPersistentStore

+(NSDictionary *)metadataForPersistentStoreWithURL:(NSURL *)url error:(NSError **)error {
   return nil;
}

-(NSString *)type {
   return NSInMemoryStoreType;
}

-(BOOL)load:(NSError **)errorp {
   return YES;
}

-(BOOL)save:(NSError **)error {
   return YES;
}

@end
