#import "Testing.h"
#import "generic.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSString.h>

int main()
{
  START_SET("NSBundle resources")
    NSBundle *bundle;
    NSArray  *arr;
  
    bundle = [NSBundle bundleWithPath: [[NSFileManager defaultManager] 
      currentDirectoryPath]];
 
    PASS([bundle isKindOfClass: [NSBundle class]],
      "+bundleWithPath returns anNSBundle");
    arr = [bundle pathsForResourcesOfType: @"m" inDirectory: nil];
    PASS([arr isKindOfClass: [NSArray class]] && [arr count],
      "-pathsForResourcesOfType: inDirectory: returns an array");
    PASS([bundle pathForResource: @"hiwelf0-2" 
			  ofType: nil 
		     inDirectory: nil] == nil,
      "-pathForResource:ofType:inDirectory: works with nil args");
    PASS([bundle pathForResource: @"hiwelf0-2" ofType: nil] == nil,
      "-pathForResource:ofType: works with nil type");
    PASS([bundle pathForResource: nil ofType: @"tiff"] == nil,
      "-pathForResource:ofType: works with nil name");
    PASS([bundle pathForResource: @"hiwelf0-2" ofType: @""] == nil,
      "-pathForResource:ofType: works with empty type");
    PASS([bundle pathForResource: @"" ofType: @"tiff"] == nil,
      "-pathForResource:ofType: works with empty name");
    PASS([[bundle resourcePath] testEquals: [[bundle bundlePath] 
      stringByAppendingPathComponent: @"Resources"]],
      "-resourcePath returns the correct path");
   
  END_SET("NSBundle resources")

#if	defined(GNUSTEP)
    START_SET("NSBundle GNUstep general")
      NSBundle *gnustepBundle;
      NSArray  *arr;

      gnustepBundle = [NSBundle bundleForLibrary: @"gnustep-base"];
      if (nil == gnustepBundle)
	SKIP("it looks like GNUstep-base is not yet installed")

      PASS([[NSBundle pathForResource: @"abbreviations" 
			       ofType: @"plist" 
			  inDirectory: [[gnustepBundle bundlePath] 
       stringByAppendingPathComponent: @"NSTimeZones"]] testForString],
       "+pathForResource:ofType:inDirectory: works");
 
      PASS([[NSBundle pathForResource: @"abbreviations" 
			       ofType: @"plist" 
			  inDirectory: [[gnustepBundle bundlePath] 
	stringByAppendingPathComponent: @"NSTimeZones"] withVersion: 0]
	testForString],
	"+pathForResource:ofType:inDirectory:withVersion: works");

      arr = [gnustepBundle pathsForResourcesOfType: @"m" 
				     inDirectory: @"NSTimeZones"];
      PASS(([arr isKindOfClass: [NSArray class]] && [arr count] > 0),
	"-pathsForResourcesOfType:inDirectory: returns an array");
      PASS([[gnustepBundle pathForResource: @"abbreviations"
				  ofType: @"plist"
			     inDirectory: @"NSTimeZones"] testForString],
	"-pathForResource:ofType:inDirectory: finds a file");
      PASS([gnustepBundle pathForResource: @"abbreviations"
				 ofType: @"8nicola8"
			    inDirectory: @"NSTimeZones"] == nil,
	"-pathForResource:ofType:inDirectory: doesn't find non-existing file");
      PASS([gnustepBundle pathForResource: @"abbreviations"
				 ofType: @"plist"
			    inDirectory: @"NSTimeZones_dummy"] == nil,
	"-pathForResource:ofType:inDirectory: doesn't find files in"
	"a non-existing dir");
      PASS([[gnustepBundle pathForResource: @"abbreviations"
				  ofType: nil
			     inDirectory: @"NSTimeZones"] testForString],
	"-pathForResource:ofType:inDirectory: with nil type finds a file");
      PASS([gnustepBundle pathForResource: @"whasssdlkf"
				 ofType: nil
			    inDirectory: @"NSTimeZones"] == nil,
	"-pathForResource:ofType:inDirectory: with nil type doesn't find"
	"non-existing files");
     
      PASS([[gnustepBundle pathForResource: @"NSTimeZones" ofType: nil]
	testForString], 
	"-pathForResource:ofType: finds a file");

    END_SET("NSBundle GNUstep resources")
#endif

  return 0;
}
