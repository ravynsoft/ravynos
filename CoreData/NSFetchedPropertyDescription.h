#import <CoreData/NSPropertyDescription.h>

@class NSFetchRequest;

@interface NSFetchedPropertyDescription : NSPropertyDescription {
    NSFetchRequest *_fetchRequest;
}

- (NSFetchRequest *)fetchRequest;

- (void)setFetchRequest:(NSFetchRequest *)value;

@end
