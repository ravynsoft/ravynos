
#import <Foundation/Foundation.h>
#import "AppKit/AppKit.h"
#import "GNUstepGUI/GSFontInfo.h"

void __objc_gui_linking(void)
{
  [GSFontInfo class];
  [NSBezierPath class];
  [NSStepper class];
}
