/* 
   NSSecureTextField.h

   Secure text field control class for hidden text entry

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Lyndon Tremblay <humasect@coolmail.com>
   Date: 1999
   
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

#ifndef _GNUstep_H_NSSecureTextField
#define _GNUstep_H_NSSecureTextField

#import <AppKit/NSTextField.h>
#import <AppKit/NSTextFieldCell.h>

@interface NSSecureTextField : NSTextField
{}
- (void) setEchosBullets:(BOOL)flag;
- (BOOL) echosBullets;
@end

@interface NSSecureTextFieldCell : NSTextFieldCell
{
  BOOL _echosBullets;
}
- (void) setEchosBullets:(BOOL)flag;
- (BOOL) echosBullets;
@end

#endif /* _GNUstep_H_NSSecureTextField */
