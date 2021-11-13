/* Copyright (c) 2006-2007 Christopher J. W. Lloyd <cjwl@objc.net>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <AppKit/NSResponder.h>
#import <AppKit/NSAlert.h>
#import <AppKit/NSEvent.h>
#import <AppKit/NSKeyboardBindingManager.h>
#import <AppKit/NSKeyboardBinding.h>
#import <AppKit/NSApplication.h>
#import <Foundation/NSKeyedArchiver.h>
#import <AppKit/NSGraphics.h>
#import <AppKit/NSRaise.h>

@implementation NSResponder

-(void)encodeWithCoder:(NSCoder *)encoder {
}

-initWithCoder:(NSCoder *)coder {
   if([coder allowsKeyedCoding]){
    // NSKeyedUnarchiver *keyed=(NSKeyedUnarchiver *)coder;
    
    //; _nextResponder=[keyed decodeObjectForKey:@"NSNextResponder"]; 
   }
   return self;
}

-(NSResponder *)nextResponder {
   return _nextResponder;
}

-(NSMenu *)menu {
   NSInvalidAbstractInvocation();
   return nil;
}

-(NSUndoManager *)undoManager {
    return [_nextResponder performSelector:_cmd];
}

-(void)setNextResponder:(NSResponder *)responder {
   _nextResponder=responder;
}

-(void)setMenu:(NSMenu *)menu {
   NSInvalidAbstractInvocation();
}

-validRequestorForSendType:(NSString *)sendType returnType:(NSString *)returnType {
   return [_nextResponder validRequestorForSendType:sendType returnType:returnType];
}

-(void)doCommandBySelector:(SEL)selector {

    if([self respondsToSelector:selector]) {
        [self performSelector:selector withObject:nil];
    }
    else
        [_nextResponder doCommandBySelector:selector];
}

-(void)interpretKeyEvents:(NSArray *)events {
   int i,icount=[events count];

   for(i=0;i<icount;i++){
    NSEvent      *event=[events objectAtIndex:i];
    NSString     *string=[event charactersIgnoringModifiers];

    NSKeyboardBinding *keyBinding=[[NSKeyboardBindingManager defaultKeyBindingManager] keyBindingWithString:string modifierFlags:[event modifierFlags]];
    NSArray      *selectorNames=[keyBinding selectorNames];
    if(selectorNames!=nil){
     int j = 0, jcount = [selectorNames count];

     for (j = 0; j < jcount; ++j) {
      //  NSLog(@"doing %@ for %@", [selectorNames objectAtIndex:j], keyBinding);
      [self doCommandBySelector:NSSelectorFromString([selectorNames objectAtIndex:j])];
     }
    }
    else if([self respondsToSelector:@selector(insertText:)]){
     string=[event characters];
 
     if([string length]>0){ // FIX THIS IN APPKIT shouldnt get 0 length 

      unsigned j,length=[string length];
      unichar  buffer[length];
		
      [string getCharacters:buffer];
      for(j=0;j<length;j++){
       unichar check=buffer[j];

        // Filter non char codes - Apple functions keys and ctrl chars
       if(check>=NSUpArrowFunctionKey && check<=NSModeSwitchFunctionKey)
        check=' ';
       else if(check<' ')
        check=' ';

       buffer[j]=check;
      }
      string=[NSString stringWithCharacters:buffer length:length];
      [self insertText:string];
     }
    }
   }
}

-(BOOL)performKeyEquivalent:(NSEvent *)event {
   return NO;
}

-(BOOL)tryToPerform:(SEL)action with:object {
   if([self respondsToSelector:action]){
    [self performSelector:action withObject:object];
    return YES;
   }

   return [_nextResponder tryToPerform:action with:object];
}

-(void)noResponderFor:(SEL)action {
   if(sel_isEqual(action,@selector(keyDown:)))
    NSBeep();
}

-(BOOL)acceptsFirstResponder {
   return NO;
}

-(BOOL)becomeFirstResponder {
   return YES;
}

-(BOOL)resignFirstResponder {
   return YES;
}

-(NSError *)willPresentError:(NSError *)error {
	// do nothing
	return error;
}

-(BOOL)presentError:(NSError *)error {
	BOOL result;
	if (self == NSApp) {
		result=NO;

		NSError *newError; //error after being modified by delegate
		id delegate = [NSApp delegate];
		if ([delegate respondsToSelector:@selector(application:willPresentError:)])
			newError = [delegate application:NSApp willPresentError:error];
		else
			newError = error;
		
		NSError *strippedError; //newError stripped of its recovery options if necessary
		BOOL needToRemoveOptions=NO;
		BOOL validRecoveryAttempter=NO;
		id recoveryAttempter = [newError recoveryAttempter];
		NSArray *recoveryOptions = [newError localizedRecoveryOptions];
		if ([recoveryOptions count]) {
			if (!recoveryAttempter) {
				NSLog(@"There are recovery options but no recovery attempter to interpret the user's choice of one of them. Ignoring the recovery options and suggestion.");
				needToRemoveOptions=YES;
			}
			else if (![recoveryAttempter respondsToSelector:@selector(attemptRecoveryFromError:optionIndex:)]) {
				NSLog(@"The recovery attempter %@ doesn't respond to -attemptRecoveryFromError:optionIndex:. Ignoring the recovery attempter, options, and suggestion.",
					  recoveryAttempter);
				needToRemoveOptions=YES;
			}
			else
				validRecoveryAttempter=YES;
		}
		if (needToRemoveOptions) {
			NSMutableDictionary *newUserInfo = [NSMutableDictionary dictionaryWithDictionary:[newError userInfo]];
			[newUserInfo removeObjectForKey:NSLocalizedRecoveryOptionsErrorKey];
			strippedError = [NSError errorWithDomain:[newError domain] code:[newError code] userInfo:newUserInfo];
		}
		else 
			strippedError = newError;
		NSInteger alertButton=[[NSAlert alertWithError:strippedError] runModal];
		if (validRecoveryAttempter)
			result = [recoveryAttempter attemptRecoveryFromError:strippedError optionIndex:alertButton];
	}
	else {
		//Forward message to nextResponder or to NSApp if there is no nextResponder
		result = [_nextResponder ? _nextResponder : NSApp presentError:[self willPresentError:error]];
	}
	return result;
}

-(void)presentError:(NSError *)error modalForWindow:(NSWindow *)window delegate:delegate didPresentSelector:(SEL)selector contextInfo:(void *)info {
	NSUnimplementedMethod();
}

-(void)flagsChanged:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)keyUp:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)keyDown:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)cursorUpdate:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)scrollWheel:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseUp:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseDown:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseMoved:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseEntered:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseExited:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)mouseDragged:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)rightMouseUp:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)rightMouseDown:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)rightMouseDragged:(NSEvent *)event {
   [_nextResponder performSelector:_cmd withObject:event];
}

-(void)noop:sender {
}

@end
