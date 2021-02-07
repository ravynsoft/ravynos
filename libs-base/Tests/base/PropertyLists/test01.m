#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSData.h>
#import <Foundation/NSPropertyList.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSValue.h>

static BOOL
test_parse_unparse(id object)
{
  return [[[object description] propertyList] isEqual: object];
}

static BOOL
test_parse_unparse_xml(id object)
{
  NSPropertyListFormat	format;
  NSData		*d;
  id			u;

  d = [NSPropertyListSerialization dataFromPropertyList: object
    format: NSPropertyListXMLFormat_v1_0 errorDescription: 0];
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  return [u isEqual: object];
}

static BOOL
test_parse_unparse_openstep(id object)
{
  NSPropertyListFormat	format;
  NSData		*d;
  id			u;

  d = [NSPropertyListSerialization dataFromPropertyList: object
    format: NSPropertyListOpenStepFormat errorDescription: 0];
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  return [u isEqual: object];
}

static BOOL
test_parse_unparse_binary(id object)
{
  NSPropertyListFormat	format;
  NSData		*d;
  id			u;

  d = [NSPropertyListSerialization dataFromPropertyList: object
    format: NSPropertyListBinaryFormat_v1_0 errorDescription: 0];
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  return [u isEqual: object];
}

#if     defined(GNUSTEP_BASE_LIBRARY)
static BOOL
test_parse_unparse_binary_old(id object)
{
  NSPropertyListFormat	format;
  NSData		*d;
  id			u;

  d = [NSPropertyListSerialization dataFromPropertyList: object
    format: NSPropertyListGNUstepBinaryFormat errorDescription: 0];
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  return [u isEqual: object];
}
static BOOL
test_parse_unparse_gnustep(id object)
{
  NSPropertyListFormat	format;
  NSData		*d;
  id			u;

  d = [NSPropertyListSerialization dataFromPropertyList: object
    format: NSPropertyListGNUstepFormat errorDescription: 0];
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  return [u isEqual: object];
}
#endif

int main()
{
  BOOL	(*func)(id);
  int	i;
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  for (i = 0; i < 6; i++)
    {
      switch (i)
        {
	  case 0:
	    func = test_parse_unparse;
	    NSLog(@"test descriptions");
	    break;
	  case 1:
	    func = test_parse_unparse_xml;
	    NSLog(@"test XML");
	    break;
	  case 2:
	    func = test_parse_unparse_binary;
	    NSLog(@"test binary");
	    break;
	  case 3:
	    func = test_parse_unparse_openstep;
	    NSLog(@"test OpenStep");
	    break;
	  case 4:
#if     defined(GNUSTEP_BASE_LIBRARY)
	    func = test_parse_unparse_gnustep;
	    NSLog(@"test GNUStep text");
#else
	    func = 0;
#endif
	  case 5:
#if     defined(GNUSTEP_BASE_LIBRARY)
	    func = test_parse_unparse_binary_old;
	    NSLog(@"test GNUStep old binary");
#else
	    func = 0;
#endif
	    break;
	}

      if (func == 0) continue;

      PASS(func(@"ariosto"),
	   "We can generate a property list from a string");

      PASS(func([@"ariosto" dataUsingEncoding: NSASCIIStringEncoding]),
	   "We can generate a property list from data");

      PASS(func([NSArray array]),
	   "We can generate a property list from an empty array");

      PASS(func([NSArray arrayWithObject: @"Palinuro"]),
	   "We can generate a property list from an array with a single object");

      PASS(func([NSArray arrayWithObjects: 
					 @"Palinuro", @"Enea", nil]),
	   "We can generate a property list from an array with two objects");

      PASS(func([NSArray arrayWithObjects: 
					 @"Palinuro", @"Enea", 
				       @"Eurialo e Niso", nil]),
       "We can generate a property list from "
       "an array with three objects and \"s");

      PASS(func([NSDictionary dictionary]),
	   "We can generate a property list from an empty dictionary");

      PASS(func([NSDictionary dictionaryWithObject: @"Virgilio"
					    forKey: @"Autore"]),
	"We can generate a property list from a "
	"dictionary with a single key/value pair");

      PASS(func([NSDictionary dictionaryWithObjectsAndKeys: 
					      @"Virgilio", @"Autore",
					    @"Eneide", @"Titolo", nil]),
	"We can generate a property list from a "
	"dictionary with two key/value pairs");

      PASS(func([NSDictionary dictionaryWithObjectsAndKeys: 
					      @"Virgilio", @"Autore",
					    [NSArray arrayWithObject: @"Molte"],
					    @"Opere", nil]),
	"We can generate a property list from a "
	"dictionary with an array inside");

      {
	id object  = [NSMutableDictionary dictionary];
	id objectA = [NSArray arrayWithObject: @"Ciao,"];
	id objectB = [NSArray arrayWithObject: objectA];
	id objectC = [NSDictionary dictionary];
	id objectD = [NSArray arrayWithObject:
	  [NSArray arrayWithObject:
	   [NSDictionary dictionaryWithObject:
	     [NSArray arrayWithObject: @"Ciao,"]
	     forKey: @"Ciao,"]]];
	[object setObject: objectA forKey: @"Utinam"];
	[object setObject: objectB forKey: @"bbb"];
	[object setObject: objectC forKey: @"ccc"];
	[object setObject: objectD forKey: @"Auri;"];
	PASS(func(object),
	     "We can generate a medium-size property list (1)");
      }
      {
	id object;
	id objectA;
	id objectA_A;
	id objectA_B;
	id objectB;
	id objectB_A;
	id objectB_A_A;
	id objectB_A_B;
	id objectB_B;

	/* objectA */
	objectA_A = [NSMutableDictionary dictionary];
	[objectA_A setObject: @"1 2 3 4 5 6 7 8 9 0" forKey: @"numeri"];
	[objectA_A setObject: @"A B C D E F G H I J" forKey: @"lettere"];

	objectA_B = [NSMutableDictionary dictionary];
	[objectA_B setObject: @"3.1415296" forKey: @"PI greco"];
	[objectA_B setObject: @"0" forKey: @"zero"];
	[objectA_B setObject: @"1" forKey: @"uno"];

	objectA = [NSMutableDictionary dictionary];
	[objectA setObject: objectA_A forKey: @"Informazioni Utili"];
	[objectA setObject: objectA_B forKey: @"Costanti Numeriche"];

	/* objectB */
	objectB_A = [NSMutableDictionary dictionary];

	objectB_A_A = [NSMutableArray array];
	[objectB_A_A addObject: @"1"];
	[objectB_A_A addObject: @"2"];
	[objectB_A_A addObject: @"3"];
	[objectB_A_A addObject: @"4"];
	[objectB_A_A addObject: @"5"];
	[objectB_A_A addObject: @"6"];
	[objectB_A_A addObject: @"7"];
	[objectB_A_A addObject: @"8"];
	[objectB_A_A addObject: @"9"];
	[objectB_A_A addObject: @"0"];
        if (func == test_parse_unparse_binary
          || func == test_parse_unparse_xml)
	  {
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 1]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 2]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 3]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 4]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 5]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 6]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 7]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 8]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 9]];
	    [objectB_A_A addObject: [NSNumber numberWithInteger: 0]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 1]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 2]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 3]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 4]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 5]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 6]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 7]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 8]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 9]];
	    [objectB_A_A addObject: [NSNumber numberWithShort: 0]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 1]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 2]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 3]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 4]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 5]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 6]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 7]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 8]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 9]];
	    [objectB_A_A addObject: [NSNumber numberWithFloat: 0]];
	  }
	[objectB_A setObject: objectB_A_A forKey: @"numeri"];

	objectB_A_B = [NSMutableArray array];
	[objectB_A_B addObject: @"A"];
	[objectB_A_B addObject: @"B"];
	[objectB_A_B addObject: @"C"];
	[objectB_A_B addObject: @"D"];
	[objectB_A_B addObject: @"E"];
	[objectB_A_B addObject: @"F"];
	[objectB_A_B addObject: @"G"];
	[objectB_A_B addObject: @"H"];
	[objectB_A_B addObject: @"I"];
	[objectB_A_B addObject: @"J"];
	[objectB_A setObject: objectB_A_B forKey: @"letterine"];

	objectB_B = [NSMutableDictionary dictionary];
	[objectB_B setObject: @"3.1415296" forKey: @"PI greca"];
	[objectB_B setObject: @"0" forKey: @"el zero"];
	[objectB_B setObject: @"1" forKey: @"el uno"];

	objectB = [NSMutableDictionary dictionary];
	[objectB setObject: objectB_A forKey: @"Informazioni Utili"];
	[objectB setObject: objectB_B forKey: @"Costanti Numeriche"];

	/* object */
	object = [NSMutableDictionary dictionary];
	[object setObject: objectA forKey: @"Un dizionario"];
	[object setObject: objectB forKey: @"Un altro dizionario"];

	PASS(func(object),
	     "We can generate a medium-size property list (2)");
      }

      PASS(func([NSData data]),
	   "We can generate a property list from an empty data");

      PASS(func([@"Questo e` un test" dataUsingEncoding: 1]),
	   "We can generate a property list from very simple data");

      PASS(func([[[NSProcessInfo processInfo] 
				 globallyUniqueString] dataUsingEncoding: 7]),
	   "We can generate a property list from very simple data (2)");

      PASS(func([NSMutableArray arrayWithObject:
				[@"*()3\"#@Q``''" dataUsingEncoding: 1]]),
	"We can generate a property list from an "
	"array containing very simple data");

      {
	id object = [NSMutableArray array];

	[object addObject: [@"*()3\"#@Q``''" dataUsingEncoding: 1]];
	[object addObject: @"nicola \" , ; <"];
	[object addObject: @"<nicola"];
	[object addObject: @"nicola;"];
	[object addObject: @"nicola,"];
	[object addObject: @"nicola>"];
	[object addObject: @"nicola@"];
	[object addObject: @"nicola "];
	[object addObject: @"nicola="];
	[object addObject: [NSArray arrayWithObject: @"="]];
	[object addObject: [NSDictionary dictionary]];

	PASS(func(object),
	  "We can generate a property list from an array containing various things");
      }
    }
#if     defined(GNUSTEP_BASE_LIBRARY)
{
  NSData        	*d = [NSData dataWithContentsOfFile: @"props"];
  NSPropertyListFormat	format;
  id			u;

  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];
  PASS(nil != u, "parses complex plist");
}
#endif

#if     defined(GNUSTEP_BASE_LIBRARY)
{
  NSData        	*d = [NSData dataWithContentsOfFile: @"cyclic.plist"];
  NSPropertyListFormat	format;
  id			u = nil;
  PASS_EXCEPTION(
  u = [NSPropertyListSerialization propertyListFromData: d
    mutabilityOption: NSPropertyListImmutable
    format: &format
    errorDescription: 0];, NSGenericException, "Does not crash on binary plist with cyclic references." );
  PASS(nil == u, "Rejects cyclic plist");
}
#endif


  [arp release]; arp = nil;
  return 0;
}

