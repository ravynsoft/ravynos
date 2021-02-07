/*
   RTFProducer.h

   Writes out a NSAttributedString as RTF

   Copyright (C) 2000 Free Software Foundation, Inc.

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: June 2000
   Modifications: Axel Katerbau <axel@objectpark.org>
   Date: April 2003

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

#ifndef _GNUstep_H_RTFDProducer
#define _GNUstep_H_RTFDProducer

#include <GNUstepGUI/GSTextConverter.h>
//#include "GSTextConverter.h"

@class NSAttributedString;
@class NSMutableDictionary;
@class NSColor;
@class NSFont;
@class NSMutableParagraphStyle;

@interface RTFDProducer: NSObject <GSTextProducer>
{
  @public
  NSAttributedString *text;
  NSMutableDictionary *fontDict;
  NSMutableDictionary *colorDict;
  NSDictionary *docDict;
  NSMutableArray *attachments;

  NSColor *fgColor;
  NSColor *bgColor;
  NSColor *ulColor;

  NSDictionary *_attributesOfLastRun; /*" holds the attributes of the last run
    to build the delta "*/

  BOOL _inlineGraphics; /*" Indicates if graphics should be inlined. "*/
  int unnamedAttachmentCounter; /*" Count the number of unnamed attachments so we can name them uniquely "*/
}

@end

@interface RTFProducer: RTFDProducer
// Subclass with no special interface
@end

#endif
