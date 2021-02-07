/** <title>GSXibKeyedUnarchiver.m</title>

 <abstract>
 These are templates for use with OSX XIB files.  These classes are the
 templates and other things which are needed for reading XIB files.
 </abstract>

 Copyright (C) 2010, 2012, 2017 Free Software Foundation, Inc.

 File created by Marcian Lytwyn on 12/30/16 from original code by:

 Author: Fred Kiefer <FredKiefer@gmx.de>
 Date: March 2010

 Author: Gregory John Casamento
 Date: 2012

 This file is part of the GNUstep GUI Library.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; see the file COPYING.LIB.
 If not, see <http://www.gnu.org/licenses/> or write to the
 Free Software Foundation, 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#import <Foundation/NSXMLDocument.h>
#import "GNUstepGUI/GSXibKeyedUnarchiver.h"
#import "GNUstepGUI/GSXibElement.h"
#import "GNUstepGUI/GSNibLoading.h"
#import "GSXib5KeyedUnarchiver.h"

@implementation GSXibKeyedUnarchiver

+ (BOOL) checkXib5: (NSData *)data
{
#if GNUSTEP_BASE_HAVE_LIBXML
  // Ensure we have a XIB 5 version...first see if we can parse the XML...
  NSXMLDocument *document = [[NSXMLDocument alloc] initWithData: data
                                                        options: 0
                                                          error: NULL];
  if (document == nil)
    {
      return NO;
    }
  else
    {
      // Test to see if this is an Xcode 5 XIB...
      NSArray *documentNodes = [document nodesForXPath: @"/document" error: NULL];

      // Need at LEAST ONE document node...we should find something a bit more
      // specific to check here...
      return [documentNodes count] != 0;
    }
#else
  // We now default to checking XIB 5 versions
  return YES;
#endif
}

+ (NSKeyedUnarchiver *) unarchiverForReadingWithData: (NSData *)data
{
  NSKeyedUnarchiver *unarchiver = nil;

  if ([self checkXib5: data])
    {
      unarchiver = [[GSXib5KeyedUnarchiver alloc] initForReadingWithData: data];
    }
  else
    {
      unarchiver = [[GSXibKeyedUnarchiver alloc] initForReadingWithData: data];
    }
  return AUTORELEASE(unarchiver);
}

- (NSData *) _preProcessXib: (NSData *)data
{
  NSData *result = data;

#if     GNUSTEP_BASE_HAVE_LIBXML
  NSXMLDocument *document = [[NSXMLDocument alloc] initWithData:data
							options:0
							  error:NULL];

  if (document == nil)
    {
      NSLog(@"%s:DOCUMENT IS NIL: %@\n", __PRETTY_FUNCTION__, document);
    }
  else
    {
      NSArray *customClassNodes = [document nodesForXPath:@"//dictionary[@key=\"flattenedProperties\"]/"
                                            @"string[contains(@key,\"CustomClassName\")]"
                                                    error:NULL];
      NSMutableDictionary *customClassDict = [NSMutableDictionary dictionary];
      if (customClassNodes)
        {
          NSDebugLLog(@"PREXIB", @"%s:customClassNodes: %@\n", __PRETTY_FUNCTION__, customClassNodes);

          // Replace the NSXMLNodes with a dictionary...
          NSInteger index = 0;
          for (index = 0; index < [customClassNodes count]; ++index)
            {
              id node = [customClassNodes objectAtIndex:index];
              if ([node isMemberOfClass:[NSXMLElement class]])
                {
                  NSString     *key  = [[node attributeForName:@"key"] stringValue];
                  if ([key rangeOfString:@"CustomClassName"].location != NSNotFound)
                    {
                      [customClassDict setObject:[node stringValue] forKey:key];
                    }
                }
            }
        }
      else
        {
          NSArray *flatProps = [document nodesForXPath:@"//object[@key=\"flattenedProperties\"]" error:NULL];
          if ([flatProps count] == 1)
            {
              NSInteger index = 0;
              NSArray *xmlKeys = [[flatProps objectAtIndex:0] nodesForXPath:
                @"//object[@key=\"flattenedProperties\"]/object[@key=\"dict.sortedKeys\"]/*" error:NULL];
              NSArray *xmlObjs = [[flatProps objectAtIndex:0] nodesForXPath:
                @"//object[@key=\"flattenedProperties\"]/object[@key=\"dict.values\"]/*" error:NULL];
              if ([xmlKeys count] != [xmlObjs count])
                {
                  NSLog(@"%s:keys to objs count mismatch - keys: %d objs: %d\n", __PRETTY_FUNCTION__,
                        (int)[xmlKeys count], (int)[xmlObjs count]);
                }
              else
                {
                  for (index = 0; index < [xmlKeys count]; ++index)
                    {
                      id key = [[xmlKeys objectAtIndex:index] stringValue];
                      if ([key rangeOfString:@"CustomClassName"].location != NSNotFound)
                        {
                          // NSString *obj = [[xmlObjs objectAtIndex:index] stringValue];
                          [customClassDict setObject:[[xmlObjs objectAtIndex:index] stringValue] forKey:key];
                        }
                    }
                }
            }
        }

      NSDebugLLog(@"PREXIB", @"%s:customClassDict: %@\n", __PRETTY_FUNCTION__, customClassDict);

      if ([customClassDict count] > 0)
        {
          NSArray *objectRecords = nil;
          NSEnumerator *en = [[customClassDict allKeys] objectEnumerator];
          NSString *key = nil;

          while ((key = [en nextObject]) != nil)
            {
              NSString *keyValue = [key stringByReplacingOccurrencesOfString:@".CustomClassName" withString:@""];
              NSString *className = [customClassDict objectForKey: key];
              NSString *objectRecordXpath = nil;

              objectRecordXpath = [NSString stringWithFormat: @"//object[@class=\"IBObjectRecord\"]/"
                                            @"int[@key=\"objectID\"][text()=\"%@\"]/../reference",
                                            keyValue];

              objectRecords = [document nodesForXPath: objectRecordXpath error: NULL];

              if (objectRecords == nil)
                {
                  // If that didn't work then it could be a 4.6+ XIB...
                  objectRecordXpath = [NSString stringWithFormat: @"//object[@class=\"IBObjectRecord\"]/"
                                                @"string[@key=\"id\"][text()=\"%@\"]/../reference",
                                                keyValue];
                  objectRecords = [document nodesForXPath: objectRecordXpath error: NULL];
                }

              NSString *refId = nil;
              if ([objectRecords count] > 0)
                {
                  id record = nil;
                  NSEnumerator *oen = [objectRecords objectEnumerator];
                  while ((record = [oen nextObject]) != nil)
                    {
                      if ([record isMemberOfClass:[NSXMLElement class]])
                        {
                          if([[[record attributeForName:@"key"] stringValue] isEqualToString:@"object"])
                            {
                              NSArray *classNodes = nil;
                              id classNode = nil;
                              NSString *refXpath = nil;

                              refId = [[record attributeForName:@"ref"] stringValue];
                              refXpath = [NSString stringWithFormat:@"//object[@id=\"%@\"]",refId];
                              classNodes = [document nodesForXPath:refXpath
                                                             error:NULL];
                              if([classNodes count] > 0)
                                {
                                  id classAttr = nil;
                                  Class cls = NSClassFromString(className);

                                  classNode = [classNodes objectAtIndex:0];
                                  classAttr = [classNode attributeForName:@"class"];
                                  [classAttr setStringValue:className];

                                  if (cls != nil)
                                    {
                                      if ([cls respondsToSelector:@selector(cellClass)])
                                        {
                                          NSArray *cellNodes = nil;
                                          id cellNode = nil;
                                          id cellClass = [cls cellClass];
                                          NSString *cellXpath = [NSString stringWithFormat:
                                            @"//object[@id=\"%@\"]/object[@key=\"NSCell\"]",refId];
                                          cellNodes = [document nodesForXPath:cellXpath
                                                                        error:NULL];
                                          if ([cellNodes count] > 0)
                                            {
                                              NSString *cellClassString = NSStringFromClass(cellClass);
                                              id cellAttr = nil;
                                              cellNode = [cellNodes objectAtIndex:0];
                                              cellAttr = [cellNode attributeForName:@"class"];
                                              [cellAttr setStringValue:cellClassString];
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

      result = [document XMLData];
      RELEASE(document);
    }
#endif
  return result;
}

- (void) _initCommon
{
  objects = [[NSMutableDictionary alloc] init];
  stack = [[NSMutableArray alloc] init];
  decoded = [[NSMutableDictionary alloc] init];
}

- (id) initForReadingWithData: (NSData*)data
{
#if     GNUSTEP_BASE_HAVE_LIBXML
  NSXMLParser *theParser;
  NSData *theData = data;

  // If we are in the interface builder app, do not replace
  // the existing classes with their custom subclasses.
  if ([NSClassSwapper isInInterfaceBuilder] == NO)
    {
      theData = [self _preProcessXib: data];
    }

  if (theData == nil)
    {
      return nil;
    }

  // Initialize...
  [self _initCommon];

  theParser = [[NSXMLParser alloc] initWithData: theData];
  [theParser setDelegate: self];

  NS_DURING
    {
      // Parse the XML data
      [theParser parse];
    }
  NS_HANDLER
    {
      NSLog(@"Exception occurred while parsing Xib: %@",[localException reason]);
      DESTROY(self);
    }
  NS_ENDHANDLER

  DESTROY(theParser);
#endif
  return self;
}

- (void) dealloc
{
  DESTROY(objects);
  DESTROY(stack);
  DESTROY(decoded);

  [super dealloc];
}

- (void) parser: (NSXMLParser*)parser
foundCharacters: (NSString*)string
{
  [currentElement setValue: string];
}

- (void) parser: (NSXMLParser*)parser
didStartElement: (NSString*)elementName
   namespaceURI: (NSString*)namespaceURI
  qualifiedName: (NSString*)qualifiedName
     attributes: (NSDictionary*)attributeDict
{
  GSXibElement *element = [[GSXibElement alloc] initWithType: elementName
                                           andAttributes: attributeDict];
  NSString *key = [attributeDict objectForKey: @"key"];
  NSString *ref = [attributeDict objectForKey: @"id"];

  // FIXME: We should use proper memory management here
  AUTORELEASE(element);

  if (key != nil)
    {
      [currentElement setElement: element forKey: key];
    }
  else
    {
      // For Arrays
      [currentElement addElement: element];
    }
  if (ref != nil)
    {
      [objects setObject: element forKey: ref];
    }

  if (![@"archive" isEqualToString: elementName] &&
      ![@"data" isEqualToString: elementName])
    {
      // only used for the root element
      // push
      [stack addObject: currentElement];
    }

  if (![@"archive" isEqualToString: elementName])
    {
      currentElement = element;
    }
}

- (void) parser: (NSXMLParser*)parser
  didEndElement: (NSString*)elementName
   namespaceURI: (NSString*)namespaceURI
  qualifiedName: (NSString*)qName
{
  if (![@"archive" isEqualToString: elementName] &&
      ![@"data" isEqualToString: elementName])
    {
      // pop
      currentElement = [stack lastObject];
      [stack removeLastObject];
    }
}

- (id) allocObjectForClassName: (NSString*)classname
{
  Class c = nil;
  id delegate = [self delegate];

  c = [self classForClassName: classname];

  if (c == nil)
    {
      c = [[self class] classForClassName: classname];
      if (c == nil)
        {
          c = NSClassFromString(classname);
          if (c == nil)
            {
              c = [delegate unarchiver: self
                            cannotDecodeObjectOfClassName: classname
                       originalClasses: nil];
              if (c == nil)
                {
                  [NSException raise: NSInvalidUnarchiveOperationException
                              format: @"[%@ -%@]: no class for name '%@'",
                               NSStringFromClass([self class]),
                               NSStringFromSelector(_cmd),
                               classname];
                }
            }
        }
    }

  // Create instance.
  return [c allocWithZone: [self zone]];
 }

- (BOOL) replaceObject: (id)oldObj withObject: (id)newObj
{
  NSEnumerator *keyEnumerator = [decoded keyEnumerator];
  id key;
  BOOL found = NO;

  while ((key = [keyEnumerator nextObject]) != nil)
    {
      id obj = [decoded objectForKey: key];
      if (obj == oldObj)
        {
          found = YES;
          break;
        }
    }

  if (found)
    {
      [decoded setObject: newObj forKey: key];
    }

  return found;
}

- (id) decodeObjectForXib: (GSXibElement*)element
             forClassName: (NSString*)classname
                   withID: (NSString*)objID
{
  GSXibElement *last;
  id o, r;
  id delegate = [self delegate];

  // Create instance.
  o = [self allocObjectForClassName: classname];
  // Make sure the object stays around, even when replaced.
  RETAIN(o);
  if (objID != nil)
    [decoded setObject: o forKey: objID];

  // push
  last = currentElement;
  currentElement = element;

  r = [o initWithCoder: self];

  // pop
  currentElement = last;

  if (r != o)
    {
      [delegate unarchiver: self
         willReplaceObject: o
                withObject: r];
      ASSIGN(o, r);
      if (objID != nil)
        [decoded setObject: o forKey: objID];
    }

  r = [o awakeAfterUsingCoder: self];
  if (r != o)
    {
      [delegate unarchiver: self
         willReplaceObject: o
                withObject: r];
      ASSIGN(o, r);
      if (objID != nil)
        [decoded setObject: o forKey: objID];
    }

  if (delegate != nil)
    {
      r = [delegate unarchiver: self didDecodeObject: o];
      if (r != o)
        {
          [delegate unarchiver: self
             willReplaceObject: o
                    withObject: r];
          ASSIGN(o, r);
          if (objID != nil)
            [decoded setObject: o forKey: objID];
        }
    }

  // Balance the retain above
  RELEASE(o);

  if (objID != nil)
    {
      NSDebugLLog(@"XIB", @"decoded object %@ for id %@", o, objID);
    }

  return AUTORELEASE(o);
}

/*
  This method is a copy of decodeObjectForXib:forClassName:withKey:
  The only difference being in the way we decode the object and the
  missing context switch.
 */
- (id) decodeDictionaryForXib: (GSXibElement*)element
                 forClassName: (NSString*)classname
                       withID: (NSString*)objID
{
  id o, r;
  id delegate = [self delegate];

  // Create instance.
  o = [self allocObjectForClassName: classname];
  // Make sure the object stays around, even when replaced.
  RETAIN(o);
  if (objID != nil)
    [decoded setObject: o forKey: objID];

  r = [o initWithDictionary: [self _decodeDictionaryOfObjectsForElement: element]];
  if (r != o)
    {
      [delegate unarchiver: self
         willReplaceObject: o
                withObject: r];
      ASSIGN(o, r);
      if (objID != nil)
        [decoded setObject: o forKey: objID];
    }

  r = [o awakeAfterUsingCoder: self];
  if (r != o)
    {
      [delegate unarchiver: self
         willReplaceObject: o
                withObject: r];
      ASSIGN(o, r);
      if (objID != nil)
        [decoded setObject: o forKey: objID];
    }

  if (delegate != nil)
    {
      r = [delegate unarchiver: self didDecodeObject: o];
      if (r != o)
        {
          [delegate unarchiver: self
             willReplaceObject: o
                    withObject: r];
          ASSIGN(o, r);
          if (objID != nil)
            [decoded setObject: o forKey: objID];
        }
    }
  // Balance the retain above
  RELEASE(o);

  if (objID != nil)
    {
      NSDebugLLog(@"XIB", @"decoded object %@ for id %@", o, objID);
    }

  return AUTORELEASE(o);
}

- (id) objectForXib: (GSXibElement*)element
{
  NSString *elementName;
  NSString *objID;

  if (element == nil)
    return nil;

  NSDebugLLog(@"XIB", @"decoding element %@", element);
  objID = [element attributeForKey: @"id"];
  if (objID)
    {
      id new = [decoded objectForKey: objID];
      if (new != nil)
        {
          // The object was already decoded as a reference
          return new;
        }
    }

  elementName = [element type];
  if ([@"object" isEqualToString: elementName])
    {
      NSString *classname = [element attributeForKey: @"class"];
      return [self decodeObjectForXib: element
                         forClassName: classname
                               withID: objID];
    }
  else if ([@"string" isEqualToString: elementName])
    {
      id new = [element value];

      // Handle newer format as well
      if ([[element attributeForKey: @"type"] isEqualToString: @"base64-UTF8"] ||
          [[element attributeForKey: @"base64-UTF8"] boolValue])
        {
          NSData *d = [[NSData alloc] initWithBase64EncodedString: new
                                                          options: NSDataBase64DecodingIgnoreUnknownCharacters];
          new = AUTORELEASE([[NSString alloc] initWithData: d
                                                  encoding: NSUTF8StringEncoding]);
          RELEASE(d);
        }

      // empty strings are not nil!
      if (new == nil)
        new = @"";

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"int" isEqualToString: elementName])
    {
      id new = [NSNumber numberWithInt: [[element value] intValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"double" isEqualToString: elementName])
    {
      id new = [NSNumber numberWithDouble: [[element value] doubleValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"bool" isEqualToString: elementName])
    {
      id new = [NSNumber numberWithBool: [[element value] boolValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"integer" isEqualToString: elementName])
    {
      NSString *value = [element attributeForKey: @"value"];
      id new = [NSNumber numberWithInteger: [value integerValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"real" isEqualToString: elementName])
    {
      NSString *value = [element attributeForKey: @"value"];
      id new = [NSNumber numberWithFloat: [value floatValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"boolean" isEqualToString: elementName])
    {
      NSString *value = [element attributeForKey: @"value"];
      id new = [NSNumber numberWithBool: [value boolValue]];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"point" isEqualToString: elementName])
    {
      NSPoint point = [self decodePointForKey: [element attributeForKey: @"key"]];
      id      new   = [NSValue valueWithPoint: point];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"size" isEqualToString: elementName])
    {
      NSSize size = [self decodeSizeForKey: [element attributeForKey: @"key"]];
      id     new  = [NSValue valueWithSize: size];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"rect" isEqualToString: elementName])
    {
      NSRect rect = [self decodeRectForKey: [element attributeForKey: @"key"]];
      id     new  = [NSValue valueWithRect: rect];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"reference" isEqualToString: elementName])
    {
      NSString *ref = [element attributeForKey: @"ref"];

      if (ref == nil)
        {
          return nil;
        }
      else
        {
          id new = [decoded objectForKey: ref];

          // FIXME: We need a marker for nil
          if (new == nil)
            {
              //NSLog(@"Decoding reference %@", ref);
              element = [objects objectForKey: ref];
              if (element != nil)
                {
                  // Decode the real object
                  new = [self objectForXib: element];
                }
            }

          return new;
        }
    }
  else if ([@"nil" isEqualToString: elementName])
    {
      return nil;
    }
  else if ([@"characters" isEqualToString: elementName])
    {
      id new = [element value];

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"bytes" isEqualToString: elementName])
    {
      id new = AUTORELEASE([[NSData alloc] initWithBase64EncodedString: [element value]
                                                               options: NSDataBase64DecodingIgnoreUnknownCharacters]);

      if (objID != nil)
        [decoded setObject: new forKey: objID];

      return new;
    }
  else if ([@"array" isEqualToString: elementName])
    {
      NSString *classname = [element attributeForKey: @"class"];

      if (classname == nil)
        {
          classname = @"NSArray";
        }
      return [self decodeObjectForXib: element
                         forClassName: classname
                               withID: objID];
    }
  else if ([@"dictionary" isEqualToString: elementName])
    {
      NSString *classname = [element attributeForKey: @"class"];

      if (classname == nil)
        {
          classname = @"NSDictionary";
        }

      return [self decodeDictionaryForXib: element
                             forClassName: classname
                                   withID: objID];
    }
  else
    {
      //NSLog(@"Unknown element type %@", elementName);
    }

  return nil;
}

- (id) _decodeArrayOfObjectsForKey: (NSString*)aKey
{
  // FIXME: This is wrong but the only way to keep the code for
  // [NSArray-initWithCoder:] working
  return [self _decodeArrayOfObjectsForElement: currentElement];
}

- (id) _decodeArrayOfObjectsForElement: (GSXibElement*)element
{
  NSArray *values = [element values];
  int max = [values count];
  id list[max];
  int i;

  for (i = 0; i < max; i++)
    {
      list[i] = [self objectForXib: [values objectAtIndex: i]];
      if (list[i] == nil)
        NSLog(@"No object for %@ at index %d", [values objectAtIndex: i], i);
    }

  return [NSArray arrayWithObjects: list count: max];
}

- (id) _decodeDictionaryOfObjectsForElement: (GSXibElement*)element
{
  NSDictionary *elements = [element elements];
  NSEnumerator *en;
  NSString *key;
  NSMutableDictionary *dict;

  dict = [[NSMutableDictionary alloc] init];
  en = [elements keyEnumerator];
  while ((key = [en nextObject]) != nil)
    {
      id obj = [self objectForXib: [elements objectForKey: key]];
      if (obj == nil)
        NSLog(@"No object for %@ at key %@", [elements objectForKey: key], key);
      else
        [dict setObject: obj forKey: key];
    }

  return AUTORELEASE(dict);
}

/*
  Extension method to decode the object id of an object referenced by its key.
 */
- (NSString *) decodeReferenceForKey: (NSString*)aKey
{
  GSXibElement *element = [currentElement elementForKey: aKey];
  NSString *objID;

  if (element == nil)
    return nil;

  objID = [element attributeForKey: @"id"];
  if (objID)
    {
      return objID;
    }

  objID = [element attributeForKey: @"ref"];
  if (objID)
    {
      return objID;
    }

  return nil;
}

- (BOOL) containsValueForKey: (NSString*)aKey
{
  GSXibElement *element = [currentElement elementForKey: aKey];

  return (element != nil);
}

- (id) decodeObjectForKey: (NSString*)aKey
{
  GSXibElement *element = [currentElement elementForKey: aKey];

  if (element == nil)
    return nil;

  return [self objectForXib: element];
}

- (BOOL) decodeBoolForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  return [o boolValue];
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return NO;
}

- (const uint8_t*) decodeBytesForKey: (NSString*)aKey
		      returnedLength: (NSUInteger*)length
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if ([o isKindOfClass: [NSData class]] == YES)
	{
	  *length = [o length];
	  return [o bytes];
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  *length = 0;
  return 0;
}

- (double) decodeDoubleForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  return [o doubleValue];
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return 0.0;
}

- (float) decodeFloatForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  return [o floatValue];
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return 0.0;
}

- (int) decodeIntForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  long long l = [o longLongValue];

	  return l;
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return 0;
}

- (int32_t) decodeInt32ForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  long long l = [o longLongValue];

	  return l;
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return 0;
}

- (int64_t) decodeInt64ForKey: (NSString*)aKey
{
  id o = [self decodeObjectForKey: aKey];

  if (o != nil)
    {
      if (([o isKindOfClass: [NSNumber class]] == YES) ||
          ([o isKindOfClass: [NSString class]] == YES))
	{
	  long long l = [o longLongValue];

	  return l;
	}
      else
	{
	  [NSException raise: NSInvalidUnarchiveOperationException
		      format: @"[%@ +%@]: value for key(%@) is '%@'",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    aKey, o];
	}
    }
  return 0;
}

@end
