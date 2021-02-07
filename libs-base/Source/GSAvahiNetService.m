/* Concrete NSNetService subclass using the avahi API.
   Copyright (C) 2010 Free Software Foundation, Inc.

   Written by:  Niels Grewe <niels.grewe@halbordnung.de>
   Date: March 2010

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
   */

#import "GSNetServices.h"
#import "GSAvahiClient.h"
#import "GNUstepBase/NSNetServices+GNUstepBase.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSData.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSDebug.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSException.h"
#import "GNUstepBase/GSObjCRuntime.h"
#import <GNUstepBase/NSStream+GNUstepBase.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/strlst.h>
#include <avahi-common/alternative.h>
#include <avahi-client/lookup.h>
#include <avahi-client/publish.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Specifies the time we allow for consecutive updates on a specific record type
 * to accumulate before the delegate is notified. Avahi seems to run some
 * internal timers set to one second, so we choose a slightly greater interval
 * to be sure that they did already fire in a simple remove/insert sequence.
 */
#define BROWSER_UPDATE_INTERVAL 1.1

/* Suggested by the mDNS RFC, but we only use the latter since this is
 * NSNet_Service_ and not NSNet_Host_!
 */
#define DEFAULT_HOST_RECORD_TTL 120
#define DEFAULT_OTHER_RECORD_TTL 4500


@interface GSAvahiNetService (GSAvahiNetServicePrivate)
- (void) newData: (NSData*)data
       forRRCode: (NSUInteger)rrCode;

- (void) removedData: (NSData*)data
           forRRCode: (NSUInteger)rrCode;

- (void) allForNowForRRCode: (NSUInteger)rrCode;

- (void) entryGroup: (AvahiEntryGroup*)group
       enteredState: (AvahiEntryGroupState)state;

- (void) avahiResolver: (AvahiServiceResolver*)resolver
  foundServiceWithName: (NSString*)name
                  type: (NSString*)type
                domain: (NSString*)domain
              hostName: (NSString*)hostName
               address: (NSData*)address
                  port: (NSInteger)port
             txtRecord: (NSData*)txtData
               ifIndex: (AvahiIfIndex)ifIndex
              protocol: (AvahiProtocol)protocol;

- (void) handleError: (int)error
           forRRCode: (int)rrcode;

- (void) handleError: (int)error;


// Methods from the GSAvahiClient behaviour:

- (id) avahiClientInit;

- (void) avahiClientHandleError: (int)error;

- (void) avahiClientDealloc;

- (NSDictionary*) errorDictWithErrorCode: (int)error;
@end

/**
 * Return the NSString corresponding to the specified resource record code.
 */
static NSString*
NSStringFromRRCode(NSUInteger code)
{
  /* NOTE: I still have a comprehensive plist for this in UnboundKit. I'll
   * import it eventually, but for now, we need just a few rrCodes;
   */
  switch (code)
    {
      case 0x01:
        return @"A";
      case 0x02:
        return @"NS";
      case 0x1C:
        return @"AAAA";
      case 0x05:
        return @"CNAME";
      case 0x06:
        return @"SOA";
      case 0x0C:
        return @"PTR";
      case 0x0D:
        return @"HINFO";
      case 0x0F:
         return @"MX";
       case 0x10:
         return @"TXT";
       case 0x21:
         return @"SRV";
       case 0x0A:
         return @"NULL";
       default:
         return nil;
    }
  return nil;
}


/**
 * Return the resource record code for the specified resource record name.
 * FIXME: This is stupid and lame, I'll make it into a static NSDictionary soon:
 */
static NSUInteger
RRCodeFromNSString(NSString* name)
{
  if ([name isEqualToString: @"A"])
    {
      return 0x01;
    }
  else if ([name isEqualToString: @"AAAA"])
    {
       return 0x1C;
    }
  else if ([name isEqualToString: @"TXT"])
    {
       return 0x10;
    }
  else if ([name isEqualToString: @"NULL"])
    {
      return 0x0A;
    }
  else if ([name isEqualToString: @"NS"])
    {
      return 0x02;
    }
  else if ([name isEqualToString: @"CNAME"])
    {
      return 0x05;
    }
  else if ([name isEqualToString: @"SOA"])
    {
       return 0x06;
    }
  else if ([name isEqualToString: @"PTR"])
    {
       return 0x0C;
    }
  else if ([name isEqualToString: @"HINFO"])
    {
      return 0x0D;
    }
  else if ([name isEqualToString: @"MX"])
    {
      return 0x0F;
    }
  else if ([name isEqualToString: @"SRV"])
    {
      return 0x21;
    }
  return 0x00;
}

/**
 * Returns the UTF8 string or NULL, if the string is empty.
 */
static inline const char*
StringOrNullIfEmpty(NSString *domain)
{
  if ((domain != nil) && ![domain isEqualToString: @""])
    {
      return [domain UTF8String];
    }
  return NULL;
}

/**
 * Serialize an AvahiStringList into an NSData object.
 */
static NSData*
NSDataFromAvahiStringList(AvahiStringList* list)
{
  /* NOTE: The record is at most 255 bytes, but should we really allocate
   * 255 bytes on the stack?
   */
  char *buffer = NULL;
  size_t len = 0;
  NSData *data = nil;

  if (list == NULL)
    {
      return nil;
    }

  /* The avahi documentation on avahi_string_list_serialize() doesn't tell us
   * this, but called with the arguments below, it just returns the size:
   */
  len = avahi_string_list_serialize(list, NULL, 0);
  if (len == 0)
    {
      return nil;
    }

  buffer = malloc(len);
  // It's better to zero the buffer: memset(buffer, '\0', len);
  if (buffer == NULL)
    {
      // Should we raise an exception?
      NSDebugLog(@"Couldn't allocate %"PRIuPTR" bytes for txt record",
	(uintptr_t)len);
      return nil;
    }

  /* Proper serialization of the string list, we ignore the returned length
   * since we already know it.
   */
  avahi_string_list_serialize(list, buffer, len);
  data = [NSData dataWithBytes: buffer
                        length: len];
  free(buffer);
  return data;
}


/**
 * Try to parse the <code>txtData</code> into an AvahiStringList. Returns 0 on
 * success.
 */
static int
GSAvahiStringListFromNSData(NSData *txtData, AvahiStringList **list)
{
  if (nil != txtData)
    {
      const void *bytes = [txtData bytes];
      size_t len = [txtData length];
      return avahi_string_list_parse(bytes, len, list);
    }
  return -1;
}

/**
 * Creates an <code>NSData</code> object (wrapping a <code>struct
 * sockaddr_storage</code>) from the necessary Avahi data structures.
 */
static NSData*
NSDataFromAvahiAddressPortAndInterface(const AvahiAddress *addr,
  uint16_t port,
  AvahiIfIndex iface)
{

  struct sockaddr_storage s;
  size_t s_len = sizeof(struct sockaddr_storage);
  if ((addr->proto != AVAHI_PROTO_INET) && (addr->proto != AVAHI_PROTO_INET6))
    {
      //We can't fill the struct if AVAHI_PROTO_UNSPEC
      return nil;
    }
  // POSIX says we shall zero the structure:
  memset(&s, '\0', s_len);

  if (addr->proto == AVAHI_PROTO_INET)
    {
      struct sockaddr_in *s4 = (struct sockaddr_in*)&s;
      // The address is already in network byte-order.
      struct in_addr a;
      a.s_addr = addr->data.ipv4.address;
      s4->sin_family = AF_INET;
      s4->sin_port = htons(port);
      s4->sin_addr = a;
    }
  else if (addr->proto == AVAHI_PROTO_INET6)
    {
      struct sockaddr_in6 *s6 = (struct sockaddr_in6*)&s;
      struct in6_addr a;
      // Copy the address from the avahi structure to in6_addr:
      memcpy(&(a.s6_addr), &(addr->data.ipv6.address), 16);
      s6->sin6_family = AF_INET6;
      s6->sin6_port = htons(port);
      s6->sin6_addr = a;
      if (IN6_IS_ADDR_LINKLOCAL(&a))
        {
           // For a link-local address set the interface index
           // NOTE: These are implementation-defined but Avahi doesn't seem to
           // mangle them.
           s6->sin6_scope_id = iface;
        }
      else
        {
           // FIXME: Do nothing. The structure was zeroed out. Please pray
           // that every sensible implementation has 0 for the global
           // scope. (Most of the time, people will be announcing link-local
           // addresses, though)
        }
    }
  else
    {
      // Shouldn't happen:
      return nil;
    }
  return [NSData dataWithBytes: &s
                        length: s_len];
}

/**
 * Creates an <code>NSData</code> object wrapping a <code>struct
 * sockaddr_storage</code> from an <code>NSData</code> object that contains a
 * plain IPv4 or IPv6 address in network byte-order along with the necessary
 * <code>port</code>, address <code>family</code> and interface
 * <code>index</code> information.
 */
static NSData*
NSDataWithAddrDataPortFamilyAndIface(NSData *old,
  NSUInteger port,
  AvahiProtocol family,
  AvahiIfIndex index)
{
  AvahiAddress addr;
  const void *addrBytes;
  if (old == nil)
    {
      return nil;
    }
  addr.proto = family;
  addrBytes = [old bytes];
  //Copy the value into the data field:
  if (family == AVAHI_PROTO_INET)
    {
      memcpy(addr.data.data, addrBytes, 4);
    }
  else if (family == AVAHI_PROTO_INET6)
    {
      memcpy(addr.data.data, addrBytes, 16);
    }
  else
    {
      return nil;
    }
  return NSDataFromAvahiAddressPortAndInterface(&addr, port, index);
}

/**
 * This callback function will be called by the Avahi layer for resolver events.
 */
static void
GSAvahiServiceResolverEvent(
  AvahiServiceResolver *resolver,
  AvahiIfIndex interface,
  AvahiProtocol protocol,
  AvahiResolverEvent event,
  const char *name,
  const char *type,
  const char *domain,
  const char *hostName,
  const AvahiAddress *addr,
  uint16_t port,
  AvahiStringList *txtRecord,
  AvahiLookupResultFlags flags,
  void *userInfo
)
{
  GSAvahiNetService *service = nil;
  if (NULL == resolver)
    {
      NSDebugLog(@"NULL pointer to AvahiServiceResolver.");
      return;
    }
  if (NULL == userInfo)
    {
      NSDebugLog(@"NULL pointer to NSNetService.");
      return;
    }
  service = (GSAvahiNetService*)userInfo;

  switch (event)
    {
      case AVAHI_RESOLVER_FAILURE:
        [service handleError:
	  avahi_client_errno(avahi_service_resolver_get_client(resolver))];
        break;

      case AVAHI_RESOLVER_FOUND:

        [service avahiResolver: resolver
          foundServiceWithName: NSStringIfNotNull(name)
	  type: NSStringIfNotNull(type)
	  domain: GSNetServiceDotTerminatedNSStringFromString(domain)
	  hostName: NSStringIfNotNull(hostName)
	  address: NSDataFromAvahiAddressPortAndInterface(addr, port, interface)
	  port: (NSInteger)port
	  txtRecord: NSDataFromAvahiStringList(txtRecord)
	  ifIndex: interface
	  protocol: protocol];
        break;
    }
}

/**
 * This callback function will be called by the Avahi layer on events for a
 * record browser.
 */
static void
GSAvahiRecordBrowserEvent(
  AvahiRecordBrowser *browser,
  AvahiIfIndex interface,
  AvahiProtocol protocol,
  AvahiBrowserEvent event,
  const char *name,
  uint16_t dnsClass,
  uint16_t type,
  const void *rdata,
  size_t size,
  AvahiLookupResultFlags flags,
  void *userInfo)
{
  GSAvahiNetService *service = nil;
  NSData *recordData = nil;
  if (NULL == browser)
    {
      NSDebugLog(@"NULL pointer to AvahiServiceResolver.");
      return;
    }
  if (NULL == userInfo)
    {
      NSDebugLog(@"NULL pointer to NSNetService.");
      return;
    }
  service = (GSAvahiNetService*)userInfo;

  if (type == 0 && (event != AVAHI_BROWSER_FAILURE))
    {
      [service handleError: NSNetServicesInvalidError];
      return;
    }

  if (rdata != NULL)
    {
      recordData = [NSData dataWithBytes: rdata
                                  length: size];
    }
  switch (event)
    {
      case AVAHI_BROWSER_NEW:
        [service newData: recordData
               forRRCode: type];
        break;

      case AVAHI_BROWSER_REMOVE:
        [service removedData: recordData
                   forRRCode: type];
        break;

      case AVAHI_BROWSER_FAILURE:
        [service handleError:
	  avahi_client_errno(avahi_record_browser_get_client(browser))
                   forRRCode: type];
        break;

      case AVAHI_BROWSER_CACHE_EXHAUSTED: // Not interesting
        break;

      case AVAHI_BROWSER_ALL_FOR_NOW:
        [service allForNowForRRCode: type];
        break;
    }
}

/**
 * This callback function will be called by the Avahi layer to notify about
 * state changes in an entry group.
 */
static void
GSAvahiEntryGroupStateChanged(AvahiEntryGroup *group,
  AvahiEntryGroupState state,
  void *userInfo)
{
  if (NULL == group)
    {
      NSDebugLog(@"NULL pointer to AvahiEntryGroup.");
      return;
    }
  if (NULL == userInfo)
    {
      NSDebugLog(@"NULL pointer to NSNetService.");
      return;
    }
  [(GSAvahiNetService*)userInfo entryGroup: group
                              enteredState: state];
}

@implementation GSAvahiNetService

+ (void) initialize
{
  if (self == [GSAvahiNetService class])
    {
      GSObjCAddClassBehavior(self, [GSAvahiClient class]);
    }
}

+ (NSData *) dataFromTXTRecordDictionary: (NSDictionary *) txtDictionary
{
  // This NULL avahi-string list is the terminating element of the linked
  // list.
  AvahiStringList *list = NULL;
  NSArray *keys = [txtDictionary allKeys];
  NSData *data = nil;

  FOR_IN(NSString*, key, keys)
    {
      id value = [txtDictionary objectForKey: key];
      if ([value isKindOfClass: [NSString class]])
        {
          list = avahi_string_list_add_pair(list,
	    [key UTF8String], [value UTF8String]);
        }
      else if ([value isKindOfClass: [NSNumber class]])
        {
          list = avahi_string_list_add_pair(list,
	    [key UTF8String], [[(NSNumber*)value stringValue] UTF8String]);
        }
      else if ([value isKindOfClass: [NSData class]])
        {
          NSUInteger len = [value length];
          /* TXT record can only contain 256bytes, so there is no point in
           * handling NSData values with len > 255.
	   */
          if (len <= 255)
            {
              list = avahi_string_list_add_pair_arbitrary(list,
		[key UTF8String], [value bytes], len);
            }
        }
      else
        {
          /* We cannot handle any other type of value. We thus free the list
           * and make sure we fail the subsequent assertion (because this is
           * what the Apple documentation will do.
           */
          if (list)
            {
              avahi_string_list_free(list);
              list = NULL;
            }
        }
      NSAssert(list,@"Error creating string list for TXT record");
    }
  END_FOR_IN(keys)

  // Convert string list into a data object:
  data = NSDataFromAvahiStringList(list);
  avahi_string_list_free(list);
  list = NULL;
  return data;
}

+ (NSDictionary *) dictionaryFromTXTRecordData: (NSData *) txtData
{
  AvahiStringList *list = NULL;
  int ret = 0;
  NSMutableDictionary *dict = nil;
  AvahiStringList *item = NULL;
  NSAssert(([txtData length] > 0),
    @"No data to convert to TXT record dictionary");
  ret = GSAvahiStringListFromNSData(txtData, &list);
  // ret == 0 on success.
  NSAssert(!ret, @"Could not parse TXT data");
  if (list == NULL)
    {
      // This means the txtData was empty (e.g. only zeros)
      return nil;
    }
  // Autoreleased:
  dict = [NSMutableDictionary dictionary];

  // Use the beginning of the list as the first element to handle:
  item = list;
  do
    {
      char *key = NULL;
      char *value = NULL;
      size_t len = 0;
      NSString *k = nil;
      NSData *v = nil;
      int ret = avahi_string_list_get_pair(item, &key, &value, &len);
      // ret == 0 indicates success.
      NSAssert(!ret, @"Could not create TXT record dictionary.");
      if (key)
        {
          k = [NSString stringWithUTF8String: key];
          avahi_free(key);
        }

      if (value)
        {
          v = [NSData dataWithBytes: (void*)value
                             length: len];
          avahi_free(value);
        }
      else
        {
          // If the value is empty, place an empty NSData object in the
          // dictionary.
          v = [NSData data];
        }
      NSAssert((k && v),@"Could not create TXT record dictionary");
      [dict setObject: v forKey: k];
    }
  while (NULL != (item = avahi_string_list_get_next(item)));
  avahi_string_list_free(list);
  return dict;
}



- (void) netService: (NSNetService*)service
 didUpdateAddresses: (NSArray*)addresses
{
  if ([[self delegate] respondsToSelector:
    @selector(netService:didUpdateAddresses:)])
    {
      [(id)[self delegate] netService: service
                   didUpdateAddresses: addresses];
    }
}

- (void) netService: (NSNetService*)service
didUpdateRecordData: (id)data
      forRecordType: (NSString*)rrType
{
  SEL theSelector = NULL;
  if ([rrType isEqualToString: @"A"] || [rrType isEqualToString: @"AAAA"])
    {
      [self netService: service didUpdateAddresses: [self addresses]];
    }
  theSelector = NSSelectorFromString([NSString stringWithFormat:
    @"netService:didUpdate%@RecordData:", rrType]);
  if ([[self delegate] respondsToSelector: theSelector])
    {
      if (([rrType isEqualToString: @"TXT"])
        && [data isKindOfClass: [NSArray class]])
        {
          /*
           * Legacy case for TXT records (user code will always expect NSData,
           * not NSArray).
           */
           data = [(NSArray*)data lastObject];
        }
      [[self delegate] performSelector: theSelector
			    withObject: service
			    withObject: data];
    }
  else if ([[self delegate] respondsToSelector:
    @selector(netService:didUpdateRecordData:forRecordType:)])
    {
      [(id)[self delegate] netService: service
                  didUpdateRecordData: data
                        forRecordType: rrType];
    }
}

- (void) netService: (NSNetService*)service
      didNotMonitor: (NSDictionary*)errorDict
      forRecordType: (NSString*)rrType
{
  SEL theSelector = NSSelectorFromString([NSString stringWithFormat:
    @"netService:didNotMonitor%@RecordData:", rrType]);

  if ([[self delegate] respondsToSelector: theSelector])
    {
      [[self delegate] performSelector: theSelector
			    withObject: service
			    withObject: errorDict];
    }
}



/**
 * Designated intializer that passes interface index and protocol information
 * alongside the usual information for a mDNS service. This is used by
 * GSAvahiNetServiceBrowser which already knows about these.
 */
- (id) initWithDomain: (NSString*)domain
                 type: (NSString*)type
                 name: (NSString*)name
                 port: (NSInteger)port
         avahiIfIndex: (AvahiIfIndex)anIfIndex
        avahiProtocol: (AvahiProtocol)aProtocol
{
  const NSMapTableValueCallBacks valueCallbacks = {NULL, NULL, NULL};
  if (nil == (self = [self avahiClientInit]))
    {
      return nil;
    }
  _infoLock = [[NSRecursiveLock alloc] init];
  _info = [[NSMutableDictionary alloc] init];
  [_info setObject: domain forKey: @"domain"];
  [_info setObject: type   forKey: @"type"];
  [_info setObject: name   forKey: @"name"];
  [_info setObject: [NSNumber numberWithInteger: port]
            forKey: @"port"];
  _browsers = NSCreateMapTable(NSIntegerMapKeyCallBacks, valueCallbacks, 10);
  _browserTimeouts = NSCreateMapTable(NSIntegerMapKeyCallBacks,
    NSObjectMapValueCallBacks, 10);
  if (port > 0)
    {
      // If port is set to a sensible value in this initializer, we are
      // initialized to publish and will create the entry group.
    }
  _ifIndex = anIfIndex;
  _protocol = aProtocol;
  return self;
}

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
         avahiIfIndex: (AvahiIfIndex)index
        avahiProtocol: (AvahiProtocol)proto
{
  return [self initWithDomain: domain
                         type: type
                         name: name
                         port: -1
                 avahiIfIndex: index
                avahiProtocol: proto];
}

- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
{
  return [self initWithDomain: domain
                         type: type
                         name: name
                         port: -1];
}
- (id) initWithDomain: (NSString *) domain
                 type: (NSString *) type
                 name: (NSString *) name
                 port: (NSInteger) port
{
  return [self initWithDomain: domain
                         type: type
                         name: name
                         port: port
                 avahiIfIndex: AVAHI_IF_UNSPEC
                avahiProtocol: AVAHI_PROTO_UNSPEC];
}

/**
 * Stores an object at a key in the info dictionary in a synchronized fashion.
 * This uses a SeqLock pattern: It obtains the lock associated with the
 * dictionary, increments a sequence number, performs the operation, increases
 * the sequence number again, and releases the lock. This way it is garantueed
 * that the sequence number is odd while an operation is in progress, and even
 * if it is safe for a reader to obtain the value.
 */
- (void)setInfoObject: (id)object
              forKey: (NSString*)key
{
  if ((object == nil) || (key == nil))
    {
      return;
    }
  [_infoLock lock];
  _infoSeq++;
  // Now the sequence number is odd: Write in progess.
  [_info setObject: object
            forKey: key];
  _infoSeq++;
  // Now the sequence number is even: Can be read
  [_infoLock unlock];
}

/**
 * Removes an object from the dictionary in a synchronized fashion. Cf. note
 * about -setInfoObject:forKey:.
 */
- (void) removeInfoObjectForKey: (NSString*)key
{
  if (key == nil)
    {
      return;
    }
  [_infoLock lock];
  _infoSeq++;
  // Now the sequence number is odd: Write in progess.
  [_info removeObjectForKey: key];
  _infoSeq++;
  // Now the sequence number is even: Can be read
  [_infoLock unlock];
}

/**
 * Thread-safe (and fast) reading of an info key by comparing the sequence
 * numbers set by the writers. Cf. note about -setInfoObject:forKey:.
 */
- (id) infoObjectForKey: (NSString*)key
{
  NSUInteger oldSeq = 0;
  NSUInteger newSeq = 0;
  BOOL isOdd = NO;
  id object = nil;
  if (key == nil)
    {
      return nil;
    }

  // Try to read the value until the sequence numbers match and are even:
  do
    {
      // Store the sequence number before the read:
      oldSeq = _infoSeq;
      // Read the value:
      object = [_info objectForKey: key];
      // Store the sequence number after the read:
      newSeq = _infoSeq;

      while ((newSeq == 0) || (oldSeq == 0))
        {
          //Add 2 to prevent (0 % 2) because it's undefined.
          newSeq = newSeq + 2;
          oldSeq = oldSeq +2;
        }
      isOdd = (BOOL)newSeq % 2;
    } while ((oldSeq != newSeq) || (isOdd));

  return object;
}

- (NSInteger) port
{
  NSInteger port = [(NSNumber*)[self infoObjectForKey: @"port"] integerValue];
  return port ? MAX(port, -1) : -1;
}


/**
 * Private method that creates an empty entry group that needs to be filled
 * before publication. There is no locking here because it will only be called
 * from methods that already obtained the lock. Returns 0 on success.
 */
- (int) createEntryGroup
{
  // We only care to publish if the service is still idle:
  if (_serviceState != GSNetServiceIdle)
    {
      [self netService: self
         didNotPublish: [self errorDictWithErrorCode: NSNetServicesActivityInProgress]];
      return NSNetServicesActivityInProgress;
    }

  //Create the entry group:
  if (NULL != _client)
    {
      _entryGroup = avahi_entry_group_new((AvahiClient*)_client,
                                          GSAvahiEntryGroupStateChanged,
                                          (void*)self);
    }
  else
    {
      // having no _client usually means that avahi-daemon (or dbus)
      // isn't running, unfortunately there's no precise errNo at this point
      // so we're providing just our best guess
      return AVAHI_ERR_NO_DAEMON;
    }

  // Handle error:
  if (NULL == _entryGroup)
    {
      int error = avahi_client_errno((AvahiClient*)_client);
      [self handleError: error];
      return error;
    }
  return 0;
}

/**
 * Private method to commit an entry group to the network. The entry group
 * cannot be modified afterwards. There is no locking here because it will only
 * be called from methods that already obtained the lock. Notifies the delegate
 * on error or success.
 */
- (void)commitEntryGroup
{
  _serviceState = GSNetServicePublishing;
  // Make sure there is an entry group to commit:
  if (_entryGroup != NULL)
    {
      // Make sure it is not empty:
      if (!avahi_entry_group_is_empty((AvahiEntryGroup*)_entryGroup))
        {
          // Make sure it is not already committed:
          if (avahi_entry_group_get_state((AvahiEntryGroup*)_entryGroup)
	    == AVAHI_ENTRY_GROUP_UNCOMMITED)
            {
              avahi_entry_group_commit((AvahiEntryGroup*)_entryGroup);
              [self netServiceWillPublish: self];
              return;
            }
          else
            {
              // The entryGroup is active:
              [self handleError: NSNetServicesActivityInProgress];
              return;
            }
        }
    }
  // The entryGroup is not properly set up for publication:
  [self handleError: NSNetServicesBadArgumentError];
  return;
}

/**
 * Private method to add a service entry to the entry group. Will only be called
 * from methods that already ensure that the entry can be added safely.
 */
- (int) addServiceEntry
{
  int ret = 0;
  const char* d = StringOrNullIfEmpty([self infoObjectForKey: @"domain"]);

  // It is possible that the TXT record has already been set, so we can
  // publish it right away:
  AvahiStringList *list = NULL;
  NSData *txtData = [self infoObjectForKey: @"TXT"];
  int res = 0;
  res = GSAvahiStringListFromNSData(txtData, &list);
  if (0 != res)
    {
      if (NULL != list)
        {
          avahi_string_list_free(list);
          list = NULL;
        }
    }
  ret = avahi_entry_group_add_service_strlst((AvahiEntryGroup*)_entryGroup,
    _ifIndex,
    _protocol,
    0, // Flags, we don't need them
    [(NSString*)[self infoObjectForKey: @"name"] UTF8String],
    [(NSString*)[self infoObjectForKey: @"type"] UTF8String],
    d, // Domain (might be NULL for default)
    NULL, // The hostname is filled automatically
    [[self infoObjectForKey: @"port"] integerValue],
    list // Possibly empty TXT record
    );
  if (NULL != list)
    {
      // If we are not using the emptyList from the stack, we need to free the
      // list that avahi created for us.
      avahi_string_list_free(list);
    }
  return ret;
}

- (BOOL) addServiceRecordWithOptions: (NSNetServiceOptions)options
{
  int ret = 0;
  [_lock lock];
  if (_serviceState != GSNetServiceIdle)
    {
      [_lock unlock];
      return NO;
    }

  if (_entryGroup == NULL)
    {
      if (0 != [self createEntryGroup])
        {
          [_lock unlock];
          return NO;
        }
    }

  if (options & NSNetServiceListenForConnections)
    {
      GSServerStream *serverStream;
      NSInteger port;

      /* setup server socket first, as port is required in
       * -[self addServiceEntry] (see below)
       */
      port = [self port];
      if (port < 0)
        {
          port = 0;
        }
      serverStream = [GSServerStream serverStreamToAddr: @"" port: port];
      if (serverStream != nil)
        {
          [serverStream setDelegate:self];
          [serverStream open];
          if ([serverStream streamStatus] != NSStreamStatusOpen)
            {
              ret = 1;
            }
          else
            {
              NSNumber *portNumber;

              [serverStream scheduleInRunLoop: [NSRunLoop currentRunLoop]
                                      forMode: NSDefaultRunLoopMode];
              [self setInfoObject: serverStream forKey: @"serverStream"];
              portNumber = [serverStream propertyForKey: GSStreamLocalPortKey];
              [self setInfoObject: portNumber forKey: @"port"];
            }
        }
      else
        {
          ret = 1;
        }

      if (ret != 0)
        {
          [self handleError: NSNetServicesBadArgumentError];
          [_lock unlock];
          return NO;
        }
    }

  /* Try adding the service to the entry group until we find an unused name
   * for it (but only if NSNetServiceNoAutoRename is not set).
   */
  while (AVAHI_ERR_COLLISION == (ret = [self addServiceEntry])
    && !(options & NSNetServiceNoAutoRename))
    {
      char *newName = avahi_alternative_service_name([[self infoObjectForKey:
	@"name"] UTF8String]);
      if (newName)
        {
          [self setInfoObject: [NSString stringWithUTF8String: newName]
                       forKey: @"name"];
          avahi_free(newName);
        }
      else
        {
          ret = AVAHI_ERR_FAILURE;
	  break;
        }
    }

  [_lock unlock];
  return ret == 0 ? YES : NO;
}

- (BOOL) addServiceRecord
{
  return [self addServiceRecordWithOptions: 0];
}

- (void) publishWithOptions: (NSNetServiceOptions)options
{
  [_lock lock];
  if (_entryGroup == NULL)
    {
      if (NO == [self addServiceRecordWithOptions: options])
        {
          [self handleError: _client ? avahi_client_errno((AvahiClient*)_client)
                                     : AVAHI_ERR_NO_DAEMON];
        }
    }
  [self commitEntryGroup];
  [_lock unlock];
}

/**
 * Convenience method to return the full name of the service.
 */
- (NSString *) fullServiceName
{
  NSString *full = nil;
  NSString *domain = [self infoObjectForKey: @"domain"];

  if (([domain isEqualToString: @""]) || domain == nil)
    {
      // Pick the default domain:
      domain = [NSString stringWithUTF8String:
	avahi_client_get_domain_name((AvahiClient*)_client)];
    }
  full = [NSString stringWithFormat: @"%@.%@.%@", [self name],
                                             [self type], domain];
  if ((unichar)'.' != [full characterAtIndex: ([full length] - 1)])
    {
      return [full stringByAppendingString: @"."];
    }
  return full;
}

- (void) publish
{
  // Publish and allow renaming:
  [self publishWithOptions: 0];
}

- (void) resolve
{
  [self resolveWithTimeout: 5];
}

/**
 * Called by the resolver timeout to cease service resolution.
 */
- (void) didTimeout: (NSTimer*)theTimer
{
  [self stop];
  _timer = nil;
}

- (void) resolveWithTimeout: (NSTimeInterval) timeout
{
  if (_serviceState < GSNetServiceResolving)
    {
      _resolver =  (void*)avahi_service_resolver_new((AvahiClient*)_client,
                                                             _ifIndex,
                                                            _protocol,
                           [[self infoObjectForKey: @"name"] UTF8String],
                           [[self infoObjectForKey: @"type"] UTF8String],
                         [[self infoObjectForKey: @"domain"] UTF8String],
                                                  AVAHI_PROTO_UNSPEC,
                                                                   0,
                                         GSAvahiServiceResolverEvent,
                                                        (void*)self);

      if (NULL == _resolver)
        {
          [self handleError: avahi_client_errno((AvahiClient*)_client)];
          return;
        }
      _serviceState = GSNetServiceResolving;
      if (_timer != nil)
        {
          [_timer invalidate];
          _timer = nil;
        }
      _timer = [[NSTimer timerWithTimeInterval: timeout
                                       target: self
                                     selector: @selector(didTimeout:)
                                     userInfo: nil
                                      repeats: NO] retain];

      [[ctx runLoop] addTimer: _timer
                      forMode: [ctx mode]];
    }
  // BOOM
}

/**
 * Main cleanup method. Will clean up all avahi-related resources in use by
 * the GSAvahiNetService. An resource record code of -1 indicates that that
 * the whole service needs to be reset. Otherwise, it will only clean up the
 * browser for the specified record type.
 */
- (void) stopWithError: (BOOL)hadError
             forRRCode: (NSInteger)rrCode
{
  /*
   * If an RRCode was set (a value of zero possibly indicating an unknown
   * RRCode, fo which we won't do anything), we only clean up the
   * corresponding browser.
   */
  if (rrCode == 0)
    {
      return;
    }
  [_lock lock];
  if (rrCode != -1)
    {
      AvahiRecordBrowser *browser
	= NSMapGet(_browsers, (void*)(uintptr_t)rrCode);
      if (browser != NULL)
        {
          avahi_record_browser_free(browser);
          NSMapRemove(_browsers, (void*)(uintptr_t)rrCode);
          [(NSTimer*)NSMapGet(_browserTimeouts,
	    (void*)(uintptr_t)rrCode) invalidate];
          NSMapRemove(_browserTimeouts, (void*)(uintptr_t)rrCode);
          if ((NSCountMapTable(_browsers) == 0)
            && (_serviceState == GSNetServiceRecordBrowsing))
            {
              _serviceState = GSNetServiceResolved;
            }
        }
      return;
    }

  if (_timer != nil)
    {
      [_timer invalidate];
      _timer = nil;
    }
  if (_resolver != NULL)
    {
      avahi_service_resolver_free((AvahiServiceResolver*)_resolver);
      _resolver = NULL;
    }
  if (_entryGroup != NULL)
    {
      //Make sure to unpublish it.
      avahi_entry_group_reset((AvahiEntryGroup*)_entryGroup);
      avahi_entry_group_free((AvahiEntryGroup*)_entryGroup);
      _entryGroup = NULL;
    }

  if ((_browsers != NULL)  && (0 != NSCountMapTable(_browsers)))
    {
      NSMapTable *enumerationTable;
      NSMapEnumerator bEnum;
      NSUInteger code;
      AvahiRecordBrowser *browser;

      enumerationTable
	= NSCopyMapTableWithZone(_browsers, NSDefaultMallocZone());
      bEnum = NSEnumerateMapTable(enumerationTable);
      while (NSNextMapEnumeratorPair(&bEnum,(void*)&code,(void*)&browser))
        {
          avahi_record_browser_free(browser);
          NSMapRemove(_browsers, (void*)code);
          [(NSTimer*)NSMapGet(_browserTimeouts,
	    (void*)(uintptr_t)code) invalidate];
          NSMapRemove(_browserTimeouts, (void*)(uintptr_t)code);
        }
      NSEndMapTableEnumeration(&bEnum);
      NSFreeMapTable(enumerationTable);
    }
  if (!hadError)
    {
      [self removeInfoObjectForKey: @"serverStream"];
      [self netServiceDidStop: self];
    }
  _serviceState = GSNetServiceIdle;
  [_lock unlock];
}

- (void) stop
{
  [self stopWithError: NO
            forRRCode: -1];
}

- (void) startMonitoringForRecordType: (NSString*)rrType
{
  NSUInteger code = RRCodeFromNSString(rrType);
  AvahiRecordBrowser *browser = NULL;
  // Raise error for bad record type:
  if (code == 0)
    {
      [self handleError: NSNetServicesBadArgumentError
              forRRCode: code];
      return;
    }
  [_lock lock];
  if ((_serviceState < GSNetServiceResolved)
    || (_serviceState > GSNetServiceRecordBrowsing))
    {
      [self handleError: NSNetServicesBadArgumentError
              forRRCode: code];
      [_lock unlock];
      return;
    }

  browser = NSMapGet(_browsers, (void*)(uintptr_t)code);
  if (browser)
    {
      NSDebugLog(@"Browser for RR code %@ already monitoring", rrType);
      [self handleError: NSNetServicesActivityInProgress];
      [_lock unlock];
      return;
    }
  browser = avahi_record_browser_new((AvahiClient*)_client,
                                    _ifIndex,
                                   _protocol,
        [[self fullServiceName] UTF8String],
                         AVAHI_DNS_CLASS_IN,
                             (uint16_t)code,
                                          0,
                   GSAvahiRecordBrowserEvent,
                                       self);
  if (browser == NULL)
    {
      // Something went wrong:
      [self handleError: avahi_client_errno((AvahiClient*)_client)];
    }
  else
    {
      // The browser was successfully created, we add it to the mapTable.
      NSMapInsert(_browsers, (void*)(uintptr_t)code, browser);
      // Set the proper state if the new browser is responsible for a state
      // change.
      if (_serviceState == GSNetServiceResolved)
        {
          _serviceState = GSNetServiceRecordBrowsing;
        }
    }
  [_lock unlock];
}

- (void) stopMonitoringForRecordType: (NSString*)rrType
{
  NSUInteger rrCode = RRCodeFromNSString(rrType);
  AvahiRecordBrowser *browser = NULL;
  [_lock lock];
  if (_serviceState > GSNetServiceRecordBrowsing)
    {
      //Don't do anything for a publishing service.
      [_lock unlock];
      return;
    }
  browser = NSMapGet(_browsers, (void*)(uintptr_t)rrCode);
  if (browser != NULL)
    {
      avahi_record_browser_free(browser);
    }
  if (0 == NSCountMapTable(_browsers))
    {
      _serviceState = GSNetServiceResolved;
    }
  [_lock unlock];
}

- (void) startMonitoring
{
  if ((_serviceState == GSNetServiceResolved)
    || (_serviceState == GSNetServiceRecordBrowsing))
    {
      [self startMonitoringForRecordType: @"TXT"];
    }
}
- (void) stopMonitoring
{
  [self stopMonitoringForRecordType: @"TXT"];
}


- (NSArray *) addresses
{
  NSArray *addresses = [self infoObjectForKey: @"addresses"];
  if (nil == addresses)
    {
      // As per Apple documentation "If no addresses were resolved for the
      // service, the returned array contains zero elements."
      return [[[NSArray alloc] init] autorelease];
    }
  return addresses;
}

/**
 * Private method to add new address data to the service.
 */
- (void) addAddressData: (NSData*)data
{
  NSMutableArray *addresses = nil;

  if (data == nil)
    {
      return;
    }
  addresses = [self infoObjectForKey: @"addresses"];
  if (addresses == nil)
    {
      // Autoreleased:
      addresses = [NSMutableArray array];
      [self setInfoObject: addresses
                   forKey: @"addresses"];
    }
  if (![addresses containsObject: data])
    {
      [addresses addObject: data];
    }
}

/**
 * Private method to remove stale address data from the service.
 */
- (void) removeAddressData: (NSData*)data
{
  NSMutableArray *addresses = nil;
  // Index of the address in the array:
  NSUInteger index = NSNotFound;
  if (data == nil)
    {
      return;
    }
  addresses = [self infoObjectForKey: @"addresses"];
  if (addresses == nil)
    {
      // Autoreleased:
      addresses = [NSMutableArray array];
      [self setInfoObject: addresses
                   forKey: @"addresses"];
    }
  index = [addresses indexOfObjectIdenticalTo: data];
  if (index != NSNotFound)
    {
      [addresses removeObjectAtIndex: index];
    }
}

- (NSString *) domain
{
  return [self infoObjectForKey: @"domain"];
}

- (NSString *) hostName
{
  return [self infoObjectForKey: @"hostName"];
}

- (NSString *) name
{
  return [self infoObjectForKey: @"name"];
}

- (NSString *) type
{
  return [self infoObjectForKey: @"type"];
}

/**
 * This method is called from the Avahi callback when a service has successfully
 * resolved.
 */
- (void) avahiResolver: (AvahiServiceResolver*)aResolver
  foundServiceWithName: (NSString*)name
		  type: (NSString*)type
		domain: (NSString*)domain
	      hostName: (NSString*)hostName
	       address: (NSData*)address
		  port: (NSInteger)port
	     txtRecord: (NSData*)txtRecord
	       ifIndex: (AvahiIfIndex)anIfIndex
	      protocol: (AvahiProtocol)aProtocol
{
  [_lock lock];
  if ((void*)aResolver != _resolver)
    {
      // This callback comes from the wrong resolver:
      [self handleError: NSNetServicesInvalidError];
      // Free the erratic resolver, the real one will have been freed by the
      // error handler.
      if (NULL != aResolver)
        {
          avahi_service_resolver_free(aResolver);
        }
      [_lock unlock];
      return;
    }
  if (![name isEqualToString: [self name]]
    || ![type isEqualToString: [self type]]
    || ![domain isEqualToString: [self domain]])
    {
      // This resolver callback is for the wrong service!
      [self handleError: NSNetServicesInvalidError];
      [_lock unlock];
      return;
    }
  if (hostName)
    {
      [self setInfoObject: hostName forKey: @"hostName"];
    }
  if (port)
    {
      [self setInfoObject: [NSNumber numberWithInteger: port]
                forKey: @"port"];
    }
  if (txtRecord)
    {
      [self setInfoObject: txtRecord forKey: @"TXT"];
    }

  if (address)
    {
      [self addAddressData: address];
    }

  // This makes sure all further actions happen on the same if/protocol
  // combination (they might have been AVAHI_(IF|PROTO)_UNSPEC before).
  _ifIndex = anIfIndex;
  _protocol = aProtocol;

  // Clean up the resolver, we don't need it anymore:
  avahi_service_resolver_free((AvahiServiceResolver*)_resolver);
  _resolver = NULL;

  _serviceState = GSNetServiceResolved;
  [self netServiceDidResolveAddress: self];
  if (_timer != nil)
    {
      [_timer invalidate];
      _timer = nil;
    }
  [_lock unlock];

}

/**
 * Callback for timers on record browsing, to mimic AllForNow.
 */
- (void) didTimeoutRRBrowsing: (NSTimer*)aTimer
{
  // Invoke our AllForNow callback, which will take care to invalidate the
  // timer.
  [self allForNowForRRCode:
    [(NSNumber*)[aTimer userInfo] unsignedIntegerValue]];
}

/**
 * Called whenever a new event appears for the resource record
 * <code>code</code>. Postpones the timeout.
 */
- (void) rescheduleBrowserTimeoutForRRCode: (NSUInteger)code
{
  NSTimer *aTimer = nil;
  [_lock lock];
  /* Do AllForNow handling to supplement what Avahi is doing. (The AllForNow
   * event seems to be sent exactly once over the lifetime of the record
   * browser, which makes it quite useless.
   */
  aTimer = (NSTimer*)NSMapGet(_browserTimeouts, (void*)(uintptr_t)code);
  if (aTimer != nil)
    {
      [aTimer invalidate];
      NSMapRemove(_browserTimeouts, (void*)(uintptr_t)code);
    }
  aTimer = [NSTimer
    timerWithTimeInterval: BROWSER_UPDATE_INTERVAL
    target: self
    selector: @selector(didTimeoutRRBrowsing:)
    userInfo: [NSNumber numberWithUnsignedInteger: code]
    repeats: NO];
  [[ctx runLoop] addTimer: aTimer
                  forMode: [ctx mode]];
  NSMapInsert(_browserTimeouts, (void*)(uintptr_t)code, aTimer);
  [_lock unlock];
}

/**
 * Private method to add new data for a record type.
 */
- (void) newData: (NSData*)data
       forRRCode: (NSUInteger)code
{
  NSString *rrType = NSStringFromRRCode(code);
  id oldValue = nil;

  [self rescheduleBrowserTimeoutForRRCode: code];
  if (data == nil)
    {
      return;
    }

  // We dynamically transform between an NSData object and an array as the
  // value for the rrType:
  [_infoLock lock];
  oldValue = [self infoObjectForKey: rrType];
  if (oldValue == nil)
    {
      [self setInfoObject: data
                forKey: rrType];
    }
  else if ([oldValue isKindOfClass: [NSData class]])
    {
      NSMutableArray *container = [NSMutableArray array];
      [container addObject: oldValue];
      [container addObject: data];
      [self setInfoObject: container
                   forKey: rrType];
    }
  else if ([oldValue isKindOfClass: [NSMutableArray class]])
    {
      [oldValue addObject: data];
    }
  else
    {
      [_infoLock unlock];
      [NSException raise: @"NSInternalInconsistencyException"
                  format: @"Invalid value of NSNetService info key %@", rrType];
      return;
    }

  // Special case for addresses, they need to update the addresses key
  // properly:
  if ((code == AVAHI_DNS_TYPE_A) || code == AVAHI_DNS_TYPE_AAAA)
    {
      AvahiProtocol proto = AVAHI_PROTO_UNSPEC;
      if (code == AVAHI_DNS_TYPE_A)
        {
          proto = AVAHI_PROTO_INET;
        }
      else if (code == AVAHI_DNS_TYPE_AAAA)
        {
          proto = AVAHI_PROTO_INET6;
        }
      [self addAddressData: NSDataWithAddrDataPortFamilyAndIface(data,
        [self port], proto, _ifIndex)];
    }
  [_infoLock unlock];
}

/**
 * Private method to remove stale data for a resource record.
 */
- (void) removedData: (NSData*)data
           forRRCode: (NSUInteger)code
{
  NSString *rrType = NSStringFromRRCode(code);
  id oldValue = nil;
  [self rescheduleBrowserTimeoutForRRCode: code];
  if (data == nil)
    {
      return;
    }
  [_infoLock lock];
  oldValue = [self infoObjectForKey: rrType];
  if (oldValue == nil)
    {
      //Nothing to be done.
      [_infoLock unlock];
      return;
    }
  else if ([oldValue isKindOfClass: [NSData class]])
    {
      if ([oldValue isEqual: data])
        {
          [self removeInfoObjectForKey: rrType];
        }
    }
  else if ([oldValue isKindOfClass: [NSMutableArray class]])
    {
      NSUInteger index = [oldValue indexOfObjectIdenticalTo: data];
      if (index != NSNotFound)
        {
          NSUInteger count;
          [oldValue removeObjectAtIndex: index];
          count = [oldValue count];
          if (count == 1)
            {
              //Go back to a plain data record:
              [self setInfoObject: [oldValue objectAtIndex: 0]
                           forKey: rrType];
            }
          else if (count == 0)
            {
               // Remove empty array:
              [self removeInfoObjectForKey: rrType];
            }
        }
    }
  else
    {
      [_infoLock unlock];
      [NSException raise: @"NSInternalInconsistencyException"
      format: @"Invalid value of NSNetService info key %@", rrType];
      return;
    }

  // Special case for address records:
  if ((code == AVAHI_DNS_TYPE_A) || code == AVAHI_DNS_TYPE_AAAA)
    {
      AvahiProtocol proto = AVAHI_PROTO_UNSPEC;
      if (code == AVAHI_DNS_TYPE_A)
        {
          proto = AVAHI_PROTO_INET;
        }
      else if (code == AVAHI_DNS_TYPE_AAAA)
        {
          proto = AVAHI_PROTO_INET6;
        }
      [self removeAddressData: NSDataWithAddrDataPortFamilyAndIface(data,
        [self port], proto, _ifIndex)];
    }
  [_infoLock unlock];
}

/**
 * Called both by the native timeout mechanism and the Avahi callback to notify
 * the delegate about record data changes.
 */
- (void) allForNowForRRCode: (NSUInteger)code
{
  NSString *rrType = nil;
  NSData *data = nil;
  [_lock lock];
    {
      // Remove a dangling timer if any:
      NSTimer *aTimer = (NSTimer*)NSMapGet(_browserTimeouts,
	(void*)(uintptr_t)code);
      if (aTimer != nil)
        {
          [aTimer invalidate];
          NSMapRemove(_browserTimeouts, (void*)(uintptr_t)code);
        }
    }
  [_lock unlock];
  rrType = NSStringFromRRCode(code);
  data = [self infoObjectForKey: rrType];
  [self netService: self didUpdateRecordData: data
     forRecordType: rrType];
}

#if GS_USE_AVAHI==1
- (id<NSObject,GSNetServiceDelegate>)delegate
#else
- (id<NSObject>)delegate
#endif
{
  return _delegate;
}

/**
 * Dispatcher method for error notifications to the delegate.
 */
- (void) handleError: (int)errorCode
           forRRCode: (int)RRCode
{
  [_lock lock];
  if (_serviceState < GSNetServiceResolved)
    {
      [self netService: self
         didNotResolve: [self errorDictWithErrorCode: errorCode]];
    }
  else if (_serviceState <= GSNetServiceRecordBrowsing)
    {
      /* Strangely enough, the Apple documentation does not specify that an
       * error should be raised when monitoring a record fails. Since we are
       * not Apple, we can actually be nice and notify the delegate, if it is
       * interested.
       */
      [self netService: self
         didNotMonitor: [self errorDictWithErrorCode: errorCode]
         forRecordType: NSStringFromRRCode(RRCode)];
    }
  else if (_serviceState > GSNetServiceRecordBrowsing)
    {
      [self netService: self
         didNotPublish: [self errorDictWithErrorCode: errorCode]];
    }
  [self stopWithError: YES
            forRRCode: RRCode];
  [self avahiClientHandleError: errorCode];
  [_lock unlock];
}

/**
 * Dispatcher method for error notifications to the delegate.
 */
- (void) handleError: (int)errorCode
{
  [self handleError: errorCode
	  forRRCode: -1];
}

- (id) recordDataForRecordType: (NSString*)key
{
  return [self infoObjectForKey: key];
}

- (NSData*) TXTRecordData
{
  id retVal = nil;
  id value = [self infoObjectForKey: @"TXT"];
  //Retain because somebody might remove it while we do this:
  [value retain];
  if ([value isKindOfClass: [NSData class]])
    {
      return [value autorelease];
    }
  else if ([value isKindOfClass: [NSArray class]])
    {
      // Only return the last (= newest) object:
      retVal = [[(NSArray*)value lastObject] retain];
    }
  [value release];
  return [retVal autorelease];
}

- (BOOL) setTXTRecordData: (NSData*)data
{
  AvahiStringList *list = NULL;
  int ret = GSAvahiStringListFromNSData(data, &list);
  if (0 == ret)
    {
      // We could successfully parse it, so we set it as the value of the
      // dictionary.
      [self setInfoObject: data
                   forKey: @"TXT"];
    }
  [_lock lock];
  switch (_serviceState)
    {
      case (GSNetServiceIdle):
        // We don't need to do anything more, the TXT will be published along
        // with the service:
        if (list != NULL)
          {
            avahi_string_list_free(list);
          }
        [_lock unlock];
        return YES;
      case (GSNetServiceResolving):
      case (GSNetServiceResolved):
      case (GSNetServiceRecordBrowsing):
      case GSNetServiceStateMax:
        // TODO: Raise error?
        if (list != NULL)
          {
            avahi_string_list_free(list);
          }
        [_lock unlock];
        return NO;
      case (GSNetServicePublishing):
      case (GSNetServicePublished):
        break;
    }

  // Ret will still be 0 at this point, so we can reuse the variable:
  ret = avahi_entry_group_update_service_txt_strlst(
    (AvahiEntryGroup*)_entryGroup,
    _ifIndex,
    _protocol,
    0, //no flags
    [[self infoObjectForKey: @"name"] UTF8String],
    [[self infoObjectForKey: @"type"] UTF8String],
    StringOrNullIfEmpty([self infoObjectForKey: @"domain"]),
    list);
  if (list != NULL)
    {
      avahi_string_list_free(list);
    }
  if (ret != 0)
    {
      // TODO: Raise error:
      [_lock unlock];
      return NO;
    }
  /*
   * FIXME: Apple's NSNetService API is a bit crappy, meaning that almost
   * everything is asynchronous, but not -setTXTRecordData:. One
   * solution would be to actually be asynchronous and run the runloop for
   * ourselves a bit. Unfortunately Avahi does not call any of its
   * callbacks to inform us about the fact that the record has
   * successfully been changed. So we are a bit out of luck. Right now,
   * this means that we might be returning 'YES' wrongly and also break
   * code that _expects_ the record to be published before the method
   * returns. Life sucks sometimes.
   */
  [_lock unlock];
  return YES;
}

- (BOOL) addRecordData: (NSData*)data
         forRecordType: (NSString*)type
               withTTL: (NSUInteger)ttl
{
  int rrCode = RRCodeFromNSString(type);
  int ret = 0;
  [_lock lock];
  if (_serviceState == GSNetServiceIdle)
    {
      if (NULL == _entryGroup)
        {
          int ret = [self createEntryGroup];
          if (ret != 0)
            {
              [_lock unlock];
              return NO;
            }
        }
    }
  else
    {
      [_lock unlock];
      return NO;
    }

  if (!rrCode)
    {
      //FIXME: Raise error.
      [self handleError: NSNetServicesBadArgumentError];
      [_lock unlock];
      return NO;
    }

  // NOTE: This function is crazly a bit different than the others: The name
  // parameter is the full service name!
  ret = avahi_entry_group_add_record((AvahiEntryGroup*)_entryGroup,
    _ifIndex,
    _protocol,
    0,
    [[self fullServiceName] UTF8String],
    AVAHI_DNS_CLASS_IN, // DNS class
    rrCode,
    ttl,
    [data bytes],
    [data length]);
  if (ret != 0)
    {
      [self handleError: ret];
      [_lock unlock];
      return NO;
    }
  [_lock unlock];
  return YES;
}

- (BOOL) addRecordData: (NSData*)data
         forRecordType: (NSString*)type
{
  return [self addRecordData: data
               forRecordType: type
                     withTTL: DEFAULT_OTHER_RECORD_TTL];
}

/**
 * Private method called upon state changes in the entry group.
 */
- (void) entryGroup: (AvahiEntryGroup*)group
       enteredState: (AvahiEntryGroupState)groupState
{
  [_lock lock];
  if (_serviceState == GSNetServicePublishing)
    {
      switch (groupState)
        {
          case (AVAHI_ENTRY_GROUP_UNCOMMITED):
          case (AVAHI_ENTRY_GROUP_REGISTERING):
            break;
          case (AVAHI_ENTRY_GROUP_COLLISION):
            [self handleError: NSNetServicesCollisionError];
            break;
          case (AVAHI_ENTRY_GROUP_FAILURE):
            [self handleError: avahi_client_errno((AvahiClient*)_client)];
            break;
          case (AVAHI_ENTRY_GROUP_ESTABLISHED):
            _serviceState = GSNetServicePublished;
            [self netServiceDidPublish: self];
            break;
        }
    }
  else if (_serviceState == GSNetServicePublished)
    {
      switch (groupState)
        {
          case AVAHI_ENTRY_GROUP_COLLISION:
          case AVAHI_ENTRY_GROUP_FAILURE:
            [self handleError: avahi_client_errno((AvahiClient*)_client)];
          case AVAHI_ENTRY_GROUP_ESTABLISHED:
          case AVAHI_ENTRY_GROUP_UNCOMMITED:
          case AVAHI_ENTRY_GROUP_REGISTERING:
            break;
        }
    }
  [_lock unlock];
}

/**
 * GSServerStream delegate method, called only when this service has been
 * published with the NSNetServiceListenForConnections option.
 */
- (void) stream:(NSStream*) stream handleEvent: (NSStreamEvent)anEvent
{
  switch (anEvent)
    {
      case NSStreamEventHasBytesAvailable:
        {
          if ([[self delegate]
			         respondsToSelector:
			         @selector(netService:didAcceptConnectionWithInputStream:outputStream:)])
            {
              NSInputStream  *is;
              NSOutputStream *os;
              GSServerStream *serverStream = [self infoObjectForKey: @"serverStream"];
			  [serverStream acceptWithInputStream: &is outputStream: &os];
			  [[self delegate] netService: self
                               didAcceptConnectionWithInputStream: is
                               outputStream: os];
            }
		  break;
		}
	  default:
        break;
	}
}

- (void) dealloc
{
  /*
   * Obtain the super-class lock so that nothing fishy can happen while
   * we clean up:
   */
  [_lock lock];

  /*
   * Unset the delegate. We might have been gone away because the delegate
   * didn't need us anymore, so there's a reasonable chance that it has also
   * been deallocated.
   */
  [self setDelegate: nil];

  /*
   * Call -stop to cleanup all avahi-related resources.
   */
  [self stop];

  /* Clean up evrything else. */
  NSFreeMapTable(_browsers);
  _browsers = NULL;
  NSFreeMapTable(_browserTimeouts);
  _browserTimeouts = NULL;
  [_info release];
  _info = nil;
  [_infoLock release];
  _infoLock = nil;
  [_lock unlock];
  [self avahiClientDealloc];
  [super dealloc];
}
@end
