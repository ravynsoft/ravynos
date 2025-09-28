#import <CFNetwork/CFSocketStream.h>
#import <Foundation/NSString.h>

/************* These values are also in NSStream, keep in sync */

const CFStringRef kCFStreamPropertyShouldCloseNativeSocket=(CFStringRef)@"kCFStreamPropertyShouldCloseNativeSocket";
const CFStringRef kCFStreamPropertySocketSecurityLevel=(CFStringRef)@"kCFStreamPropertySocketSecurityLevel";
const CFStringRef kCFStreamPropertySOCKSProxy=(CFStringRef)@"kCFStreamPropertySOCKSProxy";
const CFStringRef kCFStreamPropertySSLPeerCertificates=(CFStringRef)@"kCFStreamPropertySSLPeerCertificates";
const CFStringRef kCFStreamPropertySSLPeerTrust=(CFStringRef)@"kCFStreamPropertySSLPeerTrust";
const CFStringRef kCFStreamPropertySSLSettings=(CFStringRef)@"kCFStreamPropertySSLSettings";
const CFStringRef kCFStreamPropertyProxyLocalByPass=(CFStringRef)@"kCFStreamPropertyProxyLocalByPass";
const CFStringRef kCFStreamPropertySocketRemoteHost=(CFStringRef)@"kCFStreamPropertySocketRemoteHost";
const CFStringRef kCFStreamPropertySocketRemoteNetService=(CFStringRef)@"kCFStreamPropertySocketRemoteNetService";

const CFStringRef kCFStreamSSLLevel=(CFStringRef)@"kCFStreamSSLLevel";
const CFStringRef kCFStreamSSLAllowsExpiredCertificates=(CFStringRef)@"kCFStreamSSLAllowsExpiredCertificates";
const CFStringRef kCFStreamSSLAllowsExpiredRoots=(CFStringRef)@"kCFStreamSSLAllowsExpiredRoots";
const CFStringRef kCFStreamSSLAllowsAnyRoot=(CFStringRef)@"kCFStreamSSLAllowsAnyRoot";
const CFStringRef kCFStreamSSLValidatesCertificateChain=(CFStringRef)@"kCFStreamSSLValidatesCertificateChain";
const CFStringRef kCFStreamSSLPeerName=(CFStringRef)@"kCFStreamSSLPeerName";
const CFStringRef kCFStreamSSLCertificates=(CFStringRef)@"kCFStreamSSLCertificates";
const CFStringRef kCFStreamSSLIsServer=(CFStringRef)@"kCFStreamSSLIsServer";

const CFStringRef kCFStreamSocketSecurityLevelNone=(CFStringRef)@"kCFStreamSocketSecurityLevelNone";
const CFStringRef kCFStreamSocketSecurityLevelSSLv2=(CFStringRef)@"kCFStreamSocketSecurityLevelSSLv2";
const CFStringRef kCFStreamSocketSecurityLevelSSLv3=(CFStringRef)@"kCFStreamSocketSecurityLevelSSLv3";
const CFStringRef kCFStreamSocketSecurityLevelTLSv1=(CFStringRef)@"kCFStreamSocketSecurityLevelTLSv1";
const CFStringRef kCFStreamSocketSecurityLevelNegotiatedSSL=(CFStringRef)@"kCFStreamSocketSecurityLevelNegotiatedSSL";

const CFStringRef kCFStreamPropertySOCKSProxyHost=(CFStringRef)@"kCFStreamPropertySOCKSProxyHost";
const CFStringRef kCFStreamPropertySOCKSProxyPort=(CFStringRef)@"kCFStreamPropertySOCKSProxyPort";
const CFStringRef kCFStreamPropertySOCKSVersion=(CFStringRef)@"kCFStreamPropertySOCKSVersion";
const CFStringRef kCFStreamSocketSOCKSVersion4=(CFStringRef)@"kCFStreamSocketSOCKSVersion4";
const CFStringRef kCFStreamSocketSOCKSVersion5=(CFStringRef)@"kCFStreamSocketSOCKSVersion5";
const CFStringRef kCFStreamPropertySOCKSUser=(CFStringRef)@"kCFStreamPropertySOCKSUser";
const CFStringRef kCFStreamPropertySOCKSPassword=(CFStringRef)@"kCFStreamPropertySOCKSPassword";
