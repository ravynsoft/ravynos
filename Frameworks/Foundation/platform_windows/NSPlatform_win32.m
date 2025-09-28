/* Copyright (c) 2006-2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */
#ifdef WINDOWS

#import <Foundation/NSPlatform_win32.h>
#import <Foundation/NSString_win32.h>
#import <Foundation/NSHandleMonitor_win32.h>
#import <Foundation/NSHandleMonitorSet_win32.h>
#import <Foundation/NSTask_win32.h>
#import <Foundation/NSFileHandle_win32.h>
#import <Foundation/NSPipe_win32.h>
#import <Foundation/NSLock_win32.h>
#import <Foundation/NSRecursiveLock_win32.h>
#import <Foundation/NSConditionLock_win32.h>
#import <Foundation/NSPersistantDomain_win32.h>
#import <Foundation/NSTimeZone_win32.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSData.h>
#import <Foundation/NSPropertyListReader.h>
#import <Foundation/NSPropertyListWriter_vintage.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSError.h>
#import <stdlib.h>
#import <winsock.h>
#import <Foundation/NSSocket_windows.h>
#import <Foundation/NSParentDeathMonitor_win32.h>
#import <Foundation/NSSelectInputSourceSet.h>
#import <Foundation/NSCondition_win32.h>
#include <stdio.h>

#import <objc/runtime.h>

NSString *NSPlatformClassName=@"NSPlatform_win32";

#define MAXHOSTNAMELEN 512

@class NSConditionLock_win32;

static NSString *convertBackslashToSlash(NSString *string){
   NSUInteger i,length=[string length];
   unichar    buffer[length];
   
   [string getCharacters:buffer];
   
   for(i=0;i<length;i++){
    if(buffer[i]=='\\')
     buffer[i]='/';
   }
   
   return [NSString stringWithCharacters:buffer length:length];
}

static NSError *NSErrorForGetLastErrorCode(DWORD code)
{
	NSString *localizedDescription=@"NSErrorForGetLastError localizedDescription";
	unichar  *message;
	
	FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,code,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPWSTR) &message,0, NULL );
	localizedDescription=NSStringFromNullTerminatedUnicode(message);
	
	LocalFree(message);
	
	return [NSError errorWithDomain:NSWin32ErrorDomain code:code userInfo:[NSDictionary dictionaryWithObject:localizedDescription forKey:NSLocalizedDescriptionKey]];
}

static NSError *NSErrorForGetLastError()
{
	return NSErrorForGetLastErrorCode(GetLastError());
}

@implementation NSPlatform_win32

static NSString *processName(){
   return [[convertBackslashToSlash([NSString stringWithUTF8String:objc_mainImageName()]) lastPathComponent] stringByDeletingPathExtension];
}

-init {
   NSString   *entry;
   const char *module=class_getImageName(isa);
   HKEY        handle;
   DWORD       disposition,allowed;
   int         i;
   
   [NSSocket_windows class]; // initialize winsock

   entry=[@"SYSTEM\\CurrentControlSet\\Services\\Eventlog\\Application\\" stringByAppendingString:processName()];

   if(RegCreateKeyEx(HKEY_LOCAL_MACHINE,[entry cString],0,NULL,
     REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&handle,&disposition))
    ; // oh well, an error

   if(RegSetValueEx(handle,"EventMessageFile",0,REG_EXPAND_SZ,
      (LPBYTE)module,strlen(module)+1))
    ; // oh well, an error

   allowed=EVENTLOG_ERROR_TYPE|EVENTLOG_WARNING_TYPE|EVENTLOG_INFORMATION_TYPE;
   if(RegSetValueEx(handle,"TypesSupported",0,REG_DWORD,
      (LPBYTE)&allowed,sizeof(DWORD)))
    ; // oh well, an error

   RegCloseKey(handle);

   _parentDeathMonitor=[[NSParentDeathMonitor_win32 alloc] init];
   
   for(i=1;i<__argc;i++)
    if(strcmp(__argv[i],"-Console")==0){
    // we could check for presence of AttachConsole and use that instead
     AllocConsole();
    }
    
   return self;
}

-(NSInputSource *)parentDeathInputSource {
   return [_parentDeathMonitor handleMonitor];
}

-(Class)taskClass {
   return [NSTask_win32 class];
}

-(Class)pipeClass {
   return [NSPipe_win32 class];
}

-(Class)lockClass {
   return [NSLock_win32 class];
}

-(Class)conditionLockClass {
   return [NSConditionLock_win32 class];
}

-(Class)recursiveLockClass {
    return [NSRecursiveLock_win32 class];
}

-(Class)persistantDomainClass {
   return [NSPersistantDomain_win32 class];
}

-(Class)timeZoneClass {
    return [NSTimeZone_win32 class];
}

-(Class)conditionClass {
	return [NSCondition_win32 class];
}

-(NSString *)userName {
   NSString *result=[[[NSProcessInfo processInfo] environment] objectForKey:@"USERNAME"];

   if(result==nil)
    [NSException raise:NSInvalidArgumentException format:@"NSProcessInfo environment USERNAME failed"];

   return result;
}

-(NSString *)fullUserName {
   NSString *result=[[[NSProcessInfo processInfo] environment] objectForKey:@"USERNAME"];

   if(result==nil)
    [NSException raise:NSInvalidArgumentException format:@"NSProcessInfo environment USERNAME failed"];

   return result;
}

-(NSString *)homeDirectory {
   NSString *drive=[[[NSProcessInfo processInfo] environment] objectForKey:@"HOMEDRIVE"];
   NSString *path=[[[NSProcessInfo processInfo] environment] objectForKey:@"HOMEPATH"];

   if(drive==nil)
    return nil;

   if(path==nil)
    return nil;

   return convertBackslashToSlash([drive stringByAppendingPathComponent:path]);
}

-(NSString *)libraryDirectory {
	NSString *appdata=[[[NSProcessInfo processInfo] environment] objectForKey:@"APPDATA"];
	
	return convertBackslashToSlash([appdata stringByAppendingPathComponent:@"CocotronLibrary"]);
}

-(NSString *)temporaryDirectory {
   NSString *result=[[[NSProcessInfo processInfo] environment] objectForKey:@"TEMP"];

   if(result==nil)
    result=[[[NSProcessInfo processInfo] environment] objectForKey:@"TMP"];

   if(result==nil){
    result=[[[NSProcessInfo processInfo] environment] objectForKey:@"windir"];
    result=[result stringByAppendingPathComponent:@"Temp"];
   }
   
   if(result==nil)
    result=[[self homeDirectory] stringByAppendingPathComponent:@"Temp"];

   return convertBackslashToSlash(result);
}

NSString * const NSPlatformExecutableDirectory=@"Windows";
NSString * const NSPlatformResourceNameSuffix=@"windows";

NSString * const NSPlatformExecutableFileExtension=@"exe";
NSString * const NSPlatformLoadableObjectFileExtension=@"dll";
NSString * const NSPlatformLoadableObjectFilePrefix=@"";

-(NSArray *)arguments {
    NSMutableArray *result=[NSMutableArray array];
    int             i;
 	
    // Parse the program arguments as unicode
    LPWSTR cmd = GetCommandLineW();
    int argc = 0;
    LPWSTR *argv = CommandLineToArgvW(cmd, &argc);
    if (argv) {
        for(i=0;i<argc;i++) {
            [result addObject:[NSString stringWithCharacters:(unichar *)argv[i] length:wcslen(argv[i])]];
        }
        LocalFree(argv);
    }
 	
    return result;
}

-(NSDictionary *)environment {
   id      *objects,*keys;
   NSUInteger count;

   char  *envString=GetEnvironmentStrings();
   char **env;
   char  *keyValue;
   int    i,len,max;
   char  *run;

   for(count=0,run=envString;*run;count++)
    run+=strlen(run)+1;

   env=__builtin_alloca(sizeof(char *)*(count+1));
   for(count=0,run=envString;*run;count++){
    env[count]=run;
    run+=strlen(run)+1;
   }
   env[count]=NULL;

// env is unix style environment at this point

   max=0;
   for(count=0;env[count];count++)
    if((len=strlen(env[count]))>max)
     max=len;

   keyValue=__builtin_alloca(max+1);
   objects=__builtin_alloca(sizeof(id)*count);
   keys=__builtin_alloca(sizeof(id)*count);

   for(count=0;env[count];count++){
    len=strlen(strcpy(keyValue,env[count]));

    for(i=0;i<len;i++)
     if(keyValue[i]=='=')
      break;
    keyValue[i]='\0';

    objects[count]=[NSString stringWithCString:keyValue+i+1];
    keys[count]=[NSString stringWithCString:keyValue];
    [self checkEnvironmentKey:keys[count] value:objects[count]];
   }

   FreeEnvironmentStrings(envString);
   
   return [[NSDictionary allocWithZone:NULL] initWithObjects:objects forKeys:keys
     count:count];
}

NSTimeInterval NSPlatformTimeIntervalSinceReferenceDate() {
   SYSTEMTIME      systemTime;
   FILETIME        fileTime;

   GetSystemTime(&systemTime);
   SystemTimeToFileTime(&systemTime,&fileTime);

   return Win32TimeIntervalFromFileTime(fileTime);
}

int NSPlatformProcessID() {
   return GetCurrentProcessId();
}

NSUInteger NSPlatformThreadID() {
   return GetCurrentThreadId();
}

-(NSString *)hostName {
   DWORD length=MAX_COMPUTERNAME_LENGTH;
   char  name[length+1];

   if(!GetComputerName(name,&length)){
    name[0]='\0';
    return NO;
   }

   return [NSString stringWithCString:name];
}

-(NSString *)DNSHostName {
   char cString[MAXHOSTNAMELEN+1];
   int  err;

   cString[0]='\0';

   if((err=gethostname(cString,MAXHOSTNAMELEN))!=0){
    [NSException raise:NSInternalInconsistencyException
                format:@"gethostname failed with %d",err];
   }

   if(cString[0]=='\0'){
    [NSException raise:NSInternalInconsistencyException
                format:@"gethostname() failed with zero-length string"];
   }

   return [NSString stringWithCString:cString];
}

-(NSArray *)addressesForDNSHostName:(NSString *)name {
   NSMutableArray *result=[NSMutableArray array];
   char            cString[MAXHOSTNAMELEN+1];
   struct hostent *hp;

   [name getCString:cString maxLength:MAXHOSTNAMELEN];

   if((hp=gethostbyname(cString))==NULL)
    return nil;
   else {
    unsigned long **addr_list=(unsigned long **)hp->h_addr_list;
    int             i;

    for(i=0;addr_list[i]!=NULL;i++){
     struct in_addr addr;
     NSString      *string;

     addr.s_addr=*(addr_list[i]);

     string=[NSString stringWithCString:inet_ntoa(addr)];

     [result addObject:string];
    }

    return result;
   }
}

-(NSString *)hostNameByAddress:(NSString *)address
{
    struct in_addr addr;
    struct hostent *remoteHost;
    addr.s_addr = inet_addr([address cString]);
    if (addr.s_addr == INADDR_NONE) {
        return nil;
    }
    remoteHost = gethostbyaddr((char *) &addr, 4, AF_INET);
    if(remoteHost == NULL)
        return nil;
    
    return [NSString stringWithCString:remoteHost->h_name];
}

void NSPlatformSleepThreadForTimeInterval(NSTimeInterval interval) {
   Win32ThreadSleepForTimeInterval(interval);
}

void NSPlatformLogString(NSString *string) {
   NSData     *data=[NSPropertyListWriter_vintage nullTerminatedASCIIDataWithString:string];
   const char *cString=[data bytes];
   NSUInteger    length=[data length]-1; // skip 0

   fprintf(stderr, "%s", cString);
   if(length==0 || cString[length-1]!='\n')
    fprintf(stderr, "\n");
   fflush(stderr);

 // HANDLE handle=OpenEventLog(NULL,[processName() cString]);
   static HANDLE eventLog=NULL;
   
   if(eventLog==NULL){
    eventLog=RegisterEventSource(NULL,[processName() cString]);
   }
   
   ReportEvent(eventLog,EVENTLOG_INFORMATION_TYPE,1,1,NULL,1,0,&cString,NULL);
 //  CloseEventLog(handle);
}

void *NSPlatformContentsOfFile(NSString *path,NSUInteger *lengthp) {
   HANDLE file=CreateFileW([path fileSystemRepresentationW],GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN,NULL);
   DWORD  length,readLength;
   void  *result;

   if(file==INVALID_HANDLE_VALUE)
    return NULL;

   if(GetFileType(file)!=FILE_TYPE_DISK){
    CloseHandle(file);
    return NULL;
   }

   length=GetFileSize(file,NULL);
   if(length==0xFFFFFFFF){
    CloseHandle(file);
    return NULL;
   }

   result=NSZoneMalloc(NULL,length);

   if(!ReadFile(file,result,length,&readLength,NULL)){
    NSZoneFree(NULL,result);
    CloseHandle(file);
    return NULL;
   }

   CloseHandle(file);

   *lengthp=readLength;
   return result;
}

-(void *)mapContentsOfFile:(NSString *)path length:(NSUInteger *)lengthp {
   void    *result;
   HANDLE   file=CreateFileW([path fileSystemRepresentationW],GENERIC_READ,0,NULL,
      OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
   HANDLE   mapping;

   if(file==INVALID_HANDLE_VALUE)
    return NULL;

   *lengthp=GetFileSize(file,NULL);
   if(*lengthp==0xFFFFFFFF){
    CloseHandle(file);
    return NULL;
   }
   if(*lengthp==0)
    return "";

   mapping=CreateFileMapping(file,NULL,PAGE_READONLY,0,0,NULL);
   if(mapping==NULL){
    CloseHandle(file);
    return NULL;
   }

   result=MapViewOfFile(mapping,FILE_MAP_READ,0,0,0);
   CloseHandle(mapping);
   CloseHandle(file);

   return result;
}

-(void)unmapAddress:(void *)ptr length:(NSUInteger)length {
   if(length>0){
    if(!UnmapViewOfFile(ptr))
     Win32Assert("UnmapViewOfFile()");
   }
}

-(BOOL)writeContentsOfFile:(NSString *)path bytes:(const void *)bytes length:(NSUInteger)length options:(NSUInteger)options error:(NSError **)errorp {
   BOOL atomically=(options&NSAtomicWrite);
   HANDLE   file;
   DWORD    wrote;
   const uint16_t *pathW=[path fileSystemRepresentationW];

   atomically=NO;
   
   if(atomically){
    NSString *backup=[path stringByAppendingString:@"##"];
    const uint16_t *backupW=[backup fileSystemRepresentationW];

    file=CreateFileW(backupW,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(!WriteFile(file,bytes,length,&wrote,NULL)){
     CloseHandle(file);
    }
    else {
     CloseHandle(file);

     if(wrote!=length){
      DeleteFileW(backupW);
     }
     else {
      
      if(MoveFileExW(backupW,pathW,MOVEFILE_REPLACE_EXISTING)){
       return YES;
     }
    }
    }
    // atomic failure drops through to non-atomic
   }

   file=CreateFileW(pathW,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);   
   if(!WriteFile(file,bytes,length,&wrote,NULL)){
    if (errorp) *errorp = NSErrorForGetLastError();
    CloseHandle(file);
    return NO;
   }

   CloseHandle(file);

   return (wrote==length)?YES:NO;
}

@end

void _Win32Assert (const char *code,int line,const char *file) {
   DWORD lastError=GetLastError();

   if(lastError)
    [NSException raise:@"Win32AssertFailedException"
      format:@"%s failed with code %d at %s:%d",code,lastError,file,line];
}

#define NSTimeIntervalSince1601		12622780800.0L
NSTimeInterval Win32TimeIntervalFromFileTime(FILETIME fileTime){
   long long       file64;
   NSTimeInterval  interval;

   file64=fileTime.dwHighDateTime;
   file64<<=32;
   file64|=fileTime.dwLowDateTime;
   interval=file64;
   interval/=10000000.0;

   return interval-NSTimeIntervalSince1601;
}

void Win32ThreadSleepForTimeInterval(NSTimeInterval interval) {
   while(interval>0){
    NSTimeInterval chunk=(interval>1000000)?1000000:interval;

    Sleep(chunk*1000);

    interval-=chunk;
   }
}

BOOL NSPlatformGreaterThanOrEqualToWindowsXP(void) {
   OSVERSIONINFOEX osVersion;
    
   osVersion.dwOSVersionInfoSize=sizeof(osVersion);
   GetVersionEx((OSVERSIONINFO *)&osVersion);

   return ((osVersion.dwMajorVersion==5 && osVersion.dwMinorVersion>=1) || osVersion.dwMajorVersion>5)?YES:NO;
}

BOOL NSPlatformGreaterThanOrEqualToWindows2000(void) {
   OSVERSIONINFOEX osVersion;
    
   osVersion.dwOSVersionInfoSize=sizeof(osVersion);
   GetVersionEx((OSVERSIONINFO *)&osVersion);

   return (osVersion.dwMajorVersion>=5)?YES:NO;
}

#endif

