#import <CoreFoundation/CoreFoundation.h>
#import <CFNetwork/CFNetworkExport.h>
#import <CFNetwork/CFNetServices.h>

CFNETWORK_EXPORT const CFStringRef kCFStreamPropertyShouldCloseNativeSocket;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySocketSecurityLevel;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSProxy;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySSLPeerCertificates;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySSLPeerTrust;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySSLSettings;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertyProxyLocalByPass;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySocketRemoteHost;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySocketRemoteNetService;

CFNETWORK_EXPORT const CFStringRef kCFStreamSSLLevel;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLAllowsExpiredCertificates;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLAllowsExpiredRoots;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLAllowsAnyRoot;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLValidatesCertificateChain;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLPeerName;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLCertificates;
CFNETWORK_EXPORT const CFStringRef kCFStreamSSLIsServer;

typedef enum {
    kCFStreamSocketSecurityNone = 0,
    kCFStreamSocketSecuritySSLv2,
    kCFStreamSocketSecuritySSLv3,
    kCFStreamSocketSecuritySSLv23,
    kCFStreamSocketSecurityTLSv1
} CFStreamSocketSecurityProtocol;

CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSecurityLevelNone;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSecurityLevelSSLv2;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSecurityLevelSSLv3;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSecurityLevelTLSv1;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSecurityLevelNegotiatedSSL;

CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSProxyHost;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSProxyPort;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSVersion;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSOCKSVersion4;
CFNETWORK_EXPORT const CFStringRef kCFStreamSocketSOCKSVersion5;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSUser;
CFNETWORK_EXPORT const CFStringRef kCFStreamPropertySOCKSPassword;

CFNETWORK_EXPORT const CFIndex kCFStreamErrorDomainWinSock;

enum {
    kCFStreamErrorSOCKSSubDomainNone = 0,
    kCFStreamErrorSOCKSSubDomainVersionCode = 1,
    kCFStreamErrorSOCKS4SubDomainResponse = 2,
    kCFStreamErrorSOCKS5SubDomainUserPass = 3,
    kCFStreamErrorSOCKS5SubDomainMethod = 4,
    kCFStreamErrorSOCKS5SubDomainResponse = 5
};

/* kCFStreamErrorSOCKSSubDomainNone*/
enum {
    kCFStreamErrorSOCKS5BadResponseAddr = 1,
    kCFStreamErrorSOCKS5BadState = 2,
    kCFStreamErrorSOCKSUnknownClientVersion = 3
};

/* kCFStreamErrorSOCKS4SubDomainResponse*/
enum {
    kCFStreamErrorSOCKS4RequestFailed = 91,
    kCFStreamErrorSOCKS4IdentdFailed = 92,
    kCFStreamErrorSOCKS4IdConflict = 93
};

/* kCFStreamErrorSOCKS5SubDomainMethod*/
enum {
    kSOCKS5NoAcceptableMethod = 0xFF
};

CFNETWORK_EXPORT Boolean CFSocketStreamPairSetSecurityProtocol(CFReadStreamRef socketReadStream, CFWriteStreamRef socketWriteStream, CFStreamSocketSecurityProtocol securityProtocol);

CFNETWORK_EXPORT SInt32 CFSocketStreamSOCKSGetError(CFStreamError *error);

CFNETWORK_EXPORT SInt32 CFSocketStreamSOCKSGetErrorSubdomain(CFStreamError *error);

CFNETWORK_EXPORT void CFStreamCreatePairWithSocketToCFHost(CFAllocatorRef alloc, CFHostRef host, SInt32 port, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);

CFNETWORK_EXPORT void CFStreamCreatePairWithSocketToNetService(CFAllocatorRef alloc, CFNetServiceRef service, CFReadStreamRef *readStream, CFWriteStreamRef *writeStream);
