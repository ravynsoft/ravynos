/* Copyright (c) 2006-2007 Christopher J. W. Lloyd
   Copyright (c) 2007 Johannes Fortmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/AppKit.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSObject+BindingSupport.h>

#import "NSKeyValueBinding/NSMultipleValueBinder.h"
#import "NSKeyValueBinding/NSKVOBinder.h"
#import <AppKit/NSRaise.h>

@interface NSTableView(private)
-(void)_establishBindingsWithDestinationIfUnbound:(id)dest;
@end

@implementation NSTableColumn

-(void)encodeWithCoder:(NSCoder *)coder {
   NSUnimplementedMethod();
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    _identifier=[[keyed decodeObjectForKey:@"NSIdentifier"] retain];
    _headerCell=[[keyed decodeObjectForKey:@"NSHeaderCell"] retain];
    _dataCell=[[keyed decodeObjectForKey:@"NSDataCell"] retain];
    _width=[keyed decodeFloatForKey:@"NSWidth"];
    _minWidth=[keyed decodeFloatForKey:@"NSMinWidth"];
    _maxWidth=[keyed decodeFloatForKey:@"NSMaxWidth"];
    _isResizable=[keyed decodeBoolForKey:@"NSIsResizeable"];
    _isEditable=[keyed decodeBoolForKey:@"NSIsEditable"];
   }
   else {
    [NSException raise:NSInvalidArgumentException format:@"-[%@ %s] is not implemented for coder %@",isa,sel_getName(_cmd),coder];
   }

   return self;
}

// NSOutlineView needs to programmatically instantiated as IB/WOF4 doesn't have an editor for it..
// also theoretically -dealloc could've crashed as the cell prototypes weren't initialized at all...
-initWithIdentifier:identifier {
   _identifier=[identifier retain];
   _width=100.0;
   _minWidth=10.0;
   _maxWidth=FLT_MAX; // Doc.s say MAXFLOAT, mingw doesnt have MAXFLOAT yet

        _headerCell = [[NSTableHeaderCell alloc] initTextCell:@""];
        _dataCell = [[NSTextFieldCell alloc] initTextCell:@""];
    
    return self;
}

-(void)dealloc {
    [_identifier release];
    [_headerCell release];
    [_dataCell release];
    [_headerToolTip release];
	[_sortDescriptorPrototype release];
	
    [super dealloc];
}

-(NSString *)description {
    return [NSString stringWithFormat:@"<%@ 0x%08lx identifier: \"%@\" headerCell: %@ dataCell: %@ width: %f minWidth: %f maxWidth: %f>",
        [self class], self, _identifier, _headerCell, _dataCell, _width, _minWidth, _maxWidth];
}

-(id)identifier {
    return _identifier;
}

-(NSTableView *)tableView {
    return _tableView;
}

-(id)headerCell {
    return _headerCell;
}

-(id)dataCell {
    return _dataCell;
}

-(NSString *)headerToolTip {
   return _headerToolTip;
}

-(float)width {
    return _width;
}

-(float)minWidth {
    return _minWidth;
}

-(float)maxWidth {
    return _maxWidth;
}

-(BOOL)isResizable {
    return _isResizable;
}

-(BOOL)isEditable {
    return _isEditable;
}

-(NSUInteger)resizingMask {
   return _resizingMask;
}

-(void)setIdentifier:(id)identifier {
   identifier=[identifier retain];
    [_identifier release];
   _identifier=identifier;
}

-(void)setTableView:(NSTableView *)tableView {
    _tableView = tableView;
}

-(void)setHeaderCell:(NSCell *)cell {
   cell=[cell retain];
   [_headerCell release];
    _headerCell = cell;
}

-(void)setDataCell:(NSCell *)cell {
   cell=[cell retain];
   [_dataCell release];
   _dataCell = cell;
}

-(void)setHeaderToolTip:(NSString *)value {
   value=[value copy];
   [_headerToolTip release];
   _headerToolTip=value;
}

-(void)setWidth:(float)width {
    if (width > _maxWidth)
        width = _maxWidth;
    else if (width < _minWidth)
        width = _minWidth;
    
    _width = width;
}

-(void)setMinWidth:(float)width {
    _minWidth = width;
}

-(void)setMaxWidth:(float)width {
    _maxWidth = width;
}

-(void)setResizable:(BOOL)flag {
    _isResizable = flag;
}

-(void)setEditable:(BOOL)flag {
    _isEditable = flag;
    [_dataCell setEditable:flag];
}

-(void)setResizingMask:(NSUInteger)value {
   _resizingMask=value;
}

-(NSCell *)dataCellForRow:(int)row {
    return [self dataCell];
}


-(void)_boundValuesChanged
{

}

-(void)prepareCell:(id)cell inRow:(int)row
{
	id binders=[self _allUsedBinders];
	int count=[binders count];
	int i;
	for(i=0; i<count; i++)
	{
		id binder=[binders objectAtIndex:i];
		if([binder isKindOfClass:[_NSMultipleValueBinder class]])
		{
			[binder applyToCell:cell inRow:row];			
		}
	}	
}

-(void)_establishBindingsWithDestinationIfUnbound:(id)dest
{
	[[self tableView] _establishBindingsWithDestinationIfUnbound:dest];
}

- (NSSortDescriptor *)sortDescriptorPrototype {
    return [[_sortDescriptorPrototype retain] autorelease];
}

- (void)setSortDescriptorPrototype:(NSSortDescriptor *)value {
    if (_sortDescriptorPrototype != value) {
        [_sortDescriptorPrototype release];
        _sortDescriptorPrototype = [value copy];
    }
}



-(void)_sort
{
	id newDescriptor=[self sortDescriptorPrototype];
	if(!newDescriptor)
		return;

	id allDescs=[[[_tableView sortDescriptors] mutableCopy] autorelease];
	int i;

	for(i=0; i<[allDescs count]; i++)
	{
		id desc=[allDescs objectAtIndex:i];
		if([[newDescriptor key] isEqual:[desc key]])
		{
			if(i==0)
				newDescriptor=[desc reversedSortDescriptor];
			
			[allDescs insertObject:newDescriptor atIndex:0];
			[allDescs removeObject:desc];
			[_tableView setSortDescriptors:allDescs];
			return;
		}
	}

	[allDescs insertObject:newDescriptor atIndex:0];
	[_tableView setSortDescriptors:allDescs];

}

@end

@implementation NSTableColumn (Bindings) 

+(Class)_binderClassForBinding:(id)binding
{
	if([binding isEqual:@"headerTitle"] ||
	   [binding isEqual:@"maxWidth"] ||
	   [binding isEqual:@"minWidth"] ||
	   [binding isEqual:@"width"])
		return [_NSKVOBinder class];
	return [_NSMultipleValueBinder class];
}

- (NSString*)_headerTitle {
    return [_headerCell title];
}
- (void)_setHeaderTitle:(NSString*)title {
    return [_headerCell setTitle:title];
}

@end