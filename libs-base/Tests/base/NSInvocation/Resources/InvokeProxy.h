/* InvokeProxy.h - Definitions for simple target and proxy classes 

   Written By: Adam Fedor <fedor@gnu.org>
*/
#include <Foundation/NSObject.h>
#include <Foundation/NSGeometry.h>

typedef struct {
  char	c;
  int	i;
} smallStruct;

typedef struct {
    int		i;
    char	*s;
    float	f;
} largeStruct;

@interface InvokeTarget: NSObject
- (char) loopChar: (char)v;
- (double) loopDouble: (double)v;
- (float) loopFloat: (float)v;
- (int) loopInt: (int)v;
- (largeStruct) loopLarge: (largeStruct)v;
- (long) loopLong: (long)v;
- (largeStruct) loopLargePtr: (largeStruct*)v;
- (id) loopObject: (id)v;
- (short) loopShort: (short)v;
- (NSRect) loopRect: (NSRect)v;
- (smallStruct) loopSmall: (smallStruct)v;
- (smallStruct) loopSmallPtr: (smallStruct*)v;
- (char*) loopString: (char*)v;

- (char) retChar;
- (double) retDouble;
- (float) retFloat;
- (int) retInt;
- (largeStruct) retLarge;
- (long) retLong;
- (id) retObject;
- (short) retShort;
- (smallStruct) retSmall;
- (char*) retString;
@end

@interface InvokeProxy : NSObject
{
  id	obj;
}

- (id) initWithTarget: (id)target;

@end


