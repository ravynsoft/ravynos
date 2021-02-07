#import <Foundation/Foundation.h>

/**
 * GSSpeechEngine is an abstract speech server.  One concrete subclass should
 * be implemented for each speech engine.  Currently, only one may be compiled
 * in to the speech server at any given time.  This limitation may be removed
 * in future if pluggable speech engines are considered beneficial.
 */
@interface GSSpeechEngine : NSObject
/**
 * Begin speaking the specified string.
 */
- (void)startSpeaking: (NSString*)aString notifyWhenDone: (id)aDelegate;
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
 * Returns the name of the default voice for this speech engine.
 */
- (NSString*)defaultVoice;
@end

@interface NSObject (GSSpeechEngineDelegate)
/**
 * Called when the speech engine has finished speaking a phrase.  Should be
 * used to notify the original caller.
 */
- (void)didFinishSpeaking: (BOOL)didFinish;
@end
@interface GSSpeechEngine (Default)
/**
 * Returns a new instance of the default speech engine.
 */
+ (GSSpeechEngine*)defaultSpeechEngine;
@end
