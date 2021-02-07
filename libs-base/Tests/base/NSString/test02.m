#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSPathUtilities.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSArray *result;
  char *testPath = NULL;
  char *resultPath = NULL;

NSLog(@"Developer: %@", NSSearchPathForDirectoriesInDomains(NSDeveloperDirectory, NSAllDomainsMask, YES));

  PASS([[[@"home" pathComponents] objectAtIndex:0] isEqual: @"home"],
       "[@\"home\" pathComponents] == @\"home\"]");
  
  result = [@"/home" pathComponents];
  PASS([[result objectAtIndex:0] isEqual: @"/"]
       && [[result objectAtIndex:1] isEqual: @"home"],
       "[@\"/home\" pathComponents] == (@\"/\", @\"home\"])");
  
  result = [@"/home/" pathComponents];
  PASS([[result objectAtIndex:0] isEqual: @"/"]
       && [[result objectAtIndex:1] isEqual: @"home"]
       && [[result objectAtIndex:2] isEqual: @"/"],
       "[@\"/home\" pathComponents] == (@\"/\", @\"home\",\"/\")]");
  
  result = [@"/home/nicola" pathComponents];
  PASS([[result objectAtIndex:0] isEqual: @"/"]
       && [[result objectAtIndex:1] isEqual: @"home"]
       && [[result objectAtIndex:2] isEqual: @"nicola"],
       "[@\"/home/nicola\" pathComponents] == (@\"/\", @\"home\",\"nicola\")]");
  
  result = [@"//home//nicola" pathComponents];
  PASS([[result objectAtIndex:0] isEqual: @"/"]
       && [[result objectAtIndex:1] isEqual: @"home"]
       && [[result objectAtIndex:2] isEqual: @"nicola"],
       "[@\"//home//nicola\" pathComponents] == (@\"/\", @\"home\",\"nicola\")]");
  PASS([[@"home/nicola.jpg" pathExtension] isEqual: @"jpg"],
       "[@\"home/nicola.jpg\" pathExtension] == @\"jpg\"");

  PASS([[@"/home/nicola.jpg" pathExtension] isEqual: @"jpg"],
       "[@\"/home/nicola.jpg\\\" pathExtension] == @\"jpg\"");
  
  PASS([[@"home/nicola" pathExtension] isEqual: @""],
       "[@\"home/nicola\" pathExtension] == @\"\"");
 
  PASS([[@"home/nicola/" pathExtension] isEqual: @""],
       "[@\"home/nicola\" pathExtension] == @\"\"");
  
  PASS([[@"/home/nicola..jpg" pathExtension] isEqual: @"jpg"],
       "[@\"/home/nicola..jpg\" pathExtension] == @\"jpg\"");
  
  PASS([[@"/home/nicola.jpg/" pathExtension] isEqual: @"jpg"],
       "[@\"/home/nicola.jpg/\" pathExtension] == @\"jpg\"");
  
  PASS([[@"/home/nicola.jpg/extra" pathExtension] isEqual: @""],
       "[@\"/home/nicola.jpg/extra\" pathExtension] == @\"\"");
  
  PASS([[@"/home/nicola./" pathExtension] isEqual: @""],
       "[@\"/home/nicola./\" pathExtension] == @\"\"");
  
  PASS_EQUAL([@"/home" stringByAppendingPathComponent: @"/"], @"/home",
    "'/home' stringByAppendingPathComponent: '/' == '/home'");
  
  PASS([[@"/home" stringByAppendingPathComponent: @"nicola.jpg"] isEqual: @"/home/nicola.jpg"],
       "'/home' stringByAppendingPathComponent: 'nicola.jpg' == '/home/nicola.jpg'");
  
  PASS([[@"/home/" stringByAppendingPathComponent: @"nicola.jpg"] isEqual: @"/home/nicola.jpg"],
       "'/home/' stringByAppendingPathComponent: 'nicola.jpg' == '/home/nicola.jpg'");
    
  PASS([[@"/" stringByAppendingPathComponent: @"nicola.jpg"] isEqual: @"/nicola.jpg"],
       "'/' stringByAppendingPathComponent: 'nicola.jpg' == '/nicola.jpg'");
  
  PASS([[@"" stringByAppendingPathComponent: @"nicola.jpg"] isEqual: @"nicola.jpg"],
       "'' stringByAppendingPathComponent: 'nicola.jpg' == 'nicola.jpg'");
  
  PASS([[@"/home/nicola.jpg" stringByAppendingPathExtension: @"jpg"] isEqual: @"/home/nicola.jpg.jpg"],
       "'/home/nicola.jpg' stringByAppendingPathExtension:'jpg' == '/home/nicola.jpg.jpg'");
  
  PASS([[@"/home/nicola." stringByAppendingPathExtension: @"jpg"] isEqual: @"/home/nicola..jpg"],
       "'/home/nicola.' stringByAppendingPathExtension:'jpg' == '/home/nicola..jpg'");
  
  /* in the guile version of this test the description was different than the 
     test i've updated it for the description to be the same as the test not
     sure which is correct but the test PASSes */
  PASS([[@"/home/nicola/" stringByAppendingPathExtension: @"jpg"] isEqual: @"/home/nicola.jpg"],
       "'/home/nicola/' stringByAppendingPathExtension:'jpg' == '/home/nicola.jpg'");
  
  PASS([[@"nicola" stringByAppendingPathExtension: @"jpg"] isEqual: @"nicola.jpg"],
       "'nicola' stringByAppendingPathExtension:'jpg' == 'nicola.jpg'");
  
  PASS([[@"nicola" stringByAppendingPathExtension: @"jpg"] isEqual: @"nicola.jpg"],
       "'nicola' stringByAppendingPathExtension:'jpg' == 'nicola.jpg'");
 
 /* string by deleting last path component tests */

  PASS([[@"/home/nicola.jpg" stringByDeletingLastPathComponent] isEqual: @"/home"],
       "'/home/nicola.jpg' stringByDeletingLastPathComponent == '/home'");
  
  PASS([[@"/home/nicola/" stringByDeletingLastPathComponent] isEqual: @"/home"],
       "'/home/nicola/' stringByDeletingLastPathComponent == '/home'");
  PASS([[@"/home/nicola" stringByDeletingLastPathComponent] isEqual: @"/home"],
       "'/home/nicola' stringByDeletingLastPathComponent == '/'");
  PASS([[@"/home/" stringByDeletingLastPathComponent] isEqual: @"/"],
       "'/home/' stringByDeletingLastPathComponent == '/'");
  PASS([[@"/home" stringByDeletingLastPathComponent] isEqual: @"/"],
       "'/home' stringByDeletingLastPathComponent == '/'");
  PASS([[@"/" stringByDeletingLastPathComponent] isEqual: @"/"],
       "'/' stringByDeletingLastPathComponent == '/'");
  PASS([[@"hello" stringByDeletingLastPathComponent] isEqual: @""],
       "'hello' stringByDeletingLastPathComponent == ''");
  PASS_EQUAL([@"/hello/there/.." stringByDeletingLastPathComponent],
    @"/hello/there",
    "'/hello/there/..' stringByDeletingLastPathComponent == '/hello/there'");
  PASS_EQUAL([@"/hello/there/." stringByDeletingLastPathComponent],
    @"/hello/there",
    "'/hello/there/.' stringByDeletingLastPathComponent == '/hello/there'");
  PASS_EQUAL([@"/hello/../there" stringByDeletingLastPathComponent],
    @"/hello/..",
    "'/hello/../there' stringByDeletingLastPathComponent == '/hello/..'");
  PASS_EQUAL([@"/hello//../there" stringByDeletingLastPathComponent],
    @"/hello/..",
    "'/hello//../there' stringByDeletingLastPathComponent == '/hello/..'");

/* Check behavior for UNC absolute and relative paths.
 */
#ifdef	GNUSTEP_BASE_LIBRARY
  GSPathHandling("gnustep");

  // UNC
  PASS_EQUAL([@"//host/share/file.jpg" stringByDeletingLastPathComponent],
    @"//host/share/",
    "'//host/file.jpg' stringByDeletingLastPathComponent == '//host/'");

  // UNC
  PASS_EQUAL([@"//host/share/" stringByDeletingLastPathComponent],
    @"//host/share/",
    "'//host/share/' stringByDeletingLastPathComponent == '//host/share/'");

  // Not UNC
  PASS_EQUAL([@"///host/share/" stringByDeletingLastPathComponent],
    @"/host",
    "'///host/share/' stringByDeletingLastPathComponent == '/host'");

  // Not UNC
  PASS_EQUAL([@"//host/share" stringByDeletingLastPathComponent],
    @"/host",
    "'//host/share' stringByDeletingLastPathComponent == '/host'");

  // Not UNC
  PASS_EQUAL([@"//dir/" stringByDeletingLastPathComponent],
    @"/",
    "'//dir/' stringByDeletingLastPathComponent == '/'");

  GSPathHandling("unix");
#endif

/* Check behavior when UNC paths are not supported.
 */
  PASS([[@"//host/share/file.jpg" stringByDeletingLastPathComponent]
    isEqual: @"/host/share"],
    "'//host/file.jpg' stringByDeletingLastPathComponent == '/host/share'");
  PASS([[@"//host/share/" stringByDeletingLastPathComponent]
    isEqual: @"/host"],
    "'//host/share/' stringByDeletingLastPathComponent == '/host'");
  PASS([[@"//host/share" stringByDeletingLastPathComponent]
   isEqual: @"/host"],
   "'//host/share' stringByDeletingLastPathComponent == '/host'");
  PASS([[@"//dir/" stringByDeletingLastPathComponent] isEqual: @"/"],
   "'//dir/' stringByDeletingLastPathComponent == '/'");

#ifdef	GNUSTEP_BASE_LIBRARY
  GSPathHandling("gnustep");
#endif

  /* delete path extension tests */
  PASS([[@"/home/nicola.jpg" stringByDeletingPathExtension] isEqual: @"/home/nicola"],
       "'/home/nicola.jpg' stringByDeletingPathExtension == '/home/nicola'");
  PASS([[@"/home/" stringByDeletingPathExtension] isEqual: @"/home"],
       "'/home/' stringByDeletingPathExtension == '/home'");
  PASS([[@"nicola.jpg" stringByDeletingPathExtension] isEqual: @"nicola"],
       "'nicola.jpg' stringByDeletingPathExtension == 'nicola'");
  PASS([[@"nicola..jpg" stringByDeletingPathExtension] isEqual: @"nicola."],
       "'nicola..jpg' stringByDeletingPathExtension == 'nicola.'");
  PASS([[@".jpg" stringByDeletingPathExtension] isEqual: @".jpg"],
       "'.jpg' stringByDeletingPathExtension == '.jpg'");
  PASS([[@"/" stringByDeletingPathExtension] isEqual: @"/"],
       "'/' stringByDeletingPathExtension == '/'");
  
  /* stringByExpandingTildeInPath tests */
 
  PASS([[@"/home/nicola/nil" stringByExpandingTildeInPath] 
					isEqual: @"/home/nicola/nil"],
      "'/home/nicola/nil' stringByExpandingTildeInPath: == '/home/nicola/nil'");
  PASS(![[@"~/nil" stringByExpandingTildeInPath] 
					isEqual: @"~/nil"],
      "'~/nil' stringByExpandingTildeInPath: != '~/nil'");

#if	defined(_WIN32)
  {
    NSString *s = [@"~" stringByAppendingString: NSUserName()];
    PASS(![[s stringByExpandingTildeInPath] isEqual: s],
      "'~user' stringByExpandingTildeInPath: != '~user'");
  }
#else
  PASS(![[@"~root" stringByExpandingTildeInPath] 
				isEqual: @"~root"],
      "'~root' stringByExpandingTildeInPath: != '~root'");
#endif
  
#ifdef	GNUSTEP_BASE_LIBRARY

  GSPathHandling("windows");

  PASS_EQUAL([@"\\\\home\\user\\" stringByStandardizingPath],
    @"\\\\home\\user\\",
    "\\\\home\\user\\ stringByStandardizingPath == \\\\home\\user\\");

  PASS_EQUAL([@"c:\\." stringByStandardizingPath], @"c:\\.",
    "'c:\\.' stringByStandardizingPath == 'c:\\.'");
  
  PASS_EQUAL([@"c:\\..." stringByStandardizingPath], @"c:\\...",
    "'c:\\...' stringByStandardizingPath == 'c:\\...'");

  PASS([@"c:\\home" isAbsolutePath] == YES,
       "'c:\\home' isAbsolutePath == YES");

  GSPathHandling("right");
  
  PASS_EQUAL([@"//home/user/" stringByStandardizingPath],
    @"//home/user/",
    "//home/user/ stringByStandardizingPath == //home/user/");

  PASS_EQUAL([@"c:/." stringByStandardizingPath], @"c:/.",
    "'c:/.' stringByStandardizingPath == 'c:/.'");
  
  PASS_EQUAL([@"c:/..." stringByStandardizingPath], @"c:/...",
    "'c:/...' stringByStandardizingPath == 'c:/...'");

  PASS([@"c:/home" isAbsolutePath] == YES,
       "'c:/home' isAbsolutePath == YES");


  PASS([@"//host/share/" isAbsolutePath] == YES,
       "'//host/share/' isAbsolutePath == YES");

#endif

  PASS_EQUAL([@"/home//user/" stringByStandardizingPath], @"/home/user",
   "/home//user/ stringByStandardizingPath == /home/user");

  PASS_EQUAL([@"//home/user" stringByStandardizingPath], @"/home/user",
   "//home/user stringByStandardizingPath == /home/user");

  PASS_EQUAL([@"///home/user" stringByStandardizingPath], @"/home/user",
   "///home/user stringByStandardizingPath == /home/user");
  
  PASS_EQUAL([@"/home/./user" stringByStandardizingPath], @"/home/user",
   "/home/./user stringByStandardizingPath == /home/user");
  
  PASS_EQUAL([@"/home/user/." stringByStandardizingPath], @"/home/user",
   "/home/user/. stringByStandardizingPath == /home/user");
  
  PASS_EQUAL([@"/home/.//././user" stringByStandardizingPath], @"/home/user",
   "/home/.//././user stringByStandardizingPath == /home/user");
  
#if	defined(GNUSTEP_BASE_LIBRARY)
  GSPathHandling("unix");
#endif

  PASS_EQUAL([@"/home/../nicola" stringByStandardizingPath], @"/nicola",
   "/home/../nicola stringByStandardizingPath == /nicola");

  PASS_EQUAL([@"/here/and/there/../../nicola" stringByStandardizingPath],
   @"/here/nicola",
   "/here/and/there/../../nicola stringByStandardizingPath == /here/nicola");

  PASS_EQUAL([@"/here/../../nicola" stringByStandardizingPath],
   @"/nicola",
   "/here/../../nicola stringByStandardizingPath == /nicola");

  PASS_EQUAL([@"home/../nicola" stringByStandardizingPath], @"home/../nicola",
   "home/../nicola stringByStandardizingPath == home/../nicola");

  PASS_EQUAL([@"a/b/../c" stringByStandardizingPath], @"a/b/../c",
   "a/b/../c stringByStandardizingPath == a/b/../c");

  NSFileManager *fm = [NSFileManager defaultManager];
  NSString *cwd = [fm currentDirectoryPath];
  NSString *tmpdir = NSTemporaryDirectory();
  NSString *tmpdst = [tmpdir stringByAppendingPathComponent: @"bar"];
  NSString *tmpsrc = [tmpdir stringByAppendingPathComponent: @"foo"];

  [fm createDirectoryAtPath: tmpdst attributes: nil];
  [fm createSymbolicLinkAtPath: tmpsrc pathContent: @"bar"];

  [fm changeCurrentDirectoryPath: tmpdir];
  testHopeful = YES;
  PASS_EQUAL([@"foo" stringByResolvingSymlinksInPath], @"foo",
    "foo->bar relative symlink not expanded by stringByResolvingSymlinksInPath")
  testHopeful = NO;
  [fm changeCurrentDirectoryPath: cwd];

  [fm removeFileAtPath: tmpsrc handler: nil];
  [fm createSymbolicLinkAtPath: tmpsrc pathContent: tmpdst];

#if	!defined(_WIN32)
  PASS_EQUAL([tmpsrc stringByStandardizingPath], tmpsrc, 
    "foo->bar symlink not expanded by stringByStandardizingPath")
  PASS_EQUAL([tmpsrc stringByResolvingSymlinksInPath], tmpdst, 
    "foo->bar absolute symlink expanded by stringByResolvingSymlinksInPath")

  [fm changeCurrentDirectoryPath: tmpdir];
  PASS_EQUAL([@"foo" stringByResolvingSymlinksInPath], tmpdst, 
    "foo->bar relative symlink expanded by stringByResolvingSymlinksInPath")
#endif

  if (NSHomeDirectory() != nil)
    {
      PASS(NO == [[@"~" stringByResolvingSymlinksInPath] isEqual: @"~"], 
        "tilde is expanded by stringByResolvingSymlinksInPath")
    }
  [fm changeCurrentDirectoryPath: cwd];

  [fm removeFileAtPath: tmpdst handler: nil];
  [fm removeFileAtPath: tmpsrc handler: nil];
  
  PASS_EQUAL([@"/.." stringByStandardizingPath], @"/",
   "/.. stringByStandardizingPath == /");
  PASS_EQUAL([@"/." stringByStandardizingPath], @"/.",
   "/. stringByStandardizingPath == /. (OSX special case)");

#if	defined(GNUSTEP_BASE_LIBRARY)
  GSPathHandling("gnustep");
#endif

  
  result = [NSArray arrayWithObjects: @"nicola",@"core",nil];
  result = [@"home" stringsByAppendingPaths:result];
  PASS([result count] == 2
    && [[result objectAtIndex:0] isEqual: @"home/nicola"]
    && [[result objectAtIndex:1] isEqual: @"home/core"],
    "stringsByAppendingPaths works");
  
  PASS([@"home" isAbsolutePath] == NO,
       "'home' isAbsolutePath == NO");

#if	defined(_WIN32)
  PASS([@"/home" isAbsolutePath] == NO,
       "'/home' isAbsolutePath == NO");
  PASS([@"//host/share" isAbsolutePath] == NO,
       "'//host/share' isAbsolutePath == NO");
#else
  PASS([@"/home" isAbsolutePath] == YES,
       "'/home' isAbsolutePath == YES");
  PASS([@"//host/share" isAbsolutePath] == YES,
       "'//host/share' isAbsolutePath == YES");
#endif
  
  result = [NSArray arrayWithObjects: @"nicola",@"core",nil];
  PASS([[NSString pathWithComponents:result] isEqual: @"nicola/core"],
       "+pathWithComponents works for relative path");
  result = [NSArray arrayWithObjects: @"/", @"nicola", @"core", nil];
  PASS([[NSString pathWithComponents:result] isEqual: @"/nicola/core"],
       "+pathWithComponents works works for absolute path");
  
  [arp release]; arp = nil;
  return 0;
}
