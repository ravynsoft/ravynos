/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */

#import <Foundation/NSPrivateDecls.h>
#import "ODQuery.h"
#import "CFODQuery.h"
#import "internal.h"

// private accessor
@interface ODNode (priv)
- (ODNodeRef)__getODNodeRef;
@end

@interface ODQuery (priv)
- (ODQueryRef)__getODQueryRef;
@end

// silence warning
@interface NSObject (DeprecatedODQueryDelegate)
- (void)results:(NSArray *)results forQuery:(ODQuery *)query error:(NSError *)error;
@end

// TODO: predicate support
static void
__delegate_callback(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context)
{
	// inQuery may not really be the original query, so we use userInfo to store the original object
	// TODO: is this true?

	NSAutoreleasePool *pool = [NSAutoreleasePool new];
	ODQuery *odQuery = (ODQuery *)query;
	id delegate = _ODQueryGetDelegate([odQuery __getODQueryRef]); // TODO: which one?

	if (delegate) {
		// need to continue supporting old delegate method (results:forQuery:error:)
		if ([delegate conformsToProtocol:@protocol(ODQueryDelegate)] || [delegate respondsToSelector:@selector(results:forQuery:error:)]) {
			NSArray *results_copy = nil;
			NSError *error_copy = nil;

			if (results)
				results_copy = (NSArray *)CFArrayCreateCopy(NULL, results);
			if (error)
				error_copy = [NSError errorWithDomain:[(NSError *)error domain] code: [(NSError *)error code] userInfo: [(NSError *)error userInfo]];

			if ([delegate conformsToProtocol:@protocol(ODQueryDelegate)])
				[delegate query:odQuery foundResults:[NSMakeCollectable(results_copy) autorelease] error:error_copy];
			else
				[delegate results:[NSMakeCollectable(results_copy) autorelease] forQuery:odQuery error:error_copy];
		}
	}

	[pool drain];
}

static void
__operation_callback(ODQueryRef query, CFArrayRef results, CFErrorRef error, void *context)
{
    NSAutoreleasePool *pool = [NSAutoreleasePool new];
    NSOperationQueue *queue = (NSOperationQueue *)_ODQueryGetOperationQueue(query); // TODO: query or context? see comment above

    // retain/release these. query/context should be safe, since if the query goes away we have bigger problems
    if (results) CFRetain(results);
    if (error) CFRetain(error);

    [queue addOperationWithBlock:^{
        __delegate_callback(query, results, error, context);

        if (results) CFRelease(results);
        if (error) CFRelease(error);
    }];

    [pool drain];
}

@interface NSODQuery : ODQuery
@end
@implementation NSODQuery
extern Boolean _CFIsDeallocating(CFTypeRef);
extern CFTypeRef _CFTryRetain(CFTypeRef);
CF_CLASSIMPLEMENTATION(ODQuery)
- (BOOL)_isDeallocating { return _CFIsDeallocating((CFTypeRef)self); }
- (BOOL)_tryRetain { return _CFTryRetain((CFTypeRef)self) != NULL; }
@end

@implementation ODQuery

- (CFTypeID)_cfTypeID
{
	return ODQueryGetTypeID();
}

- (ODQueryRef)__getODQueryRef
{
	ODQueryRef query;

	if ([self class] == [NSODQuery class]) {
		query = (ODQueryRef)self;
	} else {
		query = _internal;
	}

	return query;
}

- (NSString *)description
{
	return [(NSString *)NSMakeCollectable(CFCopyDescription([self __getODQueryRef])) autorelease];
}

+ (void) initialize
{
	_ODInitialize();
}

+ (id)allocWithZone:(NSZone *)inZone
{
    if ([self class] == [ODQuery class]) {
        return (id) _ODQueryCreate(NULL);
	} else {
        return [super allocWithZone: inZone];
	}
}

+ (ODQuery *)queryWithNode:(ODNode *)node forRecordTypes:(id)recordTypeOrList attribute:(ODAttributeType)attribute
                 matchType:(ODMatchType)matchType queryValues:(id)queryValueOrList 
          returnAttributes:(id)returnAttributeOrList maximumResults:(NSInteger)maximumResults
                     error:(NSError **)error
{
	return [[[self alloc] initWithNode:node forRecordTypes:recordTypeOrList attribute:attribute matchType:matchType queryValues:queryValueOrList returnAttributes:returnAttributeOrList maximumResults:maximumResults error:error] autorelease];
}

- (id)initWithNode:(ODNode *)node forRecordTypes:(id)recordTypeOrList attribute:(ODAttributeType)attribute
         matchType:(ODMatchType)matchType queryValues:(id)queryValueOrList 
  returnAttributes:(id)returnAttributeOrList maximumResults:(NSInteger)maximumResults error:(NSError **)error
{
	CFErrorRef local_error = NULL;
	
	if ([self class] == [NSODQuery class]) {
		_ODQueryInit((ODQueryRef) self, [node __getODNodeRef], (CFTypeRef)recordTypeOrList, attribute, matchType, queryValueOrList, returnAttributeOrList, maximumResults, &local_error);
	} else {
		_internal = ODQueryCreateWithNode(NULL, [node __getODNodeRef], (CFTypeRef)recordTypeOrList, attribute, matchType, queryValueOrList, returnAttributeOrList, maximumResults, &local_error);
	}

	// if we had an error, let's release ourselves and fail
	if (local_error != NULL) {
		[self release];
		self = nil;
		
		if (error != nil) {
			(*error) = [NSMakeCollectable(local_error) autorelease];
		} else {
			CFRelease(local_error);
		}
	}	
	
	return NSMakeCollectable(self);
}

// TODO: predicate stuff (even though it doesn't work)

- (void)dealloc
{
	if ([self class] != [NSODQuery class]) {
		// TODO: should this be CFRelease?
		[(id)_internal release];
	}

	[super dealloc];
}

- (NSArray *)resultsAllowingPartial:(BOOL)allowPartial error:(NSError **)error
{
	NSArray *results = (NSArray *)ODQueryCopyResults([self __getODQueryRef], allowPartial, (CFErrorRef *)error);
	if (error) [NSMakeCollectable(*error) autorelease];
	return [NSMakeCollectable(results) autorelease];
}

- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode
{
	ODQueryScheduleWithRunLoop([self __getODQueryRef], [runLoop getCFRunLoop], (CFStringRef)mode);
}

- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode
{
	ODQueryUnscheduleFromRunLoop([self __getODQueryRef], [runLoop getCFRunLoop], (CFStringRef)mode);
}

- (id)delegate
{
	return _ODQueryGetDelegate([self __getODQueryRef]);
}

- (void)setDelegate:(id)delegate
{
	_ODQuerySetDelegate([self __getODQueryRef], delegate);
	ODQuerySetCallback([self __getODQueryRef], __delegate_callback, self);
}

- (void)synchronize
{
	ODQuerySynchronize([self __getODQueryRef]);
}

- (NSOperationQueue *)operationQueue
{
	return _ODQueryGetOperationQueue([self __getODQueryRef]);
}

- (void)setOperationQueue:(NSOperationQueue *)queue
{
	dispatch_queue_t dq = dispatch_queue_create(NULL, NULL);

	_ODQuerySetOperationQueue([self __getODQueryRef], queue);
	ODQuerySetCallback([self __getODQueryRef], __operation_callback, self);
	ODQuerySetDispatchQueue([self __getODQueryRef], dq);

	// retained by underlying ODQuery object
	dispatch_release(dq);
}

- (id)copyWithZone:(NSZone *)zone
{
	return [self retain];
}

@end
