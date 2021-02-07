/* Implementation of class GSStoryboardTransform
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Sat 04 Jul 2020 03:48:15 PM EDT

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSXMLDocument.h>
#import <Foundation/NSXMLNode.h>
#import <Foundation/NSXMLElement.h>
#import <Foundation/NSUUID.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSException.h>

#import "AppKit/NSSeguePerforming.h"
#import "AppKit/NSStoryboard.h"
#import "AppKit/NSStoryboardSegue.h"
#import "AppKit/NSNibDeclarations.h"
#import "AppKit/NSViewController.h"
#import "AppKit/NSWindowController.h"

#import "GSStoryboardTransform.h"
#import "GSFastEnumeration.h"

#define APPLICATION @"application"

@interface NSStoryboardSegue (__private__)
- (void) _setDestinationController: (id)controller; 
- (void) _setSourceController: (id)controller;
@end

@interface NSStoryboardSegue (__StoryboardPrivate__)
// Private to this class...
- (void) _setKind: (NSString *)k;
- (NSString *) _kind;
- (void) _setRelationship: (NSString *)r;
- (NSString *) _relationship;
- (void) _setPopoverAnchorView: (id)view;
- (id) _popoverAnchorView;
- (void) _setPopoverBehavior: (NSPopoverBehavior)behavior;
- (NSPopoverBehavior) _popoverBehavior;
- (void) _setPreferredEdge: (NSRectEdge)edge;
- (NSRectEdge) _preferredEdge;
@end

// this needs to be set on segues
@implementation NSStoryboardSegue (__StoryboardPrivate__)
- (void) _setKind: (NSString *)k
{
  ASSIGN(_kind, k);
}

- (NSString *) _kind
{
  return _kind;
}

- (void) _setRelationship: (NSString *)r
{
  ASSIGN(_relationship, r);
}

- (NSString *) _relationship
{
  return _relationship;
}

- (void) _setPopoverAnchorView: (id)view
{
  ASSIGN(_popoverAnchorView, view);
}

- (id) _popoverAnchorView
{
  return _popoverAnchorView;
}

- (void) _setPopoverBehavior: (NSPopoverBehavior)behavior
{
  _popoverBehavior = behavior;
}

- (NSPopoverBehavior) _popoverBehavior
{
  return _popoverBehavior;
}

- (void) _setPreferredEdge: (NSRectEdge)edge
{
  _preferredEdge = edge;
}

- (NSRectEdge) _preferredEdge
{
  return _preferredEdge;
}
@end

@implementation NSStoryboardSeguePerformAction
- (id) target
{
  return _target;
}

- (void) setTarget: (id)target
{
  ASSIGN(_target, target);
}

- (SEL) action
{
  return _action;
}

- (void) setAction: (SEL)action
{
  _action = action;
}

- (NSString *) selector
{
  return NSStringFromSelector(_action);
}

- (void) setSelector: (NSString *)s
{
  _action = NSSelectorFromString(s);
}

- (id) sender
{
  return _sender;
}

- (void) setSender: (id)sender
{
  ASSIGN(_sender, sender);
}

- (NSString *) identifier
{
  return _identifier;
}

- (void) setIdentifier: (NSString *)identifier
{
  ASSIGN(_identifier, identifier);
}

- (NSString *) kind
{
  return _kind;
}

- (void) setKind: (NSString *)kind
{
  ASSIGN(_kind, kind);
}

- (void) setPopoverAnchorView: (id)view
{
  ASSIGN(_popoverAnchorView, view);
}

- (id) popoverAnchorView
{
  return _popoverAnchorView;
}

- (NSStoryboard *) storyboard
{
  return _storyboard;
}

- (void) setStoryboard: (NSStoryboard *)storyboard
{
  ASSIGN(_storyboard, storyboard);
}

- (NSStoryboardSegue *) storyboardSegue
{
  return _storyboardSegue;
}

- (void) setStoryboardSegue: (NSStoryboardSegue *)ss
{
  ASSIGN(_storyboardSegue, ss);
}

- (void) dealloc
{
  RELEASE(_storyboard);
  RELEASE(_kind);
  RELEASE(_identifier);
  RELEASE(_popoverAnchorView);
  RELEASE(_sender);
  RELEASE(_storyboardSegue);
  [super dealloc];
}

- (IBAction) doAction: (id)sender
{
  BOOL should = YES;

  // If the instance we are testing is a controller, then the value of should is set by this method....
  // if it is not, as it is possible to initiate a segue from an NSMenuItem, then we don't, but should
  // remains set to YES so that the logic to replace the destination controller is still called.
  if ([_sender respondsToSelector: @selector(shouldPerformSegueWithIdentifier:sender:)])
    {
      should = [_sender shouldPerformSegueWithIdentifier: _identifier
                                                  sender: _sender];
    }

  if (should)
    {
      id destCon = [_storyboardSegue destinationController];
      if ([destCon isKindOfClass: [NSString class]])
        {
          // resolve the destination controller
          destCon = [_storyboard instantiateControllerWithIdentifier: destCon];
          [_storyboardSegue _setDestinationController: destCon];  // replace with actual controller...
        }
      [_storyboardSegue _setSourceController: _sender]; 
      
      if (_sender != nil &&
          [_sender respondsToSelector: @selector(performSegueWithIdentifier:sender:)])
        {
          [_sender performSegueWithIdentifier: _identifier
                                       sender: _sender];
        }
      else
        {
          [_storyboardSegue perform];
        }
    }
}

- (id) copyWithZone: (NSZone *)z
{
  NSStoryboardSeguePerformAction *pa = [[NSStoryboardSeguePerformAction allocWithZone: z] init];
  [pa setTarget: _target];
  [pa setSelector: [self selector]];
  [pa setSender: _sender];
  [pa setIdentifier: _identifier];
  [pa setPopoverAnchorView: _popoverAnchorView];
  [pa setStoryboardSegue: _storyboardSegue];
  [pa setStoryboard: _storyboard];
  return pa;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSTarget"])
        {
          [self setTarget: [coder decodeObjectForKey: @"NSTarget"]];
        }
      if ([coder containsValueForKey: @"NSSelector"])
        {
          [self setSelector: [coder decodeObjectForKey: @"NSSelector"]];
        }
      if ([coder containsValueForKey: @"NSSender"])
        {
          [self setSender: [coder decodeObjectForKey: @"NSSender"]];
        }
      if ([coder containsValueForKey: @"NSIdentifier"])
        {
          [self setIdentifier: [coder decodeObjectForKey: @"NSIdentifier"]];
        }
      if ([coder containsValueForKey: @"NSKind"])
        {
          [self setKind: [coder decodeObjectForKey: @"NSKind"]];
        }
      if ([coder containsValueForKey: @"NSPopoverAnchorView"])
        {
          [self setPopoverAnchorView: [coder decodeObjectForKey: @"NSPopoverAnchorView"]];
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // this is never encoded directly...
}
@end

@implementation NSControllerPlaceholder

- (NSString *) storyboardName
{
  return _storyboardName;
}

- (void) setStoryboardName: (NSString *)name
{
  ASSIGNCOPY(_storyboardName, name);
}

- (id) copyWithZone: (NSZone *)z
{
  NSControllerPlaceholder *c = [[NSControllerPlaceholder allocWithZone: z] init];
  [c setStoryboardName: _storyboardName];
  return c;
}

- (instancetype) initWithCoder: (NSCoder *)coder
{
  self = [super init];
  if ([coder allowsKeyedCoding])
    {
      if ([coder containsValueForKey: @"NSStoryboardName"])
        {
          [self setStoryboardName: [coder decodeObjectForKey: @"NSStoryboardName"]];
        }
    }
  return self;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  // this is never encoded directly...
}

- (id) instantiate
{
  NSStoryboard *sb = [NSStoryboard storyboardWithName: _storyboardName
                                               bundle: [NSBundle mainBundle]];
  return [sb instantiateInitialController];
}

@end

@implementation GSStoryboardTransform

- (instancetype) initWithData: (NSData *)data
{
  self = [super init];
  if (self != nil)
    {
      NSXMLDocument *xml = [[NSXMLDocument alloc] initWithData: data
                                                       options: 0
                                                         error: NULL];
      
      _scenesMap = [[NSMutableDictionary alloc] initWithCapacity: 10];
      _controllerMap = [[NSMutableDictionary alloc] initWithCapacity: 10];
      _identifierToSegueMap = [[NSMutableDictionary alloc] initWithCapacity: 10];
      
      [self processStoryboard: xml];
      RELEASE(xml);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_initialViewControllerId);
  RELEASE(_applicationSceneId);
  RELEASE(_scenesMap);
  RELEASE(_controllerMap);
  RELEASE(_identifierToSegueMap);
  [super dealloc];
}

- (NSString *) initialViewControllerId
{
  return _initialViewControllerId;
}

- (NSString *) applicationSceneId
{
  return _applicationSceneId;
}

- (NSMapTable *) segueMapForIdentifier: (NSString *)identifier
{
  return [_identifierToSegueMap objectForKey: identifier];
}

- (NSXMLElement *) createCustomObjectWithId: (NSString *)ident
                                   userLabel: (NSString *)userLabel
                                 customClass: (NSString *)className
{
  NSXMLElement *customObject =
    [[NSXMLElement alloc] initWithName: @"customObject"];
  NSXMLNode *idValue =
    [NSXMLNode attributeWithName: @"id"
                     stringValue: ident];
  NSXMLNode *usrLabel =
    [NSXMLNode attributeWithName: @"userLabel"
                     stringValue: userLabel];
  NSXMLNode *customCls =
    [NSXMLNode attributeWithName: @"customClass"
                     stringValue: className];
  
  [customObject addAttribute: idValue];
  [customObject addAttribute: usrLabel];
  [customObject addAttribute: customCls];

  AUTORELEASE(customObject);
  
  return customObject;
}

- (NSData *) dataForIdentifier: (NSString *)identifier
{
  NSString *sceneId = [_controllerMap objectForKey: identifier];
  NSXMLDocument *xml = [_scenesMap objectForKey: sceneId];
  return [xml XMLData];
}

- (void) addStandardObjects: (NSXMLElement *)objects
                classString: (NSString *) customClassString
                connections: (NSXMLNode *)appCons
           firstResponderId: (NSString *)firstResponderId
{
  NSXMLElement *customObject = nil;
  
  customObject = 
    [self createCustomObjectWithId: @"-3"
                         userLabel: @"Application"
                       customClass: @"NSObject"];
  [objects insertChild: customObject
               atIndex: 0];
  customObject = 
    [self createCustomObjectWithId: firstResponderId
                         userLabel: @"First Responder"
                       customClass: @"FirstResponder"];
  [objects insertChild: customObject
               atIndex: 0];
  customObject =
    [self createCustomObjectWithId: @"-2"
                         userLabel: @"File's Owner"
                       customClass: customClassString];
  if (appCons != nil)
    {
      [customObject addChild: appCons];
    }
  [objects insertChild: customObject
               atIndex: 0]; 
}

- (void) processChild: (NSXMLElement *)objects
              withDoc: (NSXMLElement *)doc
          withAppNode: (NSXMLNode *)appNode
              sceneId: (NSString *)sceneId
     firstResponderId: (NSString *)firstResponderId
{
  NSString *customClassString = nil;
  NSXMLNode *appCons = nil;
  
  if (appNode != nil)
    {
      NSArray *appConsArr = [appNode nodesForXPath: @"connections" error: NULL];
      
      appCons = [appConsArr objectAtIndex: 0];
      if (appCons != nil)
        {
          [appCons detach];
        }
      
      // Assign application scene...
      ASSIGN(_applicationSceneId, sceneId);
      [_controllerMap setObject: _applicationSceneId
                         forKey: APPLICATION];
      
      // Move all application children to objects...
      NSArray *appChildren = [appNode children];      
      FOR_IN(NSXMLElement*, ae, appChildren)
        [ae detach];
        [objects addChild: ae];
      END_FOR_IN(appChildren);
      
      // Remove the appNode
      [appNode detach];
      
      // create a customObject entry for NSApplication reference...
      NSXMLNode *appCustomClass = [(NSXMLElement *)appNode
                                      attributeForName: @"customClass"];
      customClassString = ([appCustomClass stringValue] == nil) ?
        @"NSApplication" : [appCustomClass stringValue];
    }
  
  [self addStandardObjects: objects
               classString: customClassString
               connections: appCons
          firstResponderId: firstResponderId];
  
  // Add it to the document
  [objects detach];
  [doc addChild: objects];
}

- (NSArray *) subclassesOfClass: (Class)clz
{
  NSMutableArray *subclasses = [GSObjCAllSubclassesOfClass(clz) mutableCopy];
  NSMutableArray *result = [NSMutableArray arrayWithCapacity: [subclasses count]];

  [subclasses insertObject: clz atIndex: 0];
  FOR_IN(Class, cls, subclasses)
    {
      NSString *className = NSStringFromClass(cls);
      NSString *classNameNoNamespace = [className substringFromIndex: 2];
      NSString *xmlClassName = [NSString stringWithFormat: @"%@%@",
                                         [[classNameNoNamespace substringToIndex: 1] lowercaseString],
                                         [classNameNoNamespace substringFromIndex: 1]];
      NSString *lowerCaseName = [xmlClassName lowercaseString];
      
      [result addObject: xmlClassName];
      [result addObject: lowerCaseName];
    }
  END_FOR_IN(subclasses);

  return result;
}
   
- (NSArray *) findSubclassesOf: (Class)clz
                    inDocument: (NSXMLDocument *)document
{
  NSArray *result = nil;
  NSArray *xmlClassNames = [self subclassesOfClass: clz];

  FOR_IN(NSString*, xmlClassName, xmlClassNames)
    {
      NSString *xpath = [NSString stringWithFormat: @"//%@",xmlClassName];
      result = [document nodesForXPath: xpath error: NULL];      
      if ([result count] > 0)
        {
          break;
        }
    }
  END_FOR_IN(xmlClassNames);

  return result;
}

- (NSString *) controllerIdWithDocument: (NSXMLDocument *)document
{
  NSString *controllerId = nil;
  NSArray *windowControllers = [self findSubclassesOf: [NSWindowController class]
                                           inDocument: document];
  NSArray *viewControllers = [self findSubclassesOf: [NSViewController class]
                                         inDocument: document];
  NSArray *controllerPlaceholders = [document nodesForXPath: @"//controllerPlaceholder"
                                                      error: NULL];
  
  if ([windowControllers count] > 0)
    {
      NSXMLElement *ce = [windowControllers objectAtIndex: 0];
      NSXMLNode *attr = [ce attributeForName: @"id"];
      controllerId = [attr stringValue];
      
      FOR_IN(NSXMLElement*, o, windowControllers)
        {
          NSXMLElement *objects = (NSXMLElement *)[o parent];
          NSArray *windows = [o nodesForXPath: @"//window" error: NULL];
          FOR_IN(NSXMLNode*, w, windows)
            {
              [w detach];
              [objects addChild: w];
            }
          END_FOR_IN(windows);
        }
      END_FOR_IN(windowControllers);
    }
  
  if ([viewControllers count] > 0)
    {
      NSXMLElement *ce = [viewControllers objectAtIndex: 0];
      NSXMLNode *attr = [ce attributeForName: @"id"];
      controllerId = [attr stringValue];
    }
  
  if ([controllerPlaceholders count] > 0)
    {
      NSXMLElement *ce = [controllerPlaceholders objectAtIndex: 0];
      NSXMLNode *attr = [ce attributeForName: @"id"];
      controllerId = [attr stringValue];
    }
  
  return controllerId;
}

- (void) processStoryboard: (NSXMLDocument *)xml
{
  NSArray *docNodes = [xml nodesForXPath: @"document" error: NULL];

  if ([docNodes count] > 0)
    {
      NSXMLElement *docNode = [docNodes objectAtIndex: 0];
      NSArray *array = [docNode nodesForXPath: @"//scene" error: NULL];
      NSArray *firstResponderIdNodes = [docNode nodesForXPath: @"//objects/customObject[@sceneMemberID =\"firstResponder\"]/@id"
                                                        error: NULL];
      NSString *firstResponderId = @"-1";

      if([firstResponderIdNodes count] > 0)
        {
          firstResponderId = [[firstResponderIdNodes objectAtIndex: 0] stringValue];
        }
      
      // Set initial view controller...
      ASSIGN(_initialViewControllerId, [[docNode attributeForName: @"initialViewController"] stringValue]);             
      FOR_IN(NSXMLElement*, e, array) 
        {
          NSXMLElement *doc = [[NSXMLElement alloc] initWithName: @"document"];
          NSArray *children = [e children];
          NSXMLDocument *document = nil;
          NSString *sceneId = [[e attributeForName: @"sceneID"] stringValue]; 
          NSString *controllerId = nil;
          // Move children...
          FOR_IN(NSXMLElement*, child, children)
            {
              if ([[child name] isEqualToString: @"point"] == YES)
                continue; // go on if it's a point element, we don't use that in the app...
              
              NSArray *subnodes = [child nodesForXPath: @"//application" error: NULL];
              NSXMLNode *appNode = [subnodes objectAtIndex: 0];
              [self processChild: child
                         withDoc: doc
                     withAppNode: appNode
                         sceneId: sceneId
                firstResponderId: firstResponderId];
              
              // fix other custom objects
              document = [[NSXMLDocument alloc] initWithRootElement: doc]; 
              controllerId = [self controllerIdWithDocument: document];
              controllerId = (controllerId != nil) ? controllerId : APPLICATION;
              RELEASE(doc);
              
              // Create document...
              [_scenesMap setObject: document
                             forKey: sceneId];
              
              // Map controllerId's to scenes...
              if (controllerId != nil)
                {
                  [_controllerMap setObject: sceneId
                                     forKey: controllerId];

                  [self processSegues: document
                      forControllerId: controllerId];
                }
              RELEASE(document);
            }
          END_FOR_IN(children);
        }
      END_FOR_IN(array);
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"No document element found in storyboard file"];
    }
}

- (BOOL) isProcessedDocument: (NSXMLDocument *)xmlIn
{
  NSArray *docArray = [xmlIn nodesForXPath: @"document" error: NULL];
  if ([docArray count] > 0)
    {
      NSXMLElement *docElem = (NSXMLElement *)[docArray objectAtIndex: 0];
      NSXMLNode *a = [docElem attributeForName: @"processed"];
      NSString *value = [a stringValue];
      if (value != nil)
        {
          return YES;
        }
      else
        {
          NSXMLNode *new_attr = [NSXMLNode attributeWithName: @"processed"
                                                      stringValue: @"true"];
          [docElem addAttribute: new_attr];
        }
    }
  return NO;
}

- (NSXMLElement *) createStoryboardProxyElementWithSelector: (NSString *)selector
                                                     target: (NSString *)dst
                                            segueIdentifier: (NSString *)ident
                                                     sender: (NSString *)src
                                                       kind: (NSString *)kind
                                                 anchorView: (NSString *)anchorView
{
  NSXMLElement *sbproxy = [NSXMLElement elementWithName: @"storyboardSeguePerformAction"];

  NSXMLNode *pselector
    = [NSXMLNode attributeWithName: @"selector"
                       stringValue: selector];
  NSXMLNode *ptarget
    = [NSXMLNode attributeWithName: @"target"
                       stringValue: dst];
  NSString *pident_value = [[NSUUID UUID] UUIDString];
  NSXMLNode *pident
    = [NSXMLNode attributeWithName: @"id"
                       stringValue: pident_value];
  NSXMLNode *psegueIdent
    = [NSXMLNode attributeWithName: @"identifier"
                       stringValue: ident];
  NSXMLNode *psender
    = [NSXMLNode attributeWithName: @"sender"
                       stringValue: src];
  NSXMLNode *pkind
    = [NSXMLNode attributeWithName: @"kind"
                       stringValue: kind];
  NSXMLNode *panchorview
    = [NSXMLNode attributeWithName: @"popoverAnchorView"
                       stringValue: anchorView];
  
  [sbproxy addAttribute: pselector];
  [sbproxy addAttribute: ptarget];
  [sbproxy addAttribute: pident];
  [sbproxy addAttribute: psegueIdent];
  [sbproxy addAttribute: psender];
  [sbproxy addAttribute: pkind];
  [sbproxy addAttribute: panchorview];

  return sbproxy;
}

- (NSMapTable *) processConnections: (NSArray *)connectionsArray
                        withObjects: (NSXMLElement *)objects
                       controllerId: (NSString *)src
{
  NSMapTable *mapTable = [NSMapTable strongToWeakObjectsMapTable];
  
  FOR_IN (NSXMLElement*, connections, connectionsArray)
    {
      NSArray *children = [connections children]; // there should be only one per set.
      
      FOR_IN (NSXMLElement*, obj, children)
        if ([[obj name] isEqualToString: @"segue"])
          {
            // get the information from the segue.
            id connections_parent = [[obj parent] parent];
            id segue_parent = connections; // [obj parent];
            NSString *connections_parent_name = [connections_parent name];
            NSXMLNode *attr = [obj attributeForName: @"destination"];
            NSString *dst = [attr stringValue];
            attr = [obj attributeForName: @"kind"];
            NSString *kind =  [attr stringValue];
            attr = [obj attributeForName: @"relationship"];
            NSString *rel = [attr stringValue];
            attr = [obj attributeForName: @"id"];
            NSString *uid = [attr stringValue];
            attr = [obj attributeForName: @"identifier"];
            NSString *ident = [attr stringValue];
            if (ident == nil)
              {
                ident = [[NSUUID UUID] UUIDString];
              }
            attr = [obj attributeForName: @"popoverAnchorView"];
            NSString *av = [attr stringValue];
            attr = [obj attributeForName: @"popoverBehavior"];
            NSString *pb = [attr stringValue];
            NSPopoverBehavior behavior = NSPopoverBehaviorApplicationDefined; 
            if ([pb isEqualToString: @"a"])
              {
                behavior = NSPopoverBehaviorApplicationDefined; 
              }
            else if ([pb isEqualToString: @"t"])
              {
                behavior = NSPopoverBehaviorTransient;
              }
            else if ([pb isEqualToString: @"s"])
              {
                behavior = NSPopoverBehaviorSemitransient;
              }
            
            attr = [obj attributeForName: @"preferredEdge"];
            NSString *pe = [attr stringValue];
            NSRectEdge edge = NSMinXEdge;
            if ([pe isEqualToString: @"maxY"])
              {
                edge = NSMaxYEdge;
              }
            else if ([pe isEqualToString: @"minY"])
              {
                edge = NSMinYEdge;
              }
            else if ([pe isEqualToString: @"maxX"])
              {
                edge = NSMaxXEdge;
              }
            else if ([pe isEqualToString: @"minX"])
              {
                edge = NSMinXEdge;
              }
            [obj detach]; // segue can't be in the archive since it doesn't conform to NSCoding
            
            // Create proxy object to invoke methods on the window controller
            NSXMLElement *sbproxy =  [self createStoryboardProxyElementWithSelector: @"doAction:"
                                                                             target: dst
                                                                    segueIdentifier: ident
                                                                             sender: src
                                                                               kind: kind
                                                                         anchorView: av];
            
            NSUInteger count = [[objects children] count];
            [objects insertChild: sbproxy
                         atIndex: count - 1];
            
            // add action to parent ONLY if it is NOT a controller..
            if (![[self subclassesOfClass: [NSWindowController class]] containsObject: connections_parent_name] &&
                ![[self subclassesOfClass: [NSViewController class]] containsObject: connections_parent_name])
              {              
                // Create action...
                NSXMLElement *action = [NSXMLElement elementWithName: @"action"];
                NSXMLNode *selector
                  = [NSXMLNode attributeWithName: @"selector"
                                     stringValue: @"doAction:"];
                NSXMLNode *target
                  = [NSXMLNode attributeWithName: @"target"
                                     stringValue: [[sbproxy attributeForName: @"id"] stringValue]];
                NSXMLNode *controller_ident
                  = [NSXMLNode attributeWithName: @"id"
                                     stringValue: uid]; 
                [action addAttribute: selector];
                [action addAttribute: target];
                [action addAttribute: controller_ident];
                [segue_parent addChild: action];
              }
            
            // Create the segue...
            NSStoryboardSegue *ss = [[NSStoryboardSegue alloc] initWithIdentifier: ident
                                                                           source: src
                                                                      destination: dst];
            [ss _setKind: kind];
            [ss _setRelationship: rel];
            [ss _setPopoverBehavior: behavior];
            [ss _setPreferredEdge: edge];
              
            // Add to maptable...
            [mapTable setObject: ss
                         forKey: ident];
            
          } // only process segue objects...
      END_FOR_IN(children);          
    } // iterate over connection objs
  END_FOR_IN(connectionsArray);

  return mapTable;
}

- (void) processSegues: (NSXMLDocument *)xml
       forControllerId: (NSString *)identifier
{
  BOOL processed = [self isProcessedDocument: xml];
  if (!processed)
    {
      NSArray *array = [xml nodesForXPath: @"//objects[1]"
                                    error: NULL];
      NSXMLElement *objects = [array objectAtIndex: 0]; // get the "objects" section
      NSArray *connectionsArray = [xml nodesForXPath: @"//connections"
                                               error: NULL];
      NSMapTable *mapTable = [self processConnections: connectionsArray
                                          withObjects: objects
                                         controllerId: identifier];           
      [_identifierToSegueMap setObject: mapTable
                                forKey: identifier];                  
    }
}

@end
