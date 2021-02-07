/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>
*/

#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSFileManager.h>

int main(int argc, char **argv)
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSFileManager		*fm = [NSFileManager defaultManager];
  NSArray		*files;
  int 			i;
  BOOL 			e, d;

  files = [fm directoryContentsAtPath: @"."];
  printf("%lu files\n", (unsigned long)[files count]);
  for (i = 0; i < [files count]; i++)
    {
      int	j;
      NSString	*f = [files objectAtIndex: i];

      e = [fm fileExistsAtPath: f
		   isDirectory: &d];
      printf("%5i: %i %i %s\n",i,e,d, [f lossyCString]);
      for (j = 0; j < [f length]; j++)
	printf("   %3i: %04x\n",j,[f characterAtIndex: j]);
    }

  /*	const char *test="hallå.txt";
  NSString *s=[NSString stringWithCString: test];
  printf("s=%s\n", [s lossyCString]);*/

  [arp release]; arp = nil;

  return 0;
}

