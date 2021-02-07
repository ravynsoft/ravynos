#import "ObjectTesting.h"
#import "../../../Headers/GNUstepBase/config.h"
#import "../../../Headers/Foundation/Foundation.h"
#import "../../../Headers/GNUstepBase/GSTLS.h"

int
main() {
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
START_SET("TLS support")
#if GS_USE_GNUTLS
#ifndef HAVE_GNUTLS_X509_PRIVKEY_IMPORT2
 testHopeful = YES;
#endif
  GSTLSPrivateKey        *k;
  GSTLSCertificateList   *c;
  NSDateFormatter        *dateFormatter;
  NSDate                 *expiresAt;

  k = [GSTLSPrivateKey keyFromFile: @"test.key" withPassword: @"asdf"]; 
  PASS(k != nil, "OpenSSL encrypted key can be loaded");

  c = [GSTLSCertificateList listFromFile: @"test.crt"];
  PASS(c != nil, "Certificate list can be loaded from a file");  

  PASS([c expiresAt: -1] == nil, "Return nil for invalid index");
  PASS([c expiresAt: 2] == nil, "Return nil for invalid index");
  expiresAt = [c expiresAt: 0];
  dateFormatter = [[NSDateFormatter alloc] init];
  [dateFormatter setDateFormat: @"yyyy-MM-dd HH:mm:ss zzz"];
  /* Test guaranteed to fail on 32-bit architectures.  */
#if __LP64__
  PASS_EQUAL(expiresAt,
    [dateFormatter dateFromString: @"2118-12-14 15:35:11 +0000"],
    "Expiration date can be retrieved");
#endif
  [dateFormatter release];
  PASS_EQUAL([c expiresAt], expiresAt,
    "Expiration for entire list is that of the single item")
#else
  SKIP("TLS support disabled");
#endif
  END_SET("TLS support");
  DESTROY(arp);
  return 0;
}
