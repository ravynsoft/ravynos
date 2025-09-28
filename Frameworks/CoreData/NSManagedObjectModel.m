/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <CoreData/NSManagedObjectModel.h>
#import <CoreData/NSEntityDescription.h>

#import <Foundation/NSKeyedUnarchiver.h>
#import <Foundation/NSRaise.h>

@implementation NSManagedObjectModel

+(NSManagedObjectModel *)modelByMergingModels:(NSArray *)models {
   NSManagedObjectModel *result=[[NSManagedObjectModel alloc] init];
   NSMutableArray       *entities=[NSMutableArray array];
   
   for(NSManagedObjectModel *merge in models){
    [entities addObjectsFromArray:[merge entities]];
   }
   
   [result setEntities:entities];
      
   return result;
}

+(NSManagedObjectModel *)mergedModelFromBundles:(NSArray *)bundles {
   NSMutableArray *models=[NSMutableArray array];
   
   if(bundles==nil)
    bundles=[NSArray arrayWithObject:[NSBundle mainBundle]];
  
   for(NSBundle *bundle in bundles){
    NSArray *moms=[bundle pathsForResourcesOfType:@"mom" inDirectory:nil];
        
    for(NSString *path in moms){
     NSURL                *url=[NSURL fileURLWithPath:path];
     NSManagedObjectModel *model=[[NSManagedObjectModel alloc] initWithContentsOfURL:url];
     
     if(model==nil)
      NSLog(@"-[%@ initWithContentsOfURL:] failed. url=%@",self,url);
     
     if(model!=nil)
      [models addObject:model];
    }
    
   }
   
   return [self modelByMergingModels:models];
}

-init {
   _entities=[[NSMutableDictionary alloc] init];
   _fetchRequestTemplates=[[NSMutableDictionary alloc] init];
   _configurations=[[NSMutableDictionary alloc] init];
   return self;
}

-initWithCoder: (NSCoder *) coder {
   if(![coder allowsKeyedCoding])
    [NSException raise:NSInvalidArgumentException format: @"%@ can not initWithCoder:%@", isa, [coder class]];

   _entities=[[coder decodeObjectForKey: @"NSEntities"] retain];
   for(NSEntityDescription *entity in [_entities allValues])
    [_entities setObject:entity forKey:[[entity name] uppercaseString]];
    
   _fetchRequestTemplates=[[coder decodeObjectForKey: @"NSFetchRequestTemplates"] retain];
   _versionIdentifiers=[[coder decodeObjectForKey: @"NSVersionIdentifiers"] retain];
   
   return self;
}

-initWithContentsOfURL:(NSURL *)url {
   [self dealloc];
   
   NSData *data=[[NSData alloc] initWithContentsOfURL:url];
   
   if(data==nil)
    return nil;

   NSKeyedUnarchiver *unarchiver=[[NSKeyedUnarchiver alloc] initForReadingWithData: data];
   NSManagedObjectModel *result=[unarchiver decodeObjectForKey: @"root"];
   
   [unarchiver release];
   [data release];
   
   return result;
}

-(NSArray *)entities {
   return [_entities allValues];
}

-(NSDictionary *)entitiesByName {
   return _entities;
}

-(NSDictionary *) localizationDictionary {
   return _localizationDictionary;
}

-(void)setEntities: (NSArray *)entities {
   [_entities removeAllObjects];
   
   for(NSEntityDescription *entity in entities){
    [_entities setObject:entity forKey:[entity name]];
    [_entities setObject:entity forKey:[[entity name] uppercaseString]];
   }
}

-(void)setLocalizationDictionary:(NSDictionary *)dictionary {
   dictionary=[dictionary copy];
   [_localizationDictionary release];
   _localizationDictionary=dictionary;
}

-(NSArray *)configurations {
   return [_configurations allKeys];
}

-(NSArray *)entitiesForConfiguration: (NSString *) configuration {
   return [_configurations objectForKey:configuration];
}

-(void)setEntities:(NSArray *)entities forConfiguration:(NSString *)configuration {
   [_configurations setObject:entities forKey:configuration];
}

-(NSFetchRequest *)fetchRequestTemplateForName: (NSString *) name {
   return [_fetchRequestTemplates objectForKey:name];
}

-(NSFetchRequest *)fetchRequestFromTemplateWithName:(NSString *)name substitutionVariables:(NSDictionary *)variables {
    NSUnimplementedMethod();
}

-(void)setFetchRequestTemplate: (NSFetchRequest *) fetchRequest forName: (NSString *) name {
   [_fetchRequestTemplates setObject:fetchRequest forKey:name];
}

@end
