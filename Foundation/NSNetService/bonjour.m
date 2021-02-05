#import "bonjour.h"
#import <Foundation/NSNetServices.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>

#ifdef WINDOWS
#include <windows.h>

HINSTANCE hDNSDLL = NULL;

static void bonjourInitializeIfNeeded() {
   static boolean initialized = 0;

   if(!initialized){
	if((hDNSDLL=LoadLibrary("dnssd.dll"))==NULL)
     NSLog(@"dnssd.dll not present");
   }
   
   initialized = 1;
}

static BOOL bonjourNotPresent() {
   bonjourInitializeIfNeeded();
        
	return (hDNSDLL == NULL)?YES:NO;
}

static FARPROC bonjour_function(const char *name) {
   if(bonjourNotPresent())
    return NULL;
   else {
    FARPROC result=GetProcAddress(hDNSDLL, name);
    
    if(result==NULL)
     NSLog(@"GetProcAddress(dnssd.dll,name) failed");
     
    return result;
   }
}
#else

static void *bonjour_function(const char *name) {
   NSLog(@"bonjour_%s",name);
   return NULL;
}

#endif

int bonjour_DNSServiceRefSockFD(bonjour_DNSServiceRef service){
   BONJOUR_CALL typeof(bonjour_DNSServiceRefSockFD) *function=(typeof(function))bonjour_function("DNSServiceRefSockFD");
    
   if(function!=NULL)
    return function(service);
   else
    return  -1;
}

bonjour_DNSServiceErrorType bonjour_DNSServiceProcessResult(bonjour_DNSServiceRef service){
   BONJOUR_CALL typeof(bonjour_DNSServiceProcessResult) *function=(typeof(function))bonjour_function("DNSServiceProcessResult");
    
   if(function!=NULL)
    return function(service);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}


void  bonjour_DNSServiceRefDeallocate(bonjour_DNSServiceRef service){
   BONJOUR_CALL typeof(bonjour_DNSServiceRefDeallocate) *function=(typeof(function))bonjour_function("DNSServiceRefDeallocate");

   if(function!=NULL)
    function(service);
}


bonjour_DNSServiceErrorType  bonjour_DNSServiceEnumerateDomains(bonjour_DNSServiceRef *service,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,bonjour_DNSServiceDomainEnumReply callBack,void *context){
   BONJOUR_CALL typeof(bonjour_DNSServiceEnumerateDomains) *function=(typeof(function))bonjour_function("DNSServiceEnumerateDomains");

   if(function!=NULL)
    return function(service,flags,interfaceIndex,callBack,context);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceRegister(bonjour_DNSServiceRef *service,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *name,     const char *regtype,const char *domain,       const char *host,         uint16_t port,uint16_t txtLen,const void *txtRecord,    bonjour_DNSServiceRegisterReply callBack,void *context){
   BONJOUR_CALL typeof(bonjour_DNSServiceRegister) *function=(typeof(function))bonjour_function("DNSServiceRegister");

   if(function!=NULL)
    return function(service,flags,interfaceIndex,name,regtype,domain,host,port,txtLen,txtRecord,callBack,context);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceAddRecord(bonjour_DNSServiceRef service,bonjour_DNSRecordRef *RecordRef,bonjour_DNSServiceFlags flags,uint16_t rrtype,uint16_t rdlen,const void *rdata,uint32_t ttl){
   BONJOUR_CALL typeof(bonjour_DNSServiceAddRecord) *function=(typeof(function))bonjour_function("DNSServiceAddRecord");

   if(function!=NULL)
    return function(service,RecordRef,flags,rrtype,rdlen,rdata,ttl);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}


bonjour_DNSServiceErrorType  bonjour_DNSServiceUpdateRecord(bonjour_DNSServiceRef service,bonjour_DNSRecordRef RecordRef,bonjour_DNSServiceFlags flags,uint16_t rdlen,const void *rdata,uint32_t ttl){
   BONJOUR_CALL typeof(bonjour_DNSServiceUpdateRecord) *function=(typeof(function))bonjour_function("DNSServiceUpdateRecord");

   if(function!=NULL)
    return function(service,RecordRef,     flags,rdlen,rdata,ttl);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}


bonjour_DNSServiceErrorType  bonjour_DNSServiceRemoveRecord(bonjour_DNSServiceRef service,bonjour_DNSRecordRef RecordRef,bonjour_DNSServiceFlags flags){
   BONJOUR_CALL typeof(bonjour_DNSServiceRemoveRecord) *function=(typeof(function))bonjour_function("DNSServiceRemoveRecord");

   if(function!=NULL)
    return function(service,RecordRef,flags);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceBrowse(bonjour_DNSServiceRef *service,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *regtype,const char *domain,    bonjour_DNSServiceBrowseReply callBack,void *context){
   BONJOUR_CALL typeof(bonjour_DNSServiceBrowse) *function=(typeof(function))bonjour_function("DNSServiceBrowse");

   if(function!=NULL)
    return function(service,flags,interfaceIndex,regtype,domain,callBack,context);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceResolve
(bonjour_DNSServiceRef *service,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *name,const char *regtype,const char *domain,bonjour_DNSServiceResolveReply callBack,void *context  ){
   BONJOUR_CALL typeof(bonjour_DNSServiceResolve) *function=(typeof(function))bonjour_function("DNSServiceResolve");

   if(function!=NULL)
    return function(service,flags,interfaceIndex,name,regtype,domain,callBack,context  );
   else
    return  bonjour_kDNSServiceErr_Unknown;
}


bonjour_DNSServiceErrorType  bonjour_DNSServiceCreateConnection(bonjour_DNSServiceRef *service){
   BONJOUR_CALL typeof(bonjour_DNSServiceCreateConnection) *function=(typeof(function))bonjour_function("DNSServiceCreateConnection");

   if(function!=NULL)
    return function(service);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceRegisterRecord(bonjour_DNSServiceRef service,bonjour_DNSRecordRef *RecordRef,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *fullname,uint16_t rrtype,uint16_t rrclass,uint16_t rdlen,const void *rdata,uint32_t ttl,bonjour_DNSServiceRegisterRecordReply callBack,void *context){
   BONJOUR_CALL typeof(bonjour_DNSServiceRegisterRecord) *function=(typeof(function))bonjour_function("DNSServiceRegisterRecord");

   if(function!=NULL)
    return function(service,RecordRef,flags,interfaceIndex,fullname,rrtype,rrclass,rdlen,rdata,ttl,callBack,context    );
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceQueryRecord(bonjour_DNSServiceRef *service,bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *fullname,uint16_t rrtype,uint16_t rrclass,bonjour_DNSServiceQueryRecordReply callBack,void *context){
   BONJOUR_CALL typeof(bonjour_DNSServiceQueryRecord) *function=(typeof(function))bonjour_function("DNSServiceQueryRecord");

   if(function!=NULL)
    return function(service,flags,interfaceIndex,fullname,rrtype,rrclass,callBack,context  );
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_DNSServiceReconfirmRecord(bonjour_DNSServiceFlags flags,uint32_t interfaceIndex,const char *fullname,uint16_t rrtype,uint16_t rrclass,uint16_t rdlen,const void *rdata){
   BONJOUR_CALL typeof(bonjour_DNSServiceReconfirmRecord) *function=(typeof(function))bonjour_function("DNSServiceReconfirmRecord");

   if(function!=NULL)
    return function(flags,interfaceIndex,fullname,rrtype,rrclass,rdlen,rdata);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

int bonjour_DNSServiceConstructFullName(char *fullName,const char *service,const char *regtype,const char *domain){
   BONJOUR_CALL typeof(bonjour_DNSServiceConstructFullName) *function=(typeof(function))bonjour_function("DNSServiceConstructFullName");

   if(function!=NULL)
    return function(fullName,service,regtype,domain);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

void  bonjour_TXTRecordCreate(bonjour_TXTRecordRef *txtRecord,uint16_t bufferLen,void *buffer){
   BONJOUR_CALL typeof(bonjour_TXTRecordCreate) *function=(typeof(function))bonjour_function("TXTRecordCreate");

   if(function!=NULL)
    function(txtRecord,bufferLen,buffer);
}

void  bonjour_TXTRecordDeallocate(bonjour_TXTRecordRef *txtRecord){
   BONJOUR_CALL typeof(bonjour_TXTRecordDeallocate) *function=(typeof(function))bonjour_function("TXTRecordDeallocate");

   if(function!=NULL)
    function(txtRecord);
}

bonjour_DNSServiceErrorType  bonjour_TXTRecordSetValue(bonjour_TXTRecordRef *txtRecord,const char *key,uint8_t valueSize,const void *value){
   BONJOUR_CALL typeof(bonjour_TXTRecordSetValue) *function=(typeof(function))bonjour_function("TXTRecordSetValue");

   if(function!=NULL)
    return function(txtRecord,key,valueSize,value);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

bonjour_DNSServiceErrorType  bonjour_TXTRecordRemoveValue(bonjour_TXTRecordRef *txtRecord,const char *key){
   BONJOUR_CALL typeof(bonjour_TXTRecordRemoveValue) *function=(typeof(function))bonjour_function("TXTRecordRemoveValue");

   if(function!=NULL)
    return function(txtRecord,key);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

uint16_t  bonjour_TXTRecordGetLength(const bonjour_TXTRecordRef *txtRecord){
   BONJOUR_CALL typeof(bonjour_TXTRecordGetLength) *function=(typeof(function))bonjour_function("TXTRecordGetLength");

   if(function!=NULL)
    return function(txtRecord);
   else
    return  0;
}

const void *bonjour_TXTRecordGetBytesPtr(const bonjour_TXTRecordRef *txtRecord){
   BONJOUR_CALL typeof(bonjour_TXTRecordGetBytesPtr) *function=(typeof(function))bonjour_function("TXTRecordGetBytesPtr");

   if(function!=NULL)
    return function(txtRecord);
   else
    return  NULL;
}

int  bonjour_TXTRecordContainsKey(uint16_t txtLen,const void *txtRecord,const char *key){
   BONJOUR_CALL typeof(bonjour_TXTRecordContainsKey) *function=(typeof(function))bonjour_function("TXTRecordContainsKey");

   if(function!=NULL)
    return function(txtLen,txtRecord,key);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

const void *  bonjour_TXTRecordGetValuePtr(uint16_t txtLen,const void *txtRecord,const char *key,uint8_t *valueLen){
   BONJOUR_CALL typeof(bonjour_TXTRecordGetValuePtr) *function=(typeof(function))bonjour_function("TXTRecordGetValuePtr");

   if(function!=NULL)
    return function(txtLen,txtRecord,key,valueLen);
   else
    return  NULL;
}

uint16_t  bonjour_TXTRecordGetCount(uint16_t txtLen,const void *txtRecord){
   BONJOUR_CALL typeof(bonjour_TXTRecordGetCount) *function=(typeof(function))bonjour_function("TXTRecordGetCount");

   if(function!=NULL)
    return function(txtLen,txtRecord);
   else
    return  0;
}

bonjour_DNSServiceErrorType  bonjour_TXTRecordGetItemAtIndex(uint16_t txtLen,const void *txtRecord,uint16_t index,uint16_t keyBufLen,char *key,uint8_t *valueLen,const void **value){
   BONJOUR_CALL typeof(bonjour_TXTRecordGetItemAtIndex) *function=(typeof(function))bonjour_function("TXTRecordGetItemAtIndex");

   if(function!=NULL)
    return function(txtLen,txtRecord,index,keyBufLen,key,valueLen,value);
   else
    return  bonjour_kDNSServiceErr_Unknown;
}

int bonjour_ConvertError(int errorCode){
  
  switch( errorCode ){
    case bonjour_kDNSServiceErr_Unknown:
      return NSNetServicesUnknownError;
    
    case bonjour_kDNSServiceErr_NoSuchName:
      return NSNetServicesNotFoundError;
    
    case bonjour_kDNSServiceErr_NoMemory:
      return NSNetServicesUnknownError;
    
    case bonjour_kDNSServiceErr_BadParam:
    case bonjour_kDNSServiceErr_BadReference:
    case bonjour_kDNSServiceErr_BadState:
    case bonjour_kDNSServiceErr_BadFlags:
      return NSNetServicesBadArgumentError;
    
    case bonjour_kDNSServiceErr_Unsupported:
      return NSNetServicesUnknownError;
    
    case bonjour_kDNSServiceErr_NotInitialized:
      return NSNetServicesInvalidError;
    
    case bonjour_kDNSServiceErr_AlreadyRegistered:
    case bonjour_kDNSServiceErr_NameConflict:
      return NSNetServicesCollisionError;
    
    case bonjour_kDNSServiceErr_Invalid:
      return NSNetServicesInvalidError;
    
    case bonjour_kDNSServiceErr_Firewall:
      return NSNetServicesUnknownError;
    
    case bonjour_kDNSServiceErr_Incompatible:
      // The client library is incompatible with the daemon
      return NSNetServicesInvalidError;
    
    case bonjour_kDNSServiceErr_BadInterfaceIndex:
    case bonjour_kDNSServiceErr_Refused:
      return NSNetServicesUnknownError;
    
    case bonjour_kDNSServiceErr_NoSuchRecord:
    case bonjour_kDNSServiceErr_NoAuth:
    case bonjour_kDNSServiceErr_NoSuchKey:
      return NSNetServicesNotFoundError;
    
    case bonjour_kDNSServiceErr_NATTraversal:
    case bonjour_kDNSServiceErr_DoubleNAT:
    case bonjour_kDNSServiceErr_BadTime:
      return NSNetServicesUnknownError;
  }

  return errorCode;
}

NSDictionary *bonjour_CreateError(id sender,int errorCode){
  NSMutableDictionary *dictionary = nil;
  int error = 0;
  
  dictionary = [NSMutableDictionary dictionary];
  error = bonjour_ConvertError(errorCode);
  
  [dictionary setObject: [NSNumber numberWithInt: error]
                 forKey: NSNetServicesErrorCode];
  [dictionary setObject: sender
                 forKey: NSNetServicesErrorDomain];

  return dictionary; // autorelease'd
}
