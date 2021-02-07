/** <title>NSMovieView</title>

   <abstract>Encapsulate a view for Quicktime movies</abstract>

   Copyright <copy>(C) 2003 Free Software Foundation, Inc.</copy>

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2003

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSMovieView
#define _GNUstep_H_NSMovieView
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSView.h>

@class NSMovie;

typedef enum {
  NSQTMovieNormalPlayback,
  NSQTMovieLoopingPlayback,
  NSQTMovieLoopingBackAndForthPlayback
} NSQTMovieLoopMode;

@interface NSMovieView : NSView
{
  @protected
    NSMovie* _movie;
    float _rate;
    float _volume;
    struct NSMovieViewFlags {
      unsigned int muted: 1;
      unsigned int loopMode: 3;
      unsigned int plays_selection_only: 1;
      unsigned int plays_every_frame: 1;
      unsigned int is_controller_visible: 1;
      unsigned int editable: 1;
      unsigned int reserved: 24;
    } _flags;
}

- (void) setMovie: (NSMovie*)movie;
- (NSMovie*) movie;

- (void) start: (id)sender;
- (void) stop: (id)sender;
- (BOOL) isPlaying;

- (void) gotoPosterFrame: (id)sender;
- (void) gotoBeginning: (id)sender;
- (void) gotoEnd: (id)sender;
- (void) stepForward: (id)sender;
- (void) stepBack: (id)sender;

- (void) setRate: (float)rate;
- (float) rate;

- (void) setVolume: (float)volume;
- (float) volume;
- (void) setMuted: (BOOL)mute;
- (BOOL) isMuted;

- (void) setLoopMode: (NSQTMovieLoopMode)mode;
- (NSQTMovieLoopMode) loopMode;
- (void) setPlaysSelectionOnly: (BOOL)flag;
- (BOOL) playsSelectionOnly;
- (void) setPlaysEveryFrame: (BOOL)flag;
- (BOOL) playsEveryFrame;

- (void) showController: (BOOL)show adjustingSize: (BOOL)adjustSize;
- (void*) movieController;
- (BOOL) isControllerVisible;

- (NSRect) movieRect;
- (void) resizeWithMagnification: (float)magnification;
- (NSSize) sizeForMagnification: (float)magnification;

- (void) setEditable: (BOOL)editable;
- (BOOL) isEditable;

- (void) cut: (id)sender;
- (void) copy: (id)sender;
- (void) paste: (id)sender;
- (void) clear: (id)sender;
- (void) undo: (id)sender;
- (void) selectAll: (id)sender;

@end

#endif /* _GNUstep_H_NSMovieView */
