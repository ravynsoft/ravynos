#import "common.h"

#import "Foundation/NSUserDefaults.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSException.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSPathUtilities.h"
#import "Foundation/NSProcessInfo.h"

#define	UNISTR(X) \
((const unichar*)[(X) cStringUsingEncoding: NSUnicodeStringEncoding])

extern void GSPropertyListMake(id,NSDictionary*,BOOL,BOOL,unsigned,id*);

@interface NSUserDefaults (Private)
- (id) initWithContentsOfFile: (NSString*)fielName;
@end

@interface NSUserDefaultsWin32 : NSUserDefaults
{
  NSString	*registryPrefix;
  NSMapTable	*registryInfo;
}
@end

@interface NSUserDefaults (Secrets)
- (id) initWithContentsOfFile: (NSString*)aPath;
- (BOOL) lockDefaultsFile: (BOOL*)wasLocked;
- (void) unlockDefaultsFile;
- (NSMutableDictionary*) readDefaults;
- (BOOL) wantToReadDefaultsSince: (NSDate*)lastSyncDate;
- (BOOL) writeDefaults: (NSDictionary*)defaults oldData: (NSDictionary*)oldData;
@end

struct NSUserDefaultsWin32_DomainInfo
{
  HKEY userKey;
  HKEY systemKey;
};

@implementation NSUserDefaultsWin32

- (void) dealloc
{
  DESTROY(registryPrefix);
  if (registryInfo != 0)
    {
      NSMapEnumerator	iter = NSEnumerateMapTable(registryInfo);
      NSString		*domain;
      struct NSUserDefaultsWin32_DomainInfo *dinfo;
  
      while (NSNextMapEnumeratorPair(&iter, (void**)&domain, (void**)&dinfo))
	{
	  LONG rc;

	  if (dinfo->userKey)
	    {
	      rc = RegCloseKey(dinfo->userKey);
	      if (rc != ERROR_SUCCESS)
		{
		  NSString	*dPath;

		  dPath = [registryPrefix stringByAppendingString: domain];
		  NSLog(@"Failed to close registry HKEY_CURRENT_USER\\%@ (%x)",
		    dPath, rc);
		}
	    }
	  if (dinfo->systemKey)
	    {
	      rc = RegCloseKey(dinfo->systemKey);
	      if (rc != ERROR_SUCCESS)
		{
		  NSString	*dPath;

		  dPath = [registryPrefix stringByAppendingString: domain];
		  NSLog(@"Failed to close registry HKEY_LOCAL_MACHINE\\%@ (%x)",
		    dPath, rc);
		}
	    }
	}
      NSEndMapTableEnumeration(&iter);
      NSResetMapTable(registryInfo);
      NSFreeMapTable(registryInfo);
      registryInfo = 0;
    }
  [super dealloc];
}

- (id) initWithUser: (NSString*)userName
{
  NSString	*path;
  NSRange	r;

  NSAssert([userName isEqual: NSUserName()],
    @"NSUserDefaultsWin32 doesn't support reading/writing to users other than the current user.");
	
  path = GSDefaultsRootForUser(userName);
  r = [path rangeOfString: @":REGISTRY:"];
  NSAssert(r.length > 0,
    @"NSUserDefaultsWin32 should only be used if defaults directory is :REGISTRY:");

  path = [path substringFromIndex: NSMaxRange(r)];
  if ([path length] == 0)
    {
      path = @"Software\\GNUstep\\";
    }
  else if ([path hasSuffix: @"\\"] == NO)
    {
      path = [path stringByAppendingString: @"\\"];
    }
  registryPrefix = RETAIN(path);
  self = [super initWithContentsOfFile: @":REGISTRY:"];
  return self;
}

- (BOOL) lockDefaultsFile: (BOOL*)wasLocked
{
  *wasLocked = NO;
  return YES;
}

- (NSMutableDictionary*) readDefaults
{
  NSArray		*allDomains;
  NSEnumerator		*iter;
  NSString		*persistentDomain;
  NSMutableDictionary	*newDict = nil;
  
  allDomains = [self persistentDomainNames];
  if ([allDomains count] == 0)
    {
      allDomains = [NSArray arrayWithObjects:
	[[NSProcessInfo processInfo] processName],
	NSGlobalDomain,
	nil];
    }
  
  if (registryInfo == 0)
    {
      registryInfo = NSCreateMapTable(NSObjectMapKeyCallBacks,
	NSOwnedPointerMapValueCallBacks, [allDomains count]);
    }

  newDict = [NSMutableDictionary dictionary];

  iter = [allDomains objectEnumerator];
  while ((persistentDomain = [iter nextObject]) != nil)
    {
      NSMutableDictionary *domainDict;
      struct NSUserDefaultsWin32_DomainInfo *dinfo;
      NSString *dPath;
      LONG rc;

      dinfo = NSMapGet(registryInfo, persistentDomain);
      if (dinfo == 0)
	{
	  dinfo = calloc(sizeof(struct NSUserDefaultsWin32_DomainInfo), 1);
	  NSMapInsertKnownAbsent(registryInfo, persistentDomain, dinfo);
	}
      dPath = [registryPrefix stringByAppendingString: persistentDomain];
      
      if (dinfo->userKey == 0)
	{
	  rc = RegOpenKeyExW(HKEY_CURRENT_USER,
	    UNISTR(dPath),
	    0,
	    STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_READ
	    |KEY_SET_VALUE|KEY_QUERY_VALUE,
	    &(dinfo->userKey));
	  if (rc == ERROR_FILE_NOT_FOUND)
	    {
	      dinfo->userKey = 0;
	    }
	  else if (rc != ERROR_SUCCESS)
	    {
	      NSLog(@"Failed to open registry HKEY_CURRENT_USER\\%@ (%x)",
		dPath, rc);
	      return nil;
	    }
	}
      if (dinfo->systemKey == 0)
	{
	  rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
	    UNISTR(dPath),
	    0,
	    STANDARD_RIGHTS_READ|KEY_QUERY_VALUE,
	    &(dinfo->systemKey));
	  if (rc == ERROR_FILE_NOT_FOUND)
	    {
	      dinfo->systemKey = 0;
	    }
	  else if (rc != ERROR_SUCCESS)
	    {
	      NSLog(@"Failed to open registry HKEY_LOCAL_MACHINE\\%@ (%x)",
		dPath, rc);
	      return nil;
	    }
	}
      
      domainDict = [newDict objectForKey: persistentDomain];
      if (domainDict == nil)
	{
	  domainDict = [NSMutableDictionary dictionary];
	  [newDict setObject: domainDict forKey: persistentDomain];
	}

      if (dinfo->systemKey)
	{
	  DWORD i = 0;
	  unichar *name = malloc(200);
	  unichar *data = malloc(1000);
	  DWORD namelenbuf = 100, datalenbuf = 1000;
	  DWORD type;

	  do
	    {
	      DWORD namelen = namelenbuf, datalen = datalenbuf;

	      rc = RegEnumValueW(dinfo->systemKey,
		i,
		name,
		&namelen,
		NULL,
		&type,
		(void*)data,
		&datalen);
	      if (rc == ERROR_SUCCESS)
		{
		  NS_DURING
		    {
		      id	v;
		      NSString	*k;

		      switch (type)
			{
			  case REG_SZ:
			    {
			      int datacharlen = datalen / 2;
			      if (datacharlen > 0 && data[datacharlen-1] == 0)
				datacharlen--;
				  
			      v = [NSString stringWithCharacters: data
							  length: datacharlen];
			    }
			    break;
			  case REG_BINARY:
			    {
			      v = [NSString stringWithCString: (char*)data
				encoding: NSASCIIStringEncoding];
			    }
			    break;
			  default:
			    NSLog(@"Bad registry type %d for '%S'", type, name);
			    v = 0;
			}
		      v = [v propertyList];
		      if (v)
			{
			  k = [NSString stringWithCharacters: name
						      length: namelen];
			  [domainDict setObject: v forKey: k];
			}
		    }
		  NS_HANDLER
		    NSLog(@"Bad registry value for '%S'", name);
		  NS_ENDHANDLER
		}
	      else if (rc == ERROR_MORE_DATA)
		{
		  if (namelen >= namelenbuf)
		    {
		      namelenbuf = namelen + 1;
		      name = realloc(name, namelenbuf * sizeof(unichar));
		    }
		  if (datalen >= datalenbuf)
		    {
		      datalenbuf = datalen + 1;
		      data = realloc(data, datalenbuf);
		    }
		  continue;
		}
	      else if (rc == ERROR_NO_MORE_ITEMS)
		{
		  break;
		}
	      else
		{
		  NSLog(@"RegEnumValueW error %d", rc);
		  break;
		}
	      i++;
	    } while (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA);
	  free(name);
	  free(data);
	}
      
      if (dinfo->userKey)
	{
	  DWORD i = 0;
	  unichar *name = malloc(200);
	  unichar *data = malloc(1000);
	  DWORD namelenbuf = 100, datalenbuf = 1000;
	  DWORD type;

	  do
	    {
	      DWORD namelen = namelenbuf, datalen = datalenbuf;

		  // RegEnumValueW returns the data as a wide string
		  // but returns the length in bytes.
		  // To add insult to injury, datalen includes the terminating
		  // NULL character, unless there isn't enough room, in which
		  // case it doesn't.
		  
	      rc = RegEnumValueW(dinfo->userKey,
		i,
		name,
		&namelen,
		NULL,
		&type,
		(void*)data,
		&datalen);
	      if (rc == ERROR_SUCCESS)
		{
		  NS_DURING
		    {
		      id	v;
		      NSString	*k;

		      switch (type)
			{
			  case REG_SZ:
			    {
			      int datacharlen = datalen / 2;
			      if (datacharlen > 0 && data[datacharlen-1] == 0)
				datacharlen--;
				  
			      v = [NSString stringWithCharacters: data
							  length: datacharlen];
			    }
			    break;
			  case REG_BINARY:
			    {
			      v = [NSString stringWithCString: (char*)data
				encoding: NSASCIIStringEncoding];
			    }
			    break;
			  default:
			    NSLog(@"Bad registry type %d for '%S'", type, name);
			    v = 0;
			}
		      v = [v propertyList];
		      if (v)
			{
			  k = [NSString stringWithCharacters: name
						      length: namelen];
			  [domainDict setObject: v forKey: k];
			}
		    }
		  NS_HANDLER
		    NSLog(@"Bad registry value for '%S'", name);
		  NS_ENDHANDLER
		}
	      else if (rc == ERROR_MORE_DATA)
		{
		  if (namelen >= namelenbuf)
		    {
		      namelenbuf = namelen + 1;
		      name = realloc(name, namelenbuf * sizeof(unichar));
		    }
		  if (datalen >= datalenbuf)
		    {
		      datalenbuf = datalen + 1;
		      data = realloc(data, datalenbuf);
		    }
		  continue;
		}
	      else if (rc == ERROR_NO_MORE_ITEMS)
		{
		  break;
		}
	      else
		{
		  NSLog(@"RegEnumValueW error %d", rc);
		  break;
		}
	      i++;
	    } while (rc == ERROR_SUCCESS || rc == ERROR_MORE_DATA);
	  free(name);
	  free(data);
	}
    }
  return newDict;
}

- (void) unlockDefaultsFile
{
  return;
}

- (BOOL) wantToReadDefaultsSince: (NSDate*)lastSyncDate
{
  if (lastSyncDate != nil && registryInfo != 0)
    {
      // Detect changes in the registry
      NSMapEnumerator	iter;
      NSString		*domain;
      struct NSUserDefaultsWin32_DomainInfo *dinfo;
      
      iter = NSEnumerateMapTable(registryInfo);
      while (NSNextMapEnumeratorPair(&iter, (void**)&domain, (void**)&dinfo))
	{
	  ULARGE_INTEGER lasttime;
	  LONG rc;
	  NSTimeInterval ti;
	  NSString	*dPath;

	  dPath = [registryPrefix stringByAppendingString: domain];

	  if (dinfo->userKey)
	    {
	      rc = RegQueryInfoKey(dinfo->userKey,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL,NULL, NULL, (PFILETIME)&lasttime);
	      if (rc != ERROR_SUCCESS)
		{
		  NSString	*dName = [@"HKEY_CURRENT_USER\\"
		    stringByAppendingString: dPath];

		  NSLog(@"Failed to query modify time on registry %@ (%x)",
		    dName, rc);
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	      ti = -12622780800.0 + lasttime.QuadPart / 10000000.0;
	      if ([lastSyncDate timeIntervalSinceReferenceDate] < ti)
		{
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	    }
	  else
	    {
	      // If the key didn't exist, but now it does, we want to read it.
	      rc = RegOpenKeyExW(HKEY_CURRENT_USER,
		UNISTR(dPath),
		0,
		STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_READ
		|KEY_SET_VALUE|KEY_QUERY_VALUE,
		&(dinfo->userKey));
	      if (rc == ERROR_FILE_NOT_FOUND)
		{
		  dinfo->userKey = 0;
		}
	      else if (rc != ERROR_SUCCESS)
		{
		  NSString	*dName = [@"HKEY_CURRENT_USER\\"
		    stringByAppendingString: dPath];

		  NSLog(@"Failed to open registry %@ (%x)", dName, rc);
		}
	      else
		{
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	    }
	  if (dinfo->systemKey)
	    {
	      rc = RegQueryInfoKey(dinfo->systemKey,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, (PFILETIME)&lasttime);
	      if (rc != ERROR_SUCCESS)
		{
		  NSLog(@"Failed to query time on HKEY_LOCAL_MACHINE\\%@ (%x)",
		    dPath, rc);
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	      ti = -12622780800.0 + lasttime.QuadPart / 10000000.0;
	      if ([lastSyncDate timeIntervalSinceReferenceDate] < ti)
		{
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	    }
	  else
	    {
	      // If the key didn't exist, but now it does, we want to read it.
	      rc = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
		UNISTR(dPath),
		0,
		STANDARD_RIGHTS_READ|KEY_QUERY_VALUE,
		&(dinfo->systemKey));
	      if (rc == ERROR_FILE_NOT_FOUND)
		{
		  dinfo->systemKey = 0;
		}
	      else if (rc != ERROR_SUCCESS)
		{
		  NSLog(@"Failed to open registry HKEY_LOCAL_MACHINE\\%@ (%x)",
		    dPath, rc);
		}
	      else
		{
		  NSEndMapTableEnumeration(&iter);
		  return YES;
		}
	    }
	}
      NSEndMapTableEnumeration(&iter);
      return NO;
    }
  return YES;
}

- (BOOL) writeDefaults: (NSDictionary*)defaults oldData: (NSDictionary*)oldData
{
  NSEnumerator *iter;
  NSString *persistentDomain;
  
  if (registryInfo == 0)
    {
      registryInfo = NSCreateMapTable(NSObjectMapKeyCallBacks,
	NSOwnedPointerMapValueCallBacks, [defaults count]);
    }

  iter = [defaults keyEnumerator];
  while ((persistentDomain = [iter nextObject]) != nil)
    {
      struct NSUserDefaultsWin32_DomainInfo *dinfo;
      NSDictionary *domainDict;
      NSDictionary *oldDomainDict;
      NSString *dPath;
      LONG rc;
      NSEnumerator *valIter;
      NSString *valName;

      dinfo = NSMapGet(registryInfo, persistentDomain);
      if (dinfo == 0)
	{
	  dinfo = calloc(sizeof(struct NSUserDefaultsWin32_DomainInfo), 1);
	  NSMapInsertKnownAbsent(registryInfo, persistentDomain, dinfo);
	}

      domainDict = [defaults objectForKey: persistentDomain];
      oldDomainDict = [oldData objectForKey: persistentDomain];
      dPath = [registryPrefix stringByAppendingString: persistentDomain];
      
      if ([domainDict count] == 0)
	{
	  continue;
	}
      if (dinfo->userKey == 0)
	{
	  rc = RegCreateKeyExW(HKEY_CURRENT_USER,
	    UNISTR(dPath),
	    0,
	    (LPWSTR) L"",
	    REG_OPTION_NON_VOLATILE,
	    STANDARD_RIGHTS_WRITE|STANDARD_RIGHTS_READ|KEY_SET_VALUE
	    |KEY_QUERY_VALUE,
	    NULL,
	    &(dinfo->userKey),
	    NULL);
	  if (rc != ERROR_SUCCESS)
	    {
	      NSLog(@"Failed to create registry HKEY_CURRENT_USER\\%@ (%x)",
		dPath, rc);
	      return NO;
	    }
	}
      
      valIter = [domainDict keyEnumerator];
      while ((valName = [valIter nextObject]))
	{
	  id value = [domainDict objectForKey: valName];
	  id oldvalue = [oldDomainDict objectForKey: valName];

	  if (oldvalue == nil || [value isEqual: oldvalue] == NO)
	    {
	      NSString			*result = nil;
	      const unichar		*ptr;

	      GSPropertyListMake(value, nil, NO, NO, 0, &result);
	      ptr = UNISTR(result);
	      rc = RegSetValueExW(dinfo->userKey,
		UNISTR(valName),
		0,
		REG_SZ,
		(void*)ptr,
		2*(wcslen(ptr) + 1));
	      if (rc != ERROR_SUCCESS)
		{
		  NSLog(@"Failed to insert HKEY_CURRENT_USER\\%@\\%@ (%x)",
		    dPath, valName, rc);
		  return NO;
		}
	    }
	}
      // Enumerate over the oldvalues and delete the deleted keys.
      valIter = [oldDomainDict keyEnumerator];
      while ((valName = [valIter nextObject]) != nil)
	{
	  if ([domainDict objectForKey: valName] == nil)
	    {
	      // Delete value from registry
	      rc = RegDeleteValueW(dinfo->userKey, UNISTR(valName));
	      if (rc != ERROR_SUCCESS)
		{
		  NSLog(@"Failed to delete HKEY_CURRENT_USER\\%@\\%@ (%x)",
		    dPath, valName, rc);
		  return NO;
		}
	    }
	}
    }
  return YES;
}

@end
