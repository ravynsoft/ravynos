/* Copyright (c) 2006-2007 Christopher J. W. Lloyd, 2007-2008 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import <AppKit/NSObjectController.h>
#import <Foundation/NSString.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSSet.h>
#import "NSControllerSelectionProxy.h"
#import "NSObservationProxy.h"

@interface NSObjectController(forward)
-(void)_selectionWillChange;
-(void)_selectionDidChange;

@end

@implementation NSObjectController
+(void)initialize {
   [self setKeys:[NSArray arrayWithObjects:@"editable", nil] triggerChangeNotificationsForDependentKey:@"canAdd"];
   [self setKeys:[NSArray arrayWithObjects:@"editable", nil] triggerChangeNotificationsForDependentKey:@"canInsert"];
   [self setKeys:[NSArray arrayWithObjects:@"editable", @"selection", nil] triggerChangeNotificationsForDependentKey:@"canRemove"];
   [self setKeys:[NSArray arrayWithObjects:@"content", nil] triggerChangeNotificationsForDependentKey:@"contentObject"];
}

- (id)init
{
	if ((self = [super init])) {
		_selection=[[NSControllerSelectionProxy alloc] initWithController:self];
	}
	return self;
}

- (id)initWithContent:(id)content
{
	if ((self=[super init])) {
		_objectClassName=@"NSMutableDictionary";
		
		_editable = YES;
		_automaticallyPreparesContent = NO;

		_observedKeys=[[NSCountedSet alloc] init];
		_selection=[[NSControllerSelectionProxy alloc] initWithController:self];
	}

	return self;
}

-initWithCoder:(NSCoder*)coder {
   if((self=[super init])) {
   
    _objectClassName=[[coder decodeObjectForKey:@"NSObjectClassName"] copy];
    if(_objectClassName==nil)
     _objectClassName=@"NSMutableDictionary";
     
    _editable = [coder decodeBoolForKey:@"NSEditable"];
    _automaticallyPreparesContent = [coder decodeBoolForKey:@"NSAutomaticallyPreparesContent"];
     
    _observedKeys=[[NSCountedSet alloc] init];
    _selection=[[NSControllerSelectionProxy alloc] initWithController:self];
   }
   return self;
}

-(void)prepareContent {
}

-(void)awakeFromNib {
   if([self automaticallyPreparesContent]){
    [self prepareContent];
   }
}


- (id)content {
    return [[_content retain] autorelease];
}

- (void)setContent:(id)value {
   if(value!=_content)
   {
       [self _selectionWillChange];
       
        [_content release];
        _content = [value retain];
       
		[self _selectionDidChange];
    }
}

-(void)_setContentWithoutKVO:(id)value
{
   if (_content != value) {
      [_content release];
      _content = [value retain];
   }
}


-(NSArray *)selectedObjects
{
	NSArray *result=[NSArray arrayWithObject:_content];
    return result;
}

-(id)selection
{
	return _selection;
}

-(id)_defaultNewObject
{
	return [[NSClassFromString(_objectClassName) alloc] init];
}

-(id)newObject
{
	return [self _defaultNewObject];
}

-(void)_selectionWillChange
{
   [self willChangeValueForKey:@"selection"];
   [_selection controllerWillChange];
}


-(void)_selectionDidChange
{
   [_selection controllerDidChange];
	[self didChangeValueForKey:@"selection"];	
}
 
-(void)dealloc
{
	[_selection release];
	[_objectClassName release];
	[_content release];
    [_observedKeys release];
	[super dealloc];
}


-(BOOL)canAdd;
{
	return [self isEditable];
}

-(BOOL)canRemove;
{
	return [self isEditable] && [[self selectedObjects] count];
}

- (BOOL)isEditable
{
	return _editable;
}

-(void)setEditable:(BOOL)value
{
	_editable=value;
}

- (BOOL)automaticallyPreparesContent {
    return _automaticallyPreparesContent;
}

- (void)setAutomaticallyPreparesContent:(BOOL)value {
	_automaticallyPreparesContent = value;
}

- (void)addObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath options:(NSKeyValueObservingOptions)options context:(void *)context
{
   [_observedKeys addObject:keyPath];
   [super addObserver:observer forKeyPath:keyPath options:options context:context];
}

- (void)removeObserver:(NSObject *)observer forKeyPath:(NSString *)keyPath
{
   [_observedKeys removeObject:keyPath];
   [super removeObserver:observer forKeyPath:keyPath];
}

-(id)_observedKeys
{
   return _observedKeys;
}

-(id)_contentObject {
   return [self content];
}

-(void)_setContentObject:(id)val {
   [self setContent:val];
}
@end
