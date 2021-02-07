#ifndef	INCLUDED_GSURLSESSIONTASKBODY_H
#define	INCLUDED_GSURLSESSIONTASKBODY_H

#import "common.h"

@class	NSData;
@class	NSError;
@class	NSInputStream;
@class	NSNumber;
@class	NSURL;

typedef NS_ENUM(NSUInteger, GSURLSessionTaskBodyType) {
    GSURLSessionTaskBodyTypeNone,
    GSURLSessionTaskBodyTypeData,
    // Body data is read from the given file URL
    GSURLSessionTaskBodyTypeFile,
    // Body data is read from the given input stream
    GSURLSessionTaskBodyTypeStream,
};

@interface GSURLSessionTaskBody : NSObject
{
  GSURLSessionTaskBodyType  _type;
  NSData                    *_data;
  NSURL                     *_fileURL;
  NSInputStream             *_inputStream;
}

- (instancetype) init;
- (instancetype) initWithData: (NSData*)data;
- (instancetype) initWithFileURL: (NSURL*)fileURL;
- (instancetype) initWithInputStream: (NSInputStream*)inputStream;

- (GSURLSessionTaskBodyType) type;

- (NSData*) data;

- (NSURL*) fileURL;

- (NSInputStream*) inputStream;

- (NSNumber*) getBodyLengthWithError: (NSError**)error;

@end

#endif
