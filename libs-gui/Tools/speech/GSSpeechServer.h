#import <Foundation/Foundation.h> 
@class GSSpeechEngine;
/**
 * GSSpeechServer handles all of the engine-agnostic operations.  Currently,
 * there aren't any, but when the on-screen text interface is added it should
 * go in here.
 */
@interface GSSpeechServer : NSObject {
	GSSpeechEngine *engine;
}
/**
 * Returns a shared instance of the speech server.
 */
+ (id)sharedServer;
/**
 * Begins speaking the string specified by the first argument.  Calls the
 * delegate method on the client when done.
 */
- (BOOL)startSpeakingString: (NSString*)aString notifyWhenDone: (id)client;
/**
 * Stop speaking.
 */
- (void)stopSpeaking;
/**
 * Returns YES if the engine is currently outputting speech.
 */
- (BOOL)isSpeaking;
/**
 * Returns an array of voices supported by this speech synthesizer.
 */
- (NSArray*)voices;
/**
 * Sets the voice.
 */
- (void)setVoice: (NSString*)aVoice;
/**
 * Returns the current voice.
 */
- (NSString*)voice;
/**
 * Returns the name of the default voice.
 */
- (NSString*)defaultVoice;
@end
