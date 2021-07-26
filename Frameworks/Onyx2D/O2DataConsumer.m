#import <Onyx2D/O2DataConsumer.h>
#import <Onyx2D/O2Exceptions.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSFileManager.h>

@implementation O2DataConsumer

static size_t O2DataConsumerDataPutBytesCallback(void *info, const void *buffer, size_t count)
{
	NSMutableData *data = (NSMutableData *)info;
	[data appendBytes:buffer length:count];
	return count;
}

static void O2DataConsumerDataReleaseInfoCallback(void *info)
{
	NSMutableData *data = (NSMutableData *)info;
	[data release];
}

static size_t O2DataConsumerFilePutBytesCallback(void *info, const void *buffer, size_t count)
{
	NSFileHandle *handle = (NSFileHandle *)info;
	NSData *data = [NSData dataWithBytes:buffer length:count];
	@try {
		[handle writeData:data];
	}
	@catch (NSException * e) {
		NSLog(@"%@: writing error : %@", handle, e);
		count = 0;
	}
	@finally {
	}
	return count;
}

static void O2DataConsumerFileReleaseInfoCallback(void *info)
{
	 NSFileHandle *handle = (NSFileHandle *)info;
	 [handle closeFile];
	 [handle release];
}


-initWithInfo:(void *)info callbacks:(const O2DataConsumerCallbacks *)callbacks 
{
	if ((self = [super init])) {
		_info = info;
		if (callbacks) {
			_callbacks = *callbacks;
		}
	}
	return self;
}

-initWithMutableData:(NSMutableData *)data 
{
	data=[data retain]; // Will be released by the release callback
	O2DataConsumerCallbacks callbacks = { 
		O2DataConsumerDataPutBytesCallback,
		O2DataConsumerDataReleaseInfoCallback
	};
	return [self initWithInfo:data callbacks:&callbacks];
}

-initWithURL:(NSURL *)url 
{
	if ([url isFileURL]) {
		NSString *path = [url path];
		// Create the file and get an handle on it
		[[NSFileManager defaultManager] createFileAtPath:path contents:nil attributes:nil];
		NSFileHandle *fileHandle = [NSFileHandle fileHandleForWritingAtPath:path];
		
		O2DataConsumerCallbacks callbacks = { 
			O2DataConsumerFilePutBytesCallback,
			O2DataConsumerFileReleaseInfoCallback
		};
		[fileHandle retain]; // Will be released by the release callback
		return [self initWithInfo:fileHandle callbacks:&callbacks];
	} else {
		NSLog(@"O2DataConsumerCreateWithURL: %@ is not a file URL", url);
		[self release];
		return nil;
	}
	return self;
}

-(void)dealloc {
    if (_callbacks.releaseConsumer) {
		_callbacks.releaseConsumer(_info);
	}
   [super dealloc];
}

- (size_t)putBytes:(const void *)buffer count:(size_t)count
{
	if (_callbacks.putBytes) {
		count = _callbacks.putBytes(_info, buffer, count);
	}
	return count;
}

// Only works with NSData based consumer
-(NSMutableData *)mutableData {
	return (NSMutableData *)_info;
}

O2DataConsumerRef O2DataConsumerCreate(void *info, const O2DataConsumerCallbacks *callbacks)
{
	return [[O2DataConsumer alloc] initWithInfo:info callbacks:callbacks];
}

O2DataConsumerRef O2DataConsumerCreateWithCFData(CFMutableDataRef data) {
   return [[O2DataConsumer alloc] initWithMutableData:(NSMutableData *)data];
}

O2DataConsumerRef O2DataConsumerCreateWithURL(CFURLRef url) {
	return [[O2DataConsumer alloc] initWithURL:(NSURL *)url];
}

O2DataConsumerRef O2DataConsumerRetain(O2DataConsumerRef self) {
   return (self!=NULL)?(O2DataConsumerRef)CFRetain(self):NULL;
}

void O2DataConsumerRelease(O2DataConsumerRef self) {
   if(self!=NULL)
    CFRelease(self);
}

size_t O2DataConsumerPutBytes(O2DataConsumerRef self,const void *buffer,size_t count) {
	return [self putBytes:buffer count:count];
}

@end

