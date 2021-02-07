/* InvokeProxyProtocol.h - protocol for simple target and proxy classes 

   Written By: Adam Fedor <fedor@gnu.org>
*/
#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>

typedef struct {
  char	c;
  int	i;
} smallStruct;

typedef struct {
    int		i;
    char	*s;
    float	f;
} largeStruct;

@protocol InvokeTarget
- (char) loopChar: (char)v;
- (double) loopDouble: (double)v;
- (float) loopFloat: (float)v;
- (int) loopInt: (int)v;
- (largeStruct) loopLarge: (largeStruct)v;
- (long) loopLong: (long)v;
- (largeStruct) loopLargePtr: (largeStruct*)v;
- (id) loopObject: (id)v;
- (NSRect) loopRect: (NSRect)v;
- (short) loopShort: (short)v;
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

@protocol InvokeProxy 
- (id) initWithTarget: (id)target;
@end


