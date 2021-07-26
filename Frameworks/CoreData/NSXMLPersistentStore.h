#import <CoreData/NSAtomicStore.h>

@class XMLDocument, NSMutableDictionary;

@interface NSXMLPersistentStore : NSAtomicStore {
    XMLDocument *_document;
    NSMutableDictionary *_referenceToCacheNode;
    NSMutableDictionary *_referenceToElement;
    NSMutableSet *_usedReferences;
}

@end
