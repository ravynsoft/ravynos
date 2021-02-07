#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSError.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSError.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSURL.h>

#ifdef  EQ
#undef  EQ
#endif
#define EPSILON (FLT_EPSILON*100)
#define EQ(x,y) ((x >= y - EPSILON) && (x <= y + EPSILON))

@interface      MyHandler : NSObject
{
  @public
  NSString      *path;
  NSError       *error;
}
- (BOOL) fileManager: (NSFileManager*)m shouldProceedAfterError: (NSError*)e;
- (void) fileManager: (NSFileManager*)m willProcessPath: (NSString*)p;
- (void) reset;
@end

@implementation MyHandler
- (void) dealloc
{
  [self reset];
  [super dealloc];
}

- (BOOL) fileManager: (NSFileManager*)m shouldProceedAfterError: (NSError*)e
{
  ASSIGN(error, e);
  return NO;
}

- (void) fileManager: (NSFileManager*)m willProcessPath: (NSString*)p
{
  ASSIGN(path, p);
}

- (void) reset
{
  DESTROY(path);
  DESTROY(error);
}
@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSFileManager *mgr = [NSFileManager defaultManager];
  NSString *dir = @"NSFileManagerTestDir"; 
  MyHandler *handler = [MyHandler new];
  NSDictionary *attr;
  NSString *dirInDir;
  NSString *str1,*str2;
  NSString *tmp;
  NSError *err;
  NSDictionary *errInfo;
  BOOL exists;
  BOOL isDir;

  dirInDir = [dir stringByAppendingPathComponent: @"WithinDirectory"];

  PASS(mgr != nil && [mgr isKindOfClass: [NSFileManager class]],
       "NSFileManager understands +defaultManager");

  /* remove test directory if it exists */
  exists = [mgr fileExistsAtPath: dir isDirectory: &isDir];
  if (exists)
    {
      [mgr removeFileAtPath: dir handler: handler];
    }
  PASS([mgr fileAttributesAtPath: dir traverseLink: NO] == nil,
    "NSFileManager returns nil for attributes of non-existent file");

  PASS([mgr createDirectoryAtPath: dir attributes: nil],
       "NSFileManager can create a directory");
  PASS([mgr fileExistsAtPath: dir isDirectory: &isDir] &&
       isDir == YES,
       "exists and is a directory");
  PASS(NO == [mgr createDirectoryAtPath: dir attributes: nil],
       "-createDirectoryAtPath:attributes: fails for existing directory");
  PASS(NO == [mgr createDirectoryAtPath: dir withIntermediateDirectories: NO
    attributes: nil error: 0],
   "-createDirectoryAtPath:withIntermediateDirectories:attributes:error:"
    " fails for existing directory if flag is NO");

  PASS(YES == [mgr createDirectoryAtPath: dir withIntermediateDirectories: YES
    attributes: nil error: 0],
   "-createDirectoryAtPath:withIntermediateDirectories:attributes:error:"
    " succeeds for existing directory if flag is YES");

  PASS([mgr fileAttributesAtPath: dir traverseLink: NO] != nil,
    "NSFileManager returns non-nil for attributes of existing file");
  attr = [mgr fileAttributesAtPath: dir traverseLink: NO];
  PASS(attr != nil,
    "NSFileManager returns non-nil for attributes of existing file");
  PASS([NSUserName() isEqual: [attr fileOwnerAccountName]],
    "newly created file is owned by current user");
//NSLog(@"'%@', '%@'", NSUserName(), [attr fileOwnerAccountName]);
  err = (id)(void*)42;
  attr = [mgr attributesOfItemAtPath: dir error: &err]; 
  PASS(attr != nil && err == (id)(void*)42, 
    "[NSFileManager attributesOfItemAtPath:error:] returns non-nil for "
    "attributes and leaves error unchanged in the case of existing file"); 
  attr = [mgr attributesOfItemAtPath:
    [dir stringByAppendingPathComponent:
      @"thispathMUSTNOTexistatyoursystem"] error: &err]; 
  PASS(attr == nil && err != nil, 
    "[NSFileManager attributesOfItemAtPath:error:] returns nil for "
    "attributes and non-nil for error in the case of non-existent file"); 
  
  PASS([mgr changeCurrentDirectoryPath: dir],
       "NSFileManager can change directories");
  
  
  {
    NSString *dir1 = [mgr currentDirectoryPath];
    PASS(dir1 != nil && [[dir1 lastPathComponent] isEqualToString: dir],
         "NSFileManager can get current dir");
  }
  
  str1 = @"A string";
  PASS([mgr createFileAtPath: @"NSFMFile" 
                    contents: [str1 dataUsingEncoding: 1]
		  attributes: nil],
       "NSFileManager creates a file");
  PASS([mgr fileExistsAtPath: @"NSFMFile"],"-fileExistsAtPath: agrees");
  
  {
    NSArray	*a;

    a = [mgr contentsOfDirectoryAtPath: @"." error: 0];
    PASS(1 == [a count] && [[a lastObject] isEqual: @"NSFMFile"],
      "-contentsOfDirectoryAtPath: agrees");
  }

  {
    NSData *dat1 = [mgr contentsAtPath: @"NSFMFile"];
    str2 = [[NSString alloc] initWithData: dat1 encoding: 1];
    PASS([str1 isEqualToString: str2], "NSFileManager file contents match");
  }
  [NSThread sleepForTimeInterval: 1.0]; // So date of file is clearly in past
  [handler reset];
  PASS([mgr copyPath: @"NSFMFile"
              toPath: @"NSFMCopy"
	     handler: handler], 
       "NSFileManager copies a file");
  PASS([mgr fileExistsAtPath: @"NSFMCopy"],"-fileExistsAtPath: agrees");
  {
    NSData *dat1 = [mgr contentsAtPath: @"NSFMCopy"];
    str2 = [[NSString alloc] initWithData: dat1 encoding: 1];
    PASS([str1 isEqual: str2],"NSFileManager copied file contents match");
  }
  NSDictionary *oa = [mgr fileAttributesAtPath: @"NSFMFile" traverseLink: NO];
  NSDictionary *na = [mgr fileAttributesAtPath: @"NSFMCopy" traverseLink: NO];
  NSTimeInterval        ot, nt;
  ot = [[oa fileCreationDate] timeIntervalSinceReferenceDate];
  nt = [[na fileCreationDate] timeIntervalSinceReferenceDate];
  PASS(EQ(ot, nt), "copy creation date equals original")
  ot = [[oa fileModificationDate] timeIntervalSinceReferenceDate];
  nt = [[na fileModificationDate] timeIntervalSinceReferenceDate];
  PASS(EQ(ot, nt), "copy modification date equals original")
  {
    NSData *dat1 = [mgr contentsAtPath: @"NSFMFile"];
    NSError *err;
    BOOL ok;

    ok = [dat1 writeToFile: @"NSFMFile"
                   options: NSDataWritingAtomic
                     error: &err];
    PASS(ok, "can rewrite data file")
    if (NO == ok) NSLog(@"Problem: %@ with %@", err, dat1);
    na = [mgr fileAttributesAtPath: @"NSFMFile" traverseLink: NO];
NSLog(@"%@\n%@", oa, na);
    ot = [[oa fileCreationDate] timeIntervalSinceReferenceDate];
    nt = [[na fileCreationDate] timeIntervalSinceReferenceDate];
    PASS(!EQ(ot, nt), "rewritten file creation date changed")
    ot = [[oa fileModificationDate] timeIntervalSinceReferenceDate];
    nt = [[na fileModificationDate] timeIntervalSinceReferenceDate];
    PASS(!EQ(ot, nt), "rewritten file modification date changed")
  }

  PASS([mgr movePath: @"NSFMFile"
              toPath: @"NSFMMove"
	     handler: handler],
       "NSFileManager moves a file");
  PASS([mgr fileExistsAtPath: @"NSFMMove"], 
       "NSFileManager move destination exists");
  PASS(![mgr fileExistsAtPath: @"NSFMFile"], 
       "NSFileManager move source doesn't exist"); 
  {
    NSData *dat1 = [mgr contentsAtPath: @"NSFMMove"];
    str2 = [[NSString alloc] initWithData: dat1 encoding: 1];
    PASS([str1 isEqualToString: str2],"NSFileManager moved file contents match")
  }

  PASS(![mgr copyPath: @"NSFMFile"
               toPath: @"NSFMDestination"
              handler: handler],
         "NSFileManager does not copy nonexistent file")

  if ([[NSProcessInfo processInfo] operatingSystem]
    != NSWindowsNTOperatingSystem)
    {
      PASS([mgr createSymbolicLinkAtPath: @"NSFMLink" pathContent: @"NSFMMove"],
       "NSFileManager creates a symbolic link");
  
      PASS([mgr fileExistsAtPath: @"NSFMLink"], "link exists");
  
      PASS([mgr removeFileAtPath: @"NSFMLink" handler: handler], 
       "NSFileManager removes a symbolic link");
  
      PASS(![mgr fileExistsAtPath: @"NSFMLink"],
       "NSFileManager removed link doesn't exist");
  
      PASS([mgr fileExistsAtPath: @"NSFMMove"],
       "NSFileManager removed link's target still exists");
    }
  
  PASS([mgr removeFileAtPath: @"NSFMMove" handler: handler], 
       "NSFileManager removes a file"); 
 
  PASS(![mgr fileExistsAtPath: @"NSFMMove"],
       "NSFileManager removed file doesn't exist");
  
  PASS([mgr isReadableFileAtPath: @"NSFMCopy"], 
       "NSFileManager isReadableFileAtPath: works");
  PASS([mgr isWritableFileAtPath: @"NSFMCopy"],
       "NSFileManager isWritableFileAtPath: works");
  PASS([mgr isDeletableFileAtPath: @"NSFMCopy"],
       "NSFileManager isDeletableFileAtPath: works");
  PASS(![mgr isExecutableFileAtPath: @"NSFMCopy"],
       "NSFileManager isExecutableFileAtPath: works");
  
  PASS_EXCEPTION([mgr removeFileAtPath: @"." handler: handler];, 
                 NSInvalidArgumentException,
		 "NSFileManager -removeFileAtPath: @\".\" throws exception");

  PASS([mgr createDirectoryAtPath: @"subdir" attributes: nil],
       "NSFileManager can create a subdirectory");
  
  PASS([mgr changeCurrentDirectoryPath: @"subdir"], 
       "NSFileManager can move into subdir");

  [mgr createDirectoryAtPath: @"sub1" attributes: nil];
  [mgr createFileAtPath: @"sub1/x" 
               contents: [@"hello" dataUsingEncoding: NSASCIIStringEncoding]
             attributes: nil],
  [mgr createDirectoryAtPath: @"sub2" attributes: nil];
  [mgr createFileAtPath: @"sub2/x" 
               contents: [@"hello" dataUsingEncoding: NSASCIIStringEncoding]
             attributes: nil];
  PASS(YES == [mgr contentsEqualAtPath: @"sub1/x" andPath: @"sub2/x"],
    "directories containing identical files are equal");
  [mgr removeFileAtPath: @"sub2/x" handler: handler],
  [mgr createFileAtPath: @"sub2/x" 
               contents: [@"goodbye" dataUsingEncoding: NSASCIIStringEncoding]
             attributes: nil];
  PASS(NO == [mgr contentsEqualAtPath: @"sub1/x" andPath: @"sub2/x"],
    "directories containing files with different content are not equal");
  PASS(YES == [mgr removeFileAtPath: @"sub1" handler: handler],
    "sub1 removed");
  PASS(YES == [mgr removeFileAtPath: @"sub2" handler: handler],
    "sub2 removed");

  err = nil;
  PASS([mgr createDirectoryAtPath: dirInDir
      withIntermediateDirectories: NO  
                       attributes: nil
                            error: &err] == NO,
       "NSFileManager refuses to create intermediate directories"); 
  PASS(err != nil, "error value is set"); 
  PASS_EQUAL([err domain], NSCocoaErrorDomain, "cocoa error domain");
  PASS((errInfo = [err userInfo]) != nil, "error user info is set"); 
  PASS([errInfo objectForKey: NSFilePathErrorKey] != nil,
    "error info has a path");

  err = nil;
  PASS([mgr createDirectoryAtPath: dirInDir
      withIntermediateDirectories: YES
                       attributes: nil
                            error: &err] && err == nil,
   "NSFileManager can create intermediate directories"); 
  PASS([mgr fileExistsAtPath: dirInDir isDirectory: &isDir] && isDir == YES,
    "NSFileManager create directory and intermediate directory");

  tmp = [mgr currentDirectoryPath];
  exists = [mgr fileExistsAtPath: tmp isDirectory: &isDir];
  PASS(YES == exists && YES == isDir, "current directory exists");
  tmp = [tmp stringByDeletingLastPathComponent];
  exists = [mgr fileExistsAtPath: tmp isDirectory: &isDir];
  PASS(YES == exists && YES == isDir, "parent directory exists");
  tmp = [tmp stringByDeletingLastPathComponent];
  exists = [mgr fileExistsAtPath: tmp isDirectory: &isDir];
  PASS(YES == exists && YES == isDir, "parent of parent directory exists");
  [mgr changeCurrentDirectoryPath: tmp];
  exists = [mgr fileExistsAtPath: dir isDirectory: &isDir];
  PASS(YES == exists && YES == isDir, "directory exists");
  if (exists && isDir)
    {
      dir = [dir stringByStandardizingPath];
      PASS([mgr removeFileAtPath: dir handler: handler], "removed directory");
      PASS(![mgr fileExistsAtPath: dir], "directory no longer exists");
GSPrintf(stdout, @"%@\n", dir);
GSPrintf(stderr, @"%@\n", dir);
    }
  
  err = nil;
  PASS([mgr createDirectoryAtURL: [NSURL fileURLWithPath:dirInDir]
      withIntermediateDirectories: NO  
                       attributes: nil
                            error: &err] == NO
    && err != nil && [[err domain] isEqual: NSCocoaErrorDomain]
    && (errInfo = [err userInfo]) != nil
    && [errInfo objectForKey: NSFilePathErrorKey] != nil,
       "NSFileManager refuses to create intermediate directories on URL"); 

  err = nil;
  PASS([mgr createDirectoryAtURL: [NSURL fileURLWithPath:dirInDir]
      withIntermediateDirectories: YES
                       attributes: nil
                            error: &err] && err == nil,
   "NSFileManager can create intermediate directories on URL"); 
  PASS([mgr fileExistsAtPath: dirInDir isDirectory: &isDir] && isDir == YES,
    "NSFileManager create directory and intermediate directory on URL");

  [mgr createDirectoryAtPath: @"sub1"
	withIntermediateDirectories: YES
			 attributes: nil
	    		      error: &err];
  [mgr createDirectoryAtPath: @"sub2"
	withIntermediateDirectories: YES
			 attributes: nil
	    		      error: &err];
  [mgr copyItemAtURL: [NSURL fileURLWithPath: @"sub1"] 
               toURL: [NSURL fileURLWithPath: @"sub2/sub1"] 
               error: &err];
  PASS([mgr fileExistsAtPath: @"sub2/sub1" isDirectory: &isDir]
    && isDir == YES, "NSFileManager copy item at URL");
  [mgr copyItemAtPath: @"sub2" toPath: @"sub1/sub2" error: &err];
  PASS([mgr fileExistsAtPath: @"sub1/sub2/sub1" isDirectory: &isDir]
    && isDir == YES, "NSFileManager copy item at Path");
  [mgr moveItemAtURL: [NSURL fileURLWithPath: @"sub2/sub1"]
	       toURL: [NSURL fileURLWithPath: @"sub1/moved"]
	       error: &err];
  PASS([mgr fileExistsAtPath: @"sub1/moved" isDirectory: &isDir]
    && isDir == YES, "NSFileManager move item at URL");
  [mgr moveItemAtPath:@"sub1/sub2" toPath:@"sub2/moved" error: &err];
  PASS([mgr fileExistsAtPath: @"sub2/moved" isDirectory: &isDir]
    && isDir == YES, "NSFileManager move item at Path");
  [mgr removeItemAtURL: [NSURL fileURLWithPath: @"sub1"]
	         error: &err];
  PASS([mgr fileExistsAtPath: @"sub1" isDirectory: &isDir] == NO,
    "NSFileManager remove item at URL");
  [mgr removeItemAtPath: @"sub2" error: &err];
  PASS([mgr fileExistsAtPath: @"sub2" isDirectory: &isDir] == NO,
    "NSFileManager remove item at Path");
  
  PASS_EXCEPTION([mgr removeFileAtPath: @"." handler: handler];, 
    NSInvalidArgumentException,
    "NSFileManager -removeFileAtPath: @\".\" throws exception");
       
  PASS_EXCEPTION([mgr removeFileAtPath: @".." handler: handler];, 
    NSInvalidArgumentException,
    "NSFileManager -removeFileAtPath: @\"..\" throws exception");
/* clean up */ 
  [mgr changeCurrentDirectoryPath: [[[mgr currentDirectoryPath] stringByDeletingLastPathComponent] stringByDeletingLastPathComponent]];
  exists = [mgr fileExistsAtPath: dir isDirectory: &isDir];
  if (exists && isDir)
    {
      PASS([mgr removeFileAtPath: dir handler: handler],
           "NSFileManager removes a directory");
      PASS(![mgr fileExistsAtPath: dir],"directory no longer exists");
    }
  
  [arp release]; arp = nil;
  return 0;
}
