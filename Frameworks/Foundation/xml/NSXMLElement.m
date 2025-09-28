/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Foundation/NSXMLElement.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSRaise.h>

@implementation NSXMLElement

-initWithName:(NSString *)name {
   [super initWithKind:NSXMLElementKind options:NSXMLNodeOptionsNone];
   _name=[name copy];
   _attributes=[[NSMutableDictionary alloc] init];
   _namespaces=[[NSMutableDictionary alloc] init];
   return self;
}

-initWithName:(NSString *)name stringValue:(NSString *)string {
   [self initWithName:name];
   _value=[string copy];
   return self;
}

-initWithName:(NSString *)name URI:(NSString *)uri {
   NSUnimplementedMethod();
   return 0;
}

-initWithXMLString:(NSString *)xml error:(NSError **)error {
   NSUnimplementedMethod();
   return 0;
}

-copyWithZone:(NSZone *)zone {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)attributes {
   return [_attributes allValues];
}

-(NSXMLNode *)attributeForLocalName:(NSString *)name URI:(NSString *)uri {
   NSUnimplementedMethod();
   return 0;
}

-(NSXMLNode *)attributeForName:(NSString *)name {
   return [_attributes objectForKey:name];
}

-(NSArray *)elementsForLocalName:(NSString *)localName URI:(NSString *)uri {
   NSUnimplementedMethod();
   return 0;
}

-(NSArray *)elementsForName:(NSString *)name {
   NSMutableArray *result=[NSMutableArray array];
   
   for(NSXMLNode *node in _children)
    if([[node name] isEqualToString:name])
     [result addObject:node];
     
   return result;
}

-(NSArray *)namespaces {
   NSUnimplementedMethod();
   return 0;
}

-(NSXMLNode *)namespaceForPrefix:(NSString *)prefix {
   NSUnimplementedMethod();
   return 0;
}

-(void)setAttributes:(NSArray *)attributes {
   NSInteger i,count=[attributes count];
   
   [_attributes removeAllObjects];
   
   for(i=0;i<count;i++){
    NSXMLNode *add=[attributes objectAtIndex:i];
    
    [_attributes setObject:add forKey:[add name]];
   }
}

-(void)setAttributesAsDictionary:(NSDictionary *)attributes {
   NSEnumerator *state=[attributes keyEnumerator];
   NSString     *name;
   
   [_attributes removeAllObjects];
   
   while((name=[state nextObject])!=nil){
    NSString  *value=[attributes objectForKey:name];
    NSXMLNode *node=[NSXMLNode attributeWithName:name stringValue:value];
    
    [_attributes setObject:node forKey:name];
   }
}

-(void)setChildren:(NSArray *)children {
   [_children setArray:children];
}

-(void)setNamespaces:(NSArray *)namespaces {
   [_namespaces removeAllObjects];
}

-(void)addChild:(NSXMLNode *)node {
   node->_parent=self;
   [_children addObject:node];
}

-(void)insertChild:(NSXMLNode *)child atIndex:(NSUInteger)index {
   [_children insertObject:child atIndex:index];
}

-(void)insertChildren:(NSArray *)children atIndex:(NSUInteger)index {
   NSInteger i,count=[children count];
   
   for(i=0;i<count;i++)
    [_children insertObject:[children objectAtIndex:i] atIndex:index+i];
}

-(void)removeChildAtIndex:(NSUInteger)index {
   [_children removeObjectAtIndex:index];
}

-(void)replaceChildAtIndex:(NSUInteger)index withNode:(NSXMLNode *)node {
   [_children replaceObjectAtIndex:index withObject:node];
}

-(void)addAttribute:(NSXMLNode *)attribute {
   [_attributes setObject:attribute forKey:[attribute name]];
}

-(void)removeAttributeForName:(NSString *)name {
   [_attributes removeObjectForKey:name];
}

-(void)addNamespace:(NSXMLNode *)namespace {
   [_namespaces setObject:namespace forKey:[namespace prefix]];
}

-(void)removeNamespaceForPrefix:(NSString *)prefix {
   [_namespaces removeObjectForKey:prefix];
}

-(void)resolveNamespaceForName:(NSString *)name {
   NSUnimplementedMethod();
}

-(void)resolvePrefixForNamespaceURI:(NSString *)uri {
   NSUnimplementedMethod();
}

-(void)normalizeAdjacentTextNodesPreservingCDATA:(BOOL)preserve {
   NSUnimplementedMethod();
}

static void appendStringWithCharacterEntities(NSMutableString *result,NSString *string){
   NSUInteger i,length=[string length],location=0;
   unichar    buffer[length];
   
   [string getCharacters:buffer];
   
   for(i=0;i<length;i++){
    unichar   code=buffer[i];
    NSString *entity=nil;
    
    if(code=='<')
     entity=@"&lt;";
    else if(code=='&')
     entity=@"&amp;";
    else if(code=='>')
     entity=@"&gt;";
    else if(code=='\"')
     entity=@"&quot;";
    else if(code=='\'')
     entity=@"&apos;";
    
    if(entity!=nil){
     if(i-location>0){
      NSString *chunk=[[NSString alloc] initWithCharacters:buffer+location length:i-location];
      [result appendString:chunk];
      [chunk release];
     }
     
     [result appendString:entity];
     location=i+1;
    }
    
   }
   
   if(location==0)
    [result appendString:string];
   else if(i-location>0){
    NSString *chunk=[[NSString alloc] initWithCharacters:buffer+location length:i-location];
    [result appendString:chunk];
    [chunk release];
   }
}

-(NSString *)XMLStringWithOptions:(NSUInteger)options {
   NSMutableString *result=[NSMutableString string];
   
   [result appendString:@"<"];
   [result appendString:[self name]];
   
   for(NSXMLNode *attribute in [self attributes]){
    [result appendString:@" "];
    [result appendString:[attribute XMLStringWithOptions:options]];
   }
   
   [result appendString:@">"];
   
   for(NSXMLNode *element in [self children]){
    [result appendString:@" "];
    [result appendString:[element XMLStringWithOptions:options]];
   }
   
   appendStringWithCharacterEntities(result,[self stringValue]);

   [result appendString:@"</"];
   [result appendString:[self name]];
   [result appendString:@">\n"];
   
   return result;
}

@end
