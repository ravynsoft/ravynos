#import <CoreFoundation/CFBase.h>
#import <Foundation/NSRaise.h>
#import <Foundation/NSCFTypeID.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSFileManager.h>
#ifdef WINDOWS
#include <windows.h>
#endif
#import <Foundation/NSPlatform.h>

const CFAllocatorRef kCFAllocatorDefault;
const CFAllocatorRef kCFAllocatorSystemDefault;
const CFAllocatorRef kCFAllocatorMalloc;
const CFAllocatorRef kCFAllocatorMallocZone;
const CFAllocatorRef kCFAllocatorNull;
const CFAllocatorRef kCFAllocatorUseContext;

CFAllocatorRef CFAllocatorGetDefault(void){
   NSUnimplementedFunction();
   return 0;
}

void CFAllocatorSetDefault(CFAllocatorRef self){
}

CFTypeID CFAllocatorGetTypeID(void){
   return kNSCFTypeAllocator;
}

CFAllocatorRef CFAllocatorCreate(CFAllocatorRef self,CFAllocatorContext *context){
   NSUnimplementedFunction();
   return 0;
}

void CFAllocatorGetContext(CFAllocatorRef self,CFAllocatorContext *context){
}
CFIndex CFAllocatorGetPreferredSizeForSize(CFAllocatorRef self,CFIndex size,CFOptionFlags hint){
   NSUnimplementedFunction();
   return 0;
}

void *CFAllocatorAllocate(CFAllocatorRef self,CFIndex size,CFOptionFlags hint){
   NSUnimplementedFunction();
   return 0;
}
void CFAllocatorDeallocate(CFAllocatorRef self,void *ptr){
   NSUnimplementedFunction();
}
void *CFAllocatorReallocate(CFAllocatorRef self,void *ptr,CFIndex size,CFOptionFlags hint){
   NSUnimplementedFunction();
   return 0;
}


CFTypeID CFGetTypeID(CFTypeRef self){
   return [(id)self _cfTypeID];
}

CFTypeRef CFRetain(CFTypeRef self){
	return (CFTypeRef)[(id) self retain];
}

void CFRelease(CFTypeRef self){
	return [(id) self release];
}
CFIndex CFGetRetainCount(CFTypeRef self){
   return [(id) self retainCount];
}

CFAllocatorRef CFGetAllocator(CFTypeRef self){
   NSUnimplementedFunction();
   return 0;
}

CFHashCode CFHash(CFTypeRef self){
	return [(id)self hash];
}

Boolean CFEqual(CFTypeRef self,CFTypeRef other){
	return [(id) self isEqual:(id)other];
}

CFStringRef CFCopyTypeIDDescription(CFTypeID typeID){
	NSUnimplementedFunction();
	return 0;
}

CFStringRef CFCopyDescription(CFTypeRef self){
   return (CFStringRef)[[(id) self description] copy];
}

CFTypeRef CFMakeCollectable(CFTypeRef self){
   //does nothing on cocotron
   return 0;
}

#ifndef MACH

uint64_t mach_absolute_time(void) {
#ifdef WINDOWS
   LARGE_INTEGER value={{0}};

// QueryPerformanceCounter() may jump ahead by seconds on old systems
// http://support.microsoft.com/default.aspx?scid=KB;EN-US;Q274323&
   
   if(!QueryPerformanceCounter(&value))
    return 0;

   return value.QuadPart;
#else
   return 0;
#endif
}

kern_return_t mach_timebase_info(mach_timebase_info_t timebase) {
#ifdef WINDOWS
   LARGE_INTEGER value={{0}};
   
   if(QueryPerformanceFrequency(&value)){
    timebase->numer=1000000000;
    timebase->denom=value.QuadPart;
    return KERN_SUCCESS;
   }
#endif
      
   timebase->numer=1;
   timebase->denom=1;
   return KERN_FAILURE;
}

#endif


#ifdef WINDOWS
unsigned int sleep(unsigned int seconds) {
    Sleep(seconds*1000);
    return 0;
}

int usleep(long useconds) {
    Sleep(useconds/1000);
    return 0;
}

size_t strlcpy(char *dst, const char *src, size_t size) {
   int i,count=size;
   
   for(i=0;i<count-1;i++)
    dst[i]=src[i];
    
   dst[i]='\0';

   return i;
}

char *strnstr(const char *s1,const char *s2, size_t n) {
   if(s2[0]=='\0')
    return (char *)s1;

   size_t i,patLength=strlen(s2);

   for(i=0;s1[i]!='\0' && i+patLength<=n;i++)
    if(strncmp(s1+i,s2,patLength)==0)
     return (char *)(s1+i);
    
   return NULL;
}

void bzero(void *ptr,size_t size){
   size_t i;
   
   for(i=0;i<size;i++)
    ((unsigned char *)ptr)[i]=0;
}

void bcopy(const void *s1, void *s2, size_t n) {
   size_t i;
   
   for(i=0;i<n;i++)
    ((unsigned char *)s2)[i]=((unsigned char *)s1)[i];
}

int bcmp(const void *s1, void *s2, size_t n) {
   size_t i;
   
   for(i=0;i<n;i++)
    if(((unsigned char *)s2)[i]!=((unsigned char *)s1)[i])
     return 1;
   
   return 0;
}

int mkstemps(char *template,int suffixlen) {
   HANDLE result=NULL;
   const char *table="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz";
   int       modulo=strlen(table);
   int       counter=GetTickCount();
   NSString *check;
   int       length=strlen(template),pos=length-suffixlen;
   
   while(--pos>=0){
    if(template[pos]!='X'){
     pos++;
     break;
    }
   }
   
   int i,failSafe=0;
   char try[length+1];
   
   do {
    
    for(i=0;i<length;i++)
     if(i<pos || i>=length-suffixlen)
      try[i]=template[i];
     else {
      try[i]=table[counter%modulo];
      counter+=i;
     }
     
    try[i]=0;
    
    check=[NSString stringWithUTF8String:try];
    
    NSLog(@"mkstemps try=%@",check);
    
    result=CreateFileW([check fileSystemRepresentationW],GENERIC_WRITE|GENERIC_READ,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    
    failSafe++;
    
   }while(result==NULL && failSafe<100);

   if(result!=NULL){
    for(i=0;i<length;i++)
     template[i]=try[i];
    template[i]=0;
   }
   
   return (int)result;
}

long random(void) {
// rand() is only good for 15 bits, random() returns 31
   return (rand()<<16)|(rand()<<1)|(rand()&0x1);
}

#endif
