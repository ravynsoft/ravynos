#import "O2PDFBlock.h"
#import <Foundation/NSArray.h>

@implementation O2PDFBlock

+pdfBlock {
   return [[[O2PDFBlock alloc] init] autorelease];
}

-init {
   _objects=[[NSMutableArray alloc] init];
   return self;
}

-(void)dealloc {
   [_objects release];
   [super dealloc];
}

-(O2PDFObjectType)objectType {
   return kO2PDFObjectTypeBlock;
}

-(NSArray *)objects {
   return _objects;
}

-(void)addObject:object {
   [_objects addObject:object];
}

-(NSString *)description {
   return [NSString stringWithFormat:@"<%@ %x> { %@ }",isa,self,_objects];
}

@end
