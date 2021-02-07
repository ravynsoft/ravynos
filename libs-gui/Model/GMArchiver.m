/*
   GMArchiver.m

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: October 1997

   Copyright (C) 1997 Free Software Foundation, Inc.
   All rights reserved.

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

/* Portions of the code are based on NSArchiver from libFoundation. See the
   COPYING file from libFoundation for copyright information. */

#ifndef GNUSTEP
#import <Foundation/Foundation.h>
#else
#import <Foundation/NSArchiver.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSHashTable.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSObjCRuntime.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSString.h>
#endif

#ifndef AUTORELEASE
#define AUTORELEASE(object)     [object autorelease]
#define RELEASE(object)         [object release]
#define RETAIN(object)          [object retain]
#endif

#import <GNUstepGUI/GMArchiver.h>

@interface GMClassInfo : NSObject
{
  NSString* className;
  int version;
  BOOL written;
}

+ classInfoWithClassName: (NSString*)className version: (int)version;

- (NSString*) className;
- (int) version;
- (void) setWasWritten: (BOOL)flag;
- (BOOL) wasWritten;

@end


@implementation GMClassInfo

+ classInfoWithClassName: (NSString*)name version: (int)_version
{
  GMClassInfo* object = [[self new] autorelease];

  object->className = [name retain];
  object->version = _version;

  return object;
}

- (NSString*) className			{ return className; }
- (int) version				{ return version; }
- (void) setWasWritten: (BOOL)flag	{ written = flag; }
- (BOOL) wasWritten			{ return written; }

@end


@implementation GMArchiver

+ (BOOL) archiveRootObject: (id)rootObject
  toFile: (NSString*)path
{
  GMArchiver* archiver = [[self new] autorelease];
  BOOL result;

  [archiver encodeRootObject: rootObject withName: @"RootObject"];
  result = [archiver writeToFile: path];

  return result;
}

- (id) init
{
  propertyList = [NSMutableDictionary new];
  topLevelObjects = [NSMutableArray new];
  [propertyList setObject: topLevelObjects forKey: @"TopLevelObjects"];
  lastObjectRepresentation = propertyList;

  objects = NSCreateMapTable (NSNonRetainedObjectMapKeyCallBacks,
			      NSObjectMapValueCallBacks, 119);
  conditionals = NSCreateHashTable (NSNonRetainedObjectHashCallBacks, 19);
  classes = NSCreateMapTable (NSObjectMapKeyCallBacks,
			      NSObjectMapValueCallBacks, 19);
  [propertyList setObject: @"1" forKey: @"Version"];

  return self;
}

- (void) dealloc
{
  [propertyList release];
  [topLevelObjects release];
  NSFreeMapTable(objects);
  NSFreeHashTable(conditionals);
  NSFreeMapTable(classes);

  [super dealloc];
}

- (NSString*) nextLabel
{
  return [NSString stringWithFormat: @"Object%5d", ++counter];
}

- (BOOL) writeToFile: (NSString*)path
{
  return [propertyList writeToFile: path atomically: YES];
}

- (id) encodeRootObject: (id)rootObject withName: (NSString*)name
{
  id originalPList = propertyList;
  int oldCounter = counter;
  id label;

  if (writingRoot)
#if 1
    [NSException raise: NSInconsistentArchiveException
      format: @"CoderHasAlreadyWrittenRootObject"];
#else
    THROW([CoderHasAlreadyWrittenRootObjectException new]);
#endif

  writingRoot = YES;

  /*
   * Prepare for writing the graph objects for which `rootObject' is the root
   * node. The algorithm consists from two passes. In the first pass it
   * determines the nodes so-called 'conditionals' - the nodes encoded *only*
   * with -encodeConditionalObject: . They represent nodes that are not
   * related directly to the graph. In the second pass objects are encoded
   * normally, except for the conditional objects which are encoded as nil.
   */

  /* First pass. */
  findingConditionals = YES;
  lastObjectRepresentation = propertyList = nil;
  NSResetHashTable(conditionals);
  NSResetMapTable(objects);
  [self encodeObject: rootObject withName: name];

  /* Second pass. */
  findingConditionals = NO;
  counter = oldCounter;
  lastObjectRepresentation = propertyList = originalPList;
  NSResetMapTable(objects);
  label = [self encodeObject: rootObject withName: name];

  writingRoot = NO;

  return label;
}

- (id) encodeConditionalObject: (id)anObject withName: (NSString*)name
{
#if 0
  if (!writingRoot)
    THROW([RootObjectHasNotBeenWrittenException new]);
#endif

  if (findingConditionals) {
    /*
     * This is the first pass of the determining the conditionals
     * algorithm. We traverse the graph and insert into the `conditionals'
     * set. In the second pass all objects that are still in this set will
     * be encoded as nil when they receive -encodeConditionalObject: . An
     * object is removed from this set when it receives -encodeObject: .
     */
    id value;

    if (!anObject)
      return nil;

    /* Lookup anObject into the `conditionals' set. If it is then the
       object is still a conditional object. */
    value = (id)NSHashGet(conditionals, anObject);
    if (value)
      return value;

    /*
     * Maybe it has received -encodeObject: 
     * and now is in the `objects' set.
     */
    value = (id)NSMapGet(objects, anObject);
    if (value)
      return value;

    /* anObject was not written previously. */
    NSHashInsert(conditionals, anObject);
  }
  else {
    /* If anObject is in the `conditionals' set, it is encoded as nil. */
    if (!anObject || NSHashGet(conditionals, anObject))
      return [self encodeObject: nil withName: name];
    else
      return [self encodeObject: anObject withName: name];
  }

  return nil;
}

- (id) encodeObject: (id)anObject withName: (NSString*)name
{
  if (!anObject) {
    if (!findingConditionals && name)
      [lastObjectRepresentation setObject: @"nil" forKey: name];
    return @"nil";
  }
  else {
    NSString *label;
    id upperObjectRepresentation;

    anObject = [anObject replacementObjectForModelArchiver: self];

    label = NSMapGet(objects, anObject);

    if (findingConditionals && !label) {
      /*
       * Look-up the object in the `conditionals' set. If the object is
       * there, then remove it because it is no longer a conditional one.
       */
      label = NSHashGet(conditionals, anObject);
      if (label) {
	NSHashRemove(conditionals, anObject);
	NSMapInsert(objects, anObject, [self nextLabel]);
	return label;
      }
    }

    if (label == nil) {
      Class archiveClass;

      /* If the object gets encoded on the top level, set the label to be
         `name'. */
      if (!level) {
	if (!name) {
	  NSLog (@"Can't encode top level object with a nil name!");
	  return nil;
	}
	label = name;
      }
      else
	label = [self nextLabel];

      NSMapInsert(objects, anObject, label);

      /* Temporary save the last object into upperObjectRepresentation so we
	 can restore the stack of objects being encoded after anObject is
	 encoded. */
      upperObjectRepresentation = lastObjectRepresentation;

      archiveClass = [anObject classForModelArchiver];

      if (!findingConditionals) {
	NSMutableDictionary* objectPList = [NSMutableDictionary dictionary];

	/* If anObject is the first object in the graph that receives the
	   -encodeObject: withName: message, save its label into the
	   topLevelObjects array. */
	if (!level)
	  [topLevelObjects addObject: (name ? name : label)];

	lastObjectRepresentation = objectPList;

	if (level) {
	  /* Encode 'name = label' in the object's representation and put the
	      description of anObject on the top level like 'label = object'.
	    */
	  if (name)
	    [upperObjectRepresentation setObject: label forKey: name];
	  [propertyList setObject: objectPList forKey: label];
	}
	else {
	  /* The encoded object is on the top level so encode it and put it
	      under the key 'name'. */
	  if (name)
	    label = name;
	  [propertyList setObject: objectPList
			forKey: label];
	}

	[objectPList setObject: NSStringFromClass(archiveClass)
		      forKey: @"isa"];
      }
      else {
	/*
	  * This is the first pass of determining the conditionals
	  * objects algorithm. Remove anObject from the `conditionals'
	  * set if it is there and insert it into the `objects' set.
	  */
	NSHashRemove(conditionals, anObject);
      }

      level++;
      [anObject encodeWithModelArchiver: self];
      level--;

      lastObjectRepresentation = upperObjectRepresentation;
    }
    else if (!findingConditionals) {
      if (name)
	[lastObjectRepresentation setObject: label forKey: name];
    }

    return label;
  }
}

- (id) encodeString: (NSString*)anObject withName: (NSString*)name
{
  if (!findingConditionals) {
    if (!anObject) {
      if (name)
	[lastObjectRepresentation setObject: @"nil" forKey: name];
      return @"nil";
    }
    else {
      if (name)
	[lastObjectRepresentation setObject: anObject forKey: name];
      return anObject;
    }
  }

  return @"nil";
}

- (id) encodeData: (NSData*)anObject withName: (NSString*)name
{
  if (!findingConditionals) {
    if (!anObject) {
      if (name)
	[lastObjectRepresentation setObject: @"nil" forKey: name];
      return @"nil";
    }
    else {
      if (name)
	[lastObjectRepresentation setObject: anObject forKey: name];
      return anObject;
    }
  }

  return @"nil";
}

- (id) encodeArray: (NSArray*)array withName: (NSString*)name
{
  if (array) {
    int i, count = [array count];
    NSMutableArray* description = [NSMutableArray arrayWithCapacity: count];

    for (i = 0; i < count; i++) {
      id object = [array objectAtIndex: i];
      [description addObject: [self encodeObject: object withName: nil]];
    }
    if (name)
      [lastObjectRepresentation setObject: description forKey: name];
    return description;
  }
  else {
    if (name)
      [lastObjectRepresentation setObject: @"nil" forKey: name];
    return @"nil";
  }
}

- (id) encodeDictionary: (NSDictionary*)dictionary withName: (NSString*)name
{
  if (dictionary) {
    id enumerator, key;
    NSMutableDictionary* description
	= [NSMutableDictionary dictionaryWithCapacity: [dictionary count]];

    enumerator = [dictionary keyEnumerator];

    while ((key = [enumerator nextObject])) {
      id value = [dictionary objectForKey: key];
      id keyDesc = [self encodeObject: key withName: nil];
      id valueDesc = [self encodeObject: value withName: nil];

      [description setObject: valueDesc forKey: keyDesc];
    }
    if (name)
      [lastObjectRepresentation setObject: description forKey: name];
    return description;
  }
  else {
    if (name)
      [lastObjectRepresentation setObject: @"nil" forKey: name];
    return @"nil";
  }
}

- (id) propertyList
{
  return propertyList;
}

- (id) encodeClass: (Class)class withName: (NSString*)name
{
  if (class)
    return [self encodeString: NSStringFromClass(class) withName: name];
  else
    return [self encodeString: nil withName: name];
}

- (id) encodeSelector: (SEL)selector withName: (NSString*)name
{
  if (selector)
    return [self encodeString: NSStringFromSelector(selector) withName: name];
  else
    return [self encodeString: nil withName: name];
}

- (void) encodeChar: (char)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%c", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeUnsignedChar: (unsigned char)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%uc", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeBOOL: (BOOL)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%s", value ? "YES" : "NO"];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeShort: (short)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%hd", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeUnsignedShort: (unsigned short)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%us", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeInt: (int)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%i", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeUnsignedInt: (unsigned int)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%u", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeLong: (long)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%ld", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeUnsignedLong: (unsigned long)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%lu", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeFloat: (float)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%f", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeDouble: (double)value withName: (NSString*)name
{
  if (!findingConditionals && name) {
    id valueString = [NSString stringWithFormat: @"%f", value];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodePoint: (NSPoint)point withName: (NSString*)name
{
  if (!findingConditionals && name) {
    /* Macosx NSStringFromPoint is not OPENstep compliant, so we do it by hand. */
    id valueString = [NSString stringWithFormat: @"{x=%f; y=%f}",
			       point.x, point.y];
    
    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeSize: (NSSize)size withName: (NSString*)name
{
  if (!findingConditionals) {
    /* Macosx NSStringFromSize is not OPENstep compliant, so we do it by hand. */
    id valueString = [NSString stringWithFormat: @"{width=%f; height=%f}",
			       size.width, size.height];

    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (void) encodeRect: (NSRect)rect withName: (NSString*)name
{
  if (!findingConditionals) {
    /* Macosx NSStringFromRect is not OPENstep compliant, so we do it by hand. */
    id valueString = [NSString stringWithFormat: 
				 @"{x=%f; y=%f; width=%f; height=%f}",
			       rect.origin.x, rect.origin.y, 
			       rect.size.width, rect.size.height];
    [lastObjectRepresentation setObject: valueString forKey: name];
  }
}

- (NSString*) classNameEncodedForTrueClassName: (NSString*)trueName
{
  NSString *inArchiveName = [(id)NSMapGet(classes, trueName) className];

  return inArchiveName ? inArchiveName : trueName;
}

/* In the following method the version of class named trueName is written as
   version for class named inArchiveName. Is this right? It is possible for
   the inArchiveName class that it could not be linked in the running process
   at the time the archive is written. */
- (void) encodeClassName: (NSString*)trueName
  intoClassName: (NSString*)inArchiveName
{
  id classInfo = [GMClassInfo classInfoWithClassName: inArchiveName
			    version: [NSClassFromString(trueName) version]];

  NSMapInsert(classes, trueName, classInfo);
}

@end /* GMArchiver */


@implementation GMUnarchiver

static NSMutableDictionary* classToAliasMappings = nil;

+ (void) initialize
{
  classToAliasMappings = [NSMutableDictionary new];
}

+ (id) unarchiverWithContentsOfFile: (NSString*)path
{
  id plist = [[NSString stringWithContentsOfFile: path] propertyList];
  GMUnarchiver* unarchiver;

  if (!plist)
    return nil;

  unarchiver = [[[self alloc] initForReadingWithPropertyList: plist]
		    autorelease];
  return unarchiver;
}

+ (id) unarchiveObjectWithName: (NSString*)name
  fromPropertyList: (id)plist
{
  GMUnarchiver* unarchiver
      = [[[self alloc] initForReadingWithPropertyList: plist] autorelease];

  return [unarchiver decodeObjectWithName: name];
}

+ (id) unarchiveObjectWithName: (NSString*)name fromFile: (NSString*)path
{
  GMUnarchiver* unarchiver = [self unarchiverWithContentsOfFile: path];
  return [unarchiver decodeObjectWithName: name];
}

- init
{
  return [self initForReadingWithPropertyList: nil];
}

- (id) initForReadingWithPropertyList: (id)plist
{
  NSString* versionString;

  propertyList = [plist copy];
  currentDecodedObjectRepresentation = propertyList;
  namesToObjects = RETAIN ([NSMutableDictionary dictionaryWithCapacity: 119]);

  /* Decode version information */
  versionString = [propertyList objectForKey: @"Version"];
  [[NSScanner scannerWithString: versionString] scanInt: &version];

  objectZone = NSDefaultMallocZone ();

  return self;
}

- (void) dealloc
{
  RELEASE (propertyList);
  RELEASE (namesToObjects);
  [super dealloc];
}

- (id) decodeObjectWithName: (NSString*)name
{
  id object, label, representation, className;
  id upperObjectRepresentation;
  BOOL objectOnTopLevel = NO;
  id newObject;
  Class class;
  NSString* decodeAsName;

  if (!name)
    return nil;

  if (level) {
    /* First try to see if the object has been already decoded */
    if ((object = [namesToObjects objectForKey: name]))
      return object;
  }

  /* The object has not been decoded yet. Read its label from the current
     object dictionary representation. */
  label = [currentDecodedObjectRepresentation objectForKey: name];

  if (label) {
    /* Try to see if the object has been decoded using `label' as name */
    if ((object = [namesToObjects objectForKey: label]))
      return object;
  }
  else {
    /* Try to find the object on the top level */
    label = [propertyList objectForKey: name];

    if (label)
      objectOnTopLevel = YES;
    else {
      /* There is no object with this name within the current object or on the
         top level. */
#if 0
      NSLog (@"No object named '%@' in object representation '%@'",
	      name, currentDecodedObjectRepresentation);
#endif
      return nil;
    }
  }

  /* If we are on the top level the description is really the representation of
     the object. Otherwise the value is the name of an object on the top level.
   */
  if (currentDecodedObjectRepresentation != propertyList
      && !objectOnTopLevel) {
    NSAssert1 ([label isKindOfClass: [NSString class]],
		@"label is not a string: '%@'!", label);

    /* label is either a name of an object on the top level dictionary or the
	string "nil" which means the object has the nil value. */
    if ([label isEqual: @"nil"])
      return nil;

    representation = [propertyList objectForKey: label];
  }
  else {
    representation = label;
    label = name;
  }

  if (!representation) {
    /* There is no object with such a label on the top level dictionary */
    NSLog (@"No object object named '%@' on the top level dictionary! (error "
	   @"within object representation '%@')",
	   label, currentDecodedObjectRepresentation);
    return nil;
  }

  /* Temporary save the current object representation */
  upperObjectRepresentation = currentDecodedObjectRepresentation;
  currentDecodedObjectRepresentation = representation;

  /* Create the object */
  className = [representation objectForKey: @"isa"];
  decodeAsName = [classToAliasMappings objectForKey: className];
  if ( decodeAsName )
  {
#if GNU_GUI_LIBRARY
    NSDebugLLog(@"GMArchiver", @"%@ to be decoded as %@", className, decodeAsName);
#endif
    className = decodeAsName;
  }
  class = NSClassFromString(className);
  object = [class createObjectForModelUnarchiver: self];

  if (!class) {
    NSLog (@"Class %@ not linked into application!", className);
    return nil;
  }

  /* Push it into the dictionary of known objects */
  [namesToObjects setObject: object forKey: label];

  /* Read it from dictionary */
  level++;
  newObject = [object initWithModelUnarchiver: self];
  level--;

  if (newObject != object) {
    object = newObject;
    [namesToObjects setObject: object forKey: label];
  }

  /* Restore the current object representation */
  currentDecodedObjectRepresentation = upperObjectRepresentation;

  return object;
}

- (NSString*) decodeStringWithName: (NSString*)name
{
  id string;

  if (!name)
    return nil;

  string = [currentDecodedObjectRepresentation objectForKey: name];
  if (!string) {
#if 0
    NSLog (@"Couldn't find the string value for key '%@' (object '%@')",
	    name, currentDecodedObjectRepresentation);
#endif
    return nil;
  }

  if (![string isKindOfClass: [NSString class]]) {
    NSLog (@"Decoded object is not a string: '%@'! (key '%@', object '%@')",
	   string, name, currentDecodedObjectRepresentation);
    return nil;
  }

  if ([string isEqualToString: @"nil"])
    return nil;

  return string;
}

- (NSData*) decodeDataWithName: (NSString*)name
{
  id data;

  if (!name)
    return nil;

  data = [currentDecodedObjectRepresentation objectForKey: name];
  if (!data) {
    NSLog (@"Couldn't find the data value for key '%@' (object '%@')",
	    name, currentDecodedObjectRepresentation);
    return nil;
  }

  if (![data isKindOfClass: [NSData class]]) {
    NSLog (@"Decoded object is not a data: '%@'! (key '%@', object '%@')",
	   data, name, currentDecodedObjectRepresentation);
    return nil;
  }

  return data;
}

- (NSArray*) decodeArrayWithName: (NSString*)name
{
  id array, decodedArray;
  int i, count;

  if (!name)
    return nil;

  array = [currentDecodedObjectRepresentation objectForKey: name];
  if (!array) {
    NSLog (@"Couldn't find the array value for key '%@' (object '%@')",
	    name, currentDecodedObjectRepresentation);
    return nil;
  }

  if (![array isKindOfClass: [NSArray class]]) {
    NSLog (@"Decoded object is not an array: '%@'! (key '%@', object '%@')",
	   array, name, currentDecodedObjectRepresentation);
    return nil;
  }

  count = [array count];
  decodedArray = [NSMutableArray arrayWithCapacity: count];
  for (i = 0; i < count; i++) {
    id label = [array objectAtIndex: i];
    id objectDescription = [propertyList objectForKey: label];

    if (!objectDescription) {
      NSLog (@"warning: couldn't find the description for object labeled '%@' "
	     @"in the array description '%@ = %@'!", label, name, array);
      continue;
    }

    [decodedArray addObject: [self decodeObjectWithName: label]];
  }

  return decodedArray;
}

- (NSDictionary*) decodeDictionaryWithName: (NSString*)name
{
  id dictionary, decodedDictionary;
  id enumerator, keyLabel, valueLabel;

  if (!name)
    return nil;

  dictionary = [currentDecodedObjectRepresentation objectForKey: name];
  if (!dictionary) {
    NSLog (@"Couldn't find the dictionary value for key '%@' (object '%@')",
	    name, currentDecodedObjectRepresentation);
    return nil;
  }

  if (![dictionary isKindOfClass: [NSDictionary class]]) {
    NSLog (@"Decoded object is not a dictionary: '%@'! (key '%@', object '%@')",
	   dictionary, name, currentDecodedObjectRepresentation);
    return nil;
  }

  decodedDictionary
      = [NSMutableDictionary dictionaryWithCapacity: [dictionary count]];
  enumerator = [dictionary keyEnumerator];
  while ((keyLabel = [enumerator nextObject])) {
    id key, value, objectDescription;

    objectDescription = [propertyList objectForKey: keyLabel];
    if (!objectDescription) {
      NSLog (@"warning: couldn't find the description for object labeled '%@' "
	     @"in the dictionary description '%@ = %@'!",
	     keyLabel, name, dictionary);
      continue;
    }
    key = [self decodeObjectWithName: keyLabel];

    valueLabel = [dictionary objectForKey: keyLabel];
    objectDescription = [propertyList objectForKey: valueLabel];
    if (!objectDescription) {
      NSLog (@"warning: couldn't find the description for object labeled '%@' "
	     @"in the dictionary description '%@ = %@'!",
	     valueLabel, name, dictionary);
      continue;
    }
    value = [self decodeObjectWithName: valueLabel];

    [decodedDictionary setObject: value forKey: key];
  }

  return decodedDictionary;
}

- (Class) decodeClassWithName: (NSString*)name
{
  NSString* className = [self decodeStringWithName: name];

  return className ? NSClassFromString (className) : Nil;
}

- (SEL) decodeSelectorWithName: (NSString*)name
{
  NSString* selectorName = [self decodeStringWithName: name];

  return selectorName ? NSSelectorFromString (selectorName) : (SEL)0;
}

- (char) decodeCharWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return 0;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return 0;

  return *[valueString cString];
}

- (unsigned char) decodeUnsignedCharWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return 0;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return 0;

  return *[valueString cString];
}

- (BOOL) decodeBOOLWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return NO;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return NO;

  return [valueString compare: @"YES" options: NSCaseInsensitiveSearch]
	    == NSOrderedSame;
}

- (short) decodeShortWithName: (NSString*)name
{
  return [self decodeIntWithName: name];
}

- (unsigned short) decodeUnsignedShortWithName: (NSString*)name
{
  return [self decodeIntWithName: name];
}

- (int) decodeIntWithName: (NSString*)name
{
  NSString* valueString;
  int value;

  if (!name)
    return 0;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return 0;

  if (![[NSScanner scannerWithString: valueString] scanInt: &value]) {
    NSLog (@"Cannot scan integer value '%@' from object '%@' under key '%@'",
	   valueString, currentDecodedObjectRepresentation, name);
    return 0;
  }

  return value;
}

- (unsigned int) decodeUnsignedIntWithName: (NSString*)name
{
  return [self decodeIntWithName: name];
}

- (long) decodeLongWithName: (NSString*)name
{
  return [self decodeIntWithName: name];
}

- (unsigned long) decodeUnsignedLongWithName: (NSString*)name
{
  return [self decodeIntWithName: name];
}

- (float) decodeFloatWithName: (NSString*)name
{
  NSString* valueString;
  float value;

  if (!name)
    return 0;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return 0;

  if (![[NSScanner scannerWithString: valueString] scanFloat: &value]) {
    NSLog (@"Cannot scan float value '%@' from object '%@' under key '%@'",
	   valueString, currentDecodedObjectRepresentation, name);
    return 0;
  }

  return value;
}

- (double) decodeDoubleWithName: (NSString*)name
{
  NSString* valueString;
  double value;

  if ( !name )
    return 0;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if ( !valueString )
    return 0;

  if ( ![[NSScanner scannerWithString: valueString] scanDouble: &value] )
    {
      NSLog(@"Cannot scan double value '%@' from object '%@' under key '%@'",
	    valueString, currentDecodedObjectRepresentation, name);
      return 0;
    }
  return value;
}

- (NSPoint) decodePointWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return NSZeroPoint;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return NSZeroPoint;

  return NSPointFromString (valueString);
}

- (NSSize) decodeSizeWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return NSZeroSize;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return NSZeroSize;

  return NSSizeFromString (valueString);
}

- (NSRect) decodeRectWithName: (NSString*)name
{
  NSString* valueString;

  if (!name)
    return NSZeroRect;

  valueString = [currentDecodedObjectRepresentation objectForKey: name];
  if (!valueString)
    return NSZeroRect;

  return NSRectFromString (valueString);
}

- (BOOL) isAtEnd
{
  // TODO
  return NO;
}

- (void) setObjectZone: (NSZone*)zone	{ objectZone = zone; }
- (unsigned int) systemVersion		{ return version; }
- (NSZone*) objectZone			{ return objectZone; }

+ (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive
{
    NSString* className = [classToAliasMappings objectForKey: nameInArchive];

    return className ? className : nameInArchive;
}

+ (void) decodeClassName: (NSString*)nameInArchive
  asClassName: (NSString*)trueName
{
    [classToAliasMappings setObject: trueName forKey: nameInArchive];
}

- (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive
{
  return nameInArchive;
}

- (void) decodeClassName: (NSString*)nameInArchive
	asClassName: (NSString*)trueName
{
}

- (unsigned int) versionForClassName: (NSString*)className
{
  return 1;
}

@end /* GMUnarchiver */


#import "GMArchiveObjects.m"
