#include "Testing.h"

#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSValue.h>

#include <AppKit/NSApplication.h>
#include <AppKit/NSSliderCell.h>
#include <AppKit/NSImage.h>

int main()
{
  CREATE_AUTORELEASE_POOL(arp);
  NSSliderCell *cell;

  START_SET("NSSliderCell GNUstep minMax")

  NS_DURING
  {
    [NSApplication sharedApplication];
  }
  NS_HANDLER
  {
    if ([[localException name] isEqualToString: NSInternalInconsistencyException ])
       SKIP("It looks like GNUstep backend is not yet installed")
  }
  NS_ENDHANDLER

  cell = [[NSSliderCell alloc] init];

  pass([cell isContinuous], "slider continuous by default");

  pass([cell minValue] == 0.0, "default min value is 0");
  pass([cell maxValue] == 1.0, "default max value is 1");
  pass([cell doubleValue] == 0.0, "default value is 0");
  pass([[cell objectValue] isEqual: [NSNumber numberWithDouble: 0]], "default objectValue is NSNumber 0");

  [cell setMinValue: 2];
  pass([cell minValue] == 2.0, "set min value to 2");
  pass([cell maxValue] == 1.0, "max value is still 1");
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
 
  [cell setDoubleValue: -100.0]; 
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
  [cell setDoubleValue: 1];
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
  [cell setDoubleValue: 1.5];
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
  [cell setDoubleValue: 2.0];
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
  [cell setDoubleValue: 2.5];
  pass([cell doubleValue] == 2.0, "when min < max, value should always be min");
 
  [cell setMaxValue: 10];
  pass([cell doubleValue] == 2.0, "value is still 2.0");

  [cell setMinValue: 3.0];
  pass([cell doubleValue] == 3.0, "changing minimum clamps value to 3.0");
  pass([cell floatValue] == 3.0, "changing minimum clamps value to 3.0");
  
  [cell setDoubleValue: 10];
  [cell setMaxValue: 9];
  pass([cell doubleValue] == 9.0, "changing max clamps value to 9.0");
  pass([cell floatValue] == 9.0, "changing max clamps value to 9.0");
 
  // Test value setters

  [cell setObjectValue: @"hello"];
  pass([cell doubleValue] == 3.0, "setting nonsense string objectValue sets value to min"); 
  [cell setDoubleValue: 9.0];

  [cell setStringValue: @"hello"];
  pass([cell doubleValue] == 3.0, "setting nonsense string stringValue sets value to min"); 
  [cell setDoubleValue: 9.0];
 
  [cell setObjectValue: nil];
  pass([cell doubleValue] == 3.0, "setting nil objectValue sets value to min"); 
  [cell setDoubleValue: 9.0];
 
  [cell setObjectValue: @"3.5"];
  pass([cell doubleValue] == 3.5, "setting @'3.5' objectValue sets value to 3.5"); 
  [cell setDoubleValue: 9.0];
 
  [cell setStringValue: @"3.5"];
  pass([cell doubleValue] == 3.5, "setting @'3.5' stringValue sets value to 3.5"); 
  [cell setDoubleValue: 9.0];

  [cell setIntValue: 3];
  pass([cell doubleValue] == 3.0, "setting 3 intValue sets value to 3.0"); 
  [cell setDoubleValue: 9.0];

  // Test setting the value out of bounds with different setters

  [cell setDoubleValue: 3.5];
  [cell setObjectValue: @"-5"];
  pass([cell doubleValue] == 3.0, "setting @'-5' objectValue sets value to min");
  pass([cell intValue] == 3, "setting @'-5' objectValue sets value to min (integer)");
  [cell setDoubleValue: 3.5];
  [cell setStringValue: @"-5"];
  pass([cell doubleValue] == 3.0, "setting @'-5' stringValue sets value to min");
  pass([cell intValue] == 3, "setting @'-5' objectValue sets value to min (integer)");
  [cell setDoubleValue: 3.5];
  [cell setIntValue: -5];
  pass([cell doubleValue] == 3.0, "setting -5 intValue sets value to min");
  pass([cell intValue] == 3, "setting @'-5' objectValue sets value to min (integer)");
  [cell setDoubleValue: 3.5];
  [cell setDoubleValue: -5];
  pass([cell doubleValue] == 3.0, "setting -5 doubleValue sets value to min");
  pass([cell intValue] == 3, "setting @'-5' objectValue sets value to min (integer)");
  [cell setDoubleValue: 3.5];
  [cell setFloatValue: -5];
  pass([cell doubleValue] == 3.0, "setting -5 floatValue sets value to min");
  pass([cell intValue] == 3, "setting @'-5' objectValue sets value to min (integer)");

  [cell setDoubleValue: 3.5];
  [cell setObjectValue: @"15"];
  pass([cell doubleValue] == 9.0, "setting @'15' objectValue sets value to max");
  pass([cell intValue] == 9, "setting @'15' objectValue sets value to max (integer)");
  [cell setDoubleValue: 3.5];
  [cell setStringValue: @"15"];
  pass([cell doubleValue] == 9.0, "setting @'15' stringValue sets value to max");
  pass([cell intValue] == 9, "setting @'15' stringValue sets value to max (integer)");
  [cell setDoubleValue: 3.5];
  [cell setIntValue: 15];
  pass([cell doubleValue] == 9.0, "setting 15 intValue sets value to max");
  pass([cell intValue] == 9, "setting 15 intValue sets value to max (integer)");
  [cell setDoubleValue: 3.5];
  [cell setDoubleValue: 15];
  pass([cell doubleValue] == 9.0, "setting 15 doubleValue sets value to max");
  pass([cell intValue] == 9, "setting 15 doubleValue sets value to max (integer)");
  [cell setDoubleValue: 3.5];
  [cell setFloatValue: 15];
  pass([cell doubleValue] == 9.0, "setting 15 floatValue sets value to max");
  pass([cell intValue] == 9, "setting 15 floatValue sets value to max (integer)");

  END_SET("NSSliderCell GNUstep minMax")

  DESTROY(arp);
  return 0;
}

