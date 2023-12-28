/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _DbeArray_H
#define _DbeArray_H

template <typename ITEM> class DbeArray
{
public:

  DbeArray (long sz)
  {
    count = 0;
    limit = sz;
    data = new ITEM[limit];
  };

  virtual
  ~DbeArray ()
  {
    delete[] data;
  }

  int
  append (const ITEM &item)
  {
    int n = allocate (1);
    ITEM *p = get (n);
    *p = item;
    return n;
  };

  ITEM *
  get (long index)
  {
    return (index < count && index >= 0) ? data + index : (ITEM *) NULL;
  };

  int
  allocate (int cnt)
  {
    count += cnt;
    resize (count);
    return count - cnt;
  };

  int
  size ()
  {
    return (int) count;
  };

  void
  reset ()
  {
    count = 0;
  };

private:

  void
  resize (long cnt)
  {
    if (limit <= cnt)
      {
	limit *= 2;
	if (limit < cnt)
	  limit = cnt + 1;
	ITEM *d = new ITEM[limit];
	if (count > 0)
	  memcpy (d, data, sizeof (ITEM) * count);
	delete[] data;
	data = d;
      }
  };

  ITEM *data;   // Pointer to data vector
  long count;   // Number of items
  long limit;   // Array length
};

#endif /* _DbeArray_H */
