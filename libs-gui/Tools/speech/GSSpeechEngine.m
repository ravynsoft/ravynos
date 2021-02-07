#import "GSSpeechEngine.h"

/**
 * Dummy implementation of a speech engine.  Doesn't do anything.
 */
@implementation GSSpeechEngine
+ (GSSpeechEngine*)defaultSpeechEngine { return [[self new] autorelease]; }
- (void)startSpeaking: (NSString*)aString notifyWhenDone: (id)anObject{}
- (void)stopSpeaking {}
- (BOOL)isSpeaking { return NO; }
- (NSArray*)voices { return [NSArray arrayWithObject: @"default"]; }
- (void)setVoice: (NSString*)aVoice {}
- (NSString*)voice { return @"default"; }
- (NSString*)defaultVoice { return @"default"; }
@end
