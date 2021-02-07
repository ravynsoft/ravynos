/* <title>linking</title>

   Copyright (C) 2005 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@gnu.org>
   
   This file is part of the GNU Objective C User Interface library.

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

#include "xlib/XGContext.h"

//extern void __objc_xgps_gsbackend_linking (void);

extern void __objc_xgcontextwindow_linking (void);
extern void __objc_xgcontextevent_linking (void);


void __objc_xgps_linking(void)
{
  //__objc_xgps_gsbackend_linking();
  __objc_xgcontextwindow_linking();
  __objc_xgcontextevent_linking();
}
