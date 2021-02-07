#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSPersonNameComponentsFormatter.h>

int main()
{
  START_SET("NSPersonNameComponentsFormatter base");

  NSPersonNameComponents *pnc = [[NSPersonNameComponents alloc] init];
  [pnc setGivenName: @"Gregory"];
  [pnc setMiddleName: @"John"];
  [pnc setFamilyName: @"Casamento"];
  [pnc setNameSuffix: @"PhD"];
  [pnc setNamePrefix: @"Dr."];
  
  NSPersonNameComponentsFormatter *fmt = [[NSPersonNameComponentsFormatter alloc] init];
  NSPersonNameComponents *pnc2 = [fmt personNameComponentsFromString:
                                        @"Dr. Gregory John Casamento PhD"];
  PASS([[pnc givenName] isEqualToString:
                          [pnc2 givenName]], "First name matches");
  PASS([[pnc middleName] isEqualToString:
                           [pnc2 middleName]], "Middle name matches");
  PASS([[pnc familyName] isEqualToString:
                           [pnc2 familyName]], "Family name matches");
  PASS([[pnc nameSuffix] isEqualToString:
                           [pnc2 nameSuffix]], "Suffix name matches");
  PASS([[pnc namePrefix] isEqualToString:
                           [pnc2 namePrefix]], "Prefix name matches");
  
  fmt = [[NSPersonNameComponentsFormatter alloc] init];
  pnc2 = [fmt personNameComponentsFromString:
                @"Gregory John Casamento PhD"];
  PASS([[pnc givenName] isEqualToString:
                          [pnc2 givenName]], "First name matches");
  PASS([[pnc middleName] isEqualToString:
                           [pnc2 middleName]], "Middle name matches");
  PASS([[pnc familyName] isEqualToString:
                           [pnc2 familyName]], "Family name matches");
  PASS([[pnc nameSuffix] isEqualToString:
                           [pnc2 nameSuffix]], "Suffix name matches");

  fmt = [[NSPersonNameComponentsFormatter alloc] init];
  pnc2 = [fmt personNameComponentsFromString:
                @"Gregory John Casamento"];
  PASS([[pnc givenName] isEqualToString:
                          [pnc2 givenName]], "First name matches");
  PASS([[pnc middleName] isEqualToString:
                           [pnc2 middleName]], "Middle name matches");
  PASS([[pnc familyName] isEqualToString:
                           [pnc2 familyName]], "Family name matches");

  END_SET("NSPersonNameComponentsFormatter base");
  return 0;
}
