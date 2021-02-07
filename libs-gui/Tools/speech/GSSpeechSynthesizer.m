#import "GSSpeechSynthesizer.h"

static GSSpeechServer *server;
static int clients;

@interface GSSpeechSynthesizer (Private)
+ (void)connectionDied: (NSNotification*)aNotification;
@end

@implementation GSSpeechSynthesizer
+ (void)initialize
{
  server = [[GSSpeechServer sharedServer] retain];
  [[NSNotificationCenter defaultCenter]
		addObserver: self
		   selector: @selector(connectionDied:)
                       name: NSConnectionDidDieNotification
                     object: nil];
}

/**
 * If the remote end exits before freeing the GSSpeechSynthesizer then we need
 * to send it a -release message to make sure it dies.
 */
+ (void)connectionDied: (NSNotification*)aNotification
{
  NSEnumerator *e = [[[aNotification object] localObjects] objectEnumerator];
  NSObject *o = nil;
  for (o = [e nextObject] ; nil != o ; o = [e nextObject])
    {
      if ([o isKindOfClass: self])
        {
          [o release];
        }
    }
}

/**
 * If no clients have been active for some time, kill the speech server to
 * conserve resources.
 */
+ (void)exitIfUnneeded: (NSTimer*)sender
{
  if (clients == 0)
    {
      exit(0);
    }
}

- (id)initWithVoice: (NSString*)aVoice
{
  clients++;
  if (nil == (self = [super init]))
    {
      return nil;
    }
  [self setVoice: currentVoice];
  return self;
}

- (id)init
{
  return [self initWithVoice: nil];
}

- (NSString*)voice
{
  return currentVoice;
}

- (id)delegate
{
  return delegate;
}

- (void)setDelegate: (id)aDelegate
{
  // Either -retain or -release can throw an exception due to DO.
  NS_DURING
    aDelegate = [aDelegate retain];
  NS_HANDLER
  NS_ENDHANDLER
  NS_DURING
    [delegate release];
  NS_HANDLER
  NS_ENDHANDLER
  delegate = aDelegate;
}

- (void)setVoice: (NSString*)aVoice
{
  if (nil == aVoice)
    {
      aVoice = [server defaultVoice];
    }
  ASSIGN(currentVoice, aVoice);
}

- (BOOL)startSpeakingString: (NSString*)aString
{
  [server setVoice: currentVoice];
  return [server startSpeakingString: aString notifyWhenDone: self];
}

- (void)didFinishSpeaking: (BOOL)didFinish
{
  // Throw the delegate away if it is throwing exceptions during
  // notification.
  NS_DURING
    [delegate speechSynthesizer: self didFinishSpeaking: didFinish];
  NS_HANDLER
    NS_DURING
      id d = delegate;
      delegate = nil;
      [d release];
    NS_HANDLER
    NS_ENDHANDLER
  NS_ENDHANDLER
}

- (void)stopSpeaking
{
  [server stopSpeaking];
}

- (void)dealloc
{
  clients--;
  [currentVoice release];
  if (clients == 0)
    {
      [NSTimer scheduledTimerWithTimeInterval: 600
                                       target: object_getClass(self)
                                     selector: @selector(exitIfUnneeded:)
                                     userInfo: nil
                                      repeats: NO];
    }
  [super dealloc];
}
@end
