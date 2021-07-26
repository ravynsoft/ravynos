/* Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#import "NSMultipleValueBinder.h"
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <AppKit/NSTableView.h>
#import <AppKit/NSTableColumn.h>
#import <AppKit/NSCell.h>
#import <AppKit/NSObject+BindingSupport.h>

static void* NSMultipleValueBinderChangeContext;
static void* NSMultipleValueBinderWholeArrayChangeContext;


@interface _NSMultipleValueWrapperArray : NSArray
{
	id object;
}
-(id)initWithObject:(id)obj;
@end



@implementation _NSMultipleValueBinder

-(NSArray *)rowValues {
    return _rowValues;
}

-(void)setRowValues:(NSArray *)value  {
    if (_rowValues != value){
        [_rowValues release];
        _rowValues = [value retain];
    }
}

-(void)applyToObject:(id)object inRow:(int)row keyPath:(id)path
{
   @try {
      [object setValue:[[_rowValues objectAtIndex:row] valueForKeyPath:_valueKeyPath] forKey:path];
   }
   @catch(id e) {
        NSLog(@"exception %@ while setting value for key path %@ for row %i", e, _valueKeyPath, row);
   }
}

-(void)applyToObject:(id)object inRow:(int)row
{
	[self applyToObject:object inRow:row keyPath:[object _replacementKeyPathForBinding:_binding]];
}

-(void)applyToCell:(id)cell inRow:(int)row
{
	[self applyToObject:cell inRow:row keyPath:[cell _replacementKeyPathForBinding:_binding]];
}

-(void)applyFromObject:(id)object inRow:(int)row keyPath:(id)keypath
{
	[[_rowValues objectAtIndex:row] setValue:[object valueForKeyPath:keypath]  forKeyPath:_valueKeyPath];
}

-(void)applyFromObject:(id)object inRow:(int)row
{
	[self applyFromObject:object inRow:row keyPath:[object _replacementKeyPathForBinding:_binding]];
}

-(void)applyFromCell:(id)cell inRow:(int)row
{
	[self applyFromObject:cell inRow:row keyPath:[cell _replacementKeyPathForBinding:_binding]];
}


-(unsigned)count {
	return [_rowValues count];
}

-(id)objectAtIndex:(unsigned)row {
	return [[_rowValues objectAtIndex:row] valueForKeyPath:_valueKeyPath];
}

-(void)cacheArrayKeyPath
{
	/*
	 Normally, the array path is the key path minus the last component.
	 In case the last component is actually a parameter to an operator,
	 we have to take the last two components.	 
	 */
	id components=[_keyPath componentsSeparatedByString:@"."];
	if([components count]==1)
	{
		_valueKeyPath=@"self";
		_arrayKeyPath=[components lastObject];
	}
	else
	{
		id secondToLast=[components objectAtIndex:[components count]-2];
		
		if([secondToLast hasPrefix:@"@"])
		{
			_valueKeyPath=_keyPath;
			_arrayKeyPath=@"self";
		}
		else
		{
			_valueKeyPath=[components lastObject];
			_arrayKeyPath= [_keyPath substringToIndex:[_keyPath length]-[_valueKeyPath length]-1];
		}
	}
	[_valueKeyPath retain];
	[_arrayKeyPath retain];
}

-(BOOL)allowsEditingForRow:(int)row
{
   if([[_rowValues objectAtIndex:row] classForCoder]==[NSDictionary class])
      return NO;
	return YES;
}


-(void)dealloc
{
	[_arrayKeyPath release];
	[_valueKeyPath release];
	[self setRowValues:nil];
	[super dealloc];
}

-(void)startObservingChanges
{
   if(!_isObserving){
    _isObserving=YES;
	@try {
      [_destination addObserver:self 
                     forKeyPath:_arrayKeyPath 
                        options:0
                        context:&NSMultipleValueBinderWholeArrayChangeContext];
      if(![_valueKeyPath hasPrefix:@"@"])
         [_rowValues addObserver:self
              toObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [_rowValues count])]
                      forKeyPath:_valueKeyPath 
                         options:0
                         context:&NSMultipleValueBinderChangeContext];
	}
   @catch(id ex) {
      NSLog(@"%@", ex);
   }
   }
}

-(void)stopObservingChanges
{
   if(_isObserving){
    _isObserving=NO;
	@try {
      [_destination removeObserver:self forKeyPath:_arrayKeyPath];
      if(![_valueKeyPath hasPrefix:@"@"])
         [_rowValues removeObserver:self
               fromObjectsAtIndexes:[NSIndexSet indexSetWithIndexesInRange:NSMakeRange(0, [_rowValues count])]
                         forKeyPath:_valueKeyPath];
   }
   @catch(id ex) {
      NSLog(@"%@", ex);
   }
   }
}

-(void)syncUp
{
	[self updateRowValues];
}

- (void)observeValueForKeyPath:(NSString *)kp ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if(context==&NSMultipleValueBinderWholeArrayChangeContext)
	{
      [self stopObservingChanges];
      //NSLog(@"bind event from %@.%@ to %@.%@ alias %@ (%@)", [_destination className], _keyPath, [_source className], _binding, _bindingPath, self);
      
      [self updateRowValues];
      
      if([_source respondsToSelector:@selector(_boundValuesChanged)])
         [_source _boundValuesChanged];
      
      [self startObservingChanges];
	}
	else if(context==&NSMultipleValueBinderChangeContext)
	{
		if([_source respondsToSelector:@selector(reloadData)])
			[_source reloadData];
		if([_source respondsToSelector:@selector(tableView)])
			[[_source tableView] reloadData];

		if([_destination respondsToSelector:@selector(_selectionMayHaveChanged)])
			[_destination performSelector:@selector(_selectionMayHaveChanged)];
	}
}

-(id)defaultBindingOptionsForBinding:(id)thisBinding
{
	return [[_source dataCell] _defaultBindingOptionsForBinding:thisBinding];
}

-(void)bind
{
	[self cacheArrayKeyPath];

	[self syncUp];
	[self startObservingChanges];
   
	if([self createsSortDescriptor] && [_binding isEqual:@"value"])
	{
		[_source setSortDescriptorPrototype:[[[NSSortDescriptor alloc] initWithKey:_valueKeyPath
																		ascending:YES] autorelease]];
	}
	if([_source respondsToSelector:@selector(_establishBindingsWithDestinationIfUnbound:)])
	{
		[_source performSelector:@selector(_establishBindingsWithDestinationIfUnbound:)
					 withObject:_destination
					 afterDelay:0.0];
	}
}



-(void)unbind
{
	[self stopObservingChanges];
}


-(NSString*)description
{
	return [NSString stringWithFormat:@"%@ %@", [super description], [self rowValues]];
}

-(void)updateRowValues {
	id value=[_destination valueForKeyPath:_arrayKeyPath];
	if(![value respondsToSelector:@selector(objectAtIndex:)])
		value=[[[_NSMultipleValueWrapperArray alloc] initWithObject:value] autorelease];
	[self setRowValues:value];
}
@end


#pragma mark -
#pragma mark Helper classes

static void* NSTableViewContentBinderChangeContext;

@class NSTableView;

@implementation _NSTableViewContentBinder

-(void)startObservingChanges {
   if(!_isObserving){
    _isObserving=YES;
// FIXME: assertions are buggy, dont until they are fixed
//	NSParameterAssert([_source isKindOfClass:[NSTableView class]]);
	[_destination addObserver:self forKeyPath:_keyPath options:NSKeyValueObservingOptionNew context:&NSTableViewContentBinderChangeContext];
   }
}

-(void)stopObservingChanges {
   if(_isObserving){
     _isObserving=NO;
	@try {
		[_destination removeObserver:self forKeyPath:_keyPath];
   } @catch (id ex) {
      NSLog(@"%@", ex);   
   }
   }
}

-(void)syncUp {
	[_source performSelector:@selector(reloadData) withObject:nil afterDelay:0.0];
	[_source reloadData];
}

- (void)observeValueForKeyPath:(NSString *)kp ofObject:(id)object change:(NSDictionary *)change context:(void *)context {
	[self stopObservingChanges];

	if(context==&NSTableViewContentBinderChangeContext)
	{
		[_source _boundValuesChanged];
	}

	[self startObservingChanges];
}

-(void)bind {
	[self syncUp];
	[self startObservingChanges];
}

-(void)unbind {
	[self stopObservingChanges];
}

-(unsigned)numberOfRows {
//NSLog(@"_desination=%@, keypath=%@",_destination,_keyPath);
	return [[_destination valueForKeyPath:_keyPath] count];
}
@end

@implementation _NSMultipleValueWrapperArray

-initWithObject:(id)obj {
   if((self = [super init])){
     object=[obj retain];
   }
   return self;
}

-(void)dealloc {
	[object release];
	[super dealloc];
}

-(NSUInteger)count {
	return 1;
}

-objectAtIndex:(NSUInteger)idx {
   return object;
}

@end
