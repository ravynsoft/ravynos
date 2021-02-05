#import <Foundation/NSDictionary.h>
#import <Foundation/NSNumber.h>

#ifdef WINDOWS
#define BONJOUR_CALL __stdcall
#else
#define BONJOUR_CALL
#endif

// Indirection layer which avoids hard dependancy on Bonjour LIB and SDK with good fallback behavior
// All types and constants must match dns_sd.h

typedef struct bonjour_DNSService *bonjour_DNSServiceRef;
typedef struct bonjour_DNSRecord *bonjour_DNSRecordRef;
typedef struct { uint64_t _[2]; } bonjour_TXTRecordRef;
typedef uint32_t bonjour_DNSServiceFlags;
typedef int32_t bonjour_DNSServiceErrorType;

typedef void(BONJOUR_CALL *bonjour_DNSServiceDomainEnumReply)(bonjour_DNSServiceRef service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, bonjour_DNSServiceErrorType errorCode, const char *replyDomain, void *context);
typedef void(BONJOUR_CALL *bonjour_DNSServiceRegisterReply)(bonjour_DNSServiceRef service, bonjour_DNSServiceFlags flags, bonjour_DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context);
typedef void(BONJOUR_CALL *bonjour_DNSServiceBrowseReply)(bonjour_DNSServiceRef service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, bonjour_DNSServiceErrorType errorCode, const char *serviceName, const char *regtype, const char *replyDomain, void *context);
typedef void(BONJOUR_CALL *bonjour_DNSServiceRegisterRecordReply)(bonjour_DNSServiceRef service, bonjour_DNSRecordRef RecordRef, bonjour_DNSServiceFlags flags, bonjour_DNSServiceErrorType errorCode, void *context);
typedef void(BONJOUR_CALL *bonjour_DNSServiceQueryRecordReply)(bonjour_DNSServiceRef DNSServiceRef, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, bonjour_DNSServiceErrorType errorCode, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl, void *context);
typedef void(BONJOUR_CALL *bonjour_DNSServiceResolveReply)(bonjour_DNSServiceRef service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, bonjour_DNSServiceErrorType errorCode, const char *fullname, const char *hosttarget, uint16_t port, uint16_t txtLen, const unsigned char *txtRecord, void *context);

enum {
    bonjour_kDNSServiceClass_IN = 1,
};

enum {
    bonjour_kDNSServiceErr_NoError = 0,
    bonjour_kDNSServiceErr_Unknown = -65537,
    bonjour_kDNSServiceErr_NoSuchName = -65538,
    bonjour_kDNSServiceErr_NoMemory = -65539,
    bonjour_kDNSServiceErr_BadParam = -65540,
    bonjour_kDNSServiceErr_BadReference = -65541,
    bonjour_kDNSServiceErr_BadState = -65542,
    bonjour_kDNSServiceErr_BadFlags = -65543,
    bonjour_kDNSServiceErr_Unsupported = -65544,
    bonjour_kDNSServiceErr_NotInitialized = -65545,
    bonjour_kDNSServiceErr_AlreadyRegistered = -65547,
    bonjour_kDNSServiceErr_NameConflict = -65548,
    bonjour_kDNSServiceErr_Invalid = -65549,
    bonjour_kDNSServiceErr_Firewall = -65550,
    bonjour_kDNSServiceErr_Incompatible = -65551,
    bonjour_kDNSServiceErr_BadInterfaceIndex = -65552,
    bonjour_kDNSServiceErr_Refused = -65553,
    bonjour_kDNSServiceErr_NoSuchRecord = -65554,
    bonjour_kDNSServiceErr_NoAuth = -65555,
    bonjour_kDNSServiceErr_NoSuchKey = -65556,
    bonjour_kDNSServiceErr_NATTraversal = -65557,
    bonjour_kDNSServiceErr_DoubleNAT = -65558,
    bonjour_kDNSServiceErr_BadTime = -65559,
};

enum {
    bonjour_kDNSServiceFlagsMoreComing = 0x1,
    bonjour_kDNSServiceFlagsAdd = 0x2,
    bonjour_kDNSServiceFlagsDefault = 0x4,
    bonjour_kDNSServiceFlagsNoAutoRename = 0x8,
    bonjour_kDNSServiceFlagsShared = 0x10,
    bonjour_kDNSServiceFlagsUnique = 0x20,
    bonjour_kDNSServiceFlagsBrowseDomains = 0x40,
    bonjour_kDNSServiceFlagsRegistrationDomains = 0x80,
    bonjour_kDNSServiceFlagsLongLivedQuery = 0x100,
    bonjour_kDNSServiceFlagsAllowRemoteQuery = 0x200,
    bonjour_kDNSServiceFlagsForceMulticast = 0x400,
    bonjour_kDNSServiceFlagsReturnCNAME = 0x800,
};

enum {
    bonjour_kDNSServiceType_A = 1,
    bonjour_kDNSServiceType_NS = 2,
    bonjour_kDNSServiceType_MD = 3,
    bonjour_kDNSServiceType_MF = 4,
    bonjour_kDNSServiceType_CNAME = 5,
    bonjour_kDNSServiceType_SOA = 6,
    bonjour_kDNSServiceType_MB = 7,
    bonjour_kDNSServiceType_MG = 8,
    bonjour_kDNSServiceType_MR = 9,
    bonjour_kDNSServiceType_NULL = 10,
    bonjour_kDNSServiceType_WKS = 11,
    bonjour_kDNSServiceType_PTR = 12,
    bonjour_kDNSServiceType_HINFO = 13,
    bonjour_kDNSServiceType_MINFO = 14,
    bonjour_kDNSServiceType_MX = 15,
    bonjour_kDNSServiceType_TXT = 16,
    bonjour_kDNSServiceType_RP = 17,
    bonjour_kDNSServiceType_AFSDB = 18,
    bonjour_kDNSServiceType_X25 = 19,
    bonjour_kDNSServiceType_ISDN = 20,
    bonjour_kDNSServiceType_RT = 21,
    bonjour_kDNSServiceType_NSAP = 22,
    bonjour_kDNSServiceType_NSAP_PTR = 23,
    bonjour_kDNSServiceType_SIG = 24,
    bonjour_kDNSServiceType_KEY = 25,
    bonjour_kDNSServiceType_PX = 26,
    bonjour_kDNSServiceType_GPOS = 27,
    bonjour_kDNSServiceType_AAAA = 28,
    bonjour_kDNSServiceType_LOC = 29,
    bonjour_kDNSServiceType_NXT = 30,
    bonjour_kDNSServiceType_EID = 31,
    bonjour_kDNSServiceType_NIMLOC = 32,
    bonjour_kDNSServiceType_SRV = 33,
    bonjour_kDNSServiceType_ATMA = 34,
    bonjour_kDNSServiceType_NAPTR = 35,
    bonjour_kDNSServiceType_KX = 36,
    bonjour_kDNSServiceType_CERT = 37,
    bonjour_kDNSServiceType_A6 = 38,
    bonjour_kDNSServiceType_DNAME = 39,
    bonjour_kDNSServiceType_SINK = 40,
    bonjour_kDNSServiceType_OPT = 41,
    bonjour_kDNSServiceType_TKEY = 249,
    bonjour_kDNSServiceType_TSIG = 250,
    bonjour_kDNSServiceType_IXFR = 251,
    bonjour_kDNSServiceType_AXFR = 252,
    bonjour_kDNSServiceType_MAILB = 253,
    bonjour_kDNSServiceType_MAILA = 254,
    bonjour_kDNSServiceType_ANY = 255

};

// indirection
int bonjour_DNSServiceRefSockFD(bonjour_DNSServiceRef service);
bonjour_DNSServiceErrorType bonjour_DNSServiceProcessResult(bonjour_DNSServiceRef service);
void bonjour_DNSServiceRefDeallocate(bonjour_DNSServiceRef service);
bonjour_DNSServiceErrorType bonjour_DNSServiceEnumerateDomains(bonjour_DNSServiceRef *service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, bonjour_DNSServiceDomainEnumReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceRegister(bonjour_DNSServiceRef *service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *name, const char *regtype, const char *domain, const char *host, uint16_t port, uint16_t txtLen, const void *txtRecord, bonjour_DNSServiceRegisterReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceAddRecord(bonjour_DNSServiceRef service, bonjour_DNSRecordRef *RecordRef, bonjour_DNSServiceFlags flags, uint16_t rrtype, uint16_t rdlen, const void *rdata, uint32_t ttl);
bonjour_DNSServiceErrorType bonjour_DNSServiceUpdateRecord(bonjour_DNSServiceRef service, bonjour_DNSRecordRef RecordRef, bonjour_DNSServiceFlags flags, uint16_t rdlen, const void *rdata, uint32_t ttl);
bonjour_DNSServiceErrorType bonjour_DNSServiceRemoveRecord(bonjour_DNSServiceRef service, bonjour_DNSRecordRef RecordRef, bonjour_DNSServiceFlags flags);
bonjour_DNSServiceErrorType bonjour_DNSServiceBrowse(bonjour_DNSServiceRef *service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *regtype, const char *domain, bonjour_DNSServiceBrowseReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceResolve(bonjour_DNSServiceRef *service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *name, const char *regtype, const char *domain, bonjour_DNSServiceResolveReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceCreateConnection(bonjour_DNSServiceRef *service);
bonjour_DNSServiceErrorType bonjour_DNSServiceRegisterRecord(bonjour_DNSServiceRef service, bonjour_DNSRecordRef *RecordRef, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata, uint32_t ttl, bonjour_DNSServiceRegisterRecordReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceQueryRecord(bonjour_DNSServiceRef *service, bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *fullname, uint16_t rrtype, uint16_t rrclass, bonjour_DNSServiceQueryRecordReply callBack, void *context);
bonjour_DNSServiceErrorType bonjour_DNSServiceReconfirmRecord(bonjour_DNSServiceFlags flags, uint32_t interfaceIndex, const char *fullname, uint16_t rrtype, uint16_t rrclass, uint16_t rdlen, const void *rdata);
int bonjour_DNSServiceConstructFullName(char *fullName, const char *service, const char *regtype, const char *domain);
void bonjour_TXTRecordCreate(bonjour_TXTRecordRef *txtRecord, uint16_t bufferLen, void *buffer);
void bonjour_TXTRecordDeallocate(bonjour_TXTRecordRef *txtRecord);
bonjour_DNSServiceErrorType bonjour_TXTRecordSetValue(bonjour_TXTRecordRef *txtRecord, const char *key, uint8_t valueSize, const void *value);
bonjour_DNSServiceErrorType bonjour_TXTRecordRemoveValue(bonjour_TXTRecordRef *txtRecord, const char *key);
uint16_t bonjour_TXTRecordGetLength(const bonjour_TXTRecordRef *txtRecord);
const void *bonjour_TXTRecordGetBytesPtr(const bonjour_TXTRecordRef *txtRecord);
int bonjour_TXTRecordContainsKey(uint16_t txtLen, const void *txtRecord, const char *key);
const void *bonjour_TXTRecordGetValuePtr(uint16_t txtLen, const void *txtRecord, const char *key, uint8_t *valueLen);
uint16_t bonjour_TXTRecordGetCount(uint16_t txtLen, const void *txtRecord);
bonjour_DNSServiceErrorType bonjour_TXTRecordGetItemAtIndex(uint16_t txtLen, const void *txtRecord, uint16_t index, uint16_t keyBufLen, char *key, uint8_t *valueLen, const void **value);

// helper function
NSDictionary *bonjour_CreateError(id sender, int errorCode);
