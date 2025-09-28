/* Copyright(c)2006 Chris B. Vetter
  
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#import <Foundation/NSObject.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDate.h>
#import <Foundation/NSArray.h>

@class NSInputStream, NSOutputStream, NSRunLoop, NSSelectInputSource, NSTimer, NSMutableDictionary;

typedef enum {
    NSNetServicesUnknownError = -72000L,
    NSNetServicesCollisionError = -72001L,
    NSNetServicesNotFoundError = -72002L,
    NSNetServicesActivityInProgress = -72003L,
    NSNetServicesBadArgumentError = -72004L,
    NSNetServicesCancelledError = -72005L,
    NSNetServicesInvalidError = -72006L,
    NSNetServicesTimeoutError = -72007L
} NSNetServicesError;

enum {
    NSNetServiceNoAutoRename = 0x01,
};

typedef NSUInteger NSNetServiceOptions;

FOUNDATION_EXPORT NSString *const NSNetServicesErrorCode;
FOUNDATION_EXPORT NSString *const NSNetServicesErrorDomain;

@interface NSNetService : NSObject {
    NSString *_domain;
    NSString *_type;
    NSString *_name;
    uint16_t _port;
    id _delegate;

    NSString *_host;
    NSMutableArray *_addresses;
    NSData *_txtRecord;

    struct bonjour_DNSService *_netService;
    NSSelectInputSource *_inputSource;

    NSTimer *_resolverTimeout;

    int _interfaceIndex;

    BOOL _isPublishing, _isMonitoring;

    NSInputStream *_inputStream;
    NSOutputStream *_outputStream;
}

+ (NSData *)dataFromTXTRecordDictionary:(NSDictionary *)txtDictionary;
+ (NSDictionary *)dictionaryFromTXTRecordData:(NSData *)txtData;

- initWithDomain:(NSString *)domain type:(NSString *)type name:(NSString *)name;
- initWithDomain:(NSString *)domain type:(NSString *)type name:(NSString *)name port:(int)port;

- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

- (void)publishWithOptions:(NSNetServiceOptions)options;
- (void)publish;

- (void)resolve;
- (void)resolveWithTimeout:(NSTimeInterval)timeout;

- (void)stop;

- (void)startMonitoring;
- (void)stopMonitoring;

- delegate;
- (void)setDelegate:delegate;

- (NSArray *)addresses;
- (NSString *)domain;
- (NSString *)hostName;
- (NSString *)name;
- (NSString *)type;
- (int)port;

- (NSString *)protocolSpecificInformation;
- (void)setProtocolSpecificInformation:(NSString *)specificInformation;

- (NSData *)TXTRecordData;
- (BOOL)setTXTRecordData:(NSData *)recordData;

- (BOOL)getInputStream:(NSInputStream **)inputStream
          outputStream:(NSOutputStream **)outputStream;

@end

@interface NSObject (NSNetServiceDelegateMethods)

- (void)netServiceWillPublish:(NSNetService *)sender;
- (void)netServiceDidPublish:(NSNetService *)sender;
- (void)netService:(NSNetService *)sender
     didNotPublish:(NSDictionary *)errorDict;
- (void)netServiceWillResolve:(NSNetService *)sender;
- (void)netServiceDidResolveAddress:(NSNetService *)sender;
- (void)netService:(NSNetService *)sender
     didNotResolve:(NSDictionary *)errorDict;
- (void)netServiceDidStop:(NSNetService *)sender;
- (void)netService:(NSNetService *)sender
    didUpdateTXTRecordData:(NSData *)data;

@end

@protocol NSNetServiceDelegate <NSObject>

@end

@class NSRunLoop, NSDictionary, NSNetService, NSSelectInputSource, NSMutableDictionary;

@interface NSNetServiceBrowser : NSObject {
  @private
    struct bonjour_DNSService *_netServiceBrowser;
    NSSelectInputSource *_inputSource;
    id _delegate;

    NSMutableDictionary *_services;
    // List of found services.
    // Key is <_name_type_domain> and value is an initialized NSNetService.

    int _interfaceIndex;
}

- init;

- delegate;
- (void)setDelegate:delegate;

- (void)scheduleInRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;
- (void)removeFromRunLoop:(NSRunLoop *)runLoop forMode:(NSString *)mode;

- (void)searchForAllDomains;
- (void)searchForBrowsableDomains;
- (void)searchForRegistrationDomains;

- (void)searchForServicesOfType:(NSString *)serviceType inDomain:(NSString *)domainName;

- (void)stop;

@end

@interface NSObject (NSNetServiceBrowserDelegate)

- (void)netServiceBrowserWillSearch:(NSNetServiceBrowser *)netServiceBrowser;
- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didNotSearch:(NSDictionary *)errorDict;
- (void)netServiceBrowserDidStopSearch:(NSNetServiceBrowser *)netServiceBrowser;
- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didFindDomain:(NSString *)domainString moreComing:(BOOL)moreComing;
- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didRemoveDomain:(NSString *)domainString moreComing:(BOOL)moreComing;
- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didFindService:(NSNetService *)netService moreComing:(BOOL)moreComing;
- (void)netServiceBrowser:(NSNetServiceBrowser *)netServiceBrowser didRemoveService:(NSNetService *)netService moreComing:(BOOL)moreComing;

@end
