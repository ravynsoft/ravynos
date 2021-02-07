#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSPropertyList.h>

static BOOL
test_parse(NSString *string, id result)
{
  return [[string propertyList] isEqual: result];
}

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];

  PASS(test_parse(@"ariosto",@"ariosto"),
       "We can parse a string");

  PASS(test_parse(@"\"ariosto\"",@"ariosto"),
       "We can parse a quoted string");

  PASS(test_parse(@"\"ari\\033osto\"",@"ari\033osto"),
       "We can parse a quoted string with octal escape");

  PASS(test_parse(@"(ariosto)",
		  [NSArray arrayWithObject: @"ariosto"]),
       "We can parse a simple array with a single object");

  PASS(test_parse(@"(\"ariosto\")",
		  [NSArray arrayWithObject: @"ariosto"]),
       "We can parse a simple array with a single object between \"s");

  PASS(test_parse(@"(ariosto, boccaccio)",
		  [NSArray arrayWithObjects: @"ariosto", @"boccaccio", nil]),
       "We can parse a simple array with two objects");

  PASS(test_parse(@"(ariosto, boccaccio, \"leopardi\")",
		  [NSArray arrayWithObjects: 
			     @"ariosto", @"boccaccio", @"leopardi", nil]),
       "We can parse a simple array with three objects, with \"s");

  PASS(test_parse(@"(ariosto /* I like this one */, boccaccio)",
		  [NSArray arrayWithObjects: @"ariosto", @"boccaccio", nil]),
       "We can parse a simple array with two objects and a C comment");

  PASS(test_parse(@"(ariosto, // I like this one\n boccaccio)",
		  [NSArray arrayWithObjects: @"ariosto", @"boccaccio", nil]),
       "We can parse a simple array with two objects and a C++ comment");

  PASS(test_parse(@"{}", [NSDictionary dictionary]),
       "We can parse an empty dictionary");

  PASS(test_parse(@"{author = ariosto; title = \"orlando furioso\"; }",
		  [NSDictionary dictionaryWithObjectsAndKeys:
				  @"ariosto", @"author",
				@"orlando furioso", @"title", nil]),
       "We can parse a simple dictionary with a two key value pairs");

  PASS(test_parse(@"{/* -*-c-*- */ author = ariosto; title = \"orlando furioso\"; }",
		  [NSDictionary dictionaryWithObjectsAndKeys:
				  @"ariosto", @"author",
				@"orlando furioso", @"title", nil]),
       "We can parse a simple dictionary with a two key value pairs and a C comment");

  PASS(test_parse(@"{// -*-c-*-\n author = ariosto; title = \"orlando furioso\";}",
		  [NSDictionary dictionaryWithObjectsAndKeys:
				  @"ariosto", @"author",
				@"orlando furioso", @"title", nil]),
       "We can parse a simple dictionary with a two key value pairs and a C++ comment");

  PASS(test_parse(@"((ariosto))",
		  [NSArray arrayWithObject:
			     [NSArray arrayWithObject: @"ariosto"]]),
       "We can parse an array containing a single array");

  PASS(test_parse(@"({author = ariosto;})",
		  [NSArray arrayWithObject:
			     [NSDictionary dictionaryWithObject: @"ariosto"
				    forKey: @"author"]]),
       "We can parse an array containing a single dictionary");

  PASS(test_parse(@"((farinata), dante)",
		  [NSArray arrayWithObjects:
			     [NSArray arrayWithObject: @"farinata"],
			   @"dante", nil]),
       "We can parse an array containing an array and a string");

  PASS(test_parse(@"((farinata), {author = ariosto;})",
		  [NSArray arrayWithObjects:
			     [NSArray arrayWithObject: @"farinata"],
			   [NSDictionary dictionaryWithObject: @"ariosto"
					 forKey: @"author"],nil]),
       "We can parse an array containing an array and a dictionary");

  [arp release]; arp = nil;
  return 0;
}

