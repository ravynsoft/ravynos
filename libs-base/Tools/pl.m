/*
   pl.m

   This file is the main driver for the plist parsing program.

   Copyright (C) 1996,1999,2000 Free Software Foundation, Inc.

   Author: Gregory John Casamento
   Date: 17 Jan 2000

   This file is part of the GNUstep Project

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   You should have received a copy of the GNU General Public
   License along with this program; see the file COPYINGv3.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#import	"common.h"

#import	"Foundation/NSArray.h"
#import	"Foundation/NSAutoreleasePool.h"
#import	"Foundation/NSData.h"
#import	"Foundation/NSException.h"
#import	"Foundation/NSFileHandle.h"
#import	"Foundation/NSFileManager.h"
#import	"Foundation/NSPathUtilities.h"
#import	"Foundation/NSProcessInfo.h"
#import	"Foundation/NSUserDefaults.h"

void create_output(id propertyList)
{
  NSFileHandle *fileHandle = nil;
  NSProcessInfo *processInfo = [NSProcessInfo processInfo];
  NSArray *arguments = [processInfo arguments];
  NSUInteger outputIndex = 0;

  // insert your code here
  outputIndex = [arguments indexOfObject: @"-output"];
  if (outputIndex == NSNotFound)
    {
      const char *buffer = [[propertyList description] cString];
      NSData *outputData;

      outputData = [NSData dataWithBytes: buffer length: strlen(buffer)];
      // setup the file handle.
      fileHandle = [NSFileHandle fileHandleWithStandardOutput];
      // Send the data to stdout
      [fileHandle writeData: outputData];
      puts("\n");
    }
  else
    {
      NSData *serializedData = nil;
      NSFileManager *fileManager = [NSFileManager defaultManager];

      // Write in the serialized plist.
      serializedData = [NSSerializer serializePropertyList: propertyList];
      [fileManager createFileAtPath: [arguments objectAtIndex: outputIndex+1]
			   contents: serializedData
			 attributes: nil];
    }
}

id process_plist(NSData *inputData)
{
  id propertyList = nil;
  NSString *string = nil;

  // Initialize a string with the contents of the file.
  string = [NSString stringWithUTF8String: (char *)[inputData bytes]];

  // Convert the string into a property list.  If there is a parsing error
  // the property list interpreter will throw an exception.
  NS_DURING
      propertyList = [string propertyList];
  NS_HANDLER
      NSLog(@"%@", localException);
  NS_ENDHANDLER

  // return the results
  return propertyList;
}

NSData *read_input()
{
  NSData *inputData = nil;
  NSFileHandle *fileHandle = nil;
  NSProcessInfo *processInfo = [NSProcessInfo processInfo];
  NSArray *arguments = [processInfo arguments];
  NSUInteger inputIndex = 0;

  // insert your code here
  inputIndex = [arguments indexOfObject: @"-input"];
  if (inputIndex == NSNotFound)
    {
      // setup the file handle.
      fileHandle = [NSFileHandle fileHandleWithStandardInput];
      // Read in the input from the file.
      inputData = [fileHandle readDataToEndOfFile];
    }
  else
    {
      NSData *serializedData = nil;
      id propertyList = nil;
      char *buffer = 0;

      // set up the file handle.
      fileHandle = [NSFileHandle fileHandleForReadingAtPath:
	[arguments objectAtIndex: inputIndex+1]];
      // read in the serialized plist.
      serializedData = [fileHandle readDataToEndOfFile];
      [fileHandle closeFile];
      propertyList = [NSDeserializer deserializePropertyListFromData:
	serializedData mutableContainers: NO];
      if (propertyList != nil)
	{
	  buffer = (char *)[[propertyList description] cString];
	  inputData = [NSData dataWithBytes: buffer length: strlen(buffer)];
	}
      else
	{
	  NSLog(@"%@ is not a serialized property list.",
	    [arguments objectAtIndex: inputIndex+1]);
	}
    }

  return inputData;
}

int main (int argc, const char *argv[])
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSData *inputData = nil;
  id propertyList = nil;

  // put your code here.
  if (argc == 1 || argc == 3|| argc == 5)
    {
      inputData = read_input();
      if (inputData != nil)
	{
	  // If the input data was sucessfully read...
	  propertyList = process_plist( inputData );
	  if (propertyList != nil)
	    {
	      // If the property list was okay...
	      create_output( propertyList );
	    }
	}
    }
  else
    {
      puts("pl {-input <serialized_file>} {-output <serialized_file>}");
      puts(
  "   - Reads an ASCII property list from standard in, or a serialized one");
      puts("     if -input is specified.");
      puts(
  "   - Writes an ASCII propert list to standard out, or a serialized one");
      puts("     if -output is specified.");
    }

  [pool drain];
  exit(0);       // insure the process exit status is 0
  return 0;      // ...and make main fit the ANSI spec.
}
