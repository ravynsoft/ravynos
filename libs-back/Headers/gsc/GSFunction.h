/* -*-objc-*-
   GSFunction - PS Function for GSGState

   Copyright (C) 2003 Free Software Foundation, Inc.

   Author:  Alexander Malmberg <alexander@malmberg.org>
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Extracted into separate class.
   
   This file is part of GNUstep.

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

#ifndef _GSFunction_h_INCLUDE
#define _GSFunction_h_INCLUDE

#include <Foundation/NSObject.h>

@interface GSFunction : NSObject
{
  /* General information about the function. */
  int num_in, num_out;

  double *domain; /* num_in * 2 */
  double *range; /* num_out * 2 */

  /* Type specific information */
  const unsigned char *data_source;
  int *size; /* num_in */
  int bits_per_sample;
  double *encode; /* num_in * 2 */
  double *decode; /* num_out * 2 */
}

- (id) initWith: (NSDictionary *)d;
- (double) getsample: (int)sample : (int) i;
- (void) eval: (double *)inValues : (double *)outValues;

@end

@interface GSFunction2in3out : GSFunction
{
  /* sample cache for in == 2, out == 3 */
  int sample_index[2];
  double sample_cache[4][3];
}

- (NSRect) affectedRect;

@end

#endif // _GSFunction_h_INCLUDE
