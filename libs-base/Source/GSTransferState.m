#import "GSTransferState.h"
#import "GSURLSessionTaskBodySource.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSCharacterSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSFileHandle.h"
#import "Foundation/NSError.h"
#import "Foundation/NSURL.h"
#import "Foundation/NSURLError.h"
#import "Foundation/NSURLResponse.h"
#import "Foundation/NSURLSession.h"

#define GS_DELIMITERS_CR 0x0d
#define GS_DELIMITERS_LR 0x0a

@implementation GSParsedResponseHeader

- (instancetype) init
{
  if (nil != (self = [super init]))
    {
      _lines = [[NSMutableArray alloc] init];
      _type = GSParsedResponseHeaderTypePartial;
    }

  return self;
}

- (void) dealloc
{
  DESTROY(_lines);
  [super dealloc];
}

- (GSParsedResponseHeaderType) type
{
  return _type;
}

- (void) setType: (GSParsedResponseHeaderType)type
{
  _type = type;
}

- (void) setLines: (NSArray*)lines
{
  ASSIGN(_lines, lines);
}

- (instancetype) byAppendingHeaderLine: (NSData*)data 
{
  NSUInteger length = [data length];

  if (length >= 2) 
    {
      uint8_t last2;
      uint8_t last1;

      [data getBytes: &last2 range: NSMakeRange(length - 2, 1)];
      [data getBytes: &last1 range: NSMakeRange(length - 1, 1)];

      if (GS_DELIMITERS_CR == last2 && GS_DELIMITERS_LR == last1) 
        {
          NSData *lineBuffer;
          NSString *line;

          lineBuffer = [data subdataWithRange: NSMakeRange(0, length - 2)];
          line = AUTORELEASE([[NSString alloc] initWithData: lineBuffer 
                                                   encoding: NSUTF8StringEncoding]);

          if (nil == line)
            {
              return nil;
            } 
          
          return [self _byAppendingHeaderLine: line];
        }
    }

  return nil;
}

- (NSHTTPURLResponse*) createHTTPURLResponseForURL: (NSURL*)URL 
{
  NSArray       *tail;
  NSArray       *startLine;
  NSDictionary  *headerFields;
  NSString      *head;
  NSString      *s, *v;

  head = [_lines firstObject];
  if (nil == head) 
    {
      return nil;
    }
  if ([_lines count] == 0)
    {
      return nil;
    } 
  
  tail = [_lines subarrayWithRange: NSMakeRange(1, [_lines count] - 1)];

  startLine = [self statusLineFromLine: head];
  if (nil == startLine) 
    {
      return nil;
    }

  headerFields = [self createHeaderFieldsFromLines: tail];

  v = [startLine objectAtIndex: 0];
  s = [startLine objectAtIndex: 1];

  return AUTORELEASE([[NSHTTPURLResponse alloc] initWithURL: URL
                                                 statusCode: [s integerValue]
                                                HTTPVersion: v
                                               headerFields: headerFields]);
}

- (NSArray*) statusLineFromLine: (NSString*)line 
{
  NSArray    *a;
  NSString   *s;
  NSInteger  status;

  a = [line componentsSeparatedByString: @" "];
  if ([a count] < 3) 
    {
      return nil;
    }

  s = [a objectAtIndex: 1];

  status = [s integerValue];
  if (status >= 100 && status <= 999) 
    {
      return a;
    } 
  else 
    {
      return nil;
    }
}

- (NSDictionary *) createHeaderFieldsFromLines: (NSArray *)lines 
{
  NSMutableDictionary *headerFields = nil;
  NSEnumerator        *e;
  NSString            *line;

  e = [_lines objectEnumerator];
  while (nil != (line = [e nextObject]))
    {
      NSRange        r;
      NSString       *head;
      NSString       *tail;
      NSCharacterSet *set;
      NSString       *key;
      NSString       *value;
      NSString       *v;

      r = [line rangeOfString: @":"];
      if (r.location != NSNotFound) 
        {
          head = [line substringToIndex: r.location];
          tail = [line substringFromIndex: r.location + 1];
          set = [NSCharacterSet whitespaceAndNewlineCharacterSet];
          key = [head stringByTrimmingCharactersInSet: set];
          value = [tail stringByTrimmingCharactersInSet: set];
          if (nil != key && nil != value) 
            {
              if (nil == headerFields) 
                {
                  headerFields = [NSMutableDictionary dictionary];
                }
              if (nil != [headerFields objectForKey: key]) 
                {
                  v = [NSString stringWithFormat:@"%@, %@", 
                    [headerFields objectForKey: key], value];
                  [headerFields setObject: v forKey: key];
                } 
              else 
                {
                  [headerFields setObject: value forKey: key];
                }
            }
        } 
      else 
        {
          continue;
        }
    }
  
  return AUTORELEASE([headerFields copy]);
}

- (instancetype) _byAppendingHeaderLine: (NSString*)line 
{
  GSParsedResponseHeader *header;

  header = AUTORELEASE([[GSParsedResponseHeader alloc] init]);

  if ([line length] == 0) 
    {
      switch (_type) 
        {
          case GSParsedResponseHeaderTypePartial: 
            {
              [header setType: GSParsedResponseHeaderTypeComplete];
              [header setLines: _lines];

              return header;
            }
          case GSParsedResponseHeaderTypeComplete:
            return header;
      }
    } 
  else 
    {
      NSMutableArray *lines = [[self partialResponseHeader] mutableCopy];
      
      [lines addObject:line];

      [header setType: GSParsedResponseHeaderTypePartial];
      [header setLines: lines];

      RELEASE(lines);

      return header;
  }
}

- (NSArray*) partialResponseHeader 
{
  switch (_type) 
    {
      case GSParsedResponseHeaderTypeComplete:
        return [NSArray array];

      case GSParsedResponseHeaderTypePartial:
        return _lines;
    }
}

@end

@implementation GSDataDrain

- (void) dealloc
{
  DESTROY(_data);
  DESTROY(_fileURL);
  DESTROY(_fileHandle);
  [super dealloc];
}

- (GSDataDrainType) type
{
  return _type;
}

- (void) setType: (GSDataDrainType)type
{
  _type = type;
}

- (NSData*) data
{
  return _data;
}

- (void) setData: (NSData*)data
{
  ASSIGN(_data, data);
}

- (NSURL*) fileURL
{
  return _fileURL;
}

- (void) setFileURL: (NSURL*)url
{
  ASSIGN(_fileURL, url);
}

- (NSFileHandle*) fileHandle
{
  return _fileHandle;
}

- (void) setFileHandle: (NSFileHandle*)handle
{
  ASSIGN(_fileHandle, handle);
}

@end

@implementation GSTransferState

- (instancetype) initWithURL: (NSURL*)url
               bodyDataDrain: (GSDataDrain*)bodyDataDrain
{
  return [self initWithURL: url
      parsedResponseHeader: nil
                  response: nil
                bodySource: nil
             bodyDataDrain: bodyDataDrain];
}

- (instancetype) initWithURL: (NSURL*)url
               bodyDataDrain: (GSDataDrain*)bodyDataDrain
                  bodySource: (id<GSURLSessionTaskBodySource>)bodySource
{
  return [self initWithURL: url
      parsedResponseHeader: nil
                  response: nil
                bodySource: bodySource
             bodyDataDrain: bodyDataDrain];
}

- (instancetype) initWithURL: (NSURL*)url
        parsedResponseHeader: (GSParsedResponseHeader*)parsedResponseHeader
                    response: (NSURLResponse*)response
                  bodySource: (id<GSURLSessionTaskBodySource>)bodySource
               bodyDataDrain: (GSDataDrain*)bodyDataDrain
{
  if (nil != (self = [super init]))
    {
      ASSIGN(_url, url);
      if (nil != parsedResponseHeader)
        {
          ASSIGN(_parsedResponseHeader, parsedResponseHeader);
        }
      else
        {
          _parsedResponseHeader = [[GSParsedResponseHeader alloc] init];
        }  
      ASSIGN(_response, response);
      ASSIGN(_requestBodySource, bodySource);
      ASSIGN(_bodyDataDrain, bodyDataDrain);
    }
  
  return self;
}

- (void) dealloc
{
  DESTROY(_url);
  DESTROY(_parsedResponseHeader);
  DESTROY(_response);
  DESTROY(_requestBodySource);
  DESTROY(_bodyDataDrain);
  [super dealloc];
}

- (instancetype) byAppendingBodyData: (NSData*)bodyData 
{
  switch ([_bodyDataDrain type]) 
    {
      case GSDataDrainInMemory: 
        {
          NSMutableData    *data;
          GSDataDrain      *dataDrain;
          GSTransferState  *ts;
          
          data = [_bodyDataDrain data] ? 
            AUTORELEASE([[_bodyDataDrain data] mutableCopy]) 
            : [NSMutableData data];
          
          [data appendData: bodyData];
          dataDrain = AUTORELEASE([[GSDataDrain alloc] init]);
          [dataDrain setType: GSDataDrainInMemory];
          [dataDrain setData: data];

          ts = [[GSTransferState alloc] initWithURL: _url
                               parsedResponseHeader: _parsedResponseHeader
                                           response: _response
                                         bodySource: _requestBodySource
                                      bodyDataDrain: dataDrain];
          return AUTORELEASE(ts);
        }
      case GSDataDrainTypeToFile: 
        {
          NSFileHandle *fileHandle;
          
          fileHandle = [_bodyDataDrain fileHandle];
          [fileHandle seekToEndOfFile];
          [fileHandle writeData: bodyData];

          return self;
        }
      case GSDataDrainTypeIgnore:
        return self;
    }
}

- (instancetype) byAppendingHTTPHeaderLineData: (NSData*)data 
                                         error: (NSError**)error 
{
  GSParsedResponseHeader *h;
  
  h = [_parsedResponseHeader byAppendingHeaderLine: data];
  if (nil == h) 
    {
      if (error != NULL) 
        {
          *error = [NSError errorWithDomain: NSURLErrorDomain
                                       code: -1
                                   userInfo: nil];
        }
      
      return nil;
  }

  if ([h type] == GSParsedResponseHeaderTypeComplete) 
    {
      NSHTTPURLResponse       *response;
      GSParsedResponseHeader  *ph;
      GSTransferState         *ts;
      
      response = [h createHTTPURLResponseForURL: _url];
      if (nil == response) 
        {
          if (error != NULL) 
            {
              *error = [NSError errorWithDomain: NSURLErrorDomain
                                           code: -1
                                       userInfo: nil];
            }

          return nil;
      }

      ph = AUTORELEASE([[GSParsedResponseHeader alloc] init]);
      ts = [[GSTransferState alloc] initWithURL: _url
                           parsedResponseHeader: ph
                                       response: response
                                     bodySource: _requestBodySource
                                  bodyDataDrain: _bodyDataDrain];
      return AUTORELEASE(ts);
    } 
  else 
    {
      GSTransferState *ts;
      
      ts = [[GSTransferState alloc] initWithURL: _url
                           parsedResponseHeader: h
                                       response: nil
                                     bodySource: _requestBodySource
                                  bodyDataDrain: _bodyDataDrain];
      return AUTORELEASE(ts);
  }
}

- (BOOL) isHeaderComplete 
{
  return _response != nil;
}

- (NSURLResponse*) response
{
  return _response;
}

- (void) setResponse: (NSURLResponse*)response
{
  ASSIGN(_response, response);
}

- (id<GSURLSessionTaskBodySource>) requestBodySource
{
  return _requestBodySource;
}

- (GSDataDrain*) bodyDataDrain
{
  return _bodyDataDrain;
}

- (NSURL*) URL
{
  return _url;
}

@end
