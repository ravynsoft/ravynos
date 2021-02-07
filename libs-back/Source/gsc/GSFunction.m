/* 
   GSFunction - Function for GSGState

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

// for floor
#include <math.h>

#include <Foundation/NSArray.h>
#include <Foundation/NSData.h>
#include <Foundation/NSDebug.h>
#include <Foundation/NSDictionary.h>
#include <Foundation/NSValue.h>
#include "gsc/GSFunction.h"


@implementation GSFunction

- (id) initWith: (NSDictionary *)d
{
  NSNumber * v = [d objectForKey: @"FunctionType"];
  NSArray *a;
  NSData *data;
  int i, j;

  if ([v intValue] != 0)
    {
      NSDebugLLog(@"GSFunction", @"FunctionType != 0 not supported.");
      RELEASE(self);
      return nil;
    }

  bits_per_sample = [[d objectForKey: @"BitsPerSample"] intValue];
  if (!(bits_per_sample == 8 || bits_per_sample == 16))
    {
      NSDebugLLog(@"GSFunction", @"BitsPerSample other than 8 or 16 aren't supported.");
      RELEASE(self);
      return nil;
    }

  data = [d objectForKey: @"DataSource"];
  if (!data || ![data isKindOfClass: [NSData class]])
    {
      NSDebugLLog(@"GSFunction", @"No valid DataSource given.");
      RELEASE(self);
      return nil;
    }
  data_source =[data bytes];

  a = [d objectForKey: @"Size"];
  num_in = [a count];
  if (!num_in)
    {
      NSDebugLLog(@"GSFunction", @"Size has no entries.");
      RELEASE(self);
      return nil;
    }

  num_out = [[d objectForKey: @"Range"] count] / 2;
  if (!num_out)
    {
      NSDebugLLog(@"GSFunction", @"Range has no entries.");
      RELEASE(self);
      return nil;
    }

  size = malloc(sizeof(int) * num_in);
  domain = malloc(sizeof(double) * num_in * 2);
  range = malloc(sizeof(double) * num_out * 2);
  encode = malloc(sizeof(double) * num_in * 2);
  decode = malloc(sizeof(double) * num_out * 2);

  if (!size || !domain || !range || !encode || !decode)
    {
      NSDebugLLog(@"GSFunction", @"Memory allocation failed.");
      RELEASE(self);
      return nil;
    }

  j = 1;
  for (i = 0; i < num_in; i++)
    {
      size[i] = [[a objectAtIndex: i] intValue];
      j *= size[i];
    }

  j *= bits_per_sample * num_out;
  j = (j +7)/8;
  if ([data length] < j)
    {
      NSDebugLLog(@"GSFunction", @"Need %i bytes of data, DataSource only has %lu bytes.", 
                  j, (unsigned long)[data length]);
      RELEASE(self);
      return nil;
    }

  a = [d objectForKey: @"Domain"];
  for (i = 0; i < num_in * 2; i++)
    {
      domain[i] = [[a objectAtIndex: i] doubleValue];
    }

  a = [d objectForKey: @"Range"];
  for (i = 0; i < num_out * 2; i++)
    {
      range[i] = [[a objectAtIndex: i] doubleValue];
    }

  a = [d objectForKey: @"Decode"];
  if (a)
    {
      for (i = 0; i < num_out * 2; i++)
        {
          decode[i] = [[a objectAtIndex: i] doubleValue];
        }
    }
  else
    {
      for (i = 0; i < num_out * 2; i++)
        {
          decode[i] = range[i];
        }
    }

  a = [d objectForKey: @"Encode"];
  if (a)
    {
      for (i = 0; i < num_in * 2; i++)
        {
          encode[i] = [[a objectAtIndex: i] doubleValue];
        }
    }
  else
    {
      for (i = 0; i < num_in; i++)
        {
          encode[i * 2 + 0] = 0;
          encode[i * 2 + 1] = size[i] - 1;
        }
    }

  return self;
}

- (void)dealloc
{
  if (size)
    free(size);
  if (domain)
    free(domain);
  if (range)
    free(range);
  if (encode)
    free(encode);
  if (decode)
    free(decode);

  [super dealloc];
}

- (double)getsample: (int)sample : (int) i
{
  double v;

  if (bits_per_sample == 8)
    {
      v = data_source[sample * num_out + i] / 255.0;
    }
  else if (bits_per_sample == 16)
    {
      int c0, c1;

      c0 = data_source[(sample * num_out + i) * 2 + 0];
      c1 = data_source[(sample * num_out + i) * 2 + 1];
      v = (c0 * 256 + c1) / 65535.0;
    }
  else
    {
      NSLog(@"unhandled bits per sample %i", bits_per_sample);
      v = 0.0;
    }

  v = decode[i * 2] + v * (decode[i * 2 + 1] - decode[i * 2]);
  if (v < range[i * 2]) 
    v = range[i * 2];
  if (v > range[i * 2 + 1]) 
    v = range[i * 2 + 1];

  return v;
}

- (void) eval: (double *)inValues : (double *)outValues;
{
  double in[num_in];
  int sample[num_in];
  int i, j, sample_index, sample_factor;
  unsigned int u, v;
  double c;

  for (i = 0; i < num_in; i++)
    {
      in[i] =(inValues[i] - domain[i * 2]) / (domain[i * 2 + 1] - domain[i * 2]);
      if (in[i] < 0.0) in[i] = 0.0;
      if (in[i] > 1.0) in[i] = 1.0;

      in[i] = encode[i * 2] + in[i] * (encode[i * 2 + 1] - encode[i * 2]);
      sample[i] = floor(in[i]);
      /* we only want sample[i] == size[i] -1 when size[i] == 1 */
      if (sample[i] >= size[i]-1) sample[i] = size[i]-2;
      if (sample[i] < 0) sample[i] = 0;

      in[i] = in[i] - sample[i];
      if (in[i] < 0.0) in[i] = 0.0;
      if (in[i] > 1.0) in[i] = 1.0;

  //    printf("  coord %i, sample %i, frac %g \n", i, sample[i], in[i]);
    }

  for (i = 0; i < num_out; i++)
    {
      double out_value;
      /*
        iterate over all corners in the num_in-dimensional
        hypercube we're in
      */
      out_value = 0.0;
      for (u = 0; u < (1 << num_in); u++)
        {
          sample_index = 0;
          sample_factor = 1;
          c = 1;
          for (v = 1, j = 0; j < num_in; j ++, v <<= 1)
            {
              sample_index += sample[j] * sample_factor;
              if (u & v)
                {
                  c *= in[j];
                  sample_index += sample_factor;
                }
              else
                {
                  c *= (1.0 - in[j]);
                }
              sample_factor *= size[j];
              if (c == 0.0)
                break;
            }
          //      printf("    %08x  index %i, factor %i, c =%g \n", u, sample_index, sample_factor, c);
          if (c > 0.0)
            out_value += c * [self getsample: sample_index : i];
        }
      //    printf("  final =%g \n", out_value);
      outValues[i] = out_value;
    }
}

@end


@implementation GSFunction2in3out

- (id) initWith: (NSDictionary *)d
{
  if (!(self = [super initWith: d]))
    return nil;

  if (num_in != 2 || num_out != 3)
    {
      NSDebugLLog(@"GSFunction", @"Function doesn't have 2 inputs and 3 outputs.");
      RELEASE(self);
      return nil;
    }
  sample_index[0] = sample_index[1] = -1;

  return self;
}

/*
special case: f->num_in == 2, f->num_out == 3
*/
- (void) eval: (double *)inValues : (double *)outValues;
{
  double in[2];
  int sample[2];
  int i;

  for (i = 0; i < 2; i ++)
    {
      in[i] = (inValues[i] - domain[i * 2]) / (domain[i * 2 + 1] - domain[i * 2]);
      if (in[i] < 0.0) in[i] = 0.0;
      if (in[i] > 1.0) in[i] = 1.0;

      in[i] = encode[i * 2]+in[i]* (encode[i * 2 + 1] - encode[i * 2]);
      sample[i] = floor(in[i]);
      /* we only want sample[i] == size[i]-1 when size[i] == 1 */
      if (sample[i] >= size[i]-1) sample[i] = size[i]-2;
      if (sample[i] < 0) sample[i] = 0;

      in[i] = in[i] - sample[i];
      if (in[i] < 0.0) in[i] = 0.0;
      if (in[i] > 1.0) in[i] = 1.0;

  //    printf("  coord %i, sample %i, frac %g \n", i, sample[i], in[i]);
    }

  if (sample[0] != sample_index[0] || sample[1] != sample_index[1])
    {
      sample_index[0] = sample[0];
      sample_index[1] = sample[1];

      for (i = 0; i < 3; i ++)
        {
          sample_cache[0][i] = 
            [self getsample: sample[0] + sample[1] * size[0] : i];
          if (sample[0] + 1 < size[0])
            sample_cache[1][i] = 
              [self getsample: sample[0] + 1 + sample[1] * size[0] : i];
          if (sample[1] + 1 < size[1])
            sample_cache[2][i] = 
              [self getsample: sample[0] + (sample[1] + 1) * size[0] : i];
          if (sample[0] + 1 < size[0] && sample[1] + 1 < size[1])
              sample_cache[3][i] = 
                [self getsample: sample[0] + 1 +(sample[1] + 1) * size[0] : i];
        }
    }

  for (i = 0; i < 3; i++)
    {
      double out_value;
      double A, B, C, D;
      double p, q, pq;

      A = sample_cache[0][i];
      B = sample_cache[1][i];
      C = sample_cache[2][i];
      D = sample_cache[3][i];

      out_value = 0.0;
      p = in[0];
      q = in[1];
      pq = p * q;
      if (p != 1.0 && q != 1.0) out_value += A * (1 - p -q + pq);
      if (p != 0.0 && q != 1.0) out_value += B * (p - pq);
      if (p != 1.0 && q != 0.0) out_value += C * (q - pq);
      if (p != 0.0 && q != 0.0) out_value += D * pq;

      outValues[i] = out_value;
    }
}

- (NSRect) affectedRect
{
  NSRect rect;

  rect.origin.x = domain[0];
  rect.size.width = domain[1] - domain[0];
  rect.origin.y = domain[2];
  rect.size.height = domain[3] - domain[2];

  return rect;  
}

@end
