/* Copyright (c) 2008 Dan Knapp

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <Foundation/Foundation.h>

@class NSFetchRequest;

@interface NSManagedObjectModel : NSObject {
    NSMutableDictionary *_entities;
    NSMutableDictionary *_fetchRequestTemplates;
    NSSet *_versionIdentifiers;
    NSDictionary *_localizationDictionary;
    NSMutableDictionary *_configurations;
}

+ (NSManagedObjectModel *)modelByMergingModels:(NSArray *)models;

+ (NSManagedObjectModel *)mergedModelFromBundles:(NSArray *)bundles;

- initWithContentsOfURL:(NSURL *)url;

- (NSArray *)entities;
- (NSDictionary *)entitiesByName;
- (NSDictionary *)localizationDictionary;

- (void)setEntities:(NSArray *)entities;
- (void)setLocalizationDictionary:(NSDictionary *)dictionary;

- (NSArray *)configurations;
- (NSArray *)entitiesForConfiguration:(NSString *)configuration;
- (void)setEntities:(NSArray *)entities forConfiguration:(NSString *)configuration;

- (NSFetchRequest *)fetchRequestTemplateForName:(NSString *)name;
- (NSFetchRequest *)fetchRequestFromTemplateWithName:(NSString *)name substitutionVariables:(NSDictionary *)variables;

- (void)setFetchRequestTemplate:(NSFetchRequest *)fetchRequest forName:(NSString *)name;

@end
