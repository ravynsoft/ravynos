#import "GSSpeechServer.h"
#import <AppKit/NSSpeechSynthesizer.h>


@interface GSSpeechSynthesizer : NSSpeechSynthesizer {
	NSString *currentVoice;
	id delegate;
}
- (id)initWithVoice: (NSString*)aVoice;
- (id)init;
- (NSString*)voice;
- (id)delegate;
- (void)setDelegate: (id)aDelegate;
- (void)setVoice: (NSString*)aVoice;
- (BOOL)startSpeakingString: (NSString*)aString;
- (void)stopSpeaking;
@end
